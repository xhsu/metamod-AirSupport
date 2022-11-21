export module Missile;

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
