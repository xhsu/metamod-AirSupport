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
}

export inline constexpr auto RADIO_KEY = 16486345;
