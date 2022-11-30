export module Localization;

export namespace Localization
{
	inline namespace Keys
	{
		inline constexpr char REJECT_NO_JET_SPAWN[] = "#AIRSUPPORT_REJECT_NO_JET_SPAWN";
		inline constexpr char REJECT_NO_VALID_TRACELINE[] = "#AIRSUPPORT_REJECT_NO_VALID_TRACELINE";
		inline constexpr char REJECT_TIME_OUT[] = "#AIRSUPPORT_REJECT_TIME_OUT";
		inline constexpr char REJECT_COVERED_LOCATION[] = "#AIRSUPPORT_REJECT_COVERED_LOCATION";
		inline constexpr char GUNSHIP_DESPAWNING[] = "#AIRSUPPORT_GUNSHIP_DESPAWNING";
		inline constexpr char GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE[] = "#AIRSUPPORT_GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE";
		inline constexpr char GUNSHIP_RESELECT_TARGET[] = "#AIRSUPPORT_GUNSHIP_RESELECT_TARGET";
	};

	namespace L_EN
	{
		inline constexpr char REJECT_NO_JET_SPAWN[] = "The pilot found nowhere to approach.";
		inline constexpr char REJECT_NO_VALID_TRACELINE[] = "The pilot has no clear sight.";
		inline constexpr char REJECT_TIME_OUT[] = "Airsupport cancelled due to insufficition communication.";
		inline constexpr char REJECT_COVERED_LOCATION[] = "The location cannot be cover by airsupport.";
		inline constexpr char GUNSHIP_DESPAWNING[] = "Gunship requires reload and leaving the area.";
		inline constexpr char GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE[] = "Another gunship had taken the air supremacy in the area!";
		inline constexpr char GUNSHIP_RESELECT_TARGET[] = "New target had been informed.";
	};

	namespace L_CH
	{
		inline constexpr char REJECT_NO_JET_SPAWN[] = u8"飛行員無法靠近地點";
		inline constexpr char REJECT_NO_VALID_TRACELINE[] = u8"飛行員沒有清晰的視線";
		inline constexpr char REJECT_TIME_OUT[] = u8"空中支援由於溝通不足終止";
		inline constexpr char REJECT_COVERED_LOCATION[] = u8"地點處於火力覆蓋範圍之外";
		inline constexpr char GUNSHIP_DESPAWNING[] = u8"空中炮艦需要重填因而離開區域";
		inline constexpr char GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE[] = u8"另一架空中炮艦已經掌握本地制空權";
		inline constexpr char GUNSHIP_RESELECT_TARGET[] = u8"已告知新目標";
	};
};
