#include <cassert>

import <array>;
import <numbers>;

import progdefs;
import util;

import Beam;
import Jet;
import Localization;
import Resources;
import Round;
import Target;
import Waypoint;
import Weapon;

import UtlRandom;

//
// Laser
// 

extern "C++" namespace Laser
{
	void Create(CBasePlayerWeapon *pWeapon) noexcept
	{
		auto const pBeam = CBeam::BeamCreate(Sprites::BEAM, 32.f);

		pBeam->SetFlags(BEAM_FSHADEOUT);	// fade out on rear end.
		pBeam->PointsInit(pWeapon->m_pPlayer->pev->origin, pWeapon->m_pPlayer->GetGunPosition());

		pBeam->pev->classname = MAKE_STRING("CarpetBombardmentIndicator");
		pBeam->pev->effects |= EF_NODRAW;
		pBeam->pev->renderfx = kRenderFxNone;
		pBeam->pev->nextthink = 0.1f;
		pBeam->pev->euser1 = pWeapon->m_pPlayer->edict();	// pev->owner gets occupied by 'starting ent'
		pBeam->pev->team = pWeapon->m_pPlayer->m_iTeam;
	}

	void Think(CBaseEntity *pEntity) noexcept
	{
		[[unlikely]]
		if (pev_valid(pEntity->pev->euser1) != 2 || !((CBasePlayer *)pEntity->pev->euser1->pvPrivateData)->IsAlive())
		{
			pEntity->pev->flags |= FL_KILLME;
			//pEntity->pev->euser2->v.flags |= FL_KILLME;
			return;
		}

		pEntity->pev->nextthink = gpGlobals->time + 0.4f;

		if (pEntity->pev->effects & EF_NODRAW)
			return;

		auto const pPlayer = (CBasePlayer *)pEntity->pev->euser1->pvPrivateData;
		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		Vector vecSrc = pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		TraceResult tr{};

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pEntity->pev->euser1, &tr);

		Beam_SetStartPos(pEntity->pev, tr.vecEndPos);
		Beam_SetEndPos(pEntity->pev, tr.vecEndPos + tr.vecPlaneNormal * 96);
	}
};

//
// CDynamicTarget
// Representing the "only I can see" target model when player holding radio.
//

CDynamicTarget::~CDynamicTarget() noexcept
{
	for (auto &&pBeam : m_rgpBeams)
	{
		if (pBeam)
			pBeam->pev->flags |= FL_KILLME;
	}
}

Task CDynamicTarget::Task_Animation() noexcept
{
	// Consider this as initialization
	m_LastAnimUpdate = std::chrono::high_resolution_clock::now();

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await TaskScheduler::NextFrame::Rank[1];	// behind coord update.

		auto const CurTime = std::chrono::high_resolution_clock::now();
		auto const flTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(CurTime - m_LastAnimUpdate).count() / 1'000'000'000.0;

		pev->framerate = float(Models::targetmdl::FPS * flTimeDelta);
		pev->frame += pev->framerate;
		pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pev->frame < 0 || pev->frame >= 256)
			pev->frame -= float((pev->frame / 256.0) * 256.0);	// model sequence is different from SPRITE, no matter now many frame you have, it will stretch/squeeze into 256.

		m_LastAnimUpdate = CurTime;
	}
}

