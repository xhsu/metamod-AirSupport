import Jet;
import Message;
import Missile;
import Resources;

import UtlRandom;

Task CJet::Task_Jet() noexcept
{
	//g_engfuncs.pfnEmitSound(pJet.Get(), CHAN_WEAPON, UTIL_GetRandomOne(Sounds::JET), VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 118));	// #INVESTIGATE
	g_engfuncs.pfnClientCommand(m_pPlayer->edict(), "spk %s\n", UTIL_GetRandomOne(Sounds::JET));
	co_await 0.01f;

	for (auto i = 0x1000; i <= 0x4000; i += 0x1000)
	{
		unsigned short const iEntAtt = (entindex() & 0x0FFF) | i;	// Attachment index start from 1 until 4. index == 0 just means pev->origin

		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_BEAMFOLLOW);
		WriteData(iEntAtt);
		WriteData((short)Sprites::m_rgLibrary[Sprites::TRAIL]);
		WriteData((byte)4);		// life
		WriteData((byte)10);	// width
		WriteData((byte)255);	// r
		WriteData((byte)255);	// g
		WriteData((byte)255);	// b
		WriteData((byte)255);	// brightness
		MsgEnd();
	}

	for (; m_pTarget;)
	{
		TraceResult tr{};
		g_engfuncs.pfnTraceLine(pev->origin, m_pTarget->pev->origin, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction > 0.99f)
		{
			m_pMissile = CPrecisionAirStrike::Create(m_pPlayer, pev->origin - Vector(0, 0, 2.5f), m_pTarget->pev->origin);
			m_pTarget->m_pMissile = m_pMissile;	// pTarget now has a missile binding to it.
			break;
		}

		co_await TaskScheduler::NextFrame::Rank[0];	// try it every other frame.
	}

	for (;;)
	{
		[[unlikely]]
		if (!IsInWorld())
		{
			MsgBroadcast(SVC_TEMPENTITY);
			WriteData(TE_KILLBEAM);
			WriteData((short)entindex());
			MsgEnd();

			pev->flags |= FL_KILLME;
			co_return;
		}

		co_await 0.1f;
	}
}

void CJet::Spawn() noexcept
{
	auto const vecDir = Vector(m_pTarget->pev->origin.x, m_pTarget->pev->origin.y, pev->origin.z) - pev->origin;

	g_engfuncs.pfnVecToAngles(vecDir, pev->angles);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PLANE[AIR_STRIKE]);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->velocity = vecDir.Normalize() * 4096;
	pev->effects |= EF_BRIGHTLIGHT;

	m_Scheduler.Enroll(Task_Jet());
}

qboolean CJet::IsInWorld() noexcept
{
	// position
	if (pev->origin.x >= 4096.0 || pev->origin.y >= 4096.0 || pev->origin.z >= 4096.0)
	{
		return false;
	}
	if (pev->origin.x <= -4096.0 || pev->origin.y <= -4096.0 || pev->origin.z <= -4096.0)
	{
		return false;
	}

	return true;	// the speed test is meaningless for jet classes.
}

CJet *CJet::Create(CBasePlayer *pPlayer, CFixedTarget *pTarget, Vector const &vecOrigin) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CJet>();

	pEdict->v.origin = vecOrigin;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_pTarget = pTarget;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
