export module Menu;

export import <array>;
export import <string>;

export import Hook;
export import Plugin;

export enum EAirSupportTypes
{
	AIR_STRIKE = 0,
	CLUSTER_BOMB,
	CARPET_BOMBARDMENT,
	GUNSHIP_STRIKE,
	FUEL_AIR_BOMB,	// thermobaric weapon
	PHOSPHORUS_MUNITION,
};

export inline std::array<EAirSupportTypes, 33> g_rgiAirSupportSelected = {};

export inline constexpr auto MENU_KEY_1 = (1 << 0);
export inline constexpr auto MENU_KEY_2 = (1 << 1);
export inline constexpr auto MENU_KEY_3 = (1 << 2);
export inline constexpr auto MENU_KEY_4 = (1 << 3);
export inline constexpr auto MENU_KEY_5 = (1 << 4);
export inline constexpr auto MENU_KEY_6 = (1 << 5);
export inline constexpr auto MENU_KEY_7 = (1 << 6);
export inline constexpr auto MENU_KEY_8 = (1 << 7);
export inline constexpr auto MENU_KEY_9 = (1 << 8);
export inline constexpr auto MENU_KEY_0 = (1 << 9);

export enum EMenu
{
	Menu_OFF,
	Menu_ChooseTeam,
	Menu_IGChooseTeam,
	Menu_ChooseAppearance,
	Menu_Buy,
	Menu_BuyPistol,
	Menu_BuyRifle,
	Menu_BuyMachineGun,
	Menu_BuyShotgun,
	Menu_BuySubMachineGun,
	Menu_BuyItem,
	Menu_Radio1,
	Menu_Radio2,
	Menu_Radio3,
	Menu_ClientBuy,

	Menu_LastItem = Menu_ClientBuy,
	Menu_AirSupport,
};

export namespace Menu
{
	namespace Text
	{
		inline constexpr char AIRSUPPORT[] =
			"\\yAir Support Selection\\w\n"
			"\n"
			"\\r1\\w Precise air strike\n"
			"\\r2\\w Cluster bomb\n"
			"\\r3\\w Carpet bombardment\n"
			"\\r4\\w Gunship strike\n"
			"\\r5\\w Thermobaric weapon\n"
			"\\r6\\w White phosphorus bomb\n"
			"\n"
			"\\r0\\w Exit\n"
			;

		// #SHOULD_DO_ON_FREE move to localization somehow?
		//inline constexpr char AIRSUPPORT_TEMPLATE[] =
		//	"\\yAir Support Selection\\w\n"
		//	"\n"
		//	"{}1. Precise air strike{}\n"
		//	"{}2. Cluster bomb{}\n"
		//	"{}3. Carpet bombardment{}\n"
		//	"{}4. Gunship strike{}\n"
		//	"{}5. Thermobaric weapon{}\n"
		//	"{}6. White phosphorus bomb{}\n"
		//	"\n"
		//	"\\w0. Exit\n"
		//	;

		inline constexpr char AIRSUPPORT_TEMPLATE[] =
			u8"\\y空襲類別選單\\w\n"
			u8"\n"
			u8"{}1. 精確導彈打擊{}\n"
			u8"{}2. 集束炸藥{}\n"
			u8"{}3. 地毯式轟炸{}\n"
			u8"{}4. 空中炮艦支援{}\n"
			u8"{}5. 雲爆彈{}\n"
			u8"{}6. 白磷彈{}\n"
			u8"\n"
			u8"\\w0. 離開\n"
			;

		//inline constexpr char SELECTED[] = " - Selected";
		inline constexpr char SELECTED[] = u8" - 已選取";
	};

	namespace Key
	{
		inline constexpr uint16_t AIRSUPPORT = MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_3 | MENU_KEY_4 | MENU_KEY_5 | MENU_KEY_6 | MENU_KEY_0;
	};
};

export extern "C++" void UTIL_ShowMenu(edict_t *pPlayer, uint16_t bitsValidSlots, std::string szText) noexcept;
export extern "C++" void OnMenuSelection(CBasePlayer *pPlayer, int iSlot) noexcept;