Task CDynamicTarget::Task_DeepEvaluation() noexcept
{
	size_t iCounter = 0;
	TraceResult tr{};

	for (;;)
	{
		co_await(gpGlobals->frametime / 3.f);

		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		[[unlikely]]
		if (pev->skin == Models::targetmdl::SKIN_GREEN)
		{
			co_return;	// It's done, stop current evaluation.
		}

		// Try to get a temp spawn location above player.
		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd{ vecSrc.x, vecSrc.y, 8192.f };

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters | ignore_glass, nullptr, &tr);

		if (Vector const vecSavedCandidate = tr.vecEndPos; g_engfuncs.pfnPointContents(vecSavedCandidate) == CONTENTS_SKY)
		{
			g_engfuncs.pfnTraceLine(vecSavedCandidate, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

			if (tr.flFraction > 0.99f)
			{
				co_await 0.1f;	// avoid red-green flashing.

				pev->skin = Models::targetmdl::SKIN_GREEN;
				co_return;
			}
		}

		iCounter = 0;

		for (const auto &vec : g_WaypointMgr.m_rgvecOrigins)
		{
			++iCounter;
			g_engfuncs.pfnTraceLine(pev->origin, vec, ignore_monsters | ignore_glass, nullptr, &tr);

			[[unlikely]]
			if (tr.flFraction > 0.99f)
			{
				co_await 0.1f;	// avoid red-green flashing.

				pev->skin = Models::targetmdl::SKIN_GREEN;
				co_return;
			}

			[[unlikely]]
			if (!(iCounter % 256))
			{
				co_await(gpGlobals->frametime / 3.f);	// gurentee resume next frame. div by 3 is to ensure priority
			}
		}
	}
}

Task CDynamicTarget::Task_QuickEval_AirStrike() noexcept
{
	TraceResult tr{};

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		// Update team info so we can hide from proper player group.

		pev->team = m_pPlayer->m_iTeam;

		if (m_pPlayer->pev->effects & EF_DIMLIGHT)	// The lit state will follow player flashlight
			pev->effects |= EF_DIMLIGHT;
		else
			pev->effects &= ~EF_DIMLIGHT;

		// Calc where does player aiming

		g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		UTIL_TraceLine(vecSrc, vecEnd, m_pPlayer->edict(), m_pPlayer->m_iTeam == TEAM_CT ? g_rgpCTs : g_rgpTers, &tr);	// Special traceline skipping all teammates.

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY)
		{
			// One cannot ask air support to 'hit the sky'

			m_Scheduler.Delist(DETAIL_ANALYZE_KEY);	// Stop evaluation now.
			m_pTargeting = nullptr;

			pev->skin = Models::targetmdl::SKIN_RED;
			goto LAB_CONTINUE;	// there's no set origin.
		}

		if (pev_valid(tr.pHit) != 2 && m_flLastValidTracking < gpGlobals->time - 0.5f)	// Snapping: compensenting bad aiming
			m_pTargeting = tr.pHit;
		else if (pev_valid(tr.pHit) == 2)
		{
			m_pTargeting = tr.pHit;
			m_flLastValidTracking = gpGlobals->time;
		}

		if (m_pTargeting && !m_pTargeting->IsBSPModel() && m_pTargeting->IsAlive())
		{
			pev->angles = Vector::Zero();	// facing up.

			Vector const vecCenter = m_pTargeting->Center();
			g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));	// snap to target.
		}
		else
		{
			g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pev->angles);
			pev->angles.x += 270.f;	// don't know why, but this is the deal.

			g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);
		}

		// Quick Evaluation

		if ((pev->origin - m_vecLastAiming).LengthSquared() > 24.0 * 24.0)
		{
			// Remove old deep think
			m_Scheduler.Delist(DETAIL_ANALYZE_KEY);

			// Is it under sky?
			g_engfuncs.pfnTraceLine(
				pev->origin,
				Vector(pev->origin.x, pev->origin.y, 8192),
				ignore_monsters | ignore_glass,
				nullptr, &tr
			);

			if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
			{
				// Start on deep analyze
				m_Scheduler.Enroll(Task_DeepEvaluation(), DETAIL_ANALYZE_KEY);

				pev->skin = Models::targetmdl::SKIN_RED;
			}
			else
			{
				pev->skin = Models::targetmdl::SKIN_GREEN;
			}

			m_vecLastAiming = pev->origin;
		}

	LAB_CONTINUE:;
		co_await TaskScheduler::NextFrame::Rank[0];
	}
}

