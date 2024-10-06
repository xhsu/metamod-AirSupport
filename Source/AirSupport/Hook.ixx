export module Hook;

export import std;

export import CBase;
export import GameRules;
export import Uranus;
export import VTFH;

export import UtlHook;

export inline constexpr size_t VFTIDX_ITEM_ADDTOPLAYER = 59;
export inline constexpr size_t VFTIDX_ITEM_DEPLOY = 64;
export inline constexpr size_t VFTIDX_ITEM_POSTFRAME = 70;
export inline constexpr size_t VFTIDX_WEAPON_PRIMARYATTACK = 87;
export inline constexpr size_t VFTIDX_WEAPON_SECONDARYATTACK = 88;
export inline constexpr size_t VFTIDX_ITEM_CANHOLSTER = 66;
export inline constexpr size_t VFTIDX_ITEM_HOLSTER = 67;
export inline constexpr size_t VFTIDX_CHalfLifeMultiplay_CleanUpMap = 63;

export using fnItemAddToPlayer_t = int(__thiscall *)(CBasePlayerItem *, CBasePlayer *) noexcept;
export using fnItemDeploy_t = int(__thiscall *)(CBasePlayerItem *) noexcept;
export using fnItemPostFrame_t = void(__thiscall *)(CBasePlayerItem *) noexcept;
export using fnItemHolster_t = void(__thiscall *)(CBasePlayerItem *, int) noexcept;
export using fnCleanUpMap_t = void(__thiscall *)(CHalfLifeMultiplay *) noexcept;

export inline fnItemAddToPlayer_t g_pfnItemAddToPlayer = nullptr;
export inline fnItemDeploy_t g_pfnItemDeploy = nullptr;
export inline fnItemPostFrame_t g_pfnItemPostFrame = nullptr;
export inline fnItemHolster_t g_pfnItemHolster = nullptr;
export inline fnCleanUpMap_t g_pfnCleanUpMap = nullptr;

extern "C++" void __fastcall OrpheuF_FireBullets(CBaseEntity * pThis, void*, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, EBulletTypes iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker) noexcept;
extern "C++" Vector* __fastcall OrpheuF_FireBullets3(CBaseEntity * pThis, void* edx, Vector * pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, qboolean bPistol, int shared_rand) noexcept;
extern "C++" void __cdecl OrpheuF_W_Precache() noexcept;
extern "C++" PFN_ENTITYINIT __cdecl OrpheuF_GetDispatch(char const* pszClassName) noexcept;

export namespace HookInfo
{
	inline FunctionHook FireBullets{ &OrpheuF_FireBullets };
	inline FunctionHook FireBullets3{ &OrpheuF_FireBullets3 };
	inline FunctionHook W_Precache{ &OrpheuF_W_Precache };
	inline FunctionHook GetDispatch{ &OrpheuF_GetDispatch };
};
