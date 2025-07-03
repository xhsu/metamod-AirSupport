module;

#include <stdio.h>

#ifdef __INTELLISENSE__
#include <algorithm>
#endif

export module Models;

#ifdef __INTELLISENSE__
import std;
#else
import std.compat;	// #MSVC_BUG_STDCOMPAT
#endif
import hlsdk;

import FileSystem;
import Platform;
import Wave;

struct TranscriptedModel
{
	char				m_szName[64]{};

	int32_t				m_iType{};

	float				m_boundingradius{};

	uint32_t			m_nummesh{};

	uint32_t			m_numverts{};	// number of unique vertices
	uint32_t			m_numnorms{};	// number of unique surface normals
	uint32_t			m_numgroups{};	// deformation groups

	TranscriptedModel(mstudiomodel_t const* src) noexcept
		: m_iType{ src->type }, m_boundingradius{ src->boundingradius },
		m_nummesh{ src->nummesh }, m_numverts{ src->numverts }, m_numnorms{ src->numnorms }, m_numgroups{ src->numgroups }
	{
		memcpy(m_szName, src->name, sizeof(m_szName));
	}
	TranscriptedModel(mstudiomodel_t const& src) noexcept : TranscriptedModel(std::addressof(src)) {}
};

struct TranscriptedPartGroup final
{
	char							m_szName[64]{};
	int32_t							m_iBase{};
	std::vector<TranscriptedModel>	m_SubModels{};

	TranscriptedPartGroup(std::byte const* buf, mstudiobodyparts_t const* src) noexcept : m_iBase{ src->base }
	{
		memcpy(m_szName, src->name, sizeof(m_szName));
		std::span const SubModels{ reinterpret_cast<mstudiomodel_t const*>(buf + src->modelindex), src->nummodels };

		m_SubModels.append_range(SubModels);
	}
	TranscriptedPartGroup(TranscriptedPartGroup const&) noexcept = default;
	TranscriptedPartGroup(TranscriptedPartGroup&&) noexcept = default;
	TranscriptedPartGroup& operator=(TranscriptedPartGroup const&) noexcept = default;
	TranscriptedPartGroup& operator=(TranscriptedPartGroup&&) noexcept = default;
	~TranscriptedPartGroup() noexcept = default;
};

struct TranscriptedSequence final
{
	char				m_szLabel[32]{};	// sequence label

	float				m_fps{};		// frames per second	
	int32_t				m_flags{};		// looping/non-looping flags

	Activity			m_activity{};
	int32_t				m_actweight{};

	std::vector<mstudioevent_t> m_Events{};

	uint32_t			m_numframes{};	// number of frames per sequence

	uint32_t			m_numpivots{};	// number of foot pivots

	Vector				m_linearmovement;

	int32_t				m_seqgroup{};		// sequence group for demand loading

	TranscriptedSequence(std::byte const* buf, mstudioseqdesc_t const* src) noexcept
		: m_fps{ src->fps }, m_flags{ src->flags }, m_activity{ (Activity)src->activity }, m_actweight{ src->actweight },
		m_numframes{ src->numframes }, m_numpivots{ src->numpivots }, m_linearmovement{ src->linearmovement },
		m_seqgroup{ src->seqgroup }
	{
		memcpy(m_szLabel, src->label, sizeof(m_szLabel));
		std::span const Events{ reinterpret_cast<mstudioevent_t const*>(buf + src->eventindex), src->numevents };

		m_Events.append_range(Events);
	}
	TranscriptedSequence(TranscriptedSequence&&) noexcept = default;
	TranscriptedSequence& operator=(TranscriptedSequence const&) noexcept = default;
	TranscriptedSequence& operator=(TranscriptedSequence&&) noexcept = default;
	~TranscriptedSequence() noexcept = default;
};

export struct TranscriptedStudio final
{
	char				m_szName[64]{};

	uint32_t			m_bitsFlags{};

