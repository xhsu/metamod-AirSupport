export module Target;

export import <array>;
export import <chrono>;

export import Beam;
export import Menu;
export import Prefab;

export struct CDynamicTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_dynamic_target";
	static inline constexpr size_t BEACON_COUNT = 12;
	static inline constexpr double CARPET_BOMBARDMENT_INTERVAL = 250.0;

	~CDynamicTarget() noexcept override;

	Task Task_Animation() noexcept;
	Task Task_DeepEvaluation() noexcept;
	Task Task_QuickEval_AirStrike() noexcept;
	Task Task_QuickEval_ClusterBomb() noexcept;
	Task Task_QuickEval_CarpetBombardment() noexcept;
	Task Task_QuickEval_Gunship() noexcept;
	Task Task_Remove() noexcept;

	void UpdateEvalMethod() noexcept;
	void EnableBeacons() noexcept;
	void DisableBeacons() noexcept;

	void Spawn() noexcept override;
	static CDynamicTarget *Create(CBasePlayer *pPlayer, CBasePlayerWeapon *pRadio) noexcept;

	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
	EHANDLE<CBasePlayerWeapon> m_pRadio{ nullptr };
	CBasePlayer *m_pPlayer{};
	Vector m_vecLastAiming{};
	float m_flLastValidTracking{};
	std::chrono::high_resolution_clock::time_point m_LastAnimUpdate{};
	std::array<EHANDLE<CBeam>, BEACON_COUNT> m_rgpBeacons{};
	bool m_bFreezed{};	// Use enable/disable beacons instead.

	static inline constexpr auto QUICK_ANALYZE_KEY = 298457034ul;
	static inline constexpr auto DETAIL_ANALYZE_KEY = 3658468ul;
};

export struct CFixedTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_fixed_target";
	static inline constexpr auto TIMEOUT_TASK_KEY = 64396365ul;
	static inline constexpr auto GUNSHIP_NEXT_TARGET_RADIUS = 500.0;

	Task Task_BeaconFx() noexcept;
	Task Task_Gunship() noexcept;
	Task Task_PrepareJetSpawn() noexcept;
	Task Task_RecruitJet() noexcept;
	Task Task_TimeOut() noexcept;
	Task Task_UpdateOrigin() noexcept;

	void Spawn() noexcept override;
	void Activate() noexcept override;

	static CFixedTarget *Create(CDynamicTarget *const pDynamicTarget) noexcept;

	CBasePlayer *m_pPlayer{};
	Vector m_vecJetSpawn{};
	Vector m_vecPlayerPosWhenCalled{};
	Vector m_vecPosForJetSpawnTesting{};
	EHANDLE<CBaseEntity> m_pMissile{ nullptr };
	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
	EHANDLE<CDynamicTarget> m_pDynamicTarget{ nullptr };	// Warning: only available in Spawn()!
	EAirSupportTypes m_AirSupportType{ AIR_STRIKE };
	decltype(CDynamicTarget::m_rgpBeacons) m_rgpBeacons{};
};
