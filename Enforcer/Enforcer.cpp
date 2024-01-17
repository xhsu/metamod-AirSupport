// Enforcer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Should output timepoint?
//#define SHOW_TIME_POINT

import <cstdint>;
import <cstdio>;
import <cstdlib>;

import <algorithm>;
import <format>;
import <fstream>;
import <iostream>;
import <map>;
import <ranges>;
import <span>;
import <vector>;

import CRC64;
import Resources;
import Sprite;
import Wave;

import studio;

using std::map;
using std::string;
using std::string_view;
using std::vector;

inline map<string, double> g_rgflSoundTime = {};
inline map<string, uint32_t> g_rgiSpriteFrameCount = {};
inline map<string, uint64_t> g_rgiCRC64 = {};
inline map<string, vector<double>> g_rgrgflAnimTime = {};
inline map<string, vector<string>> g_rgrgszSeqName = {};

extern void Precache(void) noexcept;

int AddModel(const char* psz) noexcept
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

int AddSound(const char* psz) noexcept
{
	string_view const sz{ psz };
	string const szPath = std::format(SOLUTION_DIR "Resource\\sound\\{}", psz);

	g_rgiCRC64[psz] = CRC64::CheckFile(szPath.c_str());
	g_rgflSoundTime[psz] = Wave::Length(szPath.c_str());
	return {};
}

int main() noexcept
{
	// 101 for all C++ project.
	std::ios_base::sync_with_stdio(false);

	g_engfuncs.pfnPrecacheModel = &AddModel;
	g_engfuncs.pfnPrecacheSound = &AddSound;
	g_engfuncs.pfnDecalIndex = [](const char*) noexcept -> int { return 0; };

	Precache();

	/*
	* CRC64 for all file.
	*/
	if (std::ofstream fout(PROJECT_DIR "Resource_CRC64.hpp"); fout && !g_rgiCRC64.empty())
	{
		fout
#ifdef SHOW_TIME_POINT
			<< "// File Generated at: " __DATE__ " " __TIME__ "\n"
#endif
			<< '\n'
			//<< "#include <string_view>\n"
			//<< "#include <unordered_map>\n"
			//<< '\n'
			<< "EXPORT inline std::unordered_map<std::string_view, uint64_t> const g_rgiCRC64 = " << '\n'
			<< "{" << '\n';

		auto const iter = std::ranges::max_element(g_rgiCRC64 | std::views::keys, {}, [](auto&& s) noexcept -> size_t { return s.length(); });
		auto const iExpectedSpaceBetween = (*iter).size() + 1;	// What the fuck, MSVC?

		for (auto&& [szFile, iCRC] : g_rgiCRC64)
		{
			//std::cout << std::format("{0:<{2}}{1:X}\n", szFile, iCRC, iExpectedSpaceBetween);

			if (!iCRC)
				continue;	// File does not presents here. assumed come with original CS/CZ/HL

			fout << std::format("\t{{ {0:<{2}}, 0x{1:0>16X}ui64 }},\n", std::format("\"{}\"", szFile), iCRC, iExpectedSpaceBetween + 2/* One for \", another for ',' */);
		}

		fout << "};" << '\n';
	}

	/*
	* Model Animation Time
	*/
	if (std::ofstream fout(PROJECT_DIR "Resource_ModelDetails.hpp"); fout && !g_rgrgflAnimTime.empty())
	{
		fout
#ifdef SHOW_TIME_POINT
			<< "// File Generated at: " __DATE__ " " __TIME__ "\n"
#endif
			<< '\n'
			<< "EXPORT inline std::unordered_map<std::string_view, std::vector<double>> const g_rgrgflAnimTime = " << '\n'
			<< '{' << '\n';

		auto const iter = std::ranges::max_element(g_rgrgflAnimTime | std::views::keys, {}, [](auto&& s) noexcept -> size_t { return s.length(); });
		auto const iExpectedSpaceBetween = (*iter).size() + 1;

		for (auto&& [szModel, rgflAnimTime] : g_rgrgflAnimTime)
		{
			auto &rgszAnimName = g_rgrgszSeqName[szModel];

			fout << std::format("\t{{ {0:<{1}}, std::vector<double>{{ ", std::format("\"{}\"", szModel), iExpectedSpaceBetween + 2);

			[[unlikely]]
			if (rgszAnimName.size() != rgflAnimTime.size())
				std::unreachable();

			for (auto &&[szAnim, flTime] : std::views::zip(rgszAnimName, rgflAnimTime))	// #UPDATE_AT_CPP23 Formatting Ranges (P2286R8)
				fout << std::format("/* {0} */ {1}, ", szAnim, flTime);

			fout << "} },\n";
		}

		fout << "};" << '\n';
	}

	/*
	* Sprite Frame Count
	*/
	if (std::ofstream fout(PROJECT_DIR "Resource_SpriteDetails.hpp"); fout && !g_rgiSpriteFrameCount.empty())
	{
		fout
#ifdef SHOW_TIME_POINT
			<< "// File Generated at: " __DATE__ " " __TIME__ "\n"
#endif
			<< '\n'
			<< "EXPORT inline std::unordered_map<std::string_view, int> const g_rgiSpriteFrameCount = " << '\n'
			<< '{' << '\n';

		auto const iter = std::ranges::max_element(g_rgiSpriteFrameCount | std::views::keys, {}, [](auto&& s) noexcept -> size_t { return s.length(); });
		auto const iExpectedSpaceBetween = (*iter).size() + 1;

		for (auto&& [szModel, iFrameCount] : g_rgiSpriteFrameCount)
			fout << std::format("\t{{ {0:<{2}}, {1}\t}},\n", std::format("\"{}\"", szModel), iFrameCount, iExpectedSpaceBetween + 2);

		fout << "};" << '\n';
	}

	/*
	* Sound
	*/
	if (std::ofstream fout(PROJECT_DIR "Resource_SoundDetails.hpp"); fout && !g_rgflSoundTime.empty())
	{
		fout
#ifdef SHOW_TIME_POINT
			<< "// File Generated at: " __DATE__ " " __TIME__ "\n"
#endif
			<< '\n'
			<< "EXPORT inline std::unordered_map<std::string_view, double> const g_rgflSoundTime = " << '\n'
			<< '{' << '\n';

		auto const iter = std::ranges::max_element(g_rgflSoundTime | std::views::keys, {}, [](auto&& s) noexcept -> size_t { return s.length(); });
		auto const iExpectedSpaceBetween = (*iter).size() + 1;

		for (auto&& [szModel, flSoundLength] : g_rgflSoundTime)
			fout << std::format("{3}\t{{ {0:<{2}}, {1}\t}},\n", std::format("\"{}\"", szModel), flSoundLength, iExpectedSpaceBetween + 2, flSoundLength > 0 ? "" : "//");

		fout << "};" << '\n';
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
