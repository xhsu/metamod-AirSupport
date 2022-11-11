export module Entity;

export enum EAirSupportTypes
{
	AIR_STRIKE = 0,
	CLUSTER_BOMB,
	CARPET_BOMB,
	GUNSHIP_STRIKE,
	FUEL_AIR_BOMB,	// thermobaric weapon
};

export namespace Classname
{
	inline constexpr char JET[] = "jet";
	inline constexpr char MISSILE[] = "rpgrocket";
	inline constexpr char FAE[] = "petrol_bomb";
	inline constexpr char BEAM[] = "aiming_assist_beam";
	inline constexpr char AIM[] = "aiming_pad";
}

export inline constexpr auto RADIO_KEY = 16486345;
export inline constexpr auto MISSILE_GROUPINFO = (1 << 10);
export inline constexpr auto MISSILE_SOUND_CORO_KEY = 687286ul;
