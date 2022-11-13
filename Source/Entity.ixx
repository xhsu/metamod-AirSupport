export module Entity;

export import CBase;
export import Task;

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
	inline constexpr char FIXED_TARGET[] = "info_fixed_target";
}

export inline constexpr auto RADIO_KEY = 16486345;
export inline constexpr auto MISSILE_GROUPINFO = (1 << 10);
export inline constexpr auto MISSILE_SOUND_CORO_KEY = 687286ul;

export extern "C++" namespace Missile	// To allow module export a function in a .cpp file, it must be marked as [extern "C++"]
{
	extern edict_t *Create(CBasePlayer *pPlayer, Vector const &vecSpawnOrigin, Vector const &vecTargetOrigin) noexcept;
	extern void Think(CBaseEntity *pEntity) noexcept;
};

export extern "C++" namespace Weapon
{
	extern TimedFn Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern TimedFn Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern TimedFn Task_RadioUse(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern void OnRadioHolster(CBasePlayerItem *pThis) noexcept;
};

export extern "C++" namespace Laser
{
	extern void Create(CBasePlayerWeapon *pWeapon) noexcept;
	extern void Think(CBaseEntity *pEntity) noexcept;
};

export extern "C++" namespace Target
{
	extern void Create(CBasePlayerWeapon *pWeapon) noexcept;
	extern void Think(CBaseEntity *pEntity) noexcept;
};

export extern "C++" namespace FixedTarget
{
	extern edict_t *Create(Vector const &vecOrigin, Vector const &vecAngles, CBasePlayer *const pPlayer) noexcept;
	extern void Start(CBaseEntity *pTarget) noexcept;
	extern TimedFn Think(EHANDLE<CBaseEntity> pEntity) noexcept;
};

export extern "C++" namespace Jet
{
	TimedFn Think(EHANDLE<CBaseEntity> pEntity) noexcept;
};
