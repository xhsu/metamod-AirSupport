#include <stdio.h>
#include <stdlib.h>

import std;
import hlsdk;

import Transpiler;

using std::free;
using std::calloc;

static auto UTIL_Split(std::string_view const& s, char const* delimiters) noexcept -> std::vector<std::string_view>
{
	std::vector<std::string_view> ret{};

	for (auto lastPos = s.find_first_not_of(delimiters, 0), pos = s.find_first_of(delimiters, lastPos);
		s.npos != pos || s.npos != lastPos;
		lastPos = s.find_first_not_of(delimiters, pos), pos = s.find_first_of(delimiters, lastPos)
		)
	{
		ret.emplace_back(s.substr(lastPos, pos - lastPos));
	}

	return ret;
}

void __stdcall AddSoundFromModel(const wchar_t* pszPath) noexcept
{
	if (!pszPath)
		return;

	[[likely]]
	if (auto f = _wfopen(pszPath, L"rb"); f)
	{
		fseek(f, 0, SEEK_END);
		auto const iSize = ftell(f);

		fseek(f, 0, SEEK_SET);
		auto pBuffer = calloc(1, iSize);
		fread(pBuffer, iSize, 1, f);

		auto const phdr = (studiohdr_t*)pBuffer;
		auto const pseq = (mstudioseqdesc_t*)((std::byte*)pBuffer + phdr->seqindex);

		for (unsigned i = 0; i < phdr->numseq; ++i)
		{
			auto const pevt = (mstudioevent_t*)((std::byte*)pBuffer + pseq[i].eventindex);	// still an offset from location '0' of the entire file

			for (auto j = 0; j < pseq[i].numevents; ++j)
			{
				auto&& StudioEvent = pevt[j];

				if (StudioEvent.event != 5004)	// 5004: client side sound.
					continue;

				g_engfuncs.pfnPrecacheSound(StudioEvent.options);
			}
		}

		free(pBuffer);
		fclose(f);
	}
}

void __stdcall AddSpriteFromHud(const wchar_t* pszPath) noexcept
{
	if (!pszPath)
		return;

	[[likely]]
	if (auto f = _wfopen(pszPath, L"rb"); f)
	{
		fseek(f, 0, SEEK_END);
		auto const iSize = (std::size_t)ftell(f);

		fseek(f, 0, SEEK_SET);
		auto pBuffer = calloc(1, iSize + 1);
		fread(pBuffer, iSize, 1, f);

		std::string_view const szHudText{ (char*)pBuffer, iSize };
		auto const rgszLines = UTIL_Split(szHudText, "\r\n");

		for (auto&& szLine : rgszLines)
		{
			auto pos = szLine.find("//");
			auto const sz = pos == szLine.npos ? szLine : szLine.substr(0, pos);

			auto const rgszArguments = UTIL_Split(sz, "\t ");
			if (rgszArguments.size() != 7)	// only the effective lines.
				continue;

			g_engfuncs.pfnPrecacheModel(
				std::format("sprites/{}.spr", rgszArguments[2]).c_str()
			);
		}

		free(pBuffer);
		fclose(f);
	}
}

void __stdcall ReceiveCSharpFnPtr(
	int(__cdecl *pfnLoadModel)(const char *),
	int(__cdecl *pfnLoadSound)(const char *),
	int(__cdecl *pfnLoadGeneric)(const char *)
) noexcept
{
	g_engfuncs.pfnPrecacheModel = pfnLoadModel;
	g_engfuncs.pfnPrecacheSound = pfnLoadSound;
	g_engfuncs.pfnPrecacheGeneric = pfnLoadGeneric;
	g_engfuncs.pfnDecalIndex = [](const char *) -> int { return 0; };
}
