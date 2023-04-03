export module Projectile;

export import <list>;
export import <vector>;

export import Beam;
export import Effects;
export import Prefab;

export inline constexpr auto MISSILE_GROUPINFO = (1 << 10);
export inline constexpr auto MISSILE_SOUND_CORO_KEY = 687286ul;

export struct CPrecisionAirStrike : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "missile_precision";
	static inline constexpr auto SPEED = 1000.0;

	Task Task_Deviation() noexcept;
	Task Task_EmitExhaust() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CPrecisionAirStrike *Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept;

	Vector m_vecTarget{};
	CBasePlayer *m_pPlayer{};
};

export struct CClusterCharge : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "cluster_charge";
	static inline uint64_t s_iCounter = 0;

	// Methods

	Task Task_Explo() noexcept;
	Task Task_VisualEffects() noexcept;


	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;
	bool ShouldCollide(EHANDLE<CBaseEntity> pOther) noexcept override;

	static CClusterCharge *Create(CBasePlayer *pPlayer, Vector const &vecSpawn, float const flFuseTime) noexcept;

	// Members

	CBasePlayer *m_pPlayer{};
	float m_flTotalFuseTime{};
	short m_iBounceCount{};
};

export struct CClusterBomb : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "cluster_bomb";
	static inline constexpr auto SPEED = 800.0;

	Task Task_ClusterBomb() noexcept;
	Task Task_ClusterBomb2() noexcept;

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

	Task Task_Touch() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CCarpetBombardment *Create(CBasePlayer *pPlayer, Vector const &vecSpawn, CBeam *pBeacon) noexcept;

	CBasePlayer *m_pPlayer{};
	EHANDLE<CBeam> m_pCorrespondingBeacon{ nullptr };
};

export struct CBullet : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "gunship_bullet";
	static inline constexpr double AC130_BULLET_SPEED = 4096;
	static inline constexpr double AC130_BULLET_EXPECTED_TRAVEL_TIME = 0.12;
	static inline constexpr double WHIZZ_RADIUS = 2 * 39.37;	// 2 meter.

	Task Task_Touch() noexcept;
	Task Task_Fly() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CBullet *Create(Vector const &vecOrigin, Vector const &vecVelocity, CBasePlayer *pShooter) noexcept;

	CBasePlayer *m_pShooter{};
	Vector m_vecLastTraceSrc{};
};

export struct CFuelAirExplosive : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "fuel_air_bomb";

	Task Task_GasPropagate() noexcept;
	Task Task_StopSoundAndRemove() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;
	void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) noexcept override;
	qboolean TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) noexcept override;

	static CFuelAirExplosive *Create(CBasePlayer *pPlayer, Vector const &vecOrigin) noexcept;

	CBasePlayer *m_pPlayer{};
	std::list<EHANDLE<CFuelAirCloud>> m_rgpCloud{};
	bool m_bReleasingGas{ false }, m_bGasAllOut{ false }, m_bTouched{ false };
};

export struct CIncendiaryMunition : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "proj_incendiary_bomb";
	static inline constexpr auto SPEED = 800.0;

	Task Task_Deviation() noexcept;
	Task Task_EmitExhaust() noexcept;
	Task Task_Fuse() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CIncendiaryMunition *Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept;

	Vector m_vecTarget{}, m_vecStartingPos{};
	CBasePlayer *m_pPlayer{};
};
