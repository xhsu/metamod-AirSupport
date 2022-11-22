export module Effects;

export import <chrono>;
export import <deque>;
export import <unordered_map>;

export import Prefab;

using std::deque;
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

export struct CFieldSmoke : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "env_explo_field_smoke";

	Task Task_EmitSmoke() noexcept;
	Task Task_Remove() noexcept;

	void Spawn() noexcept override;

	// Members

	float m_flRadius{ 650.f };
	deque<EHANDLE<CFlame>> m_rgpFlamesDependent{};
};

export struct CSmoke : public Prefab_t
{
	// Info

	static inline constexpr char CLASSNAME[] = "env_explo_smoke";

	// Methods

	Task Task_FadeOut() noexcept;

	void Spawn() noexcept override;

	// Members
};
