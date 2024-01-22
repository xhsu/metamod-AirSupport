#include <cassert>

import <chrono>;
import <numbers>;

import Beam;
import Effects;
import Jet;
import Localization;
import Projectile;
import Query;
import Ray;
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
	for (auto &&pBeam : m_rgpBeacons)
	{
		if (pBeam)
			pBeam->pev->flags |= FL_KILLME;
	}
}

Task CDynamicTarget::Task_Animation() noexcept
{
	// This variable should never be a part of the class. It doesn't even being ref anywhere else.
	auto LastAnimUpdate = std::chrono::high_resolution_clock::now();
	auto vecHeadOrg = Vector::Zero();

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await TaskScheduler::NextFrame::Rank[1];	// behind coord update.

		if (m_pTargeting && m_pTargeting->IsPlayer())
		{
			vecHeadOrg = UTIL_GetHeadPosition(m_pTargeting.Get());
			UTIL_SetController(&pev->controller[1], &MINIATURE_CONTROLLER, (double)vecHeadOrg.z - (double)m_pTargeting->pev->absmin.z);
		}
		else
			pev->controller[1] = (uint8_t)MINIATURE_CONTROLLER.rest;

		if (m_bFreezed)
		{
			// When player is holding LMB, just stop rotation.

			pev->framerate = 0;
			pev->frame = 0;
			pev->animtime = 0;

			continue;
		}

		auto const CurTime = std::chrono::high_resolution_clock::now();
		auto const flTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(CurTime - LastAnimUpdate).count() / 1'000'000'000.0;

		pev->framerate = float(Models::targetmdl::FPS * flTimeDelta);
		pev->frame += pev->framerate;
		pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pev->frame < 0 || pev->frame >= 256)
			pev->frame -= float((pev->frame / 256.0) * 256.0);	// model sequence is different from SPRITE, no matter now many frame you have, it will stretch/squeeze into 256.

		LastAnimUpdate = CurTime;
	}
}

