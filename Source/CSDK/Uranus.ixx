export module Uranus;

import <cassert>;

import <array>;
import <format>;
import <ranges>;

import CBase;
import Platform;

import UtlHook;

export import :Functions;


template <typename... Tys> __forceinline
void UTIL_SearchPatterns(void) noexcept
{
	(UTIL_SearchPattern<Tys>(), ...);

	std::array<const char*, sizeof...(Tys)> const InvalidFunctions{ (Tys::pfn ? nullptr : Tys::NAME)... };

#ifdef _DEBUG
	assert((... && (Tys::pfn != nullptr)));
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

export namespace Uranus
{
	// Should be call as early as possible.
	// put it in GiveFnptrsToDll() for 100% confidence.
	inline void RetrieveUranusLocal() noexcept
	{
		using namespace Uranus;

		UTIL_SearchPatterns<
			EmptyEntityHashTable, AddEntityHashValue, RemoveEntityHashValue,
			ClearMultiDamage, ApplyMultiDamage, AddMultiDamage,
			TEXTURETYPE_PlaySound,
			W_Precache, UTIL_PrecacheOther, UTIL_PrecacheOtherWeapon,
			BaseEntity::Create,
			BaseEntity::FireBullets3,
			BaseDelay::SUB_UseTargets,
			BaseWeapon::DefaultDeploy,
			BasePlayer::SetAnimation
		>();

		UTIL_SearchPatterns<
			HW::Sys_Error,
			HW::SZ_GetSpace
		>();
	}
}