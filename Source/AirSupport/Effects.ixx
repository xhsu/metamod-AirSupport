export module Effects;

export import <array>;
export import <list>;
export import <unordered_map>;

export import Prefab;

using std::array;
using std::list;
using std::unordered_map;

export namespace Color
{
	inline constexpr array Team
	{
		color24{0xFF, 0xA0, 0x00},	// TEAM_UNASSIGNED
		color24{0xD8, 0x51, 0x50},	// TEAM_TERRORIST
		color24{0xAD, 0xC9, 0xEB},	// TEAM_CT
		color24{0xC0, 0xC0, 0xC0},	// TEAM_SPECTATOR
	};
};

export enum EEfxTasks : uint64_t
{
	TASK_ANIMATION = (1 << 0),
	TASK_FADE_OUT = (1 << 1),
	TASK_FADE_IN = (1 << 2),
	TASK_COLOR_DRIFT = (1 << 3),
	TASK_REFLECTING_FLAME = (1 << 4),
	TASK_IGNITE = (1 << 5),
	TASK_HB_AND_ER = (1 << 6),
	TASK_FLYING = (1 << 7),
};

export extern "C++" Task Task_SpriteLoop(entvars_t *const pev, short const FRAME_COUNT, double const FPS) noexcept;
export extern "C++" Task Task_SpriteLoop(entvars_t* const pev, uint16_t const STARTS_AT, uint16_t const FRAME_COUNT, double const FPS) noexcept;
export extern "C++" Task Task_SpritePlayOnce(entvars_t *const pev, short const FRAME_COUNT, double const FPS) noexcept;
export extern "C++" Task Task_SpritePlayOnce(entvars_t *const pev, uint16_t const FRAME_COUNT, double const FPS, float const AWAIT, float const DECAY, float const ROLL, float const SCALE_INC) noexcept;
export extern "C++" Task Task_SpriteLoopOut(entvars_t* const pev, uint16_t const LOOP_STARTS_AT, uint16_t const LOOP_FRAME_COUNT, uint16_t const OUT_ENDS_AT, float const TIME, double const FPS) noexcept;
export extern "C++" Task Task_FadeOut(entvars_t *const pev, float const AWAIT, float const DECAY, float const ROLL) noexcept;
export extern "C++" Task Task_FadeOut(entvars_t *const pev, float const AWAIT, float const DECAY, float const ROLL, float const SCALE_INC) noexcept;
export extern "C++" Task Task_Remove(entvars_t *const pev, float const TIME) noexcept;
export extern "C++" Task Task_FadeIn(entvars_t *const pev, float const TRANSPARENT_INC, float const FINAL_VAL, float const ROLL) noexcept;
export extern "C++" Task Task_Fade(entvars_t *const pev, float const INC, float const DEC, float const PEAK, float const ROLL) noexcept;

export struct CFlame : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_explo_flame";

	// Methods

	Task Task_DetectGround() noexcept;	// Deprecated.
	Task Task_EmitLight() noexcept;
	Task Task_EmitSmoke() noexcept;

	void Spawn() noexcept override;

	void Touch_AttachingSurface(CBaseEntity *pOther) noexcept;
	void Touch_DealBurnDmg(CBaseEntity *pOther) noexcept;

	// Members

	EHANDLE<CBasePlayer> m_pOwner{ nullptr };
	unordered_map<int, float> m_rgflDamageInterval{};
};

export struct CSmoke : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_thick_smoke";
	static inline constexpr Vector MIN_SIZE = { -32, -32, -32 };
	static inline constexpr Vector MAX_SIZE = { 32, 32, 32 };
	static inline constexpr double SPHERICAL_RADIUS = 32 * std::numbers::sqrt3;

	// Methods

	Task Task_DriftColor(Vector const vecTargetColor) noexcept;	// keep it internal, thanks.
	Task Task_ReflectingFlame() noexcept;

	void LitByFlame(bool const bShouldStartingWhite) noexcept;
	void DriftToWhite(double const flPercentage) noexcept;	// [0-1]

	void Spawn() noexcept override;

	// Members

	Vector m_vecStartingLitClr{};
	Vector m_vecFlameClrToWhite{};
};

export struct CThinSmoke : public CSmoke
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_thin_smoke";

	// Methods

	void Spawn() noexcept override;
};

export struct CToxicSmoke : public CThinSmoke
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_toxic_smoke";

	// Methods

	Task Task_InFloatOut() noexcept;

	void Spawn() noexcept override;
};

export struct CThickStaticSmoke : public CSmoke
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_thick_static_smoke";
	static inline constexpr float FADEOUT_SPEED = 0.1f;

	// Methods

	Task Task_Dispatch() noexcept;
	Task Task_Scaling() noexcept;

	void Spawn() noexcept override;
};