Task CDynamicTarget::Task_DeepEval_AirStrike() noexcept
{
	size_t iCounter = 0;
	TraceResult tr{};

	for (;;)
	{
		co_await(gpGlobals->frametime / 3.f);

		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
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

Task CDynamicTarget::Task_DeepEval_Phosphorus() noexcept
{
	trace_arc_functor_t fnTraceArc{};
	size_t iCounter = 0;

	g_engfuncs.pfnSetSize(fnTraceArc, Vector(-4, -4, -4), Vector(4, 4, 4));

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		[[unlikely]]
		if (pev->skin == Models::targetmdl::SKIN_GREEN)
		{
			co_return;	// It's done, stop current evaluation.
		}

		iCounter = 0;

		for (const auto &vec : g_WaypointMgr.m_rgvecOrigins)
		{
			++iCounter;

			[[unlikely]]
			if (fnTraceArc(vec, pev->origin, ignore_monsters | ignore_glass))
			{
				co_await 0.1f;	// avoid red-green flashing.

				pev->skin = Models::targetmdl::SKIN_GREEN;
				co_return;
			}

			[[unlikely]]
			if (!(iCounter % 192))
			{
				co_await TaskScheduler::NextFrame::Rank[0];
			}
		}
	}
}

Task CDynamicTarget::Task_QuickEval_AirStrike() noexcept
{
	TraceResult tr{};

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
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
		UTIL_TraceLine(vecSrc, vecEnd, m_pPlayer->edict(), m_pPlayer->m_iTeam == TEAM_CT ? g_rgpPlayersOfCT : g_rgpPlayersOfTerrorist, &tr);	// Special traceline skipping all teammates.

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY)
		{
			// One cannot ask air support to 'hit the sky'

			m_Scheduler.Delist(TASK_DEEP_ANALYZE);	// Stop evaluation now.
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
			pev->angles = Angles::Upwards();

			auto const vecCenter = m_pTargeting->Center();
			g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));	// snap to target.
		}
		else
		{
			pev->angles = tr.vecPlaneNormal.VectorAngles();
			pev->origin = tr.vecEndPos;
		}

		// Quick Evaluation

		if ((pev->origin - m_vecLastAiming).LengthSquared() > 24.0 * 24.0)
		{
			// Remove old deep think
			m_Scheduler.Delist(TASK_DEEP_ANALYZE);

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
				m_Scheduler.Enroll(Task_DeepEval_AirStrike(), TASK_DEEP_ANALYZE);

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
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
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

		// Should really think about this. Should I limit the target to a opened space? #NO_URGENT
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

	// Set the skin to red in first few frame, in case player left click instantly.
	pev->skin = Models::targetmdl::SKIN_RED;

	for (;;)
	{
		[[unlikely]]
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
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

		if (!m_bFreezed)
		{
			g_engfuncs.pfnTraceMonsterHull(edict(), vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);

			// Is the location considered to be a wall?
			// Only run this test while not freezed. otherwise it doesn't make sense.

			if (auto const flAngleLean = std::acos(DotProduct(Vector::Up(), tr.vecPlaneNormal)/* No div len required, both len are 1. */) / std::numbers::pi * 180.0;
				flAngleLean > 50)
			{
				// Surface consider wall and no carpet bombardment allow against wall.

				pev->skin = Models::targetmdl::SKIN_RED;
				continue;
			}
		}

		// During freezed mode, we are only here to get a direction marker, we don't care about the situation of actual point.
		else
			g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);

		Vector const vecAiming = tr.vecEndPos;
		Vector const vecSurfNorm = tr.vecPlaneNormal;
		Vector const &vecSrcForSkyTest = m_bFreezed ? pev->origin : vecAiming;

		// Is it under sky?
		g_engfuncs.pfnTraceLine(
			vecSrcForSkyTest,
			Vector(vecSrcForSkyTest.x, vecSrcForSkyTest.y, 8192),
			ignore_monsters | ignore_glass,
			nullptr, &tr
		);

		Vector const &vecSkyBaseOrigin = tr.vecEndPos;

		if (g_engfuncs.pfnPointContents(vecSkyBaseOrigin) != CONTENTS_SKY)
		{
			// Differ from other evaluation, this mode does not allow the target model appears under any roof.

			pev->skin = Models::targetmdl::SKIN_RED;
			continue;
		}

		if (!m_bFreezed)
		{
			// Not pressing LMB, only the main target mdl will showed up.

			pev->angles = vecSurfNorm.VectorAngles();
			UTIL_SetController(&pev->controller[0], &ARROW_CONTROLLER, (double)-m_pPlayer->pev->angles.yaw + (double)pev->angles.yaw - 90.0);

			g_engfuncs.pfnSetOrigin(edict(), vecAiming);
			pev->skin = Models::targetmdl::SKIN_GREEN;

			continue;
		}

		// From this line, consider m_bFreezed is true.

		// Is flying route available?
		Vector vecDir((vecAiming.Make2D() - pev->origin.Make2D()).Normalize(), 0);

		// Preventing math error if use just do a quick right-click instead of hold-and-drag.
		if (vecDir == Vector::Zero())
			vecDir = Angles{ 0, pev->angles.yaw, 0 }.Front();

		g_engfuncs.pfnTraceLine(
			vecSkyBaseOrigin - Vector(0, 0, 16),
			vecSkyBaseOrigin - Vector(0, 0, 16) + vecDir * (CARPET_BOMBARDMENT_INTERVAL * BEACON_COUNT / 2),
			ignore_glass | ignore_monsters,
			nullptr, &tr2
		);	// Using tr2 to avoid wiping sky pos data.

		if (tr2.flFraction < 0.51 || tr2.fAllSolid)
		{
			// There are something in the sky, flying route not available.

			pev->skin = Models::targetmdl::SKIN_RED;
			continue;
		}

		// The aiming direction is good to go.

		pev->skin = Models::targetmdl::SKIN_GREEN;

		// Setup the locations of beams.

		Vector const &vecForward = vecDir;
		Vector const vecRight
		{
			vecForward.x * 0.0/*std::cos(270)*/ - vecForward.y * -1.0/*std::sin(270)*/,
			vecForward.x * -1.0/*std::sin(270)*/ + vecForward.y * 0.0/*std::cos(270)*/,
			0.0
		};
		auto const flMaxDistFwd = tr2.flFraction * (CARPET_BOMBARDMENT_INTERVAL * BEACON_COUNT / 2);

		UTIL_SetController(&pev->controller[0], &ARROW_CONTROLLER, -vecForward.Yaw() + pev->angles.yaw - 90.f);

		for (double flFw = 0, flRt = -48; auto &&pBeacon : m_rgpBeacons)
		{
			if (flFw > flMaxDistFwd)
			{
				pBeacon->pev->effects |= EF_NODRAW;
				goto LAB_CONTINUE;
			}

			pBeacon->pev->origin = vecSkyBaseOrigin + vecForward * flFw + vecRight * flRt;
			pBeacon->pev->effects &= ~EF_NODRAW;

			g_engfuncs.pfnTraceLine(
				pBeacon->pev->origin,
				Vector(pBeacon->pev->origin.Make2D(), -8192.0),
				ignore_glass | ignore_monsters,
				nullptr, &tr2
			);

			pBeacon->StartPos() = tr2.vecEndPos;
			pBeacon->EndPos() = tr2.vecEndPos + Vector(0, 0, 64);

		LAB_CONTINUE:;
			if (flRt > 0)
				flFw += CARPET_BOMBARDMENT_INTERVAL;

			flRt = -flRt;
		}
	}
}

