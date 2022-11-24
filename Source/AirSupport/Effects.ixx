export module Effects;

export import <chrono>;
export import <list>;
export import <unordered_map>;

export import Prefab;

using std::list;
using std::unordered_map;
using std::chrono::high_resolution_clock;

export struct CFlame : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_explo_flame";

	// Methods

	Task Task_Animation() noexcept;
	Task Task_DetectGround() noexcept;	// Deprecated.
	Task Task_EmitLight() noexcept;
	Task Task_EmitSmoke() noexcept;
	Task Task_Remove() noexcept;

	void Spawn() noexcept override;

	void Touch_AttachingSurface(CBaseEntity *pOther) noexcept;
	void Touch_DealBurnDmg(CBaseEntity *pOther) noexcept;

	// Members

	short m_iMaxFrame{};
	short m_iFlameSprIndex{};
	EHANDLE<CBasePlayer> m_pOwner{ nullptr };
	unordered_map<int, float> m_rgflDamageInterval{};
	high_resolution_clock::time_point m_LastAnimUpdate{};
};

export struct CSmoke : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_explo_smoke";
	static inline constexpr auto DRIFT_COLOR_KEY = 3896738ul;

	// Methods

	Task Task_DriftColor(Vector const vecTargetColor) noexcept;	// keep it internal, thanks.
	Task Task_FadeOut() noexcept;

	void LitByFlame() noexcept;
	void StartColorDrift(Vector const &vecTargetColor) noexcept;
	void DriftToWhite(double const flPercentage) noexcept;	// [0-1]

	void Spawn() noexcept override;

	// Members

	Vector m_vecStartingLitClr{};
	Vector m_vecFlameClrToWhite{};
};

export struct CFieldSmoke : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_explo_field_smoke";

	Task Task_AdjustSmokeColor() noexcept;
	Task Task_Remove() noexcept;

	__forceinline void EnrollFlame(CFlame *pFlame) noexcept { m_rgpFlames.emplace_back(pFlame); }
	__forceinline void EnrollSmoke(CSmoke *pSmoke) noexcept { m_rgpSmokes.emplace_back(pSmoke); }

	void Spawn() noexcept override;
	void Activate() noexcept override;	// All the initial res are enlisted.

	// Members

	list<EHANDLE<CFlame>> m_rgpFlames{};
	list<EHANDLE<CSmoke>> m_rgpSmokes{};
	size_t m_iTotalFlames{}, m_iLastFlames{};
	size_t m_iTotalSmokes{}, m_iLastSmokes{};
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
