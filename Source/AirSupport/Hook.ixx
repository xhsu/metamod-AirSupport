#define USE_CHT_VER

export module Hook;

export import std;

export import CBase;
export import ConsoleVar;
export import GameRules;
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

#ifndef USE_CHT_VER
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
	DECLARE_CVAR(target_illumination,	"1",	u8"0 or 1",		u8"Should the static target miniature illuminate?\n\tThis is only visible for the caller's team.");

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
	DECLARE_CVAR(cc_charge_min_fuze,	"0.8", u8"[0.1, ∞)",	u8"Cluster bomblets minimal fuze time.\n\tUnit: Seconds");	// these are only for type 2.
	DECLARE_CVAR(cc_charge_max_fuze,	"3.3", u8"[MIN + INC * 2, ∞)", u8"Cluster bomblets maximal fuze time.\n\tThe amount of bomblets can be calculated by the following formula:\n\t(max-min)/inc\n\tUnit: Seconds");
	DECLARE_CVAR(cc_charge_fuze_inc,	"0.1", u8"[0.05, ∞)",	u8"Cluster bomblets fuze time increment.\n\tUnit: Seconds");

	DECLARE_CVAR(cb_impact_dmg,	"125", u8"[0, ∞)", u8"Carpet bombardment impacting damage.");
	DECLARE_CVAR(cb_explo_dmg,	"200", u8"[0, ∞)", u8"Carpet bombardment exploding damage.");
	DECLARE_CVAR(cb_radius,		"250", u8"[0, ∞)", u8"Carpet bombardment exploding radius.\n\tUnit: Inches");
	DECLARE_CVAR(cb_fx_radius,	"400", u8"[0, ∞)", u8"Carpet bombardment exploding screen FX radius.\n\tUnit: Inches");

	DECLARE_CVAR(gs_radius,		"500",	u8"[0, ∞)", u8"Gunship beacon searching radius.\n\tUnit: Inches");
	DECLARE_CVAR(gs_beacon_fx,	"1",	u8"0 or 1", u8"Gunship beacon searching effect.");
	DECLARE_CVAR(gs_holding,	"25",	u8"[0, ∞)", u8"Time before gunship leaving the map.\n\tUnit: Seconds");
	DECLARE_CVAR(gs_dmg,		"90",	u8"[0, ∞)", u8"Gunship bullets' base damage.");
	DECLARE_CVAR(gs_rpm,		"300",	u8"[1, ∞)", u8"Gunship firerate.\n\tUnit: RPM");

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

