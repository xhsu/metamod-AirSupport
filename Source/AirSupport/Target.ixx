export module Target;

export import <chrono>;

export import Menu;
export import Prefab;

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
	EAirSupportTypes m_AirSupportType{ AIR_STRIKE };
};
