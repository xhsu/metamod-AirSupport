export module Hook;

export import <cstdint>;

export import <array>;

export import CBase;
export import ConsoleVar;
export import GameRules;
export import Message;
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
export inline constexpr unsigned char RADIUS_FLASH_FN_NEW_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\xD9\x84\x24\xE4\x00\x00\x00\xD8\x0D\x2A\x2A\x2A\x2A\x8D";
export inline constexpr unsigned char RADIUS_FLASH_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xE4\xF8\x81\xEC\x2A\x2A\x2A\x2A\xF3\x0F\x10\x45\x2A\x8D\x45\x08";
export inline constexpr unsigned char SELECT_ITEM_FN_NEW_PATTERN[] = "\x90\x8B\x44\x24\x04\x83\xEC\x14\x85\xC0\x53\x55\x56\x57\x8B\xE9\x0F\x84\x2A\x2A\x2A\x2A\x33";
export inline constexpr unsigned char SELECT_ITEM_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xEC\x08\x53\x8B\x5D\x08\x57\x8B\xF9\x89\x7D\xF8\x85\xDB";
export inline constexpr unsigned char SWITCH_WEAPON_FN_NEW_PATTERN[] = "\x90\x83\xEC\x0C\x56\x57\x8B\x7C\x24\x18\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90\xF8\x00\x00\x00";
export inline constexpr unsigned char SWITCH_WEAPON_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x56\x57\x8B\x7D\x08\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90";
export inline constexpr unsigned char FIRE_BULLETS_FN_NEW_PATTERN[] = "\x90\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x53\x55\x8B\xE9\x8B\x48\x40\x8B\x50\x44\x89\x8C\x24\x8C\x00\x00\x00\x8B\x48\x48\x89\x94\x24\x90\x00\x00\x00";
export inline constexpr unsigned char FIRE_BULLETS_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xEC\x78\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B";

export using fnItemAddToPlayer_t = int(__thiscall *)(CBasePlayerItem *, CBasePlayer *) noexcept;
export using fnItemDeploy_t = int(__thiscall *)(CBasePlayerItem *) noexcept;
export using fnItemPostFrame_t = void(__thiscall *)(CBasePlayerItem *) noexcept;
export using fnItemHolster_t = void(__thiscall *)(CBasePlayerItem *, int) noexcept;
export using fnRadiusFlash_t = void (*)(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage) noexcept;
export using fnSelectItem_t = void(__thiscall *)(CBasePlayer *pThis, const char *pszItemName) noexcept;
export using fnSwitchWeapon_t = qboolean(__thiscall *)(CBasePlayer *pThis, CBasePlayerItem *pWeapon) noexcept;
export using fnCleanUpMap_t = void(__thiscall *)(CHalfLifeMultiplay *) noexcept;
export using fnFireBullets_t = void(__fastcall *)(CBaseEntity *pThis, int, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) noexcept;

export inline fnItemAddToPlayer_t g_pfnItemAddToPlayer = nullptr;
export inline fnItemDeploy_t g_pfnItemDeploy = nullptr;
export inline fnItemPostFrame_t g_pfnItemPostFrame = nullptr;
export inline fnItemHolster_t g_pfnItemHolster = nullptr;
export inline fnRadiusFlash_t g_pfnRadiusFlash = nullptr;
export inline fnSelectItem_t g_pfnSelectItem = nullptr;
export inline fnSwitchWeapon_t g_pfnSwitchWeapon = nullptr;
export inline fnCleanUpMap_t g_pfnCleanUpMap = nullptr;
export inline fnFireBullets_t g_pfnFireBullets = nullptr;

export inline cvar_t *gcvarMaxSpeed = nullptr;
export inline cvar_t *gcvarMaxVelocity = nullptr;
export inline cvar_t *gcvarFriendlyFire = nullptr;

export namespace CVar
{
#define DECLARE_CVAR(name, val, ...)	inline console_variable_t name{ "airsupport_" #name, val, __VA_ARGS__ }

	DECLARE_CVAR(ct_think, "12", u8"[0, ∞)", u8"Interval between calling attempt from CT BOTs.\n\t0 to turn it off.\n\tUnit: Seconds");
	DECLARE_CVAR(ter_think, "0", u8"[0, ∞)", u8"Interval between calling attempt from TER BOTs.\n\t0 to turn it off.\n\tUnit: Seconds");

