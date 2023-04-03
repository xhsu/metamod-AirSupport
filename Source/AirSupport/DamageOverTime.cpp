import <array>;

import DamageOverTime;
import Resources;
import Round;

import CBase;
import Task;

import UtlRandom;

using std::array;

extern "C++" namespace Gas
{
	bool TryCough(CBasePlayer *pPlayer) noexcept
	{
		static array<float, 33> rgflTimeNextCough{};

		if (!pPlayer->IsAlive() || pPlayer->pev->takedamage == DAMAGE_NO)
			return false;

		if (auto const iIndex = pPlayer->entindex(); rgflTimeNextCough[iIndex] < gpGlobals->time)
		{
			rgflTimeNextCough[iIndex] = gpGlobals->time + UTIL_Random(3.f, 3.5f);
			g_engfuncs.pfnEmitSound(pPlayer->edict(), CHAN_AUTO, UTIL_GetRandomOne(Sounds::PLAYER_COUGH), VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 116));
			return true;
		}

		return false;
	}

	bool Intoxicate(CBasePlayer *pPlayer, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage) noexcept
	{
		static array<float, 33> rgflTimeNextInhale{};

		if (!pPlayer->IsAlive() || pPlayer->pev->takedamage == DAMAGE_NO)
			return false;

		if (auto const iIndex = pPlayer->entindex(); rgflTimeNextInhale[iIndex] < gpGlobals->time)
		{
			rgflTimeNextInhale[iIndex] = gpGlobals->time + UTIL_Random(1.5f, 2.5f);

			pPlayer->TakeDamage(
				pevInflictor ? pevInflictor : g_pevWorld,
				pevAttacker ? pevAttacker : g_pevWorld,
				flDamage,
				DMG_POISON
			);

			return true;
		}

		return false;
	}
};