Task CDynamicTarget::Task_QuickEval_Gunship() noexcept
{
	TraceResult tr{};
	EHANDLE<CFixedTarget> pFixedTarget{};
	float flLastAttackingVoice{};

	// We have no variable storing CFixedTarget. Have to search like this.
	for (auto &&pEntity :
		FIND_ENTITY_BY_CLASSNAME(CFixedTarget::CLASSNAME) |
		std::views::transform([](edict_t *pEdict) noexcept { return (CFixedTarget *)pEdict->pvPrivateData; }) |
		std::views::filter([&](CFixedTarget *pEntity) noexcept { return pEntity->m_pPlayer == m_pPlayer && pEntity->m_AirSupportType == GUNSHIP_STRIKE; })
		)
	{
		pFixedTarget = pEntity;
		break;	// Only one instance can exist globally.
	}

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
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
		UTIL_TraceLine(vecSrc, vecEnd, m_pPlayer->edict(), m_pPlayer->m_iTeam == TEAM_CT ? g_rgpPlayersOfCT : g_rgpPlayersOfTerrorist, &tr);	// Special traceline skipping all teammates.

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY)
		{
			// One cannot ask air support to 'hit the sky'

			m_pTargeting = nullptr;

			pev->skin = Models::targetmdl::SKIN_RED;
			continue;	// there's no set origin.
		}

		if (pev_valid(tr.pHit) != 2 && m_flLastValidTracking < gpGlobals->time - 0.5f)	// Snapping: compensating bad aiming
			m_pTargeting = tr.pHit;
		else if (pev_valid(tr.pHit) == 2)
		{
			m_pTargeting = tr.pHit;
			m_flLastValidTracking = gpGlobals->time;
		}

		if (m_pTargeting && !m_pTargeting->IsBSPModel() && m_pTargeting->IsAlive())
		{
			pev->angles = Angles::Upwards();

			Vector const vecCenter = m_pTargeting->Center();
			g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));	// snap to target.
		}
		else
		{
			g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pev->angles);
			g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);
		}

		// Quick Evaluation (color determination)

		g_engfuncs.pfnTraceLine(
			pev->origin,
			Vector(pev->origin.x, pev->origin.y, 8192),
			ignore_monsters | ignore_glass,
			nullptr, &tr
		);

		// Is it under sky?
		bool const bUnderSky = g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY;

		// It's purely depend on whether the target is under sky.
		// Hence no deep analysis required.
		pev->skin =
			(bUnderSky && !CGunship::s_bInstanceExists) ?
			Models::targetmdl::SKIN_GREEN : Models::targetmdl::SKIN_RED;

		if (bUnderSky && pFixedTarget && m_pTargeting)
		{
			// Reselect target.

			if (m_pTargeting != pFixedTarget->m_pTargeting && m_pTargeting->IsAlive() && m_pTargeting->pev->takedamage != DAMAGE_NO)
			{
				if (flLastAttackingVoice < gpGlobals->time)
				{
					flLastAttackingVoice = gpGlobals->time + 1.f;
					g_engfuncs.pfnEmitSound(m_pRadio.Get(), CHAN_STATIC, Sounds::Gunship::RESELECT_TARGET, VOL_NORM, ATTN_STATIC, 0, PITCH_NORM);
					gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::GUNSHIP_RESELECT_TARGET);
				}

				pFixedTarget->m_pTargeting = m_pTargeting;
				pev->effects |= EF_NODRAW;
			}
		}
		else
			pev->effects &= ~EF_NODRAW;
	}
}