Task CDynamicTarget::Task_QuickEval_ClusterBomb() noexcept
{
	TraceResult tr{};

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		// Update team info so we can hide from proper player group.

		pev->team = m_pPlayer->m_iTeam;

		if (m_pPlayer->pev->effects & EF_DIMLIGHT)	// The lit state will follow player flashlight
			pev->effects |= EF_DIMLIGHT;
		else
			pev->effects &= ~EF_DIMLIGHT;

		// Calc where does player aiming

		g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		//g_engfuncs.pfnTraceMonsterHull(edict(), vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);

		auto const flAngleLean = std::acos(DotProduct(Vector::Up(), tr.vecPlaneNormal)/* No div len required, both len are 1. */) / std::numbers::pi * 180.0;

		if (flAngleLean > 50)
		{
			// Surface consider wall and no cluster bomb allow against wall.

			pev->skin = Models::targetmdl::SKIN_RED;
			goto LAB_CONTINUE;	// there's no set origin.
		}

		g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pev->angles);
		pev->angles.x += 270.f;	// don't know why, but this is the deal.

		g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);

		// Quick Evaluation

		// Is it under sky?
		g_engfuncs.pfnTraceLine(
			pev->origin,
			Vector(pev->origin.x, pev->origin.y, 8192),
			ignore_monsters | ignore_glass,
			nullptr, &tr
		);

		// No deep evaluation on cluster bomb mode. The bomb has no propulsion or engine.
		// It must be drop directly from sky.

		pev->skin =
			g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY ?
			Models::targetmdl::SKIN_GREEN :
			Models::targetmdl::SKIN_RED;

	LAB_CONTINUE:;
		co_await TaskScheduler::NextFrame::Rank[0];
	}
}