	inline const std::array player_cd
	{
		console_variable_t{ "airsupport_pas_player_cd", "6", u8"[0, ∞)", "Interval between player called Precise air strike.\n\tAir support resource is shared amongst the team.\n\tUnit: Seconds", },
		console_variable_t{ "airsupport_cc_player_cd", "6", u8"[0, ∞)", "Interval between player called Cluster bomb.\n\tAir support resource is shared amongst the team.\n\tUnit: Seconds", },
		console_variable_t{ "airsupport_cb_player_cd", "6", u8"[0, ∞)", "Interval between player called Carpet bombardment.\n\tAir support resource is shared amongst the team.\n\tUnit: Seconds", },
		console_variable_t{ "airsupport_gs_player_cd", "6", u8"[0, ∞)", "Interval between player called Gunship strike.\n\tAir support resource is shared amongst the team.\n\tUnit: Seconds", },
		console_variable_t{ "airsupport_fab_player_cd", "6", u8"[0, ∞)", "Interval between player called Thermobaric weapon.\n\tAir support resource is shared amongst the team.\n\tUnit: Seconds", },
		console_variable_t{ "airsupport_pim_player_cd", "6", u8"[0, ∞)", "Interval between player called White phosphorus bomb.\n\tAir support resource is shared amongst the team.\n\tUnit: Seconds", },
	};

	DECLARE_CVAR(targeting_fx,			"9",	u8"[0, 11]",	u8"Target miniature angles transiting FX.\n\tEach value represents a different FX, one might wants to try all of them one by one.");
	DECLARE_CVAR(targeting_time,		"0.2",	u8"[0, ∞)",		u8"Target miniature angles transiting time.\n\t0 to turn all FX off.\n\tUnit: Seconds");
	DECLARE_CVAR(target_render_fx,		"15",	u8"[0, 20]",	u8"The FX of static target miniature.\n\tReference to 'enum kRenderFx' from 'const.h' at HLSDK.\n\tThe default value is 'kRenderFxDistort'(15).");	// kRenderFxDistort
	DECLARE_CVAR(target_illumination,	"1",	u8"0 or 1",		u8"Should the static target miniature illuminate?");

	DECLARE_CVAR(cloud_dmg_use_percentage, "1",	u8"0 or 1",		u8"The toxic cloud damage would be considered as a percentage of CURRENT health of victims.\n\tAffecting: 'fab_toxic_dmg' and 'pim_toxic_dmg'");

	DECLARE_CVAR(pas_proj_speed,	"1000",	u8"[0, ∞)", u8"Flying speed of Precise air strike projectile.\n\tUnit: Inches per second");
	DECLARE_CVAR(pas_dmg_impact,	"500",	u8"[0, ∞)", u8"Impact damage of Precise air strike projectile.");
	DECLARE_CVAR(pas_dmg_explo,		"275",	u8"[0, ∞)", u8"Explosion damage of Precise air strike.");
	DECLARE_CVAR(pas_dmg_radius,	"350",	u8"[0, ∞)", u8"Explosion radius of Precise air strike.\n\tUnit: Inches");
	DECLARE_CVAR(pas_fx_radius,		"700",	u8"[0, ∞)", u8"Screen effect radius of Precise air strike.\n\tUnit: Inches");
	DECLARE_CVAR(pas_fx_punch,		"12",	u8"[0, ∞)", u8"Maximum value of Precise air strike screen punching effect.\n\tUnit: Degree");
	DECLARE_CVAR(pas_fx_knock,		"2048",	u8"[0, ∞)", u8"Maximum value of Precise air strike knocking force.\n\tUnit: Inches per second");

	DECLARE_CVAR(cc_bomb_dmg,			"100", u8"[0, ∞)",		u8"Cluster bomb initial exploding damage.");
	DECLARE_CVAR(cc_bomb_radius,		"180", u8"[0, ∞)",		u8"Cluster bomb initial exploding radius.\n\tUnit: Inches");
	DECLARE_CVAR(cc_bomb_fx_radius,		"210", u8"[0, ∞)",		u8"Cluster bomb initial exploding screen FX radius.\n\tUnit: Inches");
	DECLARE_CVAR(cc_charge_dmg,			"128", u8"[0, ∞)",		u8"Cluster bomblets exploding damage.");
	DECLARE_CVAR(cc_charge_radius,		"180", u8"[0, ∞)",		u8"Cluster bomblets exploding radius.\n\tUnit: Inches");
	DECLARE_CVAR(cc_charge_fx_radius,	"210", u8"[0, ∞)",		u8"Cluster bomblets exploding screen FX radius.\n\tUnit: Inches");
	DECLARE_CVAR(cc_charge_min_fuse,	"0.8", u8"[0.1, ∞)",	u8"Cluster bomblets minimal fuze time.\n\tUnit: Seconds");	// these are only for type 2.
	DECLARE_CVAR(cc_charge_max_fuse,	"3.3", u8"[MIN + INC * 2, ∞)", u8"Cluster bomblets maximal fuze time.\n\tUnit: Seconds");
	DECLARE_CVAR(cc_charge_fuse_inc,	"0.1", u8"[0.05, ∞)",	u8"Cluster bomblets fuze time increment.\n\tUnit: Seconds");