Task CDynamicTarget::Task_QuickEval_Phosphorus() noexcept
{
	trace_arc_functor_t fnTraceArc{};
	TraceResult tr{}, tr2{};
	auto iCounter = 0;

	g_engfuncs.pfnSetSize(fnTraceArc, Vector(-4, -4, -4), Vector(4, 4, 4));

	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
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

		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd = vecSrc + m_pPlayer->pev->v_angle.Front() * 4096.f;
		g_engfuncs.pfnTraceMonsterHull(edict(), vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);	// #NO_URGENT this is so bulky and cannot be use in so many places!!
		g_engfuncs.pfnTraceLine(tr.vecEndPos, Vector(tr.vecEndPos.x, tr.vecEndPos.y, 8192), ignore_glass | ignore_monsters, nullptr, &tr2);	// measure the distance between the aiming pos and the sky.

		auto const bHeightNotEnough = (tr2.vecEndPos.z - tr.vecEndPos.z) < 800.f;

		// This is the one that needs a hint, or it would be extremely confusing to debug.
		if (bHeightNotEnough && m_flLastHintTime < (gpGlobals->time - 5.f))
		{
			auto const szHeightDiff = std::format("{}", (int)std::roundf(tr2.vecEndPos.z - tr.vecEndPos.z));
			
			gmsgTextMsg::Unmanaged<MSG_ONE>(
				g_vecZero, m_pPlayer->edict(),
				(byte)4, Localization::REJECT_HEIGHT_NOT_ENOUGH, szHeightDiff.c_str()
			);

			m_flLastHintTime = gpGlobals->time;
		}

		if (auto const flAngleLean = std::acos(DotProduct(Vector::Up(), tr.vecPlaneNormal)/* No div len required, both len are 1. */) / std::numbers::pi * 180.0;
			flAngleLean > 50 || (tr2.vecEndPos.z - tr.vecEndPos.z) < 800.f)
		{
			// Surface consider wall and no phosphorus bomb allow against wall.
			// or
			// The place is too close to the skybox, the phosphorus cannot spread properly.

			m_Scheduler.Delist(TASK_DEEP_ANALYZE);	// Stop evaluation now.

			pev->skin = Models::targetmdl::SKIN_RED;
			goto LAB_CONTINUE;	// there's no set origin.
		}

		g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pev->angles);
		g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);

		// Quick Evaluation

		if ((pev->origin - m_vecLastAiming).LengthSquared2D() > 576 /* 24 */)	// Position drifted, the old estimation could be invalidated.
		{
			// Remove old deep think
			m_Scheduler.Delist(TASK_DEEP_ANALYZE);

			// Try to get a temp spawn location above player.
			g_engfuncs.pfnTraceLine(vecSrc, Vector(vecSrc.x, vecSrc.y, 8192.f), ignore_monsters | ignore_glass, nullptr, &tr);

			if (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY && fnTraceArc(tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass))
			{
				pev->skin = Models::targetmdl::SKIN_GREEN;
			}
			else
			{
				// Start on deep analyze
				m_Scheduler.Enroll(Task_DeepEval_Phosphorus(), TASK_DEEP_ANALYZE);

				pev->skin = Models::targetmdl::SKIN_RED;
			}

			m_vecLastAiming = pev->origin;
		}

	LAB_CONTINUE:;
		co_await TaskScheduler::NextFrame::Rank[1];
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
	m_Scheduler.Delist(TASK_QUICK_ANALYZE | TASK_DEEP_ANALYZE);

	DisableBeacons();
	DisableFireSphere();

	auto const &iType = g_rgiAirSupportSelected[m_pPlayer->entindex()];

	m_iAirSupportTypeModel = iType;
	m_bShowArror = (iType == CARPET_BOMBARDMENT);
	pev->body = UTIL_CalcBody(m_rgBodyInfo);

	m_pTargeting = nullptr;

	switch (iType)
	{
	default:
	case AIR_STRIKE:
		g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());
		m_Scheduler.Enroll(Task_QuickEval_AirStrike(), TASK_QUICK_ANALYZE);
		break;

	case CLUSTER_BOMB:
	case FUEL_AIR_BOMB:
		g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());
		m_Scheduler.Enroll(Task_QuickEval_ClusterBomb(), TASK_QUICK_ANALYZE);
		break;

	case CARPET_BOMBARDMENT:
		g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, 0), Vector(32, 32, 72));
		m_Scheduler.Enroll(Task_QuickEval_CarpetBombardment(), TASK_QUICK_ANALYZE);
		break;

	case GUNSHIP_STRIKE:
		g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());
		m_Scheduler.Enroll(Task_QuickEval_Gunship(), TASK_QUICK_ANALYZE);
		break;

	case PHOSPHORUS_MUNITION:
		g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, 0), Vector(32, 32, 72));
		m_Scheduler.Enroll(Task_QuickEval_Phosphorus(), TASK_QUICK_ANALYZE);
		EnableFireSphere();
		break;
	}
}

