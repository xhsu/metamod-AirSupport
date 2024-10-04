module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

export module WinAPI;

import std;

import UtlHook;

export inline decltype(std::invoke(&MH_GetModuleBase, HMODULE{})) gSelfModuleBase{};
export inline decltype(std::invoke(&MH_GetModuleBase, HMODULE{})) gSelfModuleSize{};
export inline HMODULE gSelfModuleHandle{};

// https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-entry-point-function
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,	// handle to DLL module
	DWORD fdwReason,	// reason for calling function
	LPVOID lpReserved)	// reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.

		gSelfModuleBase = MH_GetModuleBase(hinstDLL);
		gSelfModuleSize = MH_GetModuleSize(hinstDLL);

		gSelfModuleHandle = hinstDLL;

		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}

	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
