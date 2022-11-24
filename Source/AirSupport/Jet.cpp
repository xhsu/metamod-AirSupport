import Jet;
import Menu;
import Message;
import Missile;
import Resources;

import UtlRandom;

Task CJet::Task_BeamAndSound() noexcept
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
}

Task CJet::Task_AirStrike() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[0];	// yield to Task_BeamAndSound();

	for (; m_pTarget;)
	{
		TraceResult tr{};
		g_engfuncs.pfnTraceLine(pev->origin, m_pTarget->pev->origin, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction > 0.99f)
		{
			m_pTarget->m_pMissile =	// pTarget now has a missile binding to it.
				CPrecisionAirStrike::Create(m_pPlayer, pev->origin - Vector(0, 0, 2.5f), m_pTarget->pev->origin);

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

Task CJet::Task_ClusterBomb() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[0];	// yield to Task_BeamAndSound();

	for (double flCurDist = 0, flRecordDist = std::numeric_limits<double>::max(); m_pTarget; flCurDist = (pev->origin.Make2D() - m_pTarget->pev->origin.Make2D()).LengthSquared())
	{
		if (flCurDist < flRecordDist)	// Jet is approaching
			flRecordDist = flCurDist;
		else
		{
			TraceResult tr{};
			g_engfuncs.pfnTraceLine(
				m_pTarget->pev->origin,
				Vector(m_pTarget->pev->origin.x, m_pTarget->pev->origin.y, 8192),
				ignore_glass | ignore_monsters,
				edict(),
				&tr
			);

			m_pTarget->m_pMissile = // pTarget now has a missile binding to it.
				CClusterBomb::Create(m_pPlayer, tr.vecEndPos - Vector(0, 0, 2.5), m_pTarget->pev->origin);

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
			WriteData(entindex());
			MsgEnd();

			pev->flags |= FL_KILLME;
			co_return;
		}

		co_await 0.1f;
	}
}

Task CJet::Task_CarpetBombardment() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[0];	// yield to Task_BeamAndSound();

	TraceResult tr{};
	std::array<double, CDynamicTarget::BEAM_COUNT> rgflRecDist{};
	std::array<bool, CDynamicTarget::BEAM_COUNT> rgbBombLaunched{};

	[&] <size_t... I>(std::index_sequence<I...> &&) noexcept
	{
		((rgflRecDist[I] = std::numeric_limits<double>::max()), ...);
		((rgbBombLaunched[I] = !!(m_pTarget->m_rgpBeams[I]->pev->effects & EF_NODRAW)), ...);
	}(std::make_index_sequence<CDynamicTarget::BEAM_COUNT>{});

	for (;;)
	{
		for (int i = 0; i < CDynamicTarget::BEAM_COUNT; ++i)
		{
			if (rgbBombLaunched[i])
				continue;

			auto const flCurDist = (m_pTarget->m_rgpBeams[i]->EndPos().Make2D() - pev->origin.Make2D()).LengthSquared();

			if (flCurDist < rgflRecDist[i])
				rgflRecDist[i] = flCurDist;
			else
			{
				g_engfuncs.pfnTraceLine(
					m_pTarget->m_rgpBeams[i]->EndPos(),
					m_pTarget->m_rgpBeams[i]->EndPos() + Vector(0, 0, 4096),
					ignore_glass | ignore_monsters,
					nullptr, &tr
				);

				m_pTarget->m_pMissile = CCarpetBombardment::Create(m_pPlayer, tr.vecEndPos, m_pTarget->m_rgpBeams[i]);
				rgbBombLaunched[i] = true;
			}
		}

		co_await TaskScheduler::NextFrame::Rank[0];	// try it every other frame.

		auto const bDone = [&]<size_t... I>(std::index_sequence<I...> &&) noexcept -> bool
		{
			return (rgbBombLaunched[I] && ...);
		}(std::make_index_sequence<CDynamicTarget::BEAM_COUNT>{});

		if (bDone)
			break;
	}

	for (;;)
	{
		[[unlikely]]
		if (!IsInWorld())
		{
			MsgBroadcast(SVC_TEMPENTITY);
			WriteData(TE_KILLBEAM);
			WriteData(entindex());
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
	pev->velocity = vecDir.Normalize() * (m_AirSupportType == CARPET_BOMBARDMENT ? 2048 : 4096);
	pev->effects |= EF_BRIGHTLIGHT;

	switch (m_AirSupportType)
	{
	default:
		gmsgTextMsg::Send(m_pPlayer->edict(), 4, u8"抱歉, 這邊還沒做好- -");
		[[fallthrough]];

	case AIR_STRIKE:
		m_Scheduler.Enroll(Task_AirStrike());
		break;

	case CLUSTER_BOMB:
		m_Scheduler.Enroll(Task_ClusterBomb());
		break;

	case CARPET_BOMBARDMENT:
		m_Scheduler.Enroll(Task_CarpetBombardment());
		break;
	}

	m_Scheduler.Enroll(Task_BeamAndSound());
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
	pPrefab->m_AirSupportType = pTarget->m_AirSupportType;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
