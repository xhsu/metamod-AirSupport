export module Jet;

export import Menu;
export import Target;

export struct CJet : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "jet_of_regular_airstrike";

	Task Task_AirStrike() noexcept;
	Task Task_ClusterBomb() noexcept;

	void Spawn() noexcept override;
	qboolean IsInWorld() noexcept override;

	static CJet *Create(CBasePlayer *pPlayer, CFixedTarget *pTarget, Vector const &vecOrigin) noexcept;

	CBasePlayer *m_pPlayer{};
	EHANDLE<CFixedTarget> m_pTarget{ nullptr };
	EHANDLE<CBaseEntity> m_pMissile{ nullptr };
	EAirSupportTypes m_AirSupportType{ AIR_STRIKE };
};
