export module Uranus;

import <cassert>;

import <array>;
import <format>;
import <ranges>;

import CBase;
import Platform;

import UtlHook;

export import :Functions;

export using fnFireBullets3_t = Vector *(__fastcall *)(CBaseEntity* pThis, void* edx, Vector* pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) noexcept;
export using fnSetAnimation_t = void (__fastcall *)(CBasePlayer* pPlayer, int, PLAYER_ANIM playerAnim) noexcept;

export struct uranus_func_t final
{
	std::uintptr_t m_iVersion = 1;

	fnFireBullets3_t pfnFireBullets3{};
	fnSetAnimation_t pfnSetAnimation{};
};

// Use for consumers.
export inline constexpr char URANUS_API_NAME[] = u8"E_DistributeFn";
export using fnUranusAPI_t = bool (__cdecl *)(uranus_func_t* pOutput, size_t iOutputSize) noexcept;
export inline uranus_func_t gClassFunctions;

export inline
bool RetrieveUranus() noexcept
{
	if (!UTIL_ModulePresence("Uranus.dll"))
		return false;

	if (auto pfn = (fnUranusAPI_t)UTIL_LoadLibraryFunction("Uranus.dll", URANUS_API_NAME); pfn != nullptr)
		return pfn(&gClassFunctions, sizeof(uranus_func_t));

	return false;
}

template <typename... Tys> __forceinline
void UTIL_SearchPatterns(void) noexcept
{
	(UTIL_SearchPattern<Tys>(), ...);

	std::array<const char*, sizeof...(Tys)> const InvalidFunctions{ (Tys::pfn ? nullptr : Tys::NAME)... };

#ifdef _DEBUG
	assert((... && (Tys::pfn != nullptr)))
#else
	[[unlikely]]
	if ((... || (Tys::pfn == nullptr)))
	{
		std::string szReport{ "Function: \"" };	// #UPDATE_AT_CPP23 fmt::join

		for (auto&& pszName :
			InvalidFunctions
			| std::views::filter([](auto p) { return p != nullptr; })
			)
		{
			szReport += std::format("{}\", \"", pszName);
		}

		szReport.erase(szReport.end() - 3);
		szReport += " no found!";

		UTIL_Terminate(szReport.c_str());
	}
#endif
}

// Should be call as early as possible.
// put it in GiveFnptrsToDll() to be 100% confidence.
export inline
void RetrieveUranusLocal() noexcept
{
	using namespace Uranus;

	UTIL_SearchPatterns<
		BaseEntity::FireBullets3,
		BasePlayer::SetAnimation
	>();
}
