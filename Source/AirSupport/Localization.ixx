module;

#ifdef __INTELLISENSE__
#include <array>
#include <string_view>
#endif

export module Localization;

#ifndef __INTELLISENSE__
export import <array>;
export import <string_view>;
#endif

// #PLANNED_AS_localization using array not loose format.
export namespace Localization
{
	using namespace std::literals;

	inline constexpr std::array Keys
	{
		"#AIRSUPPORT_REJECT_NO_JET_SPAWN"sv,
		"#AIRSUPPORT_REJECT_NO_VALID_TRACELINE"sv,
		"#AIRSUPPORT_REJECT_TIME_OUT"sv,
		"#AIRSUPPORT_REJECT_COVERED_LOCATION"sv,
		"#AIRSUPPORT_REJECT_HEIGHT_NOT_ENOUGH"sv,
		"#AIRSUPPORT_REJECT_COOLING_DOWN"sv,
		"#AIRSUPPORT_GUNSHIP_DESPAWNING"sv,
		"#AIRSUPPORT_GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE"sv,
		"#AIRSUPPORT_GUNSHIP_RESELECT_TARGET"sv,
		"#AIRSUPPORT_HINT_PRESS_AND_HOLD"sv,
		"#AIRSUPPORT_HINT_RESEL_TARGET"sv,
	};

	inline constexpr std::array L_EN
	{
		"The pilot found nowhere to approach."sv,
		"The pilot has no clear sight."sv,
		"Airsupport cancelled due to insufficition communication."sv,
		"The location cannot be cover by airsupport."sv,
		"The location is too elevated! (%s1)"sv,
		"The HQ is preparing for the next round!"sv,
		"Gunship requires reload and leaving the area."sv,
		"Another gunship had taken the air supremacy in the area!"sv,
		"New target had been informed."sv,
		"PRESS and HOLD to direct the bombardment."sv,
		"Take out your radio again to manually select a target."sv,
	};

	inline constexpr std::array L_CH
	{
		u8"飛行員無法靠近地點"sv,
		u8"飛行員沒有清晰的視線"sv,
		u8"空中支援由於溝通不足終止"sv,
		u8"地點處於火力覆蓋範圍之外"sv,
		u8"地形崎嶇故無法轟炸 (%s1)"sv,
		u8"總部尚在籌備下一輪空襲"sv,
		u8"空中炮艦需要重填因而離開區域"sv,
		u8"另一架空中炮艦已經掌握本地制空權"sv,
		u8"已告知新目標"sv,
		u8"按住左鍵以調整轟炸方向"sv,
		u8"手持無線電以手動選定目標"sv,
	};

	inline constexpr auto REJECT_NO_JET_SPAWN = Keys[0].data();
	inline constexpr auto REJECT_NO_VALID_TRACELINE = Keys[1].data();
	inline constexpr auto REJECT_TIME_OUT = Keys[2].data();
	inline constexpr auto REJECT_COVERED_LOCATION = Keys[3].data();
	inline constexpr auto REJECT_HEIGHT_NOT_ENOUGH = Keys[4].data();
	inline constexpr auto REJECT_COOLING_DOWN = Keys[5].data();
	inline constexpr auto GUNSHIP_DESPAWNING = Keys[6].data();
	inline constexpr auto GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE = Keys[7].data();
	inline constexpr auto GUNSHIP_RESELECT_TARGET = Keys[8].data();
	inline constexpr auto HINT_PRESS_AND_HOLD = Keys[9].data();
	inline constexpr auto HINT_RESEL_TARGET = Keys[10].data();

	inline constexpr auto KEYS_COUNT = Keys.size();

	consteval bool Verify(auto&&... arrays)
	{
		return (... && (KEYS_COUNT == std::size(arrays)));
	}

	static_assert(Verify(Keys, L_EN, L_CH), u8"All keys must be translated!");
};
