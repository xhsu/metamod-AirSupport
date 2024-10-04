module;

#ifdef __INTELLISENSE__
#include <ranges>
#endif

export module Engine;

import std;
import hlsdk;

export import Platform;

export import UtlHook;

export namespace Engine
{
	inline constexpr unsigned char BUILD_NUMBER_PATTERN[] = "\xA1\x2A\x2A\x2A\x2A\x83\xEC\x08\x2A\x33\x2A\x85\xC0";
	inline constexpr unsigned char BUILD_NUMBER_NEW_PATTERN[] = "\x55\x8B\xEC\x83\xEC\x08\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\x53\x33\xDB\x8B\x04\x9D";
	inline constexpr unsigned char BUILD_NUMBER_ANNIVERSARY_PATTERN[] = "\xA1\x2A\x2A\x2A\x2A\x53\x33\xDB\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\x57\x33\xFF\x0F\x1F\x40\x00\x66\x0F\x1F\x84\x00\x00\x00\x00\x00";
	inline constexpr unsigned char HOST_VERSION_FN_PATTERN[] = "\xCC\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x6A\x30\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x50\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\xC3\xCC";

	inline constexpr int MODERN = 3248;
	inline constexpr int NEW = 6153;
	inline constexpr int LEGACY = 8684;
	inline constexpr int ANNIVERSARY = 9000;	// 9899, DEC 01 2023

	inline int(*m_pfnBuildNumber)(void) = nullptr;
	inline int BUILD_NUMBER = 0;

	inline const char* GetDLLName(void) noexcept
	{
		// #SHOULD_DO_ON_FREE

		// NAGI says that OpenGL state can be check via console var.

		//[[unlikely]]
		//if (!g_engfuncs.pfnEngCheckParm)
		//	UTIL_Terminate("Cannot retrieve engine function 'g_engfuncs.pfnEngCheckParm'.\nEither you are using a illegal copy or you disconnected from the internet since 2009.");

		//auto const ret1 = g_engfuncs.pfnEngCheckParm("-soft", nullptr);
		//auto const ret2 = g_engfuncs.pfnEngCheckParm("-software", nullptr);
		//auto const ret3 = g_engfuncs.pfnEngCheckParm("-gl", nullptr);
		//auto const ret4 = g_engfuncs.pfnEngCheckParm("-d3d", nullptr);

		return "hw.dll";
	}

	// Call once in GameInit_Post()
	void Init(void) noexcept
	{
		m_pfnBuildNumber = (decltype(m_pfnBuildNumber))UTIL_SearchPattern(
			GetDLLName(),
			0,
			BUILD_NUMBER_PATTERN,
			BUILD_NUMBER_NEW_PATTERN,
			BUILD_NUMBER_ANNIVERSARY_PATTERN
		);

		// This is the backup search method.
		// The only function that calls BuildNumber() is Host_Version() called when you input "version" into console.
		// Search that function, and get the address we want.
		if (!m_pfnBuildNumber)
		{
			// E8 * * * *
			// E8 was to call an address with an offset
			auto const addr = (std::intptr_t)UTIL_SearchPattern(
				GetDLLName(),
				1 + (0x10051C67 - 0x10051C50),
				HOST_VERSION_FN_PATTERN
			);

			auto const diff = *reinterpret_cast<std::ptrdiff_t*>(addr);	// theoretical number 0xFFFAFB35
			auto const e8_target = addr + 4 + diff;

			m_pfnBuildNumber = reinterpret_cast<decltype(m_pfnBuildNumber)>(e8_target);
		}

		[[unlikely]]
		if (!m_pfnBuildNumber)
		{
			UTIL_Terminate("Engine is unsupported.");
		}

		BUILD_NUMBER = m_pfnBuildNumber();
	}

	constexpr int32_t LocalBuildNumber(std::string_view COMPILE_DATE = __DATE__, int32_t bias = 0) noexcept
	{
		constexpr std::string_view mon[12] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		constexpr uint8_t mond[12] =
		{ 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };

		// #UPDATE_AT_CPP23 P2647R1
		const auto today_m = COMPILE_DATE.substr(0, 3);
		const auto today_d = (COMPILE_DATE[5] - '0') + (COMPILE_DATE[4] != ' ' ? (COMPILE_DATE[4] - '0') : 0) * 10;
		const auto today_y = (COMPILE_DATE[10] - '0') + (COMPILE_DATE[9] - '0') * 10 + (COMPILE_DATE[8] - '0') * 100 + (COMPILE_DATE[7] - '0') * 1000;

		const auto this_leap = std::chrono::year{ today_y }.is_leap();

		const auto m = std::ranges::find(mon, today_m) - std::ranges::begin(mon) + 1;	// "Jan" at index 0
		const auto d = std::ranges::fold_left(mond | std::views::take(m - 1), today_d - (this_leap ? 0 : 1), std::plus<>{});
		const auto y = today_y - 1900;

		return
			d
			// the rounding down is actually dropping the years before next leap.
			// hence the averaged 0.25 wont affect the accurate value.
			// Gregorian additional rule wont take effect in the next 100 years.
			// Let's adjust the algorithm then.
			+ static_cast<decltype(d)>((y - 1) * 365.25)
			- bias;
	}

	//m_nBuildNumber -= 34995; // Oct 24 1996 (Quake)
	//m_nBuildNumber -= 35739;  // Nov 7 1998 (HL1 Gold Date)
	//m_nBuildNumber -= 37679;	// Mar 01 2004 (Condition Zero)

	inline constexpr auto BUILD_NUMBER_LOCAL = LocalBuildNumber(__DATE__, 37679);
};
