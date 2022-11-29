export module Effects;

export import <array>;
export import <chrono>;
export import <list>;
export import <unordered_map>;

export import Prefab;

import UtlRandom;

using std::array;
using std::list;
using std::unordered_map;
using std::chrono::high_resolution_clock;

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

export extern "C++" Task Task_SpriteLoop(entvars_t *const pev, short const FRAME_COUNT, double const FPS) noexcept;
export extern "C++" Task Task_SpritePlayOnce(entvars_t *const pev, short const FRAME_COUNT, double const FPS) noexcept;
export extern "C++" Task Task_FadeOut(entvars_t *const pev, float const DECAY, float const ROLL) noexcept;
export extern "C++" Task Task_Remove(entvars_t *const pev, float const TIME) noexcept;
export extern "C++" Task Task_FadeIn(entvars_t *const pev, float const TRANSPARENT_INC, float const FINAL_VAL, float const ROLL) noexcept;

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

	static inline constexpr char CLASSNAME[] = "env_explo_smoke";
	static inline constexpr auto DRIFT_COLOR_KEY = 3896738ul;
	static inline constexpr auto REFLECTING_FLAME_KEY = 332847938ul;

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

export struct CFloatingDust : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_floating_dust";
	static inline constexpr double FPS = 18.0;
	static inline constexpr short FRAME_COUNT = 40;

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
	static inline constexpr auto MAX_FRAME = 4;
	static inline constexpr auto HOLD_TIME = 0.07f;

	void Spawn() noexcept override;
};

export extern "C++" Task Task_GlobalCoughThink() noexcept;

export struct CFuelAirCloud : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "fuel_air_cloud";
	static inline constexpr double FPS = 18.0;
	static inline constexpr short MAX_FRAME = 40;
	static inline constexpr auto FADE_IN_TASK_KEY = 4325749ul;

	Task Task_FadeIn(float const TRANSPARENT_INC, float const FINAL_VAL, float const ROLL) noexcept;
	Task Task_Ignite(void) noexcept;

	__forceinline void Ignite(void) noexcept { m_Scheduler.Enroll(Task_Ignite()); }

	void Spawn() noexcept override;
	void Touch(CBaseEntity *pOther) noexcept override;

	bool m_bFadeInDone{ false };
	bool m_bIgnited{ false };
	std::uint8_t m_iIgnitedCounts{};
};
