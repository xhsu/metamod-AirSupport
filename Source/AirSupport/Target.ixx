export module Target;

export import std;

export import Beam;
export import Effects;
export import Menu;
export import PlayerItem;	// Cannot use the Weapon.ixx header, or it would be a circular dependency.
export import Prefab;
export import Task.Const;

// Out quaternion has two problems.
// 1. When the angle is UP, no rotation could happen.
// 2. When the angle is LEFT, the cross-product is therefore vecZero, which leads to no quaternion could be calculated.
export inline constexpr Vector VEC_ALMOST_RIGHT{ 0.99999946, 0, 0.0010010005 };	// #UPDATE_AT_CPP26 constexpr math expanded

export struct CDynamicTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_dynamic_target";
	static inline constexpr size_t BEACON_COUNT = 12;
	static inline constexpr double CARPET_BOMBARDMENT_INTERVAL = 250.0;
	static inline mstudiobonecontroller_t ARROW_CONTROLLER{}, MINIATURE_CONTROLLER{};
	static inline std::array<std::vector<int>, AIRSUPPORT_TYPES> FX_BONES_IDX;

	// The beam have no its own class.
	static inline constexpr char BEAM_CLASSNAME[] = "info_airsupport_beam";

	~CDynamicTarget() noexcept override;

	Task Task_Animation() noexcept;
	Task Task_AngleInterpol() noexcept;
	Task Task_DeepEval_AirStrike() noexcept;
	Task Task_DeepEval_Phosphorus() noexcept;
	Task Task_QuickEval_AirStrike() noexcept;
	Task Task_QuickEval_ClusterBomb() noexcept;
	Task Task_QuickEval_CarpetBombardment() noexcept;
	Task Task_QuickEval_Gunship() noexcept;
	Task Task_QuickEval_Phosphorus() noexcept;
	Task Task_Remove() noexcept;

	Task Task_Miniature_AirStrike() noexcept;
	Task Task_Miniature_ClusterBomb(std::string_view SPRITE, double const FPS, bool const REMOVE_ON_FRZ = false, Vector const vecOfs = g_vecZero) noexcept;
	//Task Task_Miniature_CarpetBombardment() noexcept;
	Task Task_Miniature_Gunship() noexcept;
	//Task Task_Miniature_Thermobaric() noexcept;
	Task Task_Miniature_Phosphorus() noexcept;	// Spark only

	void UpdateEvalMethod() noexcept;
	void EnableBeacons() noexcept;
	void DisableBeacons() noexcept;
	void UpdateVisualDemo() noexcept;

	void Spawn() noexcept override;
	static CDynamicTarget *Create(CBasePlayer *pPlayer, CPrefabWeapon *pRadio) noexcept;
	static void RetrieveModelInfo(void) noexcept;

	Quaternion m_qNormRotatingTo{};	// the endpos of current slerp, use it when converting to fixed target.
	Quaternion m_qPseudoanim{};
	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
	EHANDLE<CPrefabWeapon> m_pRadio{ nullptr };
	CBasePlayer *m_pPlayer{};
	std::array<EHANDLE<CSpriteDisplay>, 16> m_rgpVisualFxSpr{};	// it should be enough normally speaking.
	std::array<EHANDLE<CBeam>, BEACON_COUNT> m_rgpBeacons{};
	std::array<BodyEnumInfo_t, 4> m_rgBodyInfo{ {{0, 1}, {0, 7}, {0, 2}, {0, 2}} };
	bool m_bFreezed{};	// Use enable/disable beacons instead. Indicator of carpet bombardment direction.

	inline constexpr auto m_iAirSupportTypeModel() noexcept -> int& { return m_rgBodyInfo[1].m_index; }
	inline constexpr auto m_bShowArror() noexcept -> qboolean& { return m_rgBodyInfo[2].m_index; }
};

export struct CFixedTarget : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "info_fixed_target";
	static inline mstudiobonecontroller_t &ARROW_CONTROLLER{ CDynamicTarget::ARROW_CONTROLLER }, &MINIATURE_CONTROLLER{ CDynamicTarget::MINIATURE_CONTROLLER };

	~CFixedTarget() noexcept override;

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
	static CFixedTarget *Create(CBasePlayer* pPlayer, CBaseEntity* pTargeting) noexcept;

	CBasePlayer *m_pPlayer{};
	Vector m_vecJetSpawn{};
	Vector m_vecPlayerPosWhenCalled{};
	Vector m_vecPosForJetSpawnTesting{};
	EHANDLE<CBaseEntity> m_pMissile{ nullptr };
	EHANDLE<CBaseEntity> m_pTargeting{ nullptr };
	EAirSupportTypes m_AirSupportType{ AIR_STRIKE };
	decltype(CDynamicTarget::m_rgpBeacons) m_rgpBeacons{};
	//decltype(CDynamicTarget::m_rgpAttachedSpr) m_rgpAttachedSpr{};	// I don't think we need it. The weak_ptr inside the tasks of SPR is quite messy to transfer at the same time.
};