Task CDynamicTarget::Task_QuickEval_CarpetBombardment() noexcept
{
	TraceResult tr{}, tr2{};

	for (auto &&pBeam : m_rgpBeams)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		pBeam = CBeam::BeamCreate(Sprites::BEAM, 32.f);

		pBeam->SetFlags(BEAM_FSHADEOUT);	// fade out on rear end.
		pBeam->PointsInit(pev->origin, Vector(0, 0, 32));

		pBeam->pev->classname = MAKE_STRING("CarpetBombardmentIndicator");
		pBeam->pev->renderfx = kRenderFxNone;
		pBeam->pev->effects |= EF_NODRAW;
		pBeam->pev->nextthink = 0.1f;
		pBeam->pev->euser1 = m_pPlayer->edict();	// pev->owner gets occupied by 'starting ent'
		pBeam->pev->team = m_pPlayer->m_iTeam;
	}

	for (;;)
	{
		[[unlikely]]
		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await TaskScheduler::NextFrame::Rank[0];

		// Update team info so we can hide from proper player group.

		pev->team = m_pPlayer->m_iTeam;

		if (m_pPlayer->pev->effects & EF_DIMLIGHT)	// The lit state will follow player flashlight
			pev->effects |= EF_DIMLIGHT;
		else
			pev->effects &= ~EF_DIMLIGHT;

		// Calc where does player aiming

		g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		g_engfuncs.pfnTraceMonsterHull(edict(), vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);
		
		// Is the location considered to be a wall?
		if (auto const flAngleLean = std::acos(DotProduct(Vector::Up(), tr.vecPlaneNormal)/* No div len required, both len are 1. */) / std::numbers::pi * 180.0;
			flAngleLean > 50)
		{
			// Surface consider wall and no carpet bombardment allow against wall.

			pev->skin = Models::targetmdl::SKIN_RED;
			continue;
		}

		Vector const vecAiming = tr.vecEndPos;
		Vector const vecSurfNorm = tr.vecPlaneNormal;

		// Is it under sky?
		g_engfuncs.pfnTraceLine(
			vecAiming,
			Vector(vecAiming.x, vecAiming.y, 8192),
			ignore_monsters | ignore_glass,
			nullptr, &tr
		);

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
		{
			// Differ from other evaluation, this mode does not allow the target model appears under any roof.

			pev->skin = Models::targetmdl::SKIN_RED;
			continue;
		}

		Vector const vecDir = Vector((vecEnd - vecSrc).Normalize().Make2D(), 0).Normalize();
		g_engfuncs.pfnTraceLine(
			tr.vecEndPos - Vector(0, 0, 16),
			tr.vecEndPos - Vector(0, 0, 16) + vecDir * (CARPET_BOMBARDMENT_INTERVAL * BEAM_COUNT / 2),
			ignore_glass | ignore_monsters,
			nullptr, &tr2
		);	// Using tr2 to avoid wiping sky pos data.

		if (tr2.flFraction < 0.51 || tr2.fAllSolid)
		{
			// There are something in the sky, flying route not available.

			pev->skin = Models::targetmdl::SKIN_RED;
			continue;
		}

		// The aiming location is good to go.

		g_engfuncs.pfnVecToAngles(vecSurfNorm, pev->angles);
		pev->angles.x += 270.f;	// don't know why, but this is the deal.

		g_engfuncs.pfnSetOrigin(edict(), vecAiming);
		pev->skin = Models::targetmdl::SKIN_GREEN;

		// Setup the locations of beams.

		Vector const &vecForward = vecDir;
		Vector const vecRight
		{
			vecForward.x * 0.0/*std::cos(270)*/ - vecForward.y * -1.0/*std::sin(270)*/,
			vecForward.x * -1.0/*std::sin(270)*/ + vecForward.y * 0.0/*std::cos(270)*/,
			0.0
		};
		auto const flMaxDistFwd = tr2.flFraction * (CARPET_BOMBARDMENT_INTERVAL * BEAM_COUNT / 2);

		for (double flFw = 0, flRt = -48; auto &&pBeam : m_rgpBeams)
		{
			if (flFw > flMaxDistFwd)
			{
				pBeam->pev->effects |= EF_NODRAW;
				goto LAB_CONTINUE;
			}

			pBeam->pev->origin = tr.vecEndPos + vecForward * flFw + vecRight * flRt;
			pBeam->pev->effects &= ~EF_NODRAW;

			g_engfuncs.pfnTraceLine(
				pBeam->pev->origin,
				Vector(pBeam->pev->origin.Make2D(), -8192.0),
				ignore_glass | ignore_monsters,
				nullptr, &tr2
			);

			pBeam->StartPos() = tr2.vecEndPos;
			pBeam->EndPos() = tr2.vecEndPos + Vector(0, 0, 64);

		LAB_CONTINUE:;
			if (flRt > 0)
				flFw += CARPET_BOMBARDMENT_INTERVAL;

			flRt = -flRt;
		}
	}
}

Task CDynamicTarget::Task_Remove() noexcept
{
	for (;;)
	{
		co_await gpGlobals->frametime;

		[[unlikely]]
		if (!m_pPlayer->IsAlive()	// Including "disconnection", since client drop out will cause pev->deadflag == DEAD_DEAD
			|| !m_pRadio
			)
		{
			pev->flags |= FL_KILLME;
			co_return;
		}
	}
}

void CDynamicTarget::UpdateEvalMethod() noexcept
{
	m_Scheduler.Delist(QUICK_ANALYZE_KEY);

	for (auto &&pBeam : m_rgpBeams)
	{
		if (pBeam)
			pBeam->pev->flags |= FL_KILLME;

		pBeam = nullptr;
	}

	switch (g_rgiAirSupportSelected[m_pPlayer->entindex()])
	{
	default:
	case AIR_STRIKE:
		g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());
		m_Scheduler.Enroll(Task_QuickEval_AirStrike(), QUICK_ANALYZE_KEY);
		pev->body = 1;
		break;

	case CLUSTER_BOMB:
		g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, 0), Vector(32, 32, 72));
		m_Scheduler.Enroll(Task_QuickEval_ClusterBomb(), QUICK_ANALYZE_KEY);
		pev->body = 4;
		break;

	case CARPET_BOMBARDMENT:
		g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());
		m_Scheduler.Enroll(Task_QuickEval_CarpetBombardment(), QUICK_ANALYZE_KEY);
		pev->body = 2;
		break;
	}
}

