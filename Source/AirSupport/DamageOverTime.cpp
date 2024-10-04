import std;

import DamageOverTime;
import Hook;
import Resources;
import Round;
import Task.Const;

import CBase;
import Message;
import Task;

import UtlRandom;

using std::array;

namespace Gas
{
	bool TryCough(CBasePlayer *pPlayer) noexcept
	{
		static array<float, 33> rgflTimeNextCough{};

		if (!pPlayer->IsAlive() || pPlayer->pev->takedamage == DAMAGE_NO)
			return false;

		if (auto const iIndex = pPlayer->entindex(); rgflTimeNextCough[iIndex] < gpGlobals->time)
		{
			rgflTimeNextCough[iIndex] = gpGlobals->time + UTIL_Random(3.f, 3.5f);
			g_engfuncs.pfnEmitSound(pPlayer->edict(), CHAN_AUTO, UTIL_GetRandomOne(Sounds::PLAYER_COUGH), VOL_NORM, ATTN_STATIC, SND_FL_NONE, UTIL_Random(92, 116));
			return true;
		}

		return false;
	}

	bool Intoxicate(CBasePlayer *pPlayer, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flDmgInterval) noexcept
	{
		static array<float, 33> rgflTimeNextInhale{};

		if (!pPlayer->IsAlive() || pPlayer->pev->takedamage == DAMAGE_NO)
			return false;

		if ((bool)CVar::cloud_dmg_use_percentage)
			flDamage = std::max(flDamage, pPlayer->pev->health * (flDamage / 100.f));


		if (auto const iIndex = pPlayer->entindex(); rgflTimeNextInhale[iIndex] < gpGlobals->time)
		{
			flDmgInterval = std::max(0.6f, flDmgInterval);	// because of the -0.5f
			rgflTimeNextInhale[iIndex] = gpGlobals->time + UTIL_Random(flDmgInterval - 0.5f, flDmgInterval + 0.5f);

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

using tempent_ptr_t = std::unique_ptr<edict_t, decltype([](edict_t *pe) noexcept { g_engfuncs.pfnRemoveEntity(pe);})>;

namespace Burning
{
	Task _Internal_TakeDamage(CBasePlayer *pVictim, entvars_t *pevAttacker) noexcept
	{
		tempent_ptr_t pInf{ g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target")) };

		pInf->v.classname = MAKE_STRING("white_phosphorus");

		auto const flPercent = (float)CVar::pim_perm_burning_dmg / 100.f;
		auto const flMinCap = (float)CVar::pim_perm_burning_dmg;

		for (; pVictim->IsAlive(); co_await (float)CVar::pim_perm_burning_inv)
		{
			if (pVictim->pev->waterlevel == 3)	// water can only pause the white phosphorus fire, not eliminate it.
				continue;

			pVictim->TakeDamage(&pInf->v, pevAttacker, std::max(pVictim->pev->health * flPercent, flMinCap), DMG_SLOWBURN);
		}
	}

	Task _Internal_DisplayFlameSpr(CBasePlayer *pPlayer) noexcept
	{
		for (; pPlayer->IsAlive(); co_await 0.1f)
		{
			if (pPlayer->pev->waterlevel == 3)	// fire effect pause. one may have it removed at this moment.
				continue;

			auto const [vecMin, vecMax] = UTIL_ExtractBbox(pPlayer->edict(), pPlayer->pev->sequence);
			Vector const vecNoise{
				pPlayer->pev->origin.x + UTIL_Random(vecMin.x, vecMax.x),
				pPlayer->pev->origin.y + UTIL_Random(vecMin.y, vecMax.y),
				pPlayer->pev->origin.z + UTIL_Random(vecMin.z, vecMax.z) * 0.65,
			};
	
			MsgPVS(SVC_TEMPENTITY, pPlayer->pev->origin);
			WriteData(TE_SPRITE);
			WriteData(vecNoise);
			WriteData(Sprites::m_rgLibrary[Sprites::FLAME_ON_PLAYER]);
			WriteData((uint8_t)UTIL_Random(8, 10));
			WriteData((uint8_t)100);
			MsgEnd();

			MsgPVS(SVC_TEMPENTITY, pPlayer->pev->origin);
			WriteData(TE_DLIGHT);
			WriteData(vecNoise);	// pos
			WriteData((uint8_t)UTIL_Random(12, 14));	// rad in 10's
			WriteData((uint8_t)UTIL_Random(0xC3, 0xCD));	// r
			WriteData((uint8_t)UTIL_Random(0x3E, 0x46));	// g
			WriteData((uint8_t)UTIL_Random(0x05, 0x10));	// b
			WriteData((uint8_t)2);	// brightness
			WriteData((uint8_t)0);	// life in 10's
			WriteData((uint8_t)1);	// decay in 10's
			MsgEnd();
		}
	}

	void ByPhosphorus(CBasePlayer *pVictim, CBasePlayer *pAttacker) noexcept
	{
		if (pVictim->m_iTeam == pAttacker->m_iTeam && gcvarFriendlyFire->value < 1)
			return;

		uint64_t iPlayerTaskId = TASK_ENTITY_ON_FIRE | (1ull << uint64_t(pVictim->entindex() + 32ull));

		if (!TaskScheduler::Exist(iPlayerTaskId, false))
			TaskScheduler::Enroll(_Internal_TakeDamage(pVictim, pAttacker->pev), iPlayerTaskId);

		iPlayerTaskId = TASK_FLAME_ON_PLAYER | (1ull << uint64_t(pVictim->entindex() + 32ull));

		if (!TaskScheduler::Exist(iPlayerTaskId, false))
			TaskScheduler::Enroll(_Internal_DisplayFlameSpr(pVictim), iPlayerTaskId);
	}
};
