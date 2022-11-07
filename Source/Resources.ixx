export module Resources;

export import <array>;

export import util;

using std::array;

export enum EAirSupportTypes
{
	AIR_STRIKE = 0,
	CLUSTER_BOMB,
	CARPET_BOMB,
	GUNSHIP_STRIKE,
	FUEL_AIR_BOMB,	// thermobaric weapon
};

export namespace Models
{
	inline constexpr array PLANE =
	{
		"models/F18.mdl",
		"models/F18.mdl",
		"models/F18.mdl",
		"models/AC130_fly.mdl",
		"models/F18.mdl",
	};

	inline constexpr array PROJECTILE =
	{
		"models/mq9_missile.mdl",
		"models/mortar-rocket.mdl",
		"models/mortar-rocket.mdl",
		"models/mortar-rocket.mdl",
		"models/mortar-rocket.mdl",
	};
};

export namespace Sounds
{
	inline constexpr array RADIO =
	{
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_inpos.wav",
		"airsupport/radio/radio_affirm.wav",
	};

	inline constexpr char REQUESTING[] = "airsupport/radio/radio_use.wav";
	inline constexpr char REJECTING[] = "airsupport/radio/radio_negative.wav";
};