void CDynamicTarget::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), m_pPlayer->pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::TARGET);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxPulseFastWide;
	pev->renderamt = 128;
	pev->team = m_pPlayer->m_iTeam;

	m_Scheduler.Enroll(Task_Animation());
	m_Scheduler.Enroll(Task_Remove());

	UpdateEvalMethod();
}

CDynamicTarget *CDynamicTarget::Create(CBasePlayer *pPlayer, CBasePlayerWeapon *pRadio) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CDynamicTarget>();

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_pRadio = pRadio;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CFixedTarget
//

Task CFixedTarget::Task_PrepareJetSpawn() noexcept
{
	TraceResult tr{};
	size_t iCounter = 0;

	// The carpet bombardment is different from all others.

	if (m_AirSupportType == CARPET_BOMBARDMENT)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		auto const vecDir = Vector((m_rgpBeams[2]->pev->origin - m_rgpBeams[0]->pev->origin).Make2D(), 0).Normalize();

		g_engfuncs.pfnTraceLine(
			m_vecPosForJetSpawnTesting,
			m_vecPosForJetSpawnTesting - vecDir * 8192.0,
			ignore_glass | ignore_monsters,
			nullptr, &tr
		);

		m_vecJetSpawn = tr.vecEndPos;
		co_return;	// It's just that simple.
	}

	for (;;)
	{
		[[unlikely]]
		if (m_vecJetSpawn != Vector::Zero())
		{
			co_return;	// Nothing to do here, mate.
		}

		iCounter = 0;

		if (m_pTargeting)
		{
			g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
			m_vecPosForJetSpawnTesting = (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY) ? (tr.vecEndPos - Vector(0, 0, 16)) : pev->origin;
		}

		for (const auto &vec : g_WaypointMgr.m_rgvecOrigins)
		{
			++iCounter;
			g_engfuncs.pfnTraceLine(m_vecPosForJetSpawnTesting, vec, ignore_monsters | ignore_glass, nullptr, &tr);

			[[unlikely]]
			if (tr.flFraction > 0.99f)
			{
				m_vecJetSpawn = vec;
				co_return;
			}

			[[unlikely]]
			if (!(iCounter % 128))
				co_await TaskScheduler::NextFrame::Rank[0];
		}

		co_await TaskScheduler::NextFrame::Rank[0];

		// Additional attempt 1: from the pos when player called?

		g_engfuncs.pfnTraceLine(m_vecPlayerPosWhenCalled, Vector(m_vecPlayerPosWhenCalled.x, m_vecPlayerPosWhenCalled.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
		if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY)
		{
			g_engfuncs.pfnTraceLine(m_vecPosForJetSpawnTesting, tr.vecEndPos, ignore_monsters | ignore_glass, nullptr, &tr);

			[[unlikely]]
			if (tr.flFraction > 0.99f)
			{
				m_vecJetSpawn = tr.vecEndPos;
				co_return;
			}
		}

		// Additional attempt 2: from the current position of player?

		g_engfuncs.pfnTraceLine(m_pPlayer->pev->origin, Vector(m_pPlayer->pev->origin.x, m_pPlayer->pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
		if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY)
		{
			g_engfuncs.pfnTraceLine(m_vecPosForJetSpawnTesting, tr.vecEndPos, ignore_monsters | ignore_glass, nullptr, &tr);

			[[unlikely]]
			if (tr.flFraction > 0.99f)
			{
				m_vecJetSpawn = tr.vecEndPos;
				co_return;
			}
		}

		co_await TaskScheduler::NextFrame::Rank[0];
	}
}

Task CFixedTarget::Task_RecruitJet() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[1];	// one last chance.

	if (m_vecJetSpawn == Vector::Zero())
	{
		gmsgTextMsg::Send(m_pPlayer->edict(), (byte)4, Localization::REJECT_NO_JET_SPAWN);
		pev->flags |= FL_KILLME;
		co_return;
	}

	EHANDLE<CJet> pJet = CJet::Create(m_pPlayer, this, m_vecJetSpawn);

	for (;;)
	{
		[[unlikely]]
		if (!pJet)	// Jet found no way to launch missile
		{
			gmsgTextMsg::Send(m_pPlayer->edict(), (byte)4, Localization::REJECT_NO_VALID_TRACELINE);
			pev->flags |= FL_KILLME;
			co_return;
		}

		[[unlikely]]
		if (m_pMissile)	// Waiting for a missile binding to it.
			break;

		co_await TaskScheduler::NextFrame::Rank[1];
	}

	for (;;)
	{
		if (!m_pMissile)	// Missile entity despawned.
		{
			pev->flags |= FL_KILLME;	// So should this.
			co_return;
		}

		co_await TaskScheduler::NextFrame::Rank[1];
	}

	co_return;
}

