export module Entity;

export import <vector>;

export import CBase;
export import Task;

export import Prefab;

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
	inline constexpr char FIXED_TARGET[] = "info_fixed_target";
}

export inline constexpr auto RADIO_KEY = 16486345;
export inline constexpr auto MISSILE_GROUPINFO = (1 << 10);
export inline constexpr auto MISSILE_SOUND_CORO_KEY = 687286ul;

export inline std::vector<edict_t *> g_rgpCTs = {};
export inline std::vector<edict_t *> g_rgpTers = {};

export extern "C++" Task Task_UpdateTeams(void) noexcept;

export extern "C++" namespace Missile	// To allow module export a function in a .cpp file, it must be marked as [extern "C++"]
{
	extern edict_t *Create(CBasePlayer *pPlayer, Vector const &vecSpawnOrigin, Vector const &vecTargetOrigin) noexcept;
	extern void Think(CBaseEntity *pEntity) noexcept;
};

export extern "C++" namespace Weapon
{
	extern Task Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept;
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
	extern Task Task_ScanJetSpawn(EHANDLE<CBaseEntity> pTarget) noexcept;
};

export extern "C++" namespace FixedTarget
{
	extern edict_t *Create(Vector const &vecOrigin, Vector const &vecAngles, CBasePlayer *const pPlayer) noexcept;
	extern void Start(CBaseEntity *pTarget) noexcept;
	extern Task Task_RecruitJet(EHANDLE<CBaseEntity> pEntity) noexcept;
};

export extern "C++" namespace Jet
{
	Task Task_Jet(EHANDLE<CBaseEntity> pEntity) noexcept;
};

export struct CDynamicTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_dynamic_target";

	Task Task_Animation() noexcept;
	Task Task_DeepEvaluation() noexcept;
	Task Task_QuickEvaluation() noexcept;
	Task Task_Remove() noexcept;

	void Spawn() noexcept override;
	static CDynamicTarget *Create(CBasePlayer *pPlayer, CBasePlayerWeapon *pRadio) noexcept;

	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
	EHANDLE<CBasePlayerWeapon> m_pRadio{ nullptr };
	CBasePlayer *m_pPlayer{};
	Vector m_vecLastAiming{};

	static inline constexpr auto DETAIL_ANALYZE_KEY = 3658468ul;
};
