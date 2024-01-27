module;

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX	// LUNA: first thing to do when using windows header. It makes 'Windows SDK for C++' compatiable with C++. Fuck Microsoft.
#include <Windows.h>

#include <wchar.h>

export module Platform;

export import <algorithm>;
export import <bit>;
export import <codecvt>;
export import <locale>;
export import <stacktrace>;

export inline std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> g_utf_converter;

export template <typename... Tys> [[noreturn]] void UTIL_Terminate(const char *psz, Tys&&... args) noexcept
{
	auto const StackTraceObject = std::stacktrace::current();
	auto const szStackTraceInfo = std::to_string(StackTraceObject);

	static char sz[256]{};
	_snprintf(sz, _countof(sz) - 1U, psz, std::forward<Tys>(args)...);

	auto const szMergedInfo = szStackTraceInfo + "\n\n" + sz;

	MessageBoxA(nullptr, szMergedInfo.c_str(), nullptr, MB_OK);
	std::terminate();
}

export template <typename... Tys> [[noreturn]] void UTIL_Terminate(const wchar_t *psz, Tys&&... args) noexcept
{
	if constexpr (sizeof...(Tys) > 0)
		static_assert(((int)std::is_same_v<std::remove_cvref_t<Tys>, std::string> + ...) <= 1, "Only one UTF-8 to UTF-16 conversion can be performed during each call.");

	static constexpr auto fnArgHandle = []<typename T>(T &&arg) noexcept -> decltype(auto)
	{
		static char16_t rgu16[64]{};

		if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>)
		{
			auto const szu16 = g_utf_converter.from_bytes(arg);
			memcpy(rgu16, szu16.c_str(), std::min(sizeof(rgu16) - 1U * sizeof(char16_t), szu16.size() * sizeof(char16_t)));

			return &rgu16[0];
		}
		else
			return arg;
	};

	auto const StackTraceObject = std::stacktrace::current();
	auto const wszStackTraceInfo = std::bit_cast<std::wstring>(g_utf_converter.from_bytes(std::to_string(StackTraceObject)));

	static wchar_t wsz[256]{};
	_snwprintf(wsz, _countof(wsz) - 1U, psz, fnArgHandle(args)...);

	auto const szMergedInfo = wszStackTraceInfo + L"\n\n" + wsz;

	MessageBoxW(nullptr, szMergedInfo.c_str(), nullptr, MB_OK);
	std::terminate();
}
