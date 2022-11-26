export module Jet;

export import Menu;
export import Target;

export struct CJet : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "jet_of_regular_airstrike";

	Task Task_BeamAndSound() noexcept;
	Task Task_AirStrike() noexcept;
	Task Task_ClusterBomb() noexcept;
	Task Task_CarpetBombardment() noexcept;

	void Spawn() noexcept override;
	qboolean IsInWorld() noexcept override;

	static CJet *Create(CBasePlayer *pPlayer, CFixedTarget *pTarget, Vector const &vecOrigin) noexcept;

	CBasePlayer *m_pPlayer{};
	EHANDLE<CFixedTarget> m_pTarget{ nullptr };
	EAirSupportTypes m_AirSupportType{ AIR_STRIKE };
};

export struct CGunship : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "gunship";

	Task Task_Gunship() noexcept;

	void Spawn() noexcept override;

	static CGunship *Create(CBasePlayer *pPlayer, CFixedTarget *pTarget, Vector const &vecOrigin) noexcept;

	CBasePlayer *m_pPlayer{};
	EHANDLE<CFixedTarget> m_pTarget{};
};
