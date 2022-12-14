export module Hook;

export import <cstdint>;

export import CBase;
export import GameRules;
export import Message;

export inline constexpr size_t VFTIDX_CBASE_TRACEATTACK = 11;
export inline constexpr size_t VFTIDX_CBASE_TAKEDAMAGE = 12;
export inline constexpr size_t VFTIDX_CBASE_KILLED = 14;
export inline constexpr size_t VFTIDX_CBASE_TRACEBLEED = 16;
export inline constexpr size_t VFTIDX_CBASE_DAMAGEDECAL = 29;
export inline constexpr size_t VFTIDX_CBASE_GETNEXTTARGET = 43;
export inline constexpr size_t VFTIDX_ITEM_ADDTOPLAYER = 59;
export inline constexpr size_t VFTIDX_ITEM_DEPLOY = 64;
export inline constexpr size_t VFTIDX_ITEM_POSTFRAME = 70;
export inline constexpr size_t VFTIDX_WEAPON_PRIMARYATTACK = 87;
export inline constexpr size_t VFTIDX_WEAPON_SECONDARYATTACK = 88;
export inline constexpr size_t VFTIDX_ITEM_CANHOLSTER = 66;
export inline constexpr size_t VFTIDX_ITEM_HOLSTER = 67;
export inline constexpr size_t VFTIDX_CHalfLifeMultiplay_CleanUpMap = 63;
export inline constexpr unsigned char RADIUS_FLASH_FN_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\xD9\x84\x24\xE4\x00\x00\x00\xD8\x0D\x2A\x2A\x2A\x2A\x8D";
export inline constexpr unsigned char SELECT_ITEM_FN_PATTERN[] = "\x90\x8B\x44\x24\x04\x83\xEC\x14\x85\xC0\x53\x55\x56\x57\x8B\xE9\x0F\x84\x2A\x2A\x2A\x2A\x33";
export inline constexpr unsigned char APPLY_MULTI_DAMAGE_FN_PATTERN[] = "\x90\x8B\x0D\x2A\x2A\x2A\x2A\x85\xC9\x74\x2A\x8B\x15\x2A\x2A\x2A\x2A\x8B\x01\x52\x8B\x15\x2A\x2A\x2A\x2A\x52\x8B\x54\x24\x10\x52\x8B\x54\x24\x10";
export inline constexpr unsigned char CLEAR_MULTI_DAMAGE_FN_PATTERN[] = "\x90\x33\xC0\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xC3\x90";
export inline constexpr unsigned char ADD_MULTI_DAMAGE_FN_PATTERN[] = "\x90\x56\x8B\x74\x24\x0C\x85\xF6\x74\x2A\xA1\x2A\x2A\x2A\x2A\x8B\x4C\x24\x14";
export inline constexpr unsigned char DEFAULT_DEPLOY_FN_PATTERN[] = "\x90\x56\x8B\xF1\x8B\x06\xFF\x90\xF8\x00\x00\x00\x85\xC0\x75\x2A\x5E";
export inline constexpr unsigned char SWITCH_WEAPON_FN_PATTERN[] = "\x90\x83\xEC\x0C\x56\x57\x8B\x7C\x24\x18\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90\xF8\x00\x00\x00";
export inline constexpr unsigned char CWORLD_PRECACHE_FN_PATTERN[] = "\x90\x55\x57\x33\xFF\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x8B\xE9\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D";
export inline constexpr unsigned char FIRE_BULLETS_FN_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x53\x55\x8B\xE9\x8B\x48\x40\x8B\x50\x44\x89\x8C\x24\x8C\x00\x00\x00\x8B\x48\x48\x89\x94\x24\x90\x00\x00\x00";
export inline constexpr unsigned char FIRE_BULLETS_3_FN_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x00\x01\x00\x00\x53\x55\x89\x44\x24\x0C\xA1\x2A\x2A\x2A\x2A\x56\x57\x8B\xF9\x8B\x48\x40\x8B\x50\x44";

export using fnEntityTraceAttack_t = void(__thiscall *)(CBaseEntity *, entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) noexcept;
export using fnEntityTakeDamage_t = qboolean(__thiscall *)(CBaseEntity *, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) noexcept;
export using fnEntityKilled_t = void(__thiscall *)(CBaseEntity *, entvars_t *pevAttacker, int iGib) noexcept;
export using fnEntityTraceBleed_t = void(__thiscall *)(CBaseEntity *, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) noexcept;
export using fnEntityDamageDecal_t = int(__thiscall *)(CBaseEntity *, int bitsDamageType) noexcept;
export using fnEntityGetNextTarget_t = CBaseEntity * (__thiscall *)(CBaseEntity *) noexcept;
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
export using fnFireBullets3_t = Vector(__fastcall *)(long argument1, long argument2, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, bool bPistol, int shared_rand) noexcept;	// This one is extremely wierd.

export inline fnEntityTraceAttack_t g_pfnEntityTraceAttack = nullptr;
export inline fnEntityTakeDamage_t g_pfnEntityTakeDamage = nullptr;
export inline fnEntityKilled_t g_pfnEntityKilled = nullptr;
export inline fnEntityTraceBleed_t g_pfnEntityTraceBleed = nullptr;
export inline fnEntityDamageDecal_t g_pfnEntityDamageDecal = nullptr;
export inline fnEntityGetNextTarget_t g_pfnEntityGetNextTarget = nullptr;
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

export using gmsgScreenFade = Message_t<"ScreenFade", uint16_t, uint16_t, uint16_t, byte, byte, byte, byte>;
export using gmsgScreenShake = Message_t<"ScreenShake", uint16_t, uint16_t, uint16_t>;
export using gmsgBarTime = Message_t<"BarTime", int16_t>;
export using gmsgWeaponAnim = Message_t<"WeapAnim", byte, byte>;	// actually no such message exist. pure wrapper.
export using gmsgWeaponList = Message_t<"WeaponList", const char*, byte, byte, byte, byte, byte, byte, byte, byte>;
export using gmsgWeapPickup = Message_t<"WeapPickup", byte>;
export using gmsgTextMsg = Message_t<"TextMsg", byte, const char *>;	// 4 args more actually, but whatever.
export using gmsgShowMenu = Message_t<"ShowMenu", uint16_t, int8_t, byte, const char*>;

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
