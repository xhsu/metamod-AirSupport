export module Effects;

import <unordered_map>;

export import Prefab;

using std::unordered_map;

export struct CFlame : public Prefab_t
{
	Task Task_Animation() noexcept;
	Task Task_DetectGround() noexcept;
	Task Task_EmitLight() noexcept;
	Task Task_EmitSmoke() noexcept;
	Task Task_Remove() noexcept;

	void Spawn() noexcept override;

	void Touch_DealBurnDmg(CBaseEntity *pOther) noexcept;

	// Members

	short m_iMaxFrame{};
	short m_iFlameSprIndex{};
	EHANDLE<CBasePlayer> m_pOwner{ nullptr };
	unordered_map<int, float> m_rgflDamageInterval{};
};