	DECLARE_CVAR(cb_impact_dmg,	"125", u8"[0, ∞)", u8"Carpet bombardment impacting damage.");
	DECLARE_CVAR(cb_explo_dmg,	"200", u8"[0, ∞)", u8"Carpet bombardment exploding damage.");
	DECLARE_CVAR(cb_radius,		"250", u8"[0, ∞)", u8"Carpet bombardment exploding radius.\n\tUnit: Inches");
	DECLARE_CVAR(cb_fx_radius,	"400", u8"[0, ∞)", u8"Carpet bombardment exploding screen FX radius.\n\tUnit: Inches");

	DECLARE_CVAR(gs_radius,		"500",	u8"[0, ∞)", u8"Gunship beacon searching radius.\n\tUnit: Inches");
	DECLARE_CVAR(gs_beacon_fx,	"1",	u8"0 or 1", u8"Gunship beacon searching effect.");
	DECLARE_CVAR(gs_holding,	"25",	u8"[0, ∞)", u8"Time before gunship leaving the map.\n\tUnit: Seconds");
	DECLARE_CVAR(gs_dmg,		"90",	u8"[0, ∞)", u8"Gunship bullets' base damage.");

	DECLARE_CVAR(fab_burning_dmg_mul,	"1",	u8"[0, ∞)", u8"Fuel-air explosive damage multiplier.\n\tThe base damage of the explosion is determined by sufficiency of the aerosol-air mixing.");
	DECLARE_CVAR(fab_cloud_max,			"50",	u8"[0, ∞)", u8"The maximum amount of cloud entities in thermobaric weapon releasing phase.");
	DECLARE_CVAR(fab_impact_dmg,		"180",	u8"[0, ∞)", u8"The direct impact damage of vacuum bomb.");
	DECLARE_CVAR(fab_toxic_dmg,			"5",	u8"[0, ∞)", u8"The toxic damage caused by aerosol.");
	DECLARE_CVAR(fab_toxic_inv,			"2",	u8"[0.6, ∞)", u8"The toxic damaging interval of aerosol.\n\tUnit: Seconds");

	DECLARE_CVAR(pim_count,				"16",	u8"[0, ∞)", u8"Amount of the phosphorus burning entities.");
	DECLARE_CVAR(pim_touch_burning_inv,	"0.1",	u8"[0, ∞)", u8"The white phosphorus post-contact burning damage interval.\n\tUnit: Seconds");
	DECLARE_CVAR(pim_touch_burning_dmg,	"10",	u8"[0, ∞)", u8"The white phosphorus post-contact damage amount.\n\tWith randomness range of [val-5, val+5].");
	DECLARE_CVAR(pim_phos_burning_time,	"35",	u8"[0, ∞)", u8"The white phosphorus burning total time.\n\tUnit: Seconds");
	DECLARE_CVAR(pim_perm_burning_inv,	"0.2",	u8"[0, ∞)", u8"The white phosphorus primary-contact burning damage interval.\n\tUnit: Seconds");
	DECLARE_CVAR(pim_perm_burning_dmg,	"10",	u8"[0, ∞)", u8"The white phosphorus primary-contact burning damage amount.\n\tThe damage is determined by: max(dmg, health * dmg/100)\n\t\twhere 'health' is the health of victim.");
	DECLARE_CVAR(pim_toxic_dmg,			"4",	u8"[0, ∞)", u8"The toxic damage caused by phosphorus pentoxide emitted during combustion.");
	DECLARE_CVAR(pim_toxic_inv,			"2",	u8"[0.6, ∞)", u8"The toxic damaging interval of phosphorus pentoxide emitted during combustion.\n\tUnit: Seconds");

#undef DECLARE_CVAR
}

extern "C++" void __fastcall OrpheuF_FireBullets(CBaseEntity * pThis, int, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t * pevAttacker) noexcept;
extern "C++" Vector* __fastcall OrpheuF_FireBullets3(CBaseEntity * pThis, void* edx, Vector * pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t * pevAttacker, qboolean bPistol, int shared_rand) noexcept;
extern "C++" void __cdecl OrpheuF_W_Precache() noexcept;

export namespace HookInfo
{
	inline FunctionHook FireBullets{ &OrpheuF_FireBullets };
	inline FunctionHook FireBullets3{ &OrpheuF_FireBullets3 };
	inline FunctionHook W_Precache{ &OrpheuF_W_Precache };
};
