// Enforcer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Should output timepoint?
//#define SHOW_TIME_POINT

#ifndef __INTELLISENSE__
import <cstdint>;
import <cstdio>;
import <cstdlib>;

import <algorithm>;
import <format>;
import <fstream>;
import <map>;
import <print>;
import <ranges>;
import <span>;
import <vector>;
#else
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <format>
#include <fstream>
#include <map>
#include <print>
#include <ranges>
#include <span>
#include <vector>
#endif


import CRC64;
import Localization;
import Resources;
import Sprite;
import Wave;

import studio;

using std::map;
using std::string;
using std::string_view;
using std::vector;

using namespace std::literals;

inline map<string, double, std::less<string_view>> g_rgflSoundTime = {};
inline map<string, uint32_t, std::less<string_view>> g_rgiSpriteFrameCount = {};
inline map<string, uint64_t, std::less<string_view>> g_rgiCRC64 = {};
inline map<string, vector<double>, std::less<string_view>> g_rgrgflAnimTime = {};
inline map<string, vector<string>, std::less<string_view>> g_rgrgszSeqName = {};

extern void Precache(void) noexcept;

static int AddModel(const char* psz) noexcept
{
	string_view const sz{ psz };
	string const szPath = std::format(SOLUTION_DIR "Resource\\{}", psz);

	if (auto f = fopen(szPath.c_str(), "rb"); f)
	{
		if (sz.ends_with(".mdl") && !g_rgrgflAnimTime.contains(psz))
		{
			fseek(f, 0, SEEK_END);
			auto const iSize = ftell(f);

			fseek(f, 0, SEEK_SET);
			auto pBuffer = calloc(1, iSize);
			fread(pBuffer, iSize, 1, f);

			auto const phdr = (studiohdr_t*)pBuffer;
			auto const pseq = (mstudioseqdesc_t*)((uint8_t*)pBuffer + phdr->seqindex);
			auto const rgSeq = std::span(pseq, phdr->numseq);

			for (auto&& SeqInfo : rgSeq)
			{
				g_rgrgflAnimTime[psz].push_back((double)SeqInfo.numframes / (double)SeqInfo.fps);
				g_rgrgszSeqName[psz].emplace_back(SeqInfo.label);
			}

			free(pBuffer);
		}
		else if (sz.ends_with(".spr") && !g_rgiSpriteFrameCount.contains(psz))
		{
			GoldSrc::Sprite_t const hSprite{ f };

			g_rgiSpriteFrameCount[psz] = hSprite.m_iNumOfFrames;
		}

		g_rgiCRC64[psz] = CRC64::CheckFile(f);
		fclose(f);
	}

	return {};
}

static int AddSound(const char* psz) noexcept
{
	string_view const sz{ psz };
	string const szPath = std::format(SOLUTION_DIR "Resource\\sound\\{}", psz);

	g_rgiCRC64[psz] = CRC64::CheckFile(szPath.c_str());
	g_rgflSoundTime[psz] = Wave::Length(szPath.c_str());
	return {};
}

static long FTell(const char* psz) noexcept
{
	if (auto f = std::fopen(psz, "rb"))
	{
		fseek(f, 0, SEEK_END);
		auto const ret = ftell(f);
		fclose(f);

		return ret;
	}

	return 0;
}

static bool WriteFileIfCrcDiff(const char* file, string const& sz) noexcept
{
	auto const iOriginalFileCRC = CRC64::CheckFile(file);
	auto const iNewFileCRC = CRC64::CheckStream(reinterpret_cast<std::byte const*>(sz.c_str()), sz.length());

	if (iOriginalFileCRC != iNewFileCRC)
	{
		if (auto f = std::fopen(file, "wb"); f)
		{
			fwrite(sz.c_str(), sizeof(char), sz.length(), f);
			fclose(f);

			return true;
		}
	}

	return false;
}

inline constexpr char FILE_CRC64[] = PROJECT_DIR "Resource_CRC64.hpp";
inline constexpr char FILE_MODELS[] = PROJECT_DIR "Resource_ModelDetails.hpp";
inline constexpr char FILE_SPRITES[] = PROJECT_DIR "Resource_SpriteDetails.hpp";
inline constexpr char FILE_SOUNDS[] = PROJECT_DIR "Resource_SoundDetails.hpp";
inline constexpr char FILE_LOCALIZATION[] = PROJECT_DIR "czero_tchinese.txt";