void CDynamicTarget::EnableBeacons() noexcept
{
	m_bFreezed = true;

	for (auto &&pBeacon : m_rgpBeacons)
	{
		assert(!pBeacon);	// check leaking.

		pBeacon = CBeam::BeamCreate(Sprites::BEAM, 32.f);

		pBeacon->SetFlags(BEAM_FSHADEOUT);	// fade out on rear end.
		pBeacon->PointsInit(pev->origin, Vector(0, 0, 32));

		pBeacon->pev->classname = MAKE_STRING(BEAM_CLASSNAME);
		pBeacon->pev->renderfx = kRenderFxNone;
		pBeacon->pev->effects |= EF_NODRAW;
		pBeacon->pev->nextthink = 0.1f;
		pBeacon->pev->euser1 = m_pPlayer->edict();	// pev->owner gets occupied by 'starting ent'
		pBeacon->pev->team = m_pPlayer->m_iTeam;
	}
}

void CDynamicTarget::DisableBeacons() noexcept
{
	m_bFreezed = false;

	for (auto &&pBeacon : m_rgpBeacons)
		if (pBeacon) [[likely]]
			pBeacon->pev->flags |= FL_KILLME;
}

void CDynamicTarget::EnableFireSphere() noexcept
{
	Vector vecOrigin{};
	Angles vecAngles{};
	int idx{};

	for (auto &&pSphere : m_rgpAttachedSpr)
	{
		assert(!pSphere);

		g_engfuncs.pfnGetAttachment(edict(), idx, vecOrigin, vecAngles);

		pSphere = CSpriteDisplay::Create(vecOrigin, kRenderTransAdd, Sprites::FLAME[1]);	// flame2.spr is smaller thus fits better.
		pSphere->pev->renderamt = 0;
		pSphere->pev->scale = 0.2f;
		pSphere->m_Scheduler.Enroll(Task_SpriteOnEnt_NotOwned(pSphere->pev, this, idx, { -8, 0, 0 }, 2.f, 220.f, 3.f), TASK_ANIMATION);
		pSphere->m_Scheduler.Enroll(Task_SpriteEnterLoopOut(pSphere->pev, this, 3, 20, 24, 24), TASK_ANIMATION);

		++idx;
	}
}