#else

	DECLARE_CVAR(ct_think, "12", u8"[0, ∞)", u8"反恐菁英BOT安排空襲的間隔。\n\t置0以禁用反恐菁英空襲。\n\t單位：秒");
	DECLARE_CVAR(ter_think, "0", u8"[0, ∞)", u8"恐怖分子BOT安排空襲的間隔。\n\t置0以禁用恐怖分子空襲。\n\t單位：秒");

	inline const std::array player_cd
	{
		console_variable_t{ "airsupport_pas_player_cd", "6", u8"[0, ∞)", u8"【精確導彈打擊】玩家呼叫後的冷卻時間。\n\t空襲資源由全隊共享。\n\t單位：秒", },
		console_variable_t{ "airsupport_cc_player_cd", "6", u8"[0, ∞)", u8"【集束炸藥】玩家呼叫後的冷卻時間。\n\t空襲資源由全隊共享。\n\t單位：秒", },
		console_variable_t{ "airsupport_cb_player_cd", "6", u8"[0, ∞)", u8"【地毯式轟炸】玩家呼叫後的冷卻時間。\n\t空襲資源由全隊共享。\n\t單位：秒", },
		console_variable_t{ "airsupport_gs_player_cd", "6", u8"[0, ∞)", u8"【空中炮艦支援】玩家呼叫後的冷卻時間。\n\t空襲資源由全隊共享。\n\t單位：秒", },
		console_variable_t{ "airsupport_fab_player_cd", "6", u8"[0, ∞)", u8"【雲爆彈】玩家呼叫後的冷卻時間。\n\t空襲資源由全隊共享。\n\t單位：秒", },
		console_variable_t{ "airsupport_pim_player_cd", "6", u8"[0, ∞)", u8"【白磷彈】玩家呼叫後的冷卻時間。\n\t空襲資源由全隊共享。\n\t單位：秒", },
	};

	DECLARE_CVAR(targeting_fx,			"9",	u8"[0, 11]",	u8"標靶指示器之角度變換插值模式。\n\t每個數字代表一類插值模式，你可以逐一嘗試。");
	DECLARE_CVAR(targeting_time,		"0.2",	u8"[0, ∞)",		u8"標靶指示器之角度變換時長。\n\t置0以禁用插值。\n\t單位：秒");
	DECLARE_CVAR(target_render_fx,		"15",	u8"[0, 20]",	u8"標靶指示器指定目標後的簡易渲染特效。\n\t請參閱HLSDK中'const.h'頭文件之'enum kRenderFx'一節。\n\t默認值為'kRenderFxDistort'(15)");	// kRenderFxDistort
	DECLARE_CVAR(target_illumination,	"1",	u8"0 or 1",		u8"標靶指示器指定目標後是否發光？\n\t該光線僅空襲呼叫方可見。");

	DECLARE_CVAR(cloud_dmg_use_percentage, "1",	u8"0 or 1",		u8"是否以百分比扣除受毒氣傷害者的當前生命值。\n\t若置0，則下列CVAR將被視作傷害值而非百分比。\n\t'fab_toxic_dmg', 'pim_toxic_dmg'");

	DECLARE_CVAR(pas_proj_speed,	"1000",	u8"[0, ∞)", u8"【精確導彈打擊】導彈飛行速度。\n\t單位：英吋每秒");
	DECLARE_CVAR(pas_dmg_impact,	"500",	u8"[0, ∞)", u8"【精確導彈打擊】導彈撞擊傷害。");
	DECLARE_CVAR(pas_dmg_explo,		"275",	u8"[0, ∞)", u8"【精確導彈打擊】爆炸傷害。");
	DECLARE_CVAR(pas_dmg_radius,	"350",	u8"[0, ∞)", u8"【精確導彈打擊】爆炸半徑。\n\t單位：英吋");
	DECLARE_CVAR(pas_fx_radius,		"700",	u8"[0, ∞)", u8"【精確導彈打擊】屏幕特效：受影響半徑。\n\t單位：英吋");
	DECLARE_CVAR(pas_fx_punch,		"12",	u8"[0, ∞)", u8"【精確導彈打擊】屏幕特效：傾斜最大角度\n\t單位：角度");
	DECLARE_CVAR(pas_fx_knock,		"2048",	u8"[0, ∞)", u8"【精確導彈打擊】被爆炸擊飛之最大速度。\n\t單位：英吋每秒");

	DECLARE_CVAR(cc_bomb_dmg,			"100", u8"[0, ∞)",		u8"【集束炸藥】母彈爆炸傷害。");
	DECLARE_CVAR(cc_bomb_radius,		"180", u8"[0, ∞)",		u8"【集束炸藥】母彈爆炸半徑。\n\t單位：英吋");
	DECLARE_CVAR(cc_bomb_fx_radius,		"210", u8"[0, ∞)",		u8"【集束炸藥】屏幕特效：母彈爆炸時受影響半徑。\n\t單位：英吋");
	DECLARE_CVAR(cc_charge_dmg,			"128", u8"[0, ∞)",		u8"【集束炸藥】霰射炸藥爆炸傷害。");
	DECLARE_CVAR(cc_charge_radius,		"180", u8"[0, ∞)",		u8"【集束炸藥】霰射炸藥爆炸半徑。\n\t單位：英吋");
	DECLARE_CVAR(cc_charge_fx_radius,	"210", u8"[0, ∞)",		u8"【集束炸藥】屏幕特效：霰射炸藥爆炸時受影響半徑。\n\t單位：英吋");
	DECLARE_CVAR(cc_charge_min_fuze,	"0.8", u8"[0.1, ∞)",	u8"【集束炸藥】霰射炸藥最短引信時間。\n\t單位：秒");	// these are only for type 2.
	DECLARE_CVAR(cc_charge_max_fuze,	"3.3", u8"[MIN + INC * 2, ∞)", u8"【集束炸藥】霰射炸藥最長引信時間。\n\t霰射炸藥數量無法單獨調整，但可由下列公式估算：\n\t(max-min)/inc\n\t以默認值為例：(3.3-0.8)/0.1==25\n\t單位：秒");
	DECLARE_CVAR(cc_charge_fuze_inc,	"0.1", u8"[0.05, ∞)",	u8"【集束炸藥】霰射炸藥引信長度遞增。\n\t單位：秒");

	DECLARE_CVAR(cb_impact_dmg,	"125", u8"[0, ∞)", u8"【地毯式轟炸】撞擊傷害。");
	DECLARE_CVAR(cb_explo_dmg,	"200", u8"[0, ∞)", u8"【地毯式轟炸】爆炸傷害。");
	DECLARE_CVAR(cb_radius,		"250", u8"[0, ∞)", u8"【地毯式轟炸】爆炸半徑。\n\t單位：英吋");
	DECLARE_CVAR(cb_fx_radius,	"400", u8"[0, ∞)", u8"【地毯式轟炸】屏幕特效：受影響半徑。\n\t單位：英吋");

	DECLARE_CVAR(gs_radius,		"500",	u8"[0, ∞)", u8"【空中炮艦支援】信標索敵半徑。\n\t單位：英吋");
	DECLARE_CVAR(gs_beacon_fx,	"1",	u8"0 or 1", u8"【空中炮艦支援】啟用信標索敵效果。");
	DECLARE_CVAR(gs_holding,	"25",	u8"[0, ∞)", u8"【空中炮艦支援】炮艦停留時間。\n\t單位：秒");
	DECLARE_CVAR(gs_dmg,		"90",	u8"[0, ∞)", u8"【空中炮艦支援】彈藥基礎傷害。");
	DECLARE_CVAR(gs_rpm,		"300",	u8"[1, ∞)", u8"【空中炮艦支援】射速。\n\t單位：RPM(發/每分鐘)");

	DECLARE_CVAR(fab_burning_dmg_mul,	"1",	u8"[0, ∞)", u8"【雲爆彈】燃燒傷害乘數。\n\t基準傷害由氣溶膠與空氣混合程度決定，無法調節。");
	DECLARE_CVAR(fab_cloud_max,			"50",	u8"[0, ∞)", u8"【雲爆彈】氣溶膠實體最大數量。");
	DECLARE_CVAR(fab_impact_dmg,		"180",	u8"[0, ∞)", u8"【雲爆彈】撞擊傷害。");
	DECLARE_CVAR(fab_toxic_dmg,			"5",	u8"[0, ∞)", u8"【雲爆彈】氣溶膠毒性傷害。");
	DECLARE_CVAR(fab_toxic_inv,			"2",	u8"[0.6, ∞)", u8"【雲爆彈】氣溶膠毒性傷害間隔。\n\t單位：秒");

	DECLARE_CVAR(pim_count,				"16",	u8"[0, ∞)", u8"【白磷彈】霰射後白磷實體數量。");
	DECLARE_CVAR(pim_touch_burning_inv,	"0.1",	u8"[0, ∞)", u8"【白磷彈】次要接觸傷害間隔。\n\t單位：秒");
	DECLARE_CVAR(pim_touch_burning_dmg,	"10",	u8"[0, ∞)", u8"【白磷彈】次要接觸傷害。\n\t傷害將在[val-5, val+5]範圍內隨機。");
	DECLARE_CVAR(pim_phos_burning_time,	"35",	u8"[0, ∞)", u8"【白磷彈】彈藥燃燒時間。\n\t單位：秒");
	DECLARE_CVAR(pim_perm_burning_inv,	"0.2",	u8"[0, ∞)", u8"【白磷彈】覆蓋性燃燒傷害間隔。\n\t單位：秒");
	DECLARE_CVAR(pim_perm_burning_dmg,	"10",	u8"[0, ∞)", u8"【白磷彈】覆蓋性燃燒傷害。\n\t傷害公式：max(dmg, health * dmg/100)\n\t\t其中：'health'為受害者當前生命值。");
	DECLARE_CVAR(pim_toxic_dmg,			"4",	u8"[0, ∞)", u8"【白磷彈】五氧化二磷白煙的毒性傷害。");
	DECLARE_CVAR(pim_toxic_inv,			"2",	u8"[0.6, ∞)", u8"【白磷彈】五氧化二磷白煙的毒性傷害間隔。\n\t單位：秒");

#endif
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
