export module Entity;

export import <vector>;
export import <chrono>;

export import CBase;
export import Task;

export import Prefab;

export namespace Classname
{
	inline constexpr char JET[] = "jet";
	inline constexpr char FAE[] = "petrol_bomb";
	inline constexpr char BEAM[] = "aiming_assist_beam";
}

export inline constexpr auto RADIO_KEY = 16486345;
export inline constexpr auto MISSILE_GROUPINFO = (1 << 10);
export inline constexpr auto MISSILE_SOUND_CORO_KEY = 687286ul;

export inline std::vector<edict_t *> g_rgpCTs = {};
export inline std::vector<edict_t *> g_rgpTers = {};

export extern "C++" Task Task_UpdateTeams(void) noexcept;

export extern "C++" namespace Weapon
{
	extern Task Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept;
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
	float m_flLastValidTracking{};
	std::chrono::high_resolution_clock::time_point m_LastAnimUpdate{};

	static inline constexpr auto DETAIL_ANALYZE_KEY = 3658468ul;
};

export struct CFixedTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_fixed_target";

	Task Task_PrepareJetSpawn() noexcept;
	Task Task_RecruitJet() noexcept;
	Task Task_TimeOut() noexcept;
	Task Task_UpdateOrigin() noexcept;

	void Spawn() noexcept override;
	void Activate() noexcept override;

	static CFixedTarget *Create(Vector const &vecOrigin, Vector const &vecAngles, CBasePlayer *const pPlayer, CBaseEntity *const pTarget) noexcept;

	CBasePlayer *m_pPlayer{};
	Vector m_vecJetSpawn{};
	Vector m_vecPlayerPosWhenCalled{};
	Vector m_vecPosForJetSpawnTesting{};
	EHANDLE<CBaseEntity> m_pMissile{ nullptr };
	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
};
