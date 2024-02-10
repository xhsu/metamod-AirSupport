import Jet;
import Menu;
import Message;
import Projectile;
import Ray;
import Resources;
import Round;

import UtlRandom;

//
// CJet
//

static Vector UTIL_EstOrigin(Vector const& vecSrc, float const flSpeed, CBaseEntity* pEnemy, double const flLastTimeEst = 0, int32_t iIterate = 0) noexcept
{
	auto const vecPosEst = pEnemy->pev->origin + pEnemy->pev->velocity * flLastTimeEst;
	auto const flTimeEst = (vecSrc - vecPosEst).Length() / flSpeed;

	if (iIterate > 4)
		return vecPosEst;
	else
		return UTIL_EstOrigin(vecSrc, flSpeed, pEnemy, flTimeEst, iIterate + 1);
}

Task CJet::Task_BeamAndSound() noexcept
{
	for (edict_t *pEdict : (m_pPlayer->m_iTeam == TEAM_CT ? g_rgpPlayersOfTerrorist : g_rgpPlayersOfCT))
		g_engfuncs.pfnClientCommand(pEdict, "spk %s\n", Sounds::ALERT_AIRSTRIKE);	// AC130 will be played in another class.

	switch (m_AirSupportType)
	{
	case CARPET_BOMBARDMENT:
		g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::BOMBER[m_iFlyingSoundIndex], VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 118));
		break;

	default:
		g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::JET[m_iFlyingSoundIndex], VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 118));
		break;
	}

	co_await 0.1f;

	for (auto i = 0x1000; i <= 0x4000; i += 0x1000)
	{
		unsigned short const iEntAtt = (entindex() & 0x0FFF) | i;	// Attachment index start from 1 until 4. index == 0 just means pev->origin

		MsgAll(SVC_TEMPENTITY);
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

Task CJet::Task_StuckCheck() noexcept
{
	Vector vecLastOrigin{ 0, 0, -8192 };

	for (; !(pev->flags & FL_KILLME);)
	{
		co_await 0.1f;

		// The speed value can only be 2048 or 4096.
		// If the position of this jet does not change that much between frames, it must be stuck.

		[[likely]]
		if ((vecLastOrigin - pev->origin).LengthSquared() > (1500 * 1500))
			continue;

		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_KILLBEAM);
		WriteData((short)entindex());
		MsgEnd();

		pev->flags |= FL_KILLME;
		co_return;
	}
}

