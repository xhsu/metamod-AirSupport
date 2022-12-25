#include <stdio.h>
#include <stdlib.h>

import studio;

import Transpiler;

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
		auto const pseq = (mstudioseqdesc_t*)((byte*)pBuffer + phdr->seqindex);

		for (auto i = 0; i < phdr->numseq; ++i)
		{
			auto const pevt = (mstudioevent_t*)((byte*)pBuffer + pseq[i].eventindex);	// still an offset from location '0' of the entire file

			for (auto j = 0; j < pseq[i].numevents; ++j)
			{
				auto&& StudioEvent = pevt[j];

				if (StudioEvent.event != 5004)	// 5004: client side sound.
					continue;

				g_engfuncs.pfnPrecacheSound(StudioEvent.options);
			}
		}

		fclose(f);
	}
}

void __stdcall ReceiveCSharpFnPtr(int(__cdecl *pfnLoadModel)(const char *), int(__cdecl *pfnLoadSound)(const char *)) noexcept
{
	g_engfuncs.pfnPrecacheModel = pfnLoadModel;
	g_engfuncs.pfnPrecacheSound = pfnLoadSound;
	g_engfuncs.pfnDecalIndex = [](const char *) -> int { return 0; };
}
