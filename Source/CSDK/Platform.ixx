module;

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX	// LUNA: first thing to do when using windows header. It makes 'Windows SDK for C++' compatiable with C++. Fuck Microsoft.
#include <Windows.h>

#include <stdio.h>

export module Platform;

import std;


export template <typename... Tys> [[noreturn]] void UTIL_Terminate(const char *psz, Tys&&... args) noexcept
{
	auto const StackTraceObject = std::stacktrace::current();
	auto const szStackTraceInfo = std::to_string(StackTraceObject);

	auto const iSize = _snprintf(nullptr, 0, psz, std::forward<Tys>(args)...);
	auto const pbuf = new char[iSize + 1] {};
	_snprintf(pbuf, iSize + 1, psz, std::forward<Tys>(args)...);

	auto const szMergedInfo = szStackTraceInfo + "\n\n" + pbuf;
	delete[] pbuf;

	MessageBoxA(nullptr, szMergedInfo.c_str(), nullptr, MB_OK);
	std::terminate();
}
