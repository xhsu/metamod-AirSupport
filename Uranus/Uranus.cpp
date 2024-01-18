
import <cassert>;

import UtlHook;

import Uranus;

inline constexpr unsigned char SET_ANIMATION_FN_NEW_PATTERN[] = "\x90\x83\xEC\x4C\x53\x55\x8B\x2A\x56\x57\x8B\x4D\x04\x8B\x2A\x2A\x2A\x2A\x2A\x85\xC0";
inline constexpr unsigned char SET_ANIMATION_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x54\xA1\x2A\x2A\x2A\x2A\x33\xC4\x89\x44\x24\x50";

inline constexpr unsigned char FIRE_BULLETS_3_FN_NEW_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x00\x01\x00\x00\x53\x55\x89\x44\x24\x0C\xA1\x2A\x2A\x2A\x2A\x56\x57\x8B\xF9\x8B\x48\x40\x8B\x50\x44";
inline constexpr unsigned char FIRE_BULLETS_3_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xEC\x74\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B";


void FindUranusFunctions() noexcept
{
	static bool s_bInitialized = false;

	[[likely]]
	if (s_bInitialized)
		return;

	gClassFunctions.pfnSetAnimation = (fnSetAnimation_t)UTIL_SearchPattern("mp.dll", 1, SET_ANIMATION_FN_ANNIV_PATTERN, SET_ANIMATION_FN_NEW_PATTERN);
	gClassFunctions.pfnFireBullets3 = (fnFireBullets3_t)UTIL_SearchPattern("mp.dll", 1, FIRE_BULLETS_3_FN_ANNIV_PATTERN, FIRE_BULLETS_3_FN_NEW_PATTERN);

	assert(
		gClassFunctions.pfnSetAnimation
		&& gClassFunctions.pfnFireBullets3
	);

	s_bInitialized = true;
}

// Export!
bool __cdecl E_DistributeFn(uranus_func_t* pOutput, size_t iOutputSize) noexcept
{
	FindUranusFunctions();

	std::memcpy(pOutput, &gClassFunctions, iOutputSize);

	assert(iOutputSize == sizeof(uranus_func_t));
	return true;
}
static_assert(std::is_same_v<decltype(&E_DistributeFn), fnUranusAPI_t>);