int main() noexcept
{
	// 101 for all C++ project.
	std::ios_base::sync_with_stdio(false);

	g_engfuncs.pfnPrecacheModel = &AddModel;
	g_engfuncs.pfnPrecacheSound = &AddSound;
	g_engfuncs.pfnDecalIndex = [](const char*) noexcept -> int { return 0; };

	Precache();

	// All of the folllowing file writing would have binary in use.
	// Otherwise \r\n won't be interpret as \n

	/*
	* CRC64 for all file.
	*/
	if (!g_rgiCRC64.empty())
	{
		string sz{};
		sz.reserve(FTell(FILE_CRC64));

#ifdef SHOW_TIME_POINT
		sz += "// File Generated at: " __DATE__ " " __TIME__ "\n";
#endif
		sz += '\n';
		sz += "EXPORT inline std::unordered_map<std::string_view, uint64_t> const g_rgiCRC64 = \n";
		sz += "{\n";

		auto const iter = std::ranges::max_element(g_rgiCRC64 | std::views::keys, {}, &string::length);
		auto const iExpectedSpaceBetween = (*iter).size() + 1;	// What the fuck, MSVC?

		for (auto&& [szFile, iCRC] : g_rgiCRC64)
		{
			// CRC == 0 means file does not presents here. Assumed come with original CS/CZ/HL
			sz += std::format(
				"{3}\t{{ {0:<{2}}, 0x{1:0>16X}ui64 }},\n",
				std::format("\"{}\"", szFile), iCRC,
				iExpectedSpaceBetween + 2/* One for \", another for ',' */, iCRC == 0 ? "//"sv : ""sv
			);
		}

		sz += "};\n";

		WriteFileIfCrcDiff(FILE_CRC64, sz);
	}

	/*
	* Model Animation Time
	*/
	if (!g_rgrgflAnimTime.empty())	// #UPDATE_AT_CPP26 P2447R4 std::span over an initializer list
	{
		string sz{};
		sz.reserve(FTell(FILE_MODELS));

#ifdef SHOW_TIME_POINT
		sz += "// File Generated at: " __DATE__ " " __TIME__ "\n";
#endif
		sz += '\n';
		sz += "EXPORT inline std::unordered_map<std::string_view, std::vector<double>> const g_rgrgflAnimTime = \n";
		sz += "{\n";

		auto const iter = std::ranges::max_element(g_rgrgflAnimTime | std::views::keys, {}, &string::length);
		auto const iExpectedSpaceBetween = (*iter).size() + 1;

		for (auto&& [szModel, rgflAnimTime] : g_rgrgflAnimTime)
		{
			auto& rgszAnimName = g_rgrgszSeqName[szModel];

			sz += std::format("\t{{ {0:<{1}}, std::vector<double>{{ ", std::format("\"{}\"", szModel), iExpectedSpaceBetween + 2);

			[[unlikely]]
			if (rgszAnimName.size() != rgflAnimTime.size())
				std::unreachable();

			for (auto&& [szAnim, flTime] : std::views::zip(rgszAnimName, rgflAnimTime))	// #UPDATE_AT_CPP23 Formatting Ranges (P2286R8)
				sz += std::format("/* {0} */ {1}, ", szAnim, flTime);

			sz += "} },\n";
		}

		sz += "};\n";

		WriteFileIfCrcDiff(FILE_MODELS, sz);
	}

	/*
	* Sprite Frame Count
	*/
	if (!g_rgiSpriteFrameCount.empty())
	{
		string sz{};
		sz.reserve(FTell(FILE_SPRITES));

#ifdef SHOW_TIME_POINT
		sz += "// File Generated at: " __DATE__ " " __TIME__ "\n";
#endif
		sz += '\n';
		sz += "EXPORT inline std::unordered_map<std::string_view, int> const g_rgiSpriteFrameCount = \n";
		sz += "{\n";

		auto const iter = std::ranges::max_element(g_rgiSpriteFrameCount | std::views::keys, {}, &string::length);
		auto const iExpectedSpaceBetween = (*iter).size() + 1;

		for (auto&& [szModel, iFrameCount] : g_rgiSpriteFrameCount)
			sz += std::format("\t{{ {0:<{2}}, {1}\t}},\n", std::format("\"{}\"", szModel), iFrameCount, iExpectedSpaceBetween + 2);

		sz += "};\n";

		WriteFileIfCrcDiff(FILE_SPRITES, sz);
	}

	/*
	* Sound
	*/
	if (!g_rgflSoundTime.empty())
	{
		string sz{};
		sz.reserve(FTell(FILE_SOUNDS));

#ifdef SHOW_TIME_POINT
		sz += "// File Generated at: " __DATE__ " " __TIME__ "\n";
#endif
		sz += '\n';
		sz += "EXPORT inline std::unordered_map<std::string_view, double> const g_rgflSoundTime = \n";
		sz += "{\n";

		auto const iter = std::ranges::max_element(g_rgflSoundTime | std::views::keys, {}, [](auto&& s) noexcept -> size_t { return s.length(); });
		auto const iExpectedSpaceBetween = (*iter).size() + 1;

		for (auto&& [szModel, flSoundLength] : g_rgflSoundTime)
			sz += std::format("{3}\t{{ {0:<{2}}, {1}\t}},\n", std::format("\"{}\"", szModel), flSoundLength, iExpectedSpaceBetween + 2, flSoundLength > 0 ? "" : "//");

		sz += "};\n";

		WriteFileIfCrcDiff(FILE_SOUNDS, sz);
	}

	/*
	* Generate Localization File
	*/

	if (auto f = std::fopen(FILE_LOCALIZATION, "wt"); f)
	{
		for (auto&& [K, E, C] : std::views::zip(Localization::Keys, Localization::L_EN, Localization::L_CH))
		{
			std::print(f, "\"{}\"\t\"{}\"\n", K.substr(1), C);
			std::print(f, "\"[english]{}\"\t\"{}\"\n", K.substr(1), E);
		}

		fclose(f);
	}
}