export struct CFloatingDust : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_floating_dust";
	static inline constexpr double FPS = 18.0;
	static inline constexpr short FRAME_COUNT = 40;

	static inline constexpr Vector MIN_SIZE = { -128, -128, -128 };
	static inline constexpr Vector MAX_SIZE = { 128, 128, 128 };
	static inline constexpr double SPHERICAL_RADIUS = 72 * std::numbers::sqrt3;

	void Spawn() noexcept override;
};

export struct CDebris : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "debris_from_explo";

	// Methods

	Task Task_Debris() noexcept;

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;
};

export struct CSparkMdl : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "3d_spark_of_gunshot";
	static inline constexpr auto HOLD_TIME = 0.07f;

	void Spawn() noexcept override;
};

export struct CGunshotSmoke : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_gunshot_smoke";
	static inline constexpr double FPS = 30.0;

	void Spawn() noexcept override;

	static CGunshotSmoke *Create(const TraceResult &tr) noexcept;

	TraceResult m_tr{};
};

export struct CGroundedDust : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_smoke_grounded_dust";
	static inline constexpr double FPS = 12.0;
	static inline constexpr short FRAME_COUNT = 25;

	void Spawn() noexcept override;
};

export struct CSparkSpr : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_spark_spr";
	static inline constexpr auto FRAME_COUNT = 4;
	static inline constexpr auto HOLD_TIME = 0.07f;

	void Spawn() noexcept override;
};

export extern "C++" Task Task_GlobalCoughThink() noexcept;

export struct CFuelAirCloud : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "fuel_air_cloud";
	static inline constexpr double FPS = 18.0;
	static inline constexpr short FRAME_COUNT = 40;

	static inline list<EHANDLE<CFuelAirCloud>> s_rgpAeroClouds{};

	~CFuelAirCloud() noexcept override { s_rgpAeroClouds.remove(this); }

	Task Task_FadeIn(float const TRANSPARENT_INC, float const FINAL_VAL, float const ROLL) noexcept;
	Task Task_Ignite(void) noexcept;
	Task Task_EmitLight(void) noexcept;
	Task Task_TimeOut(void) noexcept;

	__forceinline void Ignite(void) noexcept { if (!m_Scheduler.Exist(TASK_IGNITE)) m_Scheduler.Enroll(Task_Ignite(), TASK_IGNITE); }

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	static CFuelAirCloud *Create(CBasePlayer *pPlayer, Vector const &vecOrigin) noexcept;
	static void ApplySuffocation(CBasePlayer *pPlayer) noexcept;
	static Task Task_AirPressure() noexcept;
	static Task Task_PlayerSuffocation(CBasePlayer *pPlayer, entvars_t *pevWorld) noexcept;
	static Task Task_GlobalSuffocation() noexcept;
	static void OnTraceAttack(TraceResult const &tr, EHANDLE<CBaseEntity> pSkippedEntity) noexcept;

	CBasePlayer *m_pPlayer{};
	bool m_bFadeInDone{ false };
	bool m_bIgnited{ false };
	bool m_bBurning{ false };
	std::uint8_t m_iIgnitedCounts{};
};

export struct CSpriteDisplay : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "CSpriteDisplay";

	bool ShouldCollide(EHANDLE<CBaseEntity> pOther) noexcept override { return false; }	// just a SPR, collide with absolutely nothing.

	static CSpriteDisplay *Create(Vector const& vecOrigin, kRenderFn iRenderMethod, std::string_view szModel) noexcept;
};

export struct CPhosphorus : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "proj_phosphorus";

	void Spawn() noexcept override;

	void Touch_Flying(CBaseEntity *pOther) noexcept;
	void Touch_Burning(CBaseEntity *pOther) noexcept;

	Task Task_Gravity() noexcept;
	Task Task_EmitExhaust() noexcept;
	Task Task_EmitSmoke() noexcept;
	Task Task_EmitSpark() noexcept;

	static CPhosphorus *Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector2D const &vecInitVel) noexcept;	// Start with designated velocity
	static CPhosphorus *Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept;	// Horizontal projectile, flying to target location.
	static CPhosphorus *Create(CBasePlayer *pPlayer, Vector const &vecOrigin) noexcept;	// Fly towards random surrounding direction.

	CBasePlayer *m_pPlayer{};
	TraceResult m_tr{};	// trace result agaist current attached surface.
	unordered_map<int, float> m_rgflDamageInterval{};
};

export struct CShower2 : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "spark_shower_with_lit";

	Task Task_Flashing() noexcept;

	int ObjectCaps() noexcept override { return FCAP_DONT_SAVE; }
	void Touch(CBaseEntity *pOther) noexcept override;

	static CShower2 *Create(Vector const &vecOrigin, Vector const &vecDir) noexcept;
};
