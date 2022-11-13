import Entity;
import Resources;

import UtlRandom;

extern "C++" namespace Jet
{
	TimedFn Task_Jet(EHANDLE<CBaseEntity> pJet) noexcept
	{
		auto pPlayer = EHANDLE<CBasePlayer>(pJet->pev->euser1);
		auto pTarget = EHANDLE<CBaseEntity>(pJet->pev->euser2);
		auto pMissile = EHANDLE<CBaseEntity>(nullptr);

		//g_engfuncs.pfnEmitSound(pJet.Get(), CHAN_WEAPON, UTIL_GetRandomOne(Sounds::JET), VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 118));	// #INVESTIGATE
		g_engfuncs.pfnClientCommand(pPlayer.Get(), "spk %s\n", UTIL_GetRandomOne(Sounds::JET));

		for (; pJet && pTarget;)
		{
			co_await 0.01f;	// try it every other frame.

			TraceResult tr{};
			g_engfuncs.pfnTraceLine(pJet->pev->origin, pTarget->pev->origin, ignore_monsters, pJet.Get(), &tr);

			if (tr.flFraction > 0.99f)
			{
				pMissile = Missile::Create(pPlayer, pJet->pev->origin - Vector(0, 0, 2.5f), pTarget->pev->origin);
				pTarget->pev->euser2 = pMissile.Get();	// pTarget now has a missile binding to it.
				break;
			}
		}

		for (; pJet;)
		{
			co_await 0.2f;

			[[unlikely]]
			if (pJet->pev->origin.x >= 4096 || pJet->pev->origin.y >= 4096 || pJet->pev->origin.z >= 4096
				|| pJet->pev->origin.x <= -4096 || pJet->pev->origin.y <= -4096 || pJet->pev->origin.z <= -4096)	// pJet->IsInWorld() have a speed limit, but we don't need it here.
			{
				pJet->pev->flags |= FL_KILLME;
				co_return;
			}
		}
	}
};
