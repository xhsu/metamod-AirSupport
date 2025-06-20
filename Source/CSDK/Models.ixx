module;

#include <stdio.h>

#ifdef __INTELLISENSE__
#include <algorithm>
#endif

export module Models;

import std;
import hlsdk;

import FileSystem;
import Platform;
import Wave;

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