	std::vector<mstudiobone_t>				m_Bones{};
	std::vector<mstudiobonecontroller_t>	m_Controllers{};
	std::vector<mstudiobbox_t>				m_Hitboxes{};
	std::vector<TranscriptedSequence>		m_Sequences{};
	std::vector<mstudioseqgroup_t>			m_SeqGroups{};	// demand loaded sequences
	std::vector<mstudiotexture_t>			m_Textures{};
	std::vector<std::vector<uint16_t>>		m_Skins{};	// Method from HLAM
	std::vector<TranscriptedPartGroup>		m_Parts{};
	std::vector<mstudioattachment_t>		m_Attachments{};
	std::vector<std::vector<uint8_t>>		m_Transitions{};	// animation node to animation node transition graph

	TranscriptedStudio(std::byte const* buf) noexcept
	{
		auto const pHeader = reinterpret_cast<studiohdr_t const*>(buf);

#ifdef _DEBUG
		[[maybe_unused]] std::span const Bones{ (mstudiobone_t*)(buf + pHeader->boneindex), (size_t)pHeader->numbones };
		[[maybe_unused]] std::span const Controllers{ (mstudiobone_t*)(buf + pHeader->bonecontrollerindex), (size_t)pHeader->numbonecontrollers };
		[[maybe_unused]] std::span const HitBoxes{ (mstudiobbox_t*)(buf + pHeader->hitboxindex), (size_t)pHeader->numhitboxes };
		[[maybe_unused]] std::span const Sequences{ (mstudioseqdesc_t*)(buf + pHeader->seqindex), (size_t)pHeader->numseq };
		[[maybe_unused]] std::span const SeqGroups{ (mstudioseqgroup_t*)(buf + pHeader->seqgroupindex), (size_t)pHeader->numseqgroups };
		[[maybe_unused]] std::span const Textures{ (mstudiotexture_t*)(buf + pHeader->textureindex), (size_t)pHeader->numtextures };

		[[maybe_unused]] std::span const BodyParts{ (mstudiobodyparts_t*)(buf + pHeader->bodypartindex), (size_t)pHeader->numbodyparts };
		[[maybe_unused]] std::span const Attachments{ (mstudioattachment_t*)(buf + pHeader->attachmentindex), (size_t)pHeader->numattachments };
#endif

		memcpy(m_szName, pHeader->name, sizeof(m_szName));
		m_bitsFlags = pHeader->flags;

		m_Bones.resize(pHeader->numbones);
		memcpy(m_Bones.data(), buf + pHeader->boneindex, sizeof(mstudiobone_t) * pHeader->numbones);

		m_Controllers.resize(pHeader->numbonecontrollers);
		memcpy(m_Controllers.data(), buf + pHeader->bonecontrollerindex, sizeof(mstudiobonecontroller_t) * pHeader->numbonecontrollers);

		m_Hitboxes.resize(pHeader->numhitboxes);
		memcpy(m_Hitboxes.data(), buf + pHeader->hitboxindex, sizeof(mstudiobbox_t) * pHeader->numhitboxes);

		{
			m_Sequences.reserve(pHeader->numseq);
			auto source = reinterpret_cast<mstudioseqdesc_t const*>(buf + pHeader->seqindex);
			for (size_t i = 0; i < pHeader->numseq; ++i, ++source)
				m_Sequences.emplace_back(buf, source);
		}

		m_SeqGroups.resize(pHeader->numseqgroups);
		memcpy(m_SeqGroups.data(), buf + pHeader->seqgroupindex, sizeof(mstudioseqgroup_t) * pHeader->numseqgroups);

		m_Textures.resize(pHeader->numtextures);
		memcpy(m_Textures.data(), buf + pHeader->textureindex, sizeof(mstudiotexture_t) * pHeader->numtextures);

		{
			// Method from HLAM
			m_Skins.reserve(pHeader->numskinfamilies);
			auto source = reinterpret_cast<const uint16_t*>(buf + pHeader->skinindex);
			for (size_t i = 0; i < pHeader->numskinfamilies; ++i, source += pHeader->numskinref)
				m_Skins.emplace_back(source, source + pHeader->numskinref);
		}

		{
			m_Parts.reserve(pHeader->numbodyparts);
			auto source = reinterpret_cast<mstudiobodyparts_t const*>(buf + pHeader->bodypartindex);
			for (size_t i = 0; i < pHeader->numbodyparts; ++i, ++source)
				m_Parts.emplace_back(buf, source);
		}

		m_Attachments.resize(pHeader->numattachments);
		memcpy(m_Attachments.data(), buf + pHeader->attachmentindex, sizeof(mstudioattachment_t) * pHeader->numattachments);

		{
			m_Transitions.reserve(pHeader->numtransitions);
			for (size_t i = 0; i < pHeader->numtransitions; ++i)
			{
				auto source = (uint8_t*)(buf + pHeader->transitionindex) + (pHeader->numtransitions * i);
				std::vector<std::uint8_t> res;
				res.resize(pHeader->numtransitions);
				memcpy(res.data(), source, res.size());

				m_Transitions.push_back(std::move(res));
			}
		}
	}
	TranscriptedStudio(TranscriptedStudio const&) noexcept = default;
	TranscriptedStudio(TranscriptedStudio&&) noexcept = default;
	TranscriptedStudio& operator=(TranscriptedStudio const&) noexcept = default;
	TranscriptedStudio& operator=(TranscriptedStudio&&) noexcept = default;
	~TranscriptedStudio() noexcept = default;
};

