module;

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

export enum EEventTypeId
{
	STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_0 = 0x1389,
	STUDIOEV_MUZZLESPARK = 0x138A,
	STUDIOEV_PLAYSOUND = 0x138C,
	STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_1 = 0x1393,
	STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_2 = 0x139D,
	STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_3 = 0x13A7,
};

export struct TranscriptedModel
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

export struct TranscriptedPartGroup final
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

export struct TranscriptedSequence final
{
	char				m_szLabel[32]{};	// sequence label
	int32_t				m_index{};			// LUNA: added for convenience.

	float				m_fps{};		// frames per second	
	int32_t				m_flags{};		// looping/non-looping flags

	Activity			m_activity{};
	int32_t				m_actweight{};

	std::vector<mstudioevent_t> m_Events{};
	std::vector<std::any> m_EventExtraInfo{};

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
		m_EventExtraInfo.resize(Events.size());

		for (auto&& [Event, ExtraInfo] : std::views::zip(m_Events, m_EventExtraInfo))
		{
			switch (Event.event)
			{
			case STUDIOEV_PLAYSOUND:
			{
				auto const szPath = std::format("sound/{}", Event.options);
				auto const RegisteredPath = FileSystem::RelativeToWorkingDir(szPath);
				ExtraInfo = Wave::Length(RegisteredPath.c_str());

				break;
			}

			case STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_0:
			case STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_1:
			case STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_2:
			case STUDIOEV_MUZZLEFLASH_AT_ATTACHMENT_3:
			case STUDIOEV_MUZZLESPARK:
				ExtraInfo = std::atoi(Event.options);
				break;

			default:
				break;
			}
		}
	}
	TranscriptedSequence(TranscriptedSequence&&) noexcept = default;
	TranscriptedSequence& operator=(TranscriptedSequence const&) noexcept = default;
	TranscriptedSequence& operator=(TranscriptedSequence&&) noexcept = default;
	~TranscriptedSequence() noexcept = default;

	[[nodiscard]] constexpr auto GetFirstEventTime() const noexcept -> float
	{
		if (m_Events.empty())
			return -1;

		return decltype(m_fps)(m_Events.front().frame) / m_fps;
	}

	[[nodiscard]] constexpr auto GetTotalLength() const noexcept -> float
	{
		return (decltype(m_fps))m_numframes / m_fps;
	}

	[[nodiscard]] auto GetTimeAfterLastWav() const noexcept -> float
	{
		auto const SearchResult = std::ranges::find_last(m_Events, STUDIOEV_PLAYSOUND, &mstudioevent_t::event);
		if (SearchResult.empty())
			return -1;

		auto& Event = *SearchResult.begin();
		auto const EventIndex = std::addressof(Event) - m_Events.data();
		auto const flEventTimeSpot = Event.frame / m_fps;

		try
		{
			return flEventTimeSpot + std::any_cast<float>(m_EventExtraInfo.at(EventIndex));
		}
		catch ([[maybe_unused]] const std::exception& e)
		{
			return -1;
		}

	}
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
				m_Sequences.emplace_back(buf, source).m_index = (int)i;
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

	[[nodiscard]] auto AtSequence(std::string_view szLabel) const noexcept -> TranscriptedSequence const*
	{
		auto res = std::ranges::find(m_Sequences, szLabel, &TranscriptedSequence::m_szLabel);
		if (res != std::ranges::cend(m_Sequences))
			return std::addressof(*res);

		return nullptr;
	}
};
