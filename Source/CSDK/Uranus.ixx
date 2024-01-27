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
void UTIL_SearchPatterns(bool bDoNoFoundCheck = true) noexcept
{
	(UTIL_SearchPattern<Tys>(), ...);

	if (!bDoNoFoundCheck)
		return;

	std::array<const char*, sizeof...(Tys)> const InvalidFunctionNames{ (Tys::pfn ? nullptr : Tys::NAME)... };
	std::array<const char*, sizeof...(Tys)> const InvalidFunctionSources{ (Tys::pfn ? nullptr : Tys::MODULE)... };

#ifdef _DEBUG
	assert((... && (Tys::pfn != nullptr)));
#else
	[[unlikely]]
	if ((... || (Tys::pfn == nullptr)))
	{
		std::string szReport{ "Function: \"" };	// #UPDATE_AT_CPP23 fmt::join

		for (auto&& [pszModule, pszName] : std::views::zip(InvalidFunctionSources, InvalidFunctionNames))
		{
			if (!pszModule || !pszName)
				continue;

			szReport += std::format("{}{}\", \"", pszModule, pszName);
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
	inline void RetrieveUranusLocal(bool bDoNoFoundCheck = true) noexcept
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
		>(bDoNoFoundCheck);

		UTIL_SearchPatterns<
			HW::Sys_Error,
			HW::SZ_GetSpace
		>(bDoNoFoundCheck);
	}
}
