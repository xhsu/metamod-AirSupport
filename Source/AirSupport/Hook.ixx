export module Hook;

export import <cstdint>;

export import CBase;
export import GameRules;
export import Message;
export import VTFH;

export inline constexpr size_t VFTIDX_ITEM_ADDTOPLAYER = 59;
export inline constexpr size_t VFTIDX_ITEM_DEPLOY = 64;
export inline constexpr size_t VFTIDX_ITEM_POSTFRAME = 70;
export inline constexpr size_t VFTIDX_WEAPON_PRIMARYATTACK = 87;
export inline constexpr size_t VFTIDX_WEAPON_SECONDARYATTACK = 88;
export inline constexpr size_t VFTIDX_ITEM_CANHOLSTER = 66;
export inline constexpr size_t VFTIDX_ITEM_HOLSTER = 67;
export inline constexpr size_t VFTIDX_CHalfLifeMultiplay_CleanUpMap = 63;
export inline constexpr unsigned char RADIUS_FLASH_FN_NEW_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\xD9\x84\x24\xE4\x00\x00\x00\xD8\x0D\x2A\x2A\x2A\x2A\x8D";
export inline constexpr unsigned char RADIUS_FLASH_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xE4\xF8\x81\xEC\x2A\x2A\x2A\x2A\xF3\x0F\x10\x45\x2A\x8D\x45\x08";
export inline constexpr unsigned char SELECT_ITEM_FN_NEW_PATTERN[] = "\x90\x8B\x44\x24\x04\x83\xEC\x14\x85\xC0\x53\x55\x56\x57\x8B\xE9\x0F\x84\x2A\x2A\x2A\x2A\x33";
export inline constexpr unsigned char SELECT_ITEM_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xEC\x08\x53\x8B\x5D\x08\x57\x8B\xF9\x89\x7D\xF8\x85\xDB";
export inline constexpr unsigned char APPLY_MULTI_DAMAGE_FN_NEW_PATTERN[] = "\x90\x8B\x0D\x2A\x2A\x2A\x2A\x85\xC9\x74\x2A\x8B\x15\x2A\x2A\x2A\x2A\x8B\x01\x52\x8B\x15\x2A\x2A\x2A\x2A\x52\x8B\x54\x24\x10\x52\x8B\x54\x24\x10";
export inline constexpr unsigned char APPLY_MULTI_DAMAGE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x8B\x0D\x2A\x2A\x2A\x2A\x85\xC9\x74\x1F\xFF\x35\x2A\x2A\x2A\x2A\xF3\x0F\x10\x05";
export inline constexpr unsigned char CLEAR_MULTI_DAMAGE_FN_NEW_PATTERN[] = "\x90\x33\xC0\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xC3\x90";
export inline constexpr unsigned char CLEAR_MULTI_DAMAGE_FN_ANNIV_PATTERN[] = "\xC3\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC3\xCC";
export inline constexpr unsigned char ADD_MULTI_DAMAGE_FN_NEW_PATTERN[] = "\x90\x56\x8B\x74\x24\x0C\x85\xF6\x74\x2A\xA1\x2A\x2A\x2A\x2A\x8B\x4C\x24\x14";
export inline constexpr unsigned char ADD_MULTI_DAMAGE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x56\x8B\x75\x0C\x85\xF6\x74\x6A\xA1\x2A\x2A\x2A\x2A\x0B\x45";
export inline constexpr unsigned char DEFAULT_DEPLOY_FN_NEW_PATTERN[] = "\x90\x56\x8B\xF1\x8B\x06\xFF\x90\xF8\x00\x00\x00\x85\xC0\x75\x2A\x5E";
export inline constexpr unsigned char DEFAULT_DEPLOY_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x56\x8B\xF1\x8B\x06\xFF\x90\x2A\x2A\x2A\x2A\x85\xC0";
export inline constexpr unsigned char SWITCH_WEAPON_FN_NEW_PATTERN[] = "\x90\x83\xEC\x0C\x56\x57\x8B\x7C\x24\x18\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90\xF8\x00\x00\x00";
export inline constexpr unsigned char SWITCH_WEAPON_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x56\x57\x8B\x7D\x08\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90";
export inline constexpr unsigned char FIRE_BULLETS_FN_NEW_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x53\x55\x8B\xE9\x8B\x48\x40\x8B\x50\x44\x89\x8C\x24\x8C\x00\x00\x00\x8B\x48\x48\x89\x94\x24\x90\x00\x00\x00";
export inline constexpr unsigned char FIRE_BULLETS_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xEC\x78\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B";
export inline constexpr unsigned char FIRE_BULLETS_3_FN_NEW_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x00\x01\x00\x00\x53\x55\x89\x44\x24\x0C\xA1\x2A\x2A\x2A\x2A\x56\x57\x8B\xF9\x8B\x48\x40\x8B\x50\x44";
export inline constexpr unsigned char FIRE_BULLETS_3_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xEC\x74\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B";

