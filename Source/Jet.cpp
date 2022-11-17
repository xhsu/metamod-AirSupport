import Entity;
import Message;
import Missile;
import Resources;

import UtlRandom;

extern "C++" namespace Jet
{
	Task Task_Jet(EHANDLE<CBaseEntity> pJet) noexcept
	{
		auto pPlayer = EHANDLE<CBasePlayer>(pJet->pev->euser1);
		auto pTarget = EHANDLE<CFixedTarget>(pJet->pev->euser2);
		auto pMissile = EHANDLE<CPrecisionAirStrike>(nullptr);

		//g_engfuncs.pfnEmitSound(pJet.Get(), CHAN_WEAPON, UTIL_GetRandomOne(Sounds::JET), VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 118));	// #INVESTIGATE
		g_engfuncs.pfnClientCommand(pPlayer.Get(), "spk %s\n", UTIL_GetRandomOne(Sounds::JET));
		co_await 0.01f;

		for (auto i = 0x1000; i <= 0x4000; i += 0x1000)
		{
			unsigned short const iEntAtt = (pJet->entindex() & 0x0FFF) | i;	// Attachment index start from 1 until 4. index == 0 just means pev->origin

			MsgBroadcast(SVC_TEMPENTITY);
			WriteData(TE_BEAMFOLLOW);
			WriteData(iEntAtt);
			WriteData((short)Sprite::m_rgLibrary[Sprite::TRAIL]);
			WriteData((byte)4);		// life
			WriteData((byte)10);	// width
			WriteData((byte)255);	// r
			WriteData((byte)255);	// g
			WriteData((byte)255);	// b
			WriteData((byte)255);	// brightness
			MsgEnd();
		}

		for (; pJet && pTarget;)
		{
			TraceResult tr{};
			g_engfuncs.pfnTraceLine(pJet->pev->origin, pTarget->pev->origin, ignore_monsters, pJet.Get(), &tr);

			if (tr.flFraction > 0.99f)
			{
				pMissile = CPrecisionAirStrike::Create(pPlayer, pJet->pev->origin - Vector(0, 0, 2.5f), pTarget->pev->origin);
				pTarget->m_pMissile = pMissile;	// pTarget now has a missile binding to it.
				break;
			}

			co_await gpGlobals->frametime;	// try it every other frame.
		}

		for (; pJet;)
		{
			[[unlikely]]
			if (pJet->pev->origin.x >= 4096 || pJet->pev->origin.y >= 4096 || pJet->pev->origin.z >= 4096
				|| pJet->pev->origin.x <= -4096 || pJet->pev->origin.y <= -4096 || pJet->pev->origin.z <= -4096)	// pJet->IsInWorld() have a speed limit, but we don't need it here.
			{
				MsgBroadcast(SVC_TEMPENTITY);
				WriteData(TE_KILLBEAM);
				WriteData((short)pJet->entindex());
				MsgEnd();

				pJet->pev->flags |= FL_KILLME;
				co_return;
			}

			co_await 0.2f;
		}
	}
};
