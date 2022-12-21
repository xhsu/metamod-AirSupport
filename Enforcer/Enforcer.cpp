// Enforcer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

import <cstdint>;

import <algorithm>;
import <format>;
import <fstream>;
import <iostream>;
import <ranges>;
import <map>;

import CRC64;
import Resources;

using std::string;
using std::map;

inline map<string, uint64_t> g_rgiCRC64 = {};

extern void Precache(void) noexcept;

int AddModel(const char* psz) noexcept
{
	g_rgiCRC64[psz] = CRC64::CheckFile(std::format(SOLUTION_DIR "Resource\\{}", psz).c_str());
	return {};
}

int AddSound(const char* psz) noexcept
{
	g_rgiCRC64[psz] = CRC64::CheckFile(std::format(SOLUTION_DIR "Resource\\sound\\{}", psz).c_str());
	return {};
}

int main() noexcept
{
	g_engfuncs.pfnPrecacheModel = &AddModel;
	g_engfuncs.pfnPrecacheSound = &AddSound;
	g_engfuncs.pfnDecalIndex = [](const char*) -> int { return 0; };

	Precache();

	if (std::ofstream fout(PROJECT_DIR "ResourceCRC64.hpp"); fout && !g_rgiCRC64.empty())
	{
		fout << "// File Generated at: " __DATE__ " " __TIME__ "\n";
		fout << '\n';
		fout << "static std::unordered_map<std::string, uint64_t> const g_rgiCRC64 = " << '\n';
		fout << "{" << '\n';

		auto const iter = std::ranges::max_element(g_rgiCRC64 | std::views::keys, {}, [](auto&& s) noexcept -> size_t { return s.length(); });
		auto const iExpectedSpaceBetween = (*iter).size() + 1;	// What the fuck, MSVC?

		for (auto&& [szFile, iCRC] : g_rgiCRC64)
		{
			std::cout << std::format("{0:<{2}}{1:X}\n", szFile, iCRC, iExpectedSpaceBetween);

			if (!iCRC)
				continue;	// File does not presents here. assumed come with original CS/CZ/HL

			fout << std::format("\t{{ {0:<{2}}, 0x{1:0>16X}ui64 }},\n", std::format("\"{}\"", szFile), iCRC, iExpectedSpaceBetween + 2/* One for \", another for ',' */);
		}

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
