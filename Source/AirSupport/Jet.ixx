export module Jet;

export import Missile;
export import Target;

export struct CJet : public Prefab_t
{
	static inline constexpr char CLASSNAME[] = "jet_of_regular_airstrike";

	Task Task_Jet() noexcept;

	void Spawn() noexcept override;
	qboolean IsInWorld() noexcept override;

	static CJet *Create(CBasePlayer *pPlayer, CFixedTarget *pTarget, Vector const &vecOrigin) noexcept;

	CBasePlayer *m_pPlayer{};
	EHANDLE<CFixedTarget> m_pTarget{ nullptr };
	EHANDLE<CPrecisionAirStrike> m_pMissile{ nullptr };
};