void CDynamicTarget::DisableFireSphere() noexcept
{
	for (auto &&pSphere : m_rgpAttachedSpr)
		if (pSphere) [[likely]]
			pSphere->pev->flags |= FL_KILLME;
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

CDynamicTarget *CDynamicTarget::Create(CBasePlayer *pPlayer, CPrefabWeapon *pRadio) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CDynamicTarget>();

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_pRadio = pRadio;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

void CDynamicTarget::RetrieveModelInfo(void) noexcept
{
	static bool bDone = false;

	[[likely]]
	if (bDone)
		return;

	auto const pEdict = g_engfuncs.pfnCreateEntity();
	g_engfuncs.pfnSetModel(pEdict, Models::TARGET);

	auto const pstudiohdr = g_engfuncs.pfnGetModelPtr(pEdict);
	auto pbonecontroller = (mstudiobonecontroller_t *)((byte *)pstudiohdr + pstudiohdr->bonecontrollerindex);

	memcpy(&ARROW_CONTROLLER, pbonecontroller, sizeof(ARROW_CONTROLLER));
	memcpy(&MINIATURE_CONTROLLER, ++pbonecontroller, sizeof(MINIATURE_CONTROLLER));

	g_engfuncs.pfnRemoveEntity(pEdict);
	bDone = true;
}

//
// CFixedTarget
//

Task CFixedTarget::Task_AdjustMiniature() noexcept
{
	Vector vecHeadOrg{};

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();	// behind coord update.

		if (m_pTargeting && m_pTargeting->IsPlayer())
		{
			vecHeadOrg = UTIL_GetHeadPosition(m_pTargeting.Get());
			UTIL_SetController(&pev->controller[1], &MINIATURE_CONTROLLER, (double)vecHeadOrg.z - (double)m_pTargeting->pev->absmin.z);
		}
		else
			pev->controller[1] = (uint8_t)MINIATURE_CONTROLLER.rest;
	}
}

Task CFixedTarget::Task_BeaconFx() noexcept
{
	for (;;)
	{
		co_await 1.f;

		if (m_pTargeting && m_pTargeting->IsAlive())
			continue;

		UTIL_Shockwave(pev->origin, GUNSHIP_NEXT_TARGET_RADIUS, Sprites::m_rgLibrary[Sprites::SHOCKWAVE], 0, 0, 1.f, 6.f, 0, Color::Team[pev->team], 192, 0);
	}
}

Task CFixedTarget::Task_Gunship() noexcept
{
	Vector vecSkyPos{ -8192.0, -8192.0, -8192.0, };
	TraceResult tr{};

	// Its lifetime is depending on us, so we are on control.
	CGunship::Create(m_pPlayer, this);

	for (;;)
	{
		// Lie flat on the surface we are on.
		g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.Make2D(), pev->origin.z - 96.0), ignore_monsters | ignore_glass, nullptr, &tr);
		g_engfuncs.pfnVecToAngles(
			tr.vecPlaneNormal == Vector::Zero() ? Vector::Up() : tr.vecPlaneNormal,
			pev->angles
		);

		if (!m_pTargeting)
		{
			//
			// attempt to find a new target.
			//

			const auto fnUnderSky = [&](CBasePlayer *pPlayer) noexcept -> bool
			{
				g_engfuncs.pfnTraceLine(pPlayer->pev->origin, Vector(pPlayer->pev->origin.Make2D(), 8192.0), ignore_glass | ignore_monsters, nullptr, &tr);
				return g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY;
			};

			auto vecCandidates =

				// First of all, it must be a living person...
				Query::all_living_players() |

				// Don't select your teammate!
				std::views::filter([&](CBasePlayer *pPlayer) noexcept { return pPlayer->m_iTeam != pev->team; }) |

				// Must exposed under sky.
				std::views::filter(fnUnderSky) |

				// Store them in a vector.
				std::ranges::to<std::vector>();

			// Select the closest player. The height doesn't matter, as long as they are exposed under sky.
			std::ranges::sort(vecCandidates, std::less{}, [&](CBasePlayer *pPlayer) noexcept { return (pPlayer->pev->origin - pev->origin).Make2D().LengthSquared(); });

			if (!vecCandidates.empty() && (vecCandidates.front()->pev->origin - pev->origin).Make2D().LengthSquared() < (GUNSHIP_NEXT_TARGET_RADIUS * GUNSHIP_NEXT_TARGET_RADIUS))
				m_pTargeting = vecCandidates.front();
		}

		// Rest.
		co_await 0.05f;
	}
}