export struct seq_timing_t final
{
	std::int32_t m_iSeqIdx{ -1 };
	Activity m_Activity{ ACT_INVALID };
	float m_flGroundSpeed{ 0 };
	float m_flFrameRate{ 256.f };
	float m_fz_begin{ -1 };
	float m_fz_end{ -1 };
	float m_total_length{ -1 };
};

struct CaseIgnoredStrLess final
{
	static constexpr char to_lower(char c) noexcept
	{
		if ((c & 0b1000'0000) == 0b1000'0000)	// utf-8 character, keep it as it is.
			return c;

		if (c >= 'A' && c <= 'Z')
			return static_cast<char>(c ^ 0b0010'0000);	// Flip the 6th bit

		return c;
	}

	static constexpr bool operator() (std::string_view lhs, std::string_view rhs) noexcept
	{
		return std::ranges::lexicographical_compare(
			lhs,
			rhs,
			{},
			&to_lower,
			&to_lower
		);
	}

	using is_transparent = int;
};

namespace GoldSrc
{
	// gStudioInfo["v_ak47.mdl"]["idle"] => std::pair
	using studio_timing_t = std::map<std::string, seq_timing_t, CaseIgnoredStrLess>;

	export inline std::vector<std::string> m_rgszCurrentError{};
	export inline std::map<std::string, studio_timing_t, CaseIgnoredStrLess> m_StudioInfo;

	export [[nodiscard]] inline
	studio_timing_t& GetStudioModelTiming(std::string_view szRelativePath) noexcept
	{
		auto const RegisteredPath = FileSystem::RelativeToWorkingDir(szRelativePath);

		try
		{
			return m_StudioInfo.at(RegisteredPath);
		}
		catch (const std::exception& e)
		{
			UTIL_Terminate("Exception: %s\nFile '%s' accessed without cached!", e.what(), RegisteredPath);
		}
	}

	inline double WeaponSoundLength(std::string_view sz5004Option) noexcept
	{
		auto const szPath = std::format("sound/{}", sz5004Option);
		auto const RegisteredPath = FileSystem::RelativeToWorkingDir(szPath);

		return Wave::Length(RegisteredPath.c_str());
	}

	inline void BuildStudioModelInfo(FILE* f, studio_timing_t* pStudioInfo) noexcept
	{
		auto& StudioInfo = *pStudioInfo;

		fseek(f, 0, SEEK_END);
		auto const iSize = ftell(f);

		fseek(f, 0, SEEK_SET);
		auto pBuffer = new std::byte[iSize]{};
		fread(pBuffer, iSize, 1, f);

		auto const phdr = reinterpret_cast<studiohdr_t*>(pBuffer);
		auto const pseq = reinterpret_cast<mstudioseqdesc_t*>(pBuffer + phdr->seqindex);
		auto const seqs = std::span{ pseq, static_cast<size_t>(phdr->numseq) };

		for (auto&& seq : seqs)
		{
			if (StudioInfo.contains(seq.label))
			{
				m_rgszCurrentError.emplace_back(std::format(
					"{}: Multiple sequences with same name '{}'.",
					__FUNCTION__,
					seq.label
				));

				continue;
			}

			// https://en.cppreference.com/w/cpp/language/structured_binding
			// The type deduction indicates these are copy.
			// But they are not.
			// decltype(flFzBegin) == float, is because it is a float in the original type declaration.
			auto& [iSeqIdx, iAct, flGrSpd, flFR, flFzBegin, flFzEnd, flAnimLen] =
				StudioInfo.try_emplace(seq.label).first->second;

			// Idx could just be ptr diff
			iSeqIdx = std::addressof(seq) - pseq;

			iAct = (Activity)seq.activity;

			if (seq.numevents)
			{
				auto const pevents = reinterpret_cast<mstudioevent_t*>(pBuffer + seq.eventindex);
				auto const events = std::span{ pevents, static_cast<size_t>(seq.numevents) };

				// Find the begin of the first sound event.
				if (auto ev = std::ranges::find(events, 5004, &mstudioevent_t::event);
					ev != std::end(events))
				{
					flFzBegin = (float)((double)ev->frame / (double)seq.fps);
				}

				// Find the end of last sound event.
				if (auto rev = std::ranges::find_last(events, 5004, &mstudioevent_t::event);
					rev.cbegin() != rev.cend())
				{
					auto ev = rev.begin();
					flFzEnd = (float)((double)ev->frame / (double)seq.fps);

					if (auto const flSoundLen = WeaponSoundLength(ev->options); flSoundLen > 0)
						flFzEnd += (float)flSoundLen;
				}
			}

			// Total len
			flAnimLen = (float)((double)seq.numframes / (double)seq.fps);

			// Speed in inch per second
			GetSequenceInfo(phdr, iSeqIdx, &flFR, &flGrSpd);
		}

		delete[] pBuffer; pBuffer = nullptr;
	}

	export bool CacheStudioModelInfo(std::string_view szRelativePath) noexcept
	{
		// Fresh start.
		m_rgszCurrentError.clear();

		auto const RegisteredPath = FileSystem::RelativeToWorkingDir(szRelativePath);
		auto&& [iter, bNewEntry] = m_StudioInfo.try_emplace(RegisteredPath);
		auto& StudioInfo = iter->second;

		[[unlikely]]
		if (!bNewEntry)
		{
			m_rgszCurrentError.emplace_back(
				std::format("{}: Studio model '{}' had been loaded.", __FUNCTION__, RegisteredPath)
			);

			return true;
		}

		[[likely]]
		if (auto f = std::fopen(RegisteredPath.c_str(), "rb"); f)
		{
			BuildStudioModelInfo(f, &StudioInfo);

			// Save the simplified path as well, like "models/v_ak47.mdl"
			// It's more useful than "cstrike/models/v_ak47.mdl"
			m_StudioInfo.try_emplace(std::string{ szRelativePath }, StudioInfo);

			fclose(f);
		}
		else
		{
			m_rgszCurrentError.emplace_back(
				std::format("{}: File '{}' no found.", __FUNCTION__, RegisteredPath)
			);

			return false;
		}

		return true;
	}
}

// Const version of the namespaced one. Just for the sake of safty.
export inline auto const& gStudioInfo{ GoldSrc::m_StudioInfo };
