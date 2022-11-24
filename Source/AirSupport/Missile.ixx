export module Missile;

export import <vector>;

export import Beam;
export import Prefab;

export inline constexpr auto MISSILE_GROUPINFO = (1 << 10);
export inline constexpr auto MISSILE_SOUND_CORO_KEY = 687286ul;

export struct CPrecisionAirStrike : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "missile_precision";
	static inline constexpr auto SPEED = 800.0;

	~CPrecisionAirStrike() noexcept override;

	Task Task_SFX() noexcept;
	Task Task_Trail() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CPrecisionAirStrike *Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept;

	Vector m_vecTarget{};
	CBasePlayer *m_pPlayer{};
};

export struct CClusterBomb : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "cluster_bomb";
	static inline constexpr auto SPEED = 800.0;

	Task Task_ClusterBomb() noexcept;

	void Spawn() noexcept override;

	static CClusterBomb *Create(CBasePlayer *pPlayer, Vector const &vecSpawn, Vector const &vecTargetOrigin) noexcept;

	double m_flDetonationHeight{};
	CBasePlayer *m_pPlayer{};
	Vector m_vecTargetGround{};
	std::vector<Vector> m_rgvecExploOrigins{};
};

export struct CCarpetBombardment : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "carpet_bombardment_per_charge";

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CCarpetBombardment *Create(CBasePlayer *pPlayer, Vector const &vecSpawn, CBeam *pBeacon) noexcept;

	CBasePlayer *m_pPlayer{};
	EHANDLE<CBeam> m_pCorrespondingBeacon{ nullptr };
};
