export module DamageOverTime;

export import CBase;
export import Query;

export extern "C++" namespace Gas
{
	bool TryCough(CBasePlayer *pPlayer) noexcept;
	bool Intoxicate(CBasePlayer *pPlayer, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flDmgInterval) noexcept;
};

export extern "C++" namespace Burning
{
	void ByPhosphorus(CBasePlayer *pVictim, CBasePlayer *pAttacker) noexcept;
};