Task CJet::Task_AirStrike() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[1];	// yield to Task_BeamAndSound();

	Vector vecLaunchingSpot = pev->origin - Vector(0, 0, 8);
	trace_hull_functor_t fnTraceHull{ Vector(-7, -7, -7), Vector(7, 7, 7) };

	for (TraceResult tr{}; m_pTarget;)
	{
		vecLaunchingSpot = pev->origin - Vector(0, 0, 8);
		fnTraceHull(vecLaunchingSpot, m_pTarget->pev->origin, dont_ignore_monsters, edict(), &tr);

		// Sky height issue?
		if ((tr.flFraction > 0.95f || (m_pTarget->pev->origin - tr.vecEndPos).LengthSquared() < (10.0 * 10.0))
			&& !tr.fAllSolid && !tr.fStartSolid && tr.fInOpen)
		{
			auto const vecAimingAt = m_pTarget->m_pTargeting ?
				UTIL_EstOrigin(vecLaunchingSpot, (float)CVar::pas_proj_speed, m_pTarget->m_pTargeting) : m_pTarget->pev->origin;

			m_pTarget->m_pMissile =	// pTarget now has a missile binding to it.
				CPrecisionAirStrike::Create(m_pPlayer, vecLaunchingSpot, vecAimingAt, m_pTarget->m_pTargeting);

			break;
		}

		co_await TaskScheduler::NextFrame::Rank[1];	// try it every other frame.
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
	std::array<double, CDynamicTarget::BEACON_COUNT> rgflRecDist{};
	std::array<bool, CDynamicTarget::BEACON_COUNT> rgbBombLaunched{};

	[&] <size_t... I>(std::index_sequence<I...> &&) noexcept
	{
		((rgflRecDist[I] = std::numeric_limits<double>::max()), ...);
		((rgbBombLaunched[I] = !!(m_pTarget->m_rgpBeacons[I]->pev->effects & EF_NODRAW)), ...);
	}(std::make_index_sequence<CDynamicTarget::BEACON_COUNT>{});

	for (;;)
	{
		for (int i = 0; i < CDynamicTarget::BEACON_COUNT; ++i)
		{
			if (rgbBombLaunched[i] || !m_pTarget->m_rgpBeacons[i])
				continue;

			auto const flCurDist = (m_pTarget->m_rgpBeacons[i]->EndPos().Make2D() - pev->origin.Make2D()).LengthSquared();

			if (flCurDist < rgflRecDist[i])
				rgflRecDist[i] = flCurDist;
			else
			{
				g_engfuncs.pfnTraceLine(
					m_pTarget->m_rgpBeacons[i]->EndPos(),
					m_pTarget->m_rgpBeacons[i]->EndPos() + Vector(0, 0, 4096),
					ignore_glass | ignore_monsters,
					nullptr, &tr
				);

				m_pTarget->m_pMissile = CCarpetBombardment::Create(m_pPlayer, tr.vecEndPos, m_pTarget->m_rgpBeacons[i]);
				rgbBombLaunched[i] = true;
			}
		}

		co_await TaskScheduler::NextFrame::Rank[0];	// try it every other frame.

		auto const bDone = [&]<size_t... I>(std::index_sequence<I...> &&) noexcept -> bool
		{
			return (rgbBombLaunched[I] && ...);
		}(std::make_index_sequence<CDynamicTarget::BEACON_COUNT>{});

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

Task CJet::Task_FuelAirBomb() noexcept
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
				CFuelAirExplosive::Create(m_pPlayer, tr.vecEndPos - Vector(0, 0, 2.5));

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

Task CJet::Task_Phosphorus() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[1];	// yield to Task_BeamAndSound();

	trace_hull_functor_t fnTraceHull{ Vector(-7, -7, -7), Vector(7, 7, 7) };

	for (TraceResult tr{}; m_pTarget;)
	{
		if (auto const flDist = (pev->origin - m_pTarget->pev->origin).Length(); flDist < 800)
			goto LAB_CONTINUE;

		fnTraceHull(pev->origin - Vector(0, 0, 8), m_pTarget->pev->origin, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction > 0.95f && !tr.fAllSolid && !tr.fStartSolid && tr.fInOpen)
		{
			m_pTarget->m_pMissile =	// pTarget now has a missile binding to it.
				CIncendiaryMunition::Create(m_pPlayer, pev->origin - Vector(0, 0, 8), m_pTarget->pev->origin);

			break;
		}

	LAB_CONTINUE:;
		co_await TaskScheduler::NextFrame::Rank[1];	// try it every other frame.
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
	g_engfuncs.pfnSetModel(edict(), Models::PLANE[m_AirSupportType]);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->velocity = vecDir.Normalize() * (m_AirSupportType == CARPET_BOMBARDMENT ? 2048 : 4096);
	pev->effects |= EF_BRIGHTLIGHT;

	switch (m_AirSupportType)
	{
	default:
		gmsgTextMsg::Send(m_pPlayer->edict(), 4, u8"你不應該看到這則信息 - 請聯絡作者\nYou shouldn't see this text!");
		[[fallthrough]];

	case AIR_STRIKE:
		m_Scheduler.Enroll(Task_AirStrike());
		m_iFlyingSoundIndex = UTIL_Random(0u, Sounds::JET.size() - 1);
		break;

	case CLUSTER_BOMB:
		m_Scheduler.Enroll(Task_ClusterBomb());
		m_iFlyingSoundIndex = UTIL_Random(0u, Sounds::JET.size() - 1);
		break;

	case CARPET_BOMBARDMENT:
		m_Scheduler.Enroll(Task_CarpetBombardment());
		m_iFlyingSoundIndex = UTIL_Random(0u, Sounds::BOMBER.size() - 1);
		break;

	case FUEL_AIR_BOMB:
		m_Scheduler.Enroll(Task_FuelAirBomb());
		m_iFlyingSoundIndex = UTIL_Random(0u, Sounds::BOMBER.size() - 1);
		break;

	case PHOSPHORUS_MUNITION:
		m_Scheduler.Enroll(Task_Phosphorus());
		m_iFlyingSoundIndex = UTIL_Random(0u, Sounds::JET.size() - 1);
		break;
	}

	m_Scheduler.Enroll(Task_BeamAndSound());
	m_Scheduler.Enroll(Task_StuckCheck());
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

//
// CGunship
//

Task CGunship::Task_Gunship() noexcept
{
	for (edict_t *pEdict : (m_pPlayer->m_iTeam == TEAM_CT ? g_rgpPlayersOfTerrorist : g_rgpPlayersOfCT))
		g_engfuncs.pfnClientCommand(pEdict, "spk %s\n", Sounds::ALERT_AC130);

	g_engfuncs.pfnEmitAmbientSound(m_pPlayer->edict(), m_pPlayer->pev->origin, Sounds::Gunship::NOISE_PILOT, VOL_NORM, ATTN_STATIC, 0, PITCH_NORM);
	g_engfuncs.pfnEmitAmbientSound(m_pPlayer->edict(), m_pPlayer->pev->origin, Sounds::Gunship::AC130_IS_IN_AIR, VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::Gunship::AC130_AMBIENT[m_iAmbientSoundIndex], VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 108));

	co_await (float)g_rgflSoundTime.at(Sounds::Gunship::AC130_IS_IN_AIR);

	for (TraceResult tr{}; m_pTarget;)
	{
		auto &pEnemy = m_pTarget->m_pTargeting;

		// Hand the control back to CFixedTarget if nothing gets locked.

		if (!pEnemy)
		{
			co_await 0.1f;
			continue;
		}

		//
		// Aiming something
		//

		if (pEnemy->entindex() == 0)	// refuse to shoot the ground.
		{
			pEnemy = nullptr;
			co_await 0.1f;
			continue;
		}
		else if (!pEnemy->IsAlive())
		{
			std::string_view const szKillConfirmed = UTIL_GetRandomOne(Sounds::Gunship::KILL_CONFIRMED);

			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_AUTO, szKillConfirmed.data(), VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));

			pEnemy = nullptr;	// Only after we set it to null will the CFixedTarget to find another target.
			co_await (float)(g_rgflSoundTime.at(szKillConfirmed) + 0.1);
			continue;
		}

		// Instead of our own position. This is because of the drifting VFX under this mode.
		Vector const &vecAimingPos = pEnemy->pev->origin;

		// Is the old sky pos not good anymore?
		g_engfuncs.pfnTraceLine(pev->origin, vecAimingPos, ignore_monsters | ignore_glass, nullptr, &tr);

		if (tr.flFraction < 1 || tr.fAllSolid || tr.fStartSolid)
		{
			// Theoratically CFixedTarget should follow the m_pTargeting. But due to the drifting vfx, it's not accurate.
			g_engfuncs.pfnTraceLine(vecAimingPos, Vector(vecAimingPos.x, vecAimingPos.y, 8192.0), ignore_monsters | ignore_glass, nullptr, &tr);

			if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)	// This guy runs into shelter.
			{
				g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_AUTO, Sounds::Gunship::TARGET_RAN_TO_COVER, VOL_NORM, ATTN_STATIC, 0, PITCH_NORM);
				pEnemy = nullptr;
				continue;
			}

			// okay, we are moving to a new location.
			g_engfuncs.pfnSetOrigin(edict(), Vector(tr.vecEndPos.x, tr.vecEndPos.y, tr.vecEndPos.z - 1.0));

			// Rest for 1 frame.
			co_await TaskScheduler::NextFrame::Rank[0];
		}

		Vector const vecTargetLocation =
			pEnemy->IsPlayer() ?
			UTIL_GetHeadPosition(pEnemy.Get()) : pEnemy->Center();	// Sometimes aiming the head is not a good option - the bullet needs to fly to there and it's very likely to miss it.

		Vector const vecDir = (vecTargetLocation + pEnemy->pev->velocity * CBullet::AC130_BULLET_EXPECTED_TRAVEL_TIME) - pev->origin;

		/* This is not the exact time. The exact time must be calced by (origin + vel * t), but the t is what we required here. */
		//auto const flEstimateTime = (vecTargetLocation - pev->origin).Length() / CBullet::AC130_BULLET_SPEED;
		auto const flSpeed = vecDir.Length() / CBullet::AC130_BULLET_EXPECTED_TRAVEL_TIME;

		CBullet::Create(
			pev->origin,
			/* with position prediction */
			//((vecTargetLocation + pEnemy->pev->velocity * flEstimateTime) - pev->origin).Normalize() * CBullet::AC130_BULLET_SPEED,
			vecDir.Normalize() * flSpeed,
			m_pPlayer
		);

		//gmsgTextMsg::Send(m_pPlayer->edict(), 3, std::format("{}", flSpeed).c_str());

		g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::Gunship::AC130_FIRE_25MM), VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 116));
		co_await (60.f / std::max(1.f, (float)CVar::gs_rpm));	// firerate.
	}

	// "Reloading"
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::Gunship::AC130_RELOAD[m_iReloadSoundIndex], VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 116));
	co_await (float)g_rgflSoundTime.at(Sounds::Gunship::AC130_RELOAD[m_iReloadSoundIndex]);

	// Randomly select another SFX as AC130 moving out, with fadeout fx. Stop the old one first.
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::Gunship::AC130_AMBIENT[m_iAmbientSoundIndex], VOL_NORM, ATTN_NONE, SND_STOP, UTIL_Random(92, 108));
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::Gunship::AC130_DEPARTURE[m_iDepartureSoundIndex], VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 108));
	co_await (float)g_rgflSoundTime.at(Sounds::Gunship::AC130_DEPARTURE[m_iDepartureSoundIndex]);

	// Stop the radio background noise
	g_engfuncs.pfnEmitAmbientSound(m_pPlayer->edict(), m_pPlayer->pev->origin, Sounds::Gunship::NOISE_PILOT, VOL_NORM, ATTN_STATIC, SND_STOP, PITCH_NORM);

	// Die with CFixedTarget
	pev->flags |= FL_KILLME;
}

void CGunship::Spawn() noexcept
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->gravity = 0;
	pev->effects = EF_NODRAW;

	s_bInstanceExists = true;
	m_iAmbientSoundIndex = UTIL_Random(0u, Sounds::Gunship::AC130_AMBIENT.size() - 1);
	m_iDepartureSoundIndex = UTIL_Random(0u, Sounds::Gunship::AC130_DEPARTURE.size() - 1);
	m_iReloadSoundIndex = UTIL_Random(0u, Sounds::Gunship::AC130_RELOAD.size() - 1);

	m_Scheduler.Enroll(Task_Gunship());
}

CGunship *CGunship::Create(CBasePlayer *pPlayer, CFixedTarget *pTarget) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CGunship>();

	pEdict->v.origin = Vector(8192, 8192, 8192);	// pending on the first run auto correction

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_pTarget = pTarget;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