Task CFixedTarget::Task_TimeOut() noexcept
{
	co_await 10.f;

	gmsgTextMsg::FreeBegin<MSG_ONE>(Vector::Zero(), m_pPlayer->edict(), (byte)4, Localization::REJECT_TIME_OUT);

	pev->flags |= FL_KILLME;
}

Task CFixedTarget::Task_UpdateOrigin() noexcept
{
	for (; m_pTargeting && m_pTargeting->IsAlive();)
	{
		Vector const vecCenter = m_pTargeting->Center();
		g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));

		co_await TaskScheduler::NextFrame::Rank[1];
	}
}

void CFixedTarget::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::TARGET);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;	// Fuck the useless MOVETYPE_FOLLOW
	pev->effects |= EF_DIMLIGHT;
	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxDistort;
	pev->renderamt = 0;
	pev->skin = Models::targetmdl::SKIN_BLUE;
	pev->body = m_pDynamicTarget->pev->body;
	pev->nextthink = 0.1f;
	pev->team = m_pPlayer->m_iTeam;

	m_vecPlayerPosWhenCalled = m_pPlayer->pev->origin;
	m_rgpBeams = m_pDynamicTarget->m_rgpBeams;
	m_pDynamicTarget->m_rgpBeams.fill(nullptr);	// Transfer the ownership of these entities to the fixed targets.

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	m_vecPosForJetSpawnTesting = (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY) ? (tr.vecEndPos - Vector(0, 0, 16)) : pev->origin;

	m_Scheduler.Enroll(Task_PrepareJetSpawn());
	m_Scheduler.Enroll(Task_TimeOut());
	m_Scheduler.Enroll(Task_UpdateOrigin());
}

void CFixedTarget::Activate() noexcept
{
	[[likely]]
	if (!m_Scheduler.Exist(RADIO_KEY))
		m_Scheduler.Enroll(Task_RecruitJet(), RADIO_KEY);
}

CFixedTarget *CFixedTarget::Create(CDynamicTarget *const pDynamicTarget) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CFixedTarget>();

	pEdict->v.angles = pDynamicTarget->pev->angles;
	pEdict->v.origin = pDynamicTarget->pev->origin;

	pPrefab->m_pTargeting = pDynamicTarget->m_pTargeting;
	pPrefab->m_pPlayer = pDynamicTarget->m_pPlayer;
	pPrefab->m_pDynamicTarget = pDynamicTarget;
	pPrefab->m_AirSupportType = g_rgiAirSupportSelected[pDynamicTarget->m_pPlayer->entindex()];
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
