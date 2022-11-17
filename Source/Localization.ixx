export module Localization;

export namespace Localization
{
	inline namespace Keys
	{
		inline constexpr char REJECT_NO_JET_SPAWN[] = "#AIRSUPPORT_REJECT_NO_JET_SPAWN";
		inline constexpr char REJECT_NO_VALID_TRACELINE[] = "#AIRSUPPORT_REJECT_NO_VALID_TRACELINE";
		inline constexpr char REJECT_TIME_OUT[] = "#AIRSUPPORT_REJECT_TIME_OUT";
		inline constexpr char REJECT_COVERED_LOCATION[] = "#AIRSUPPORT_REJECT_COVERED_LOCATION";
	};

	namespace L_EN
	{
		inline constexpr char REJECT_NO_JET_SPAWN[] = "The pilot found nowhere to approach.";
		inline constexpr char REJECT_NO_VALID_TRACELINE[] = "The pilot has no clear sight.";
		inline constexpr char REJECT_TIME_OUT[] = "Airsupport cancelled due to insufficition communication.";
		inline constexpr char REJECT_COVERED_LOCATION[] = "The location cannot be cover by airsupport.";
	};

	namespace L_CH
	{
		inline constexpr char REJECT_NO_JET_SPAWN[] = u8"飛行員無法靠近地點";
		inline constexpr char REJECT_NO_VALID_TRACELINE[] = u8"飛行員沒有清晰的視線";
		inline constexpr char REJECT_TIME_OUT[] = u8"空中支援由於溝通不足終止";
		inline constexpr char REJECT_COVERED_LOCATION[] = u8"地點處於火力覆蓋範圍之外";
	};
};