export using fnItemAddToPlayer_t = int(__thiscall *)(CBasePlayerItem *, CBasePlayer *) noexcept;
export using fnItemDeploy_t = int(__thiscall *)(CBasePlayerItem *) noexcept;
export using fnItemPostFrame_t = void(__thiscall *)(CBasePlayerItem *) noexcept;
export using fnItemHolster_t = void(__thiscall *)(CBasePlayerItem *, int) noexcept;
export using fnRadiusFlash_t = void (*)(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage) noexcept;
export using fnSelectItem_t = void(__thiscall *)(CBasePlayer *pThis, const char *pszItemName) noexcept;
export using fnApplyMultiDamage_t = void (*)(entvars_t *pevInflictor, entvars_t *pevAttacker) noexcept;
export using fnClearMultiDamage_t = void (*)(void) noexcept;
export using fnAddMultiDamage_t = void (*)(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType) noexcept;
export using fnDefaultDeploy_t = qboolean(__thiscall *)(CBasePlayerWeapon *pThis, const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, qboolean skiplocal);
export using fnSwitchWeapon_t = qboolean(__thiscall *)(CBasePlayer *pThis, CBasePlayerItem *pWeapon) noexcept;
export using fnCleanUpMap_t = void(__thiscall *)(CHalfLifeMultiplay *) noexcept;
export using fnFireBullets_t = void(__thiscall *)(CBaseEntity *pThis, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) noexcept;
export using fnFireBullets3_t = Vector*(__fastcall *)(CBaseEntity *pThis, void* edx, Vector* pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, qboolean bPistol, int shared_rand) noexcept; // check the comment of its callback for more info.

export inline fnItemAddToPlayer_t g_pfnItemAddToPlayer = nullptr;
export inline fnItemDeploy_t g_pfnItemDeploy = nullptr;
export inline fnItemPostFrame_t g_pfnItemPostFrame = nullptr;
export inline fnItemHolster_t g_pfnItemHolster = nullptr;
export inline fnRadiusFlash_t g_pfnRadiusFlash = nullptr;
export inline fnSelectItem_t g_pfnSelectItem = nullptr;
export inline fnApplyMultiDamage_t g_pfnApplyMultiDamage = nullptr;
export inline fnClearMultiDamage_t g_pfnClearMultiDamage = nullptr;
export inline fnAddMultiDamage_t g_pfnAddMultiDamage = nullptr;
export inline fnDefaultDeploy_t g_pfnDefaultDeploy = nullptr;
export inline fnSwitchWeapon_t g_pfnSwitchWeapon = nullptr;
export inline fnCleanUpMap_t g_pfnCleanUpMap = nullptr;
export inline fnFireBullets_t g_pfnFireBullets = nullptr;
export inline fnFireBullets3_t g_pfnFireBullets3 = nullptr;

export inline cvar_t *gcvarMaxSpeed = nullptr;
export inline cvar_t *gcvarMaxVelocity = nullptr;
export inline cvar_t *gcvarFriendlyFire = nullptr;

export struct FunctionHook_t
{
	unsigned char m_OriginalBytes[5]{};
	unsigned char m_PatchedBytes[5]{};
	void *m_Address{};
};

export namespace HookInfo
{
	inline FunctionHook_t FireBullets{};
	inline FunctionHook_t FireBullets3{};
};
