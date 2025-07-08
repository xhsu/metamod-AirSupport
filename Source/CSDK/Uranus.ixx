module;

#include <cassert>

export module Uranus;

import std;

import CBase;
import Engine;
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
		std::string szReport{ "Function: \"" };	// #UPDATE_AT_CPP26 fmt::join

		for (auto&& [pszModule, pszName] : std::views::zip(InvalidFunctionSources, InvalidFunctionNames))
		{
			if (!pszModule || !pszName)
				continue;

			szReport += std::format("{}{}\", \"", pszModule, pszName);
		}

		szReport.erase(szReport.end() - 4);	// end - 1 is the actual last.
		szReport += " no found!";
		szReport += std::format("\nPlugin Build Number: {} {}", Engine::BUILD_NUMBER_LOCAL, __TIME__);

		UTIL_Terminate(szReport.c_str());
	}
#endif
}

export namespace Uranus
{
	// Should be call as early as possible.
	// put it in GiveFnptrsToDll() for 100% confidence.
	inline void RetrieveUranusLocal(bool bCrashIfNoFound = true) noexcept
	{
		using namespace Uranus;

		// #UPDATE_AT_CPP26 reflection
		UTIL_SearchPatterns<
			EmptyEntityHashTable, AddEntityHashValue, RemoveEntityHashValue,
			ClearMultiDamage, ApplyMultiDamage, AddMultiDamage,
			TEXTURETYPE_PlaySound, UTIL_TextureHit,
			UTIL_SharedRandomFloat,
			W_Precache, UTIL_PrecacheOther, UTIL_PrecacheOtherWeapon, AddAmmoNameToAmmoRegistry, packPlayerItem, WriteSigonMessages,
			CheckStartMoney, BuyGunAmmo, HandleBuyAliasCommands, BuyItem,
			RadiusFlash,
			BaseEntity::Create, BaseEntity::FireBullets, BaseEntity::FireBullets3,
			BaseDelay::SUB_UseTargets,
			BaseWeapon::DefaultDeploy,
			BasePlayer::HintMessage, BasePlayer::SetAnimation, BasePlayer::DropShield, BasePlayer::CanPlayerBuy, BasePlayer::AddAccount, BasePlayer::SelectItem, BasePlayer::SwitchWeapon, BasePlayer::DropPlayerItem
		>(bCrashIfNoFound);

		UTIL_SearchPatterns<
			HW::Sys_Error,
			HW::SZ_GetSpace,
			HW::GetDispatch
		>(bCrashIfNoFound);
	}
}
