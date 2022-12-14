export module Target;

export import <array>;

export import Beam;
export import Menu;
export import Prefab;

export enum ETargetTasks : uint64_t
{
	TASK_QUICK_ANALYZE = (1 << 8),
	TASK_DEEP_ANALYZE = (1 << 9),
	TASK_TIME_OUT = (1 << 10),
	TASK_ACTION = (1 << 11),
};

export struct CDynamicTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_dynamic_target";
	static inline constexpr size_t BEACON_COUNT = 12;
	static inline constexpr double CARPET_BOMBARDMENT_INTERVAL = 250.0;
	static inline mstudiobonecontroller_t ARROW_CONTROLLER{}, MINIATURE_CONTROLLER{};

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
	static void RetrieveModelInfo(void) noexcept;

	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
	EHANDLE<CBasePlayerWeapon> m_pRadio{ nullptr };
	CBasePlayer *m_pPlayer{};
	Vector m_vecLastAiming{};
	float m_flLastValidTracking{};
	std::array<EHANDLE<CBeam>, BEACON_COUNT> m_rgpBeacons{};
	std::array<BodyEnumInfo_t, 3> m_rgBodyInfo{ {{0, 1}, {0, 7}, {0, 2}} };
	int &m_iAirSupportTypeModel{ m_rgBodyInfo[1].m_index };
	qboolean &m_bShowArror{ m_rgBodyInfo[2].m_index };
	bool m_bFreezed{};	// Use enable/disable beacons instead.
};

export struct CFixedTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_fixed_target";
	static inline constexpr auto GUNSHIP_NEXT_TARGET_RADIUS = 500.0;
	static inline mstudiobonecontroller_t &ARROW_CONTROLLER{ CDynamicTarget::ARROW_CONTROLLER }, &MINIATURE_CONTROLLER{ CDynamicTarget::MINIATURE_CONTROLLER };

	Task Task_AdjustMiniature() noexcept;
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