Task CFixedTarget::Task_PrepareJetSpawn() noexcept
{
	TraceResult tr{};
	size_t iCounter = 0;

	// The carpet bombardment is different from all others.

	if (m_AirSupportType == CARPET_BOMBARDMENT)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		auto const vecDir = Vector((m_rgpBeacons[2]->pev->origin - m_rgpBeacons[0]->pev->origin).Make2D(), 0).Normalize();

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

	if (auto const pFuelAirExplosive = m_pMissile.As<CFuelAirExplosive>(); pFuelAirExplosive != nullptr)
	{
		for (;;)
		{
			if (!pFuelAirExplosive || pFuelAirExplosive->m_bReleasingGas)	// This means the fuel air bomb had been detonated.
			{
				pev->flags |= FL_KILLME;
				co_return;
			}

			co_await TaskScheduler::NextFrame::Rank[1];
		}
	}
	else
	{
		for (;;)
		{
			if (!m_pMissile)	// Missile entity despawned.
			{
				pev->flags |= FL_KILLME;	// So should this.
				co_return;
			}

			co_await TaskScheduler::NextFrame::Rank[1];
		}
	}

	// It will always waiting in the loop .
	std::unreachable();
}

Task CFixedTarget::Task_TimeOut() noexcept
{
	switch (m_AirSupportType)
	{
	case GUNSHIP_STRIKE:
		co_await 25.f;
		gmsgTextMsg::Send(m_pPlayer->edict(), (byte)4, Localization::GUNSHIP_DESPAWNING);
		break;

	default:
		co_await 10.f;
		gmsgTextMsg::Send(m_pPlayer->edict(), (byte)4, Localization::REJECT_TIME_OUT);
		break;
	}

	pev->flags |= FL_KILLME;
}

Task CFixedTarget::Task_UpdateOrigin() noexcept
{
	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[1];

		if (!m_pTargeting || !m_pTargeting->IsAlive())
			continue;

		Vector const vecTargetCenter = m_pTargeting->Center();
		Vector const vecIdealPos = Vector(vecTargetCenter.x, vecTargetCenter.y, m_pTargeting->pev->absmin.z + 1.0);
		Vector const vecDelta = vecIdealPos - pev->origin;

		g_engfuncs.pfnSetOrigin(edict(), pev->origin + vecDelta * std::min(1.f, gpGlobals->frametime * 2.f));	// The drift distance cannot surplus the entire delta value (1.0f)
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
	pev->sequence = m_pDynamicTarget->pev->sequence;
	pev->frame = m_pDynamicTarget->pev->frame;
	pev->nextthink = 0.1f;
	pev->team = m_pPlayer->m_iTeam;
	pev->controller[0] = m_pDynamicTarget->pev->controller[0];
	pev->controller[1] = m_pDynamicTarget->pev->controller[1];

	m_vecPlayerPosWhenCalled = m_pPlayer->pev->origin;
	m_rgpBeacons = m_pDynamicTarget->m_rgpBeacons;
	m_pDynamicTarget->m_rgpBeacons.fill(nullptr);	// Transfer the ownership of these entities to the fixed targets.

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	m_vecPosForJetSpawnTesting = (g_engfuncs.pfnPointContents(tr.vecEndPos) == CONTENTS_SKY) ? (tr.vecEndPos - Vector(0, 0, 16)) : pev->origin;

	m_Scheduler.Enroll(Task_PrepareJetSpawn());
	m_Scheduler.Enroll(Task_TimeOut(), TASK_TIME_OUT);
	m_Scheduler.Enroll(Task_UpdateOrigin());
	m_Scheduler.Enroll(Task_AdjustMiniature());
}

void CFixedTarget::Activate() noexcept
{
	switch (m_AirSupportType)
	{
	case GUNSHIP_STRIKE:
		//m_Scheduler.Delist(TIMEOUT_TASK_KEY);

		[[likely]]
		if (!m_Scheduler.Exist(TASK_ACTION))
		{
			m_Scheduler.Enroll(Task_Gunship(), TASK_ACTION);
			m_Scheduler.Enroll(Task_BeaconFx(), TASK_ACTION);
		}

		break;

	default:
		[[likely]]
		if (!m_Scheduler.Exist(TASK_ACTION))
			m_Scheduler.Enroll(Task_RecruitJet(), TASK_ACTION);
		break;
	}
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
