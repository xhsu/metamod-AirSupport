#include <cassert>

#ifdef __INTELLISENSE__
#include <ranges>
#endif

import std;
import hlsdk;

import Beam;
import Configurations;
import Effects;
import Jet;
import Localization;
import Math;
import Message;
import Projectile;
import Query;
import Ray;
import Resources;
import Round;
import Target;
import Waypoint;
import Weapon;

import UtlRandom;

using namespace std::literals;

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
		if (pev_valid(pEntity->pev->euser1) != EValidity::Full || !((CBasePlayer *)pEntity->pev->euser1->pvPrivateData)->IsAlive())
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

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters | ignore_glass, pEntity->pev->euser1, &tr);

		Beam_SetStartPos(pEntity->pev, tr.vecEndPos);
		Beam_SetEndPos(pEntity->pev, tr.vecEndPos + tr.vecPlaneNormal * 96);
	}
};

//
// Common Tasks
//

inline constexpr auto NEXTFRAME_RANK_REMOVAL = TaskScheduler::NextFrame::Rank[0];
inline constexpr auto NEXTFRAME_RANK_ANGULER_VEL = TaskScheduler::NextFrame::Rank[1];
inline constexpr auto NEXTFRAME_RANK_MODE_EVAL = TaskScheduler::NextFrame::Rank[2];
inline constexpr auto NEXTFRAME_RANK_ANIMATION = TaskScheduler::NextFrame::Rank[3];
inline constexpr auto NEXTFRAME_RANK_REORDER_ANGVEL = TaskScheduler::NextFrame::Rank[4];
inline constexpr auto NEXTFRAME_RANK_DEEP_EVAL = TaskScheduler::NextFrame::Rank[5];

static Task Task_AngleAlter(entvars_t* const pev, Vector const vStart, Vector const vEnd, float const flTimeFrame) noexcept
{
	auto const FX = (int)CVar::targeting_fx;
	auto const flStartTime = gpGlobals->time;
	Vector vNow{};
	float t{};

	if (vStart.Approx(vEnd, 0.01f))
		co_return;

	for (auto passed = gpGlobals->time - flStartTime;
		passed <= flTimeFrame;
		passed = gpGlobals->time - flStartTime)
	{
		co_await NEXTFRAME_RANK_ANGULER_VEL;

		switch (FX)
		{
		default:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame);	// linear
			break;

		case 1:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::smooth_step);
			break;

		case 2:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::spring<0.25>);
			break;

		case 3:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::acce_then_dece);
			break;

		case 4:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::bounce);
			break;

		case 5:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::accelerated<1.75>);
			break;

		case 6:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::anticipate<2.0>);
			break;

		case 7:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::antic_then_overshoot<2.0 * 1.5>);
			break;

		case 8:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::cycle<1.0>);
			break;

		case 9:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::decelerated<1.75>);
			break;

		case 10:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::overshoot<2.0>);
			break;

		case 11:
			vNow = vector_slerp(vStart, vEnd, passed / flTimeFrame, &Interpolation::cubic_hermite<4.0, 4.0>);
			break;
		}

		pev->angles = vNow.VectorAngles();
	}

	// Just in case that the time gets too short and the 100% frame does not called.
	co_await NEXTFRAME_RANK_ANGULER_VEL;
	pev->angles = vEnd.VectorAngles();

	co_return;
}

static Task Task_AngleAlter(entvars_t* const pev, Quaternion const qStart, Quaternion const qEnd, float const flTimeFrame) noexcept
{
	auto const pTarget = reinterpret_cast<CDynamicTarget*>(pev->pContainingEntity->pvPrivateData);
	auto const FX = (int)CVar::targeting_fx;
	auto const flStartTime = gpGlobals->time;
	Quaternion qNow{};
	float t{};

	if (qStart.Approx(qEnd, 0.01f))
		co_return;

	for (auto passed = gpGlobals->time - flStartTime;
		passed <= flTimeFrame;
		passed = gpGlobals->time - flStartTime)
	{
		co_await NEXTFRAME_RANK_ANGULER_VEL;

		// Just like みんなの大好きなMatrix, quaternions are composed from right to left.

		// Step 2: Rotate to the NORM of the surface. Now comes with a interpole!
		switch (FX)
		{
		default:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame);	// linear
			break;

		case 1:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::smooth_step);
			break;

		case 2:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::spring<0.25>);
			break;

		case 3:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::acce_then_dece);
			break;

		case 4:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::bounce);
			break;

		case 5:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::accelerated<1.75>);
			break;

		case 6:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::anticipate<2.0>);
			break;

		case 7:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::antic_then_overshoot<2.0 * 1.5>);
			break;

		case 8:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::cycle<1.0>);
			break;

		case 9:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::decelerated<1.75>);
			break;

		case 10:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::overshoot<2.0>);
			break;

		case 11:
			qNow = quat_slerp(qStart, qEnd, passed / flTimeFrame, &Interpolation::cubic_hermite<4.0, 4.0>);
			break;
		}

		// Step 1: Simulating the pre-miniature update rotational animations.
		qNow *= pTarget->m_qPseudoanim;

		pev->angles = qNow.Euler();
		pev->angles[0] = -pev->angles[0];
	}

	// Just in case that the time gets too short and the 100% frame does not called.
	co_await NEXTFRAME_RANK_ANGULER_VEL;

	pev->angles = (qEnd * pTarget->m_qPseudoanim).Euler();
	pev->angles[0] = -pev->angles[0];

	co_return;
}

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
	using std::chrono::duration_cast;
	using std::chrono::high_resolution_clock;
	using std::chrono::nanoseconds;

	// This variable should never be a part of the class. It doesn't even being ref anywhere else.
	auto LastAnimUpdate = high_resolution_clock::now();
	auto vecHeadOrg = Vector::Zero();

	// Enables the client model prediction/interpole, even we don't have an actual MDL anim.
	pev->framerate = 1.0f;
	pev->frame = 0;
	pev->animtime = gpGlobals->time;

	for (;;)
	{
		[[unlikely]]
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await NEXTFRAME_RANK_ANIMATION;

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

			//pev->framerate = 0;	// Only used for an actual MDL anim.
			//pev->frame = 0;
			//pev->animtime = 0;

			continue;
		}

		auto const CurTime = high_resolution_clock::now();
		auto const flTimeDelta = duration_cast<nanoseconds>(CurTime - LastAnimUpdate).count() / 1'000'000'000.0;

		// LUNA: no more MDL animation since miniature update!
		// Now simply using a quaternion to represent the rotating targeting model.
		m_qPseudoanim = Quaternion::AxisAngle(VEC_ALMOST_RIGHT, flTimeDelta * 30);

		if (m_bShowArror())	// We are unable to resolve this one mathmatically so far...
		{
			// Not being able to calculate the controll if we are using quaternion on a slope.
			m_qPseudoanim = Quaternion::Identity();
		}
		else if (!m_Scheduler.Exist(TASK_ANGLE_INTERPOL))
		{
			pev->angles = (m_qNormRotatingTo * m_qPseudoanim).Euler();
			pev->angles[0] = -pev->angles[0];
		}
/*
		pev->framerate = float(Models::targetmdl::FPS * flTimeDelta);
		pev->frame += pev->framerate;
		pev->animtime = gpGlobals->time;

		// model sequence is different from SPRITE, no matter now many frame you have, it will stretch/squeeze into 256.

		[[unlikely]]
		if (pev->frame < 0 || pev->frame >= 256)
			pev->frame -= float((pev->frame / 256.0) * 256.0);

		LastAnimUpdate = CurTime;
*/
	}
}

Task CDynamicTarget::Task_AngleInterpol() noexcept
{
	Vector vecLastNorm{ Vector::Up() };
	Quaternion qLastNorm{ Quaternion::Rotate(VEC_ALMOST_RIGHT, Vector::Up()) };

	for (;;)
	{
		[[unlikely]]
		if (m_pPlayer->m_pActiveItem != m_pRadio)	// #AIRSUPPORT_verify_radio
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await NEXTFRAME_RANK_REORDER_ANGVEL;

		if (!qLastNorm.Approx(m_qNormRotatingTo, 0.01f))
		{
			if ((float)CVar::targeting_time > 0.0)
			{
				m_Scheduler.Enroll(Task_AngleAlter(
					pev,
					// LUNA: WHAT THE HECK does the v_angle doing here?!
					Quaternion::Euler(Angles{ -pev->angles.pitch, pev->angles.yaw, pev->angles.roll }.ToRadian()),
					m_qNormRotatingTo,
					(float)CVar::targeting_time
				), TASK_ANGLE_INTERPOL, true);
			}

			qLastNorm = m_qNormRotatingTo;
		}
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
		co_await NEXTFRAME_RANK_DEEP_EVAL;

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
				co_await NEXTFRAME_RANK_DEEP_EVAL;
			}
		}
	}
}

Task CDynamicTarget::Task_QuickEval_AirStrike() noexcept
{
	TraceResult tr{};
	Vector vecLastAiming{};
	float flLastValidTracking{};

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

		if (pev_valid(tr.pHit) != EValidity::Full && flLastValidTracking < gpGlobals->time - 0.5f)	// Snapping: compensenting bad aiming
			m_pTargeting = nullptr;
		else if (pev_valid(tr.pHit) == EValidity::Full)
		{
			m_pTargeting = tr.pHit;
			flLastValidTracking = gpGlobals->time;
		}

		if (m_pTargeting && !m_pTargeting->IsBSPModel() && m_pTargeting->IsAlive())
		{
			m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, Vector::Up());

			auto const vecCenter = m_pTargeting->Center();
			g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));	// snap to target.
		}
		else
		{
			m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, tr.vecPlaneNormal);
			pev->origin = tr.vecEndPos;
		}

		// Quick Evaluation

		if ((pev->origin - vecLastAiming).LengthSquared() > 24.0 * 24.0)
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

			vecLastAiming = pev->origin;
		}

	LAB_CONTINUE:;
		co_await NEXTFRAME_RANK_MODE_EVAL;
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

		m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, tr.vecPlaneNormal);
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
		co_await NEXTFRAME_RANK_MODE_EVAL;
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

		co_await NEXTFRAME_RANK_MODE_EVAL;

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

			m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, vecSurfNorm);
			UTIL_SetController(&pev->controller[0], &ARROW_CONTROLLER, (double)-m_pPlayer->pev->angles.yaw + vecSurfNorm.Yaw() - 90.0);

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
	Vector vecLastNorm{};
	float flLastValidTracking{}, flLastAttackingVoice{};

	// We have no variable storing CFixedTarget. Have to search like this.
	for (auto&& pEntity :
		Query::all_instances_of<CFixedTarget>()
		| std::views::filter([&](CFixedTarget* pEntity) noexcept { return pEntity->m_pPlayer == m_pPlayer && pEntity->m_AirSupportType == GUNSHIP_STRIKE; })
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

		co_await NEXTFRAME_RANK_MODE_EVAL;

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

		if (pev_valid(tr.pHit) != EValidity::Full && flLastValidTracking < gpGlobals->time - 0.5f)	// Snapping: compensating bad aiming
			m_pTargeting = nullptr;
		else if (pev_valid(tr.pHit) == EValidity::Full)
		{
			m_pTargeting = tr.pHit;
			flLastValidTracking = gpGlobals->time;
		}

		if (m_pTargeting && !m_pTargeting->IsBSPModel() && m_pTargeting->IsAlive())
		{
			m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, Vector::Up());

			auto const vecCenter = m_pTargeting->Center();
			g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));	// snap to target.
		}
		else
		{
			m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, tr.vecPlaneNormal);
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
					g_engfuncs.pfnEmitSound(m_pRadio.Get(), CHAN_AUTO, Sounds::Gunship::RESELECT_TARGET, VOL_NORM, ATTN_STATIC, SND_FL_NONE, PITCH_NORM);
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
	Vector vecLastAiming{};
	auto iCounter = 0;
	float flLastValidTracking{}, flLastHintTime{};

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
		if (bHeightNotEnough && flLastHintTime < (gpGlobals->time - 5.f))
		{
			auto const szHeightDiff = std::format("{}", (int)std::roundf(tr2.vecEndPos.z - tr.vecEndPos.z));
			
			gmsgTextMsg::Unmanaged<MSG_ONE>(
				g_vecZero, m_pPlayer->edict(),
				(uint8_t)4, Localization::REJECT_HEIGHT_NOT_ENOUGH, szHeightDiff.c_str()
			);

			flLastHintTime = gpGlobals->time;
		}

		if (auto const flAngleLean = std::acos(DotProduct(Vector::Up(), tr.vecPlaneNormal)/* No div len required, both len are 1. */) / std::numbers::pi * 180.0;
			flAngleLean > 50 || bHeightNotEnough)
		{
			// Surface consider wall and no phosphorus bomb allow against wall.
			// or
			// The place is too close to the skybox, the phosphorus cannot spread properly.

			m_Scheduler.Delist(TASK_DEEP_ANALYZE);	// Stop evaluation now.

			pev->skin = Models::targetmdl::SKIN_RED;
			goto LAB_CONTINUE;	// there's no set origin.
		}

		m_qNormRotatingTo = Quaternion::Rotate(VEC_ALMOST_RIGHT, tr.vecPlaneNormal);
		g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);

		// Quick Evaluation

		if ((pev->origin - vecLastAiming).LengthSquared2D() > 576 /* 24 */)	// Position drifted, the old estimation could be invalidated.
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

			vecLastAiming = pev->origin;
		}

	LAB_CONTINUE:;
		co_await NEXTFRAME_RANK_MODE_EVAL;
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

Task CDynamicTarget::Task_Miniature_AirStrike() noexcept
{
	// Purpose: The exhaust smoke effect in miniature.
	// Should be index 1-3, as 0 is the flame.

	for (;;)
	{
		for (int i = 3; i > 0; --i)
		{
			auto& pExhaust = m_rgpVisualFxSpr[i];

			pExhaust = CSpriteDisplay::Create(pev->origin, Sprites::ROCKET_TRAIL_SMOKE[0], m_pPlayer);
			pExhaust->pev->renderamt = 220;
			pExhaust->pev->rendercolor = Vector(255, 255, UTIL_Random(192, 255));
			pExhaust->pev->frame = (float)UTIL_Random(17, 22);
			pExhaust->pev->scale = 0.25f * i;	// the up-most one should be largest one.
			pExhaust->m_Scheduler.Enroll(Task_SpriteOnBone(pExhaust->pev, this, FX_BONES_IDX[AIR_STRIKE][i], g_vecZero, 0, 0, 3.f, false), TASK_ANIMATION);
			pExhaust->m_Scheduler.Enroll(Task_FadeOut(pExhaust->pev, /*STAY*/ 0, /*DECAY*/ 0.75f, /*ROLL*/ 0, /*SCALE_INC*/ 1.f), TASK_ANIMATION);

			co_await 0.5f;
		}

		// Wait until the existing SPR dies out.
		while (m_rgpVisualFxSpr[1] || m_rgpVisualFxSpr[2] || m_rgpVisualFxSpr[3])
			co_await 0.5f;
	}

	co_return;
}

Task CDynamicTarget::Task_Miniature_ClusterBomb(std::string_view SPRITE, double const FPS, bool const REMOVE_ON_FRZ, Vector const vecOfs) noexcept
{
	// Purpose: detonate the 'balls' in cluster bomb.

	auto const& iType = g_rgiAirSupportSelected[m_pPlayer->entindex()];
	auto const iFrameCount = g_rgiSpriteFrameCount.at(SPRITE);

	for (;;)
	{
		for (auto&& [pBlast, iBone] : std::views::zip(m_rgpVisualFxSpr, FX_BONES_IDX[iType]))
		{
			pBlast = CSpriteDisplay::Create(pev->origin, SPRITE, m_pPlayer);
			pBlast->pev->renderamt = 0x80;
			pBlast->pev->rendercolor = Vector(255, 255, 255);
			pBlast->pev->scale = 0.35f;
			pBlast->m_Scheduler.Enroll(Task_SpriteOnBone(pBlast->pev, this, iBone, vecOfs, 0, 0, 3.f, false), TASK_ANIMATION);
			pBlast->m_Scheduler.Enroll(Task_SpritePlayOnce(pBlast->pev, iFrameCount, FPS), TASK_ANIMATION);

			co_await 0.12f;
		}

		co_await float(iFrameCount / FPS);

		if (REMOVE_ON_FRZ && m_bFreezed)
			co_return;
	}

	co_return;
}

Task CDynamicTarget::Task_Miniature_Gunship() noexcept
{
	// Purpose: shoot some trace...
	// The first one is source, the rest are dests.

	auto const iSrc = FX_BONES_IDX[GUNSHIP_STRIKE][0];
	std::span iDests{ FX_BONES_IDX[GUNSHIP_STRIKE] | std::views::drop(1) };
	TraceResult tr{};

	for (;;)
	{
		for (auto&& iDest : iDests)
		{
			auto pMuzzle = CSpriteDisplay::Create(pev->origin, Sprites::AIRBURST, m_pPlayer);
			pMuzzle->pev->renderamt = 220;
			pMuzzle->pev->rendercolor = Vector(255, 255, UTIL_Random(192, 255));
			pMuzzle->pev->frame = (float)UTIL_Random(0, 2);
			pMuzzle->pev->scale = 0.2f;
			pMuzzle->m_Scheduler.Enroll(Task_SpriteOnBone(pMuzzle->pev, this, iSrc, g_vecZero, 0, 0, 3.f, false), TASK_ANIMATION);
			pMuzzle->m_Scheduler.Enroll(Task_FadeOut(pMuzzle->pev, /*STAY*/ 0, /*DECAY*/ 20.f, /*ROLL*/ 0, /*SCALE_INC*/ 1.f), TASK_ANIMATION);

			auto const vecSrc = UTIL_GetBonePosition(edict(), iSrc);
			auto const vecDest = UTIL_GetBonePosition(edict(), iDest);

			tr.vecEndPos = (vecDest - vecSrc).Normalize();	// just q quick borrow.
			g_engfuncs.pfnTraceLine(vecSrc, vecSrc + tr.vecEndPos * 300.f, dont_ignore_monsters | dont_ignore_glass, edict(), &tr);

			MsgSend(m_pPlayer->pev, SVC_TEMPENTITY);	// demo is only visible to oneself.
			WriteData(TE_TRACER);
			WriteData(vecSrc);
			WriteData(tr.vecEndPos);
			MsgEnd();

			co_await 0.1f;
		}
	}

	co_return;
}

Task CDynamicTarget::Task_Miniature_Phosphorus() noexcept
{
	for (;;)
	{
		for (auto&& iBone : FX_BONES_IDX[PHOSPHORUS_MUNITION])
		{
			if (UTIL_Random())
			{
				auto const vecPos =
					UTIL_GetBonePosition(edict(), iBone);

				MsgSend(m_pPlayer->pev, SVC_TEMPENTITY);	// demo is only visible to oneself.
				WriteData(TE_SPARKS);
				WriteData(vecPos);
				MsgEnd();
			}

			co_await UTIL_Random(0.1f, 0.2f);
		}
	}

	co_return;
}

void CDynamicTarget::UpdateEvalMethod() noexcept
{
	m_Scheduler.Delist(TASK_QUICK_ANALYZE | TASK_DEEP_ANALYZE);

	DisableBeacons();
	UpdateVisualDemo();

	auto const &iType = g_rgiAirSupportSelected[m_pPlayer->entindex()];

	m_iAirSupportTypeModel() = iType;
	m_bShowArror() = (iType == CARPET_BOMBARDMENT);
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

void CDynamicTarget::UpdateVisualDemo() noexcept
{
	// Clear old vfx demos.
	for (auto&& pSprite : m_rgpVisualFxSpr)
	{
		if (pSprite)
			pSprite->pev->flags |= FL_KILLME;
		else
			break;	// it's safe to assume the following part are also unused.
	}

	// Clear old miniature coros
	m_Scheduler.Delist(TASK_MINIATURE);

	auto const& iType = g_rgiAirSupportSelected[m_pPlayer->entindex()];

	switch (iType)
	{
	case AIR_STRIKE:
	{
		auto& pExhaustFlame = m_rgpVisualFxSpr[0];
		pExhaustFlame = CSpriteDisplay::Create(pev->origin, Sprites::ROCKET_EXHAUST_FLAME, m_pPlayer);
		pExhaustFlame->pev->renderamt = 0;
		pExhaustFlame->pev->scale = 0.35f;
		pExhaustFlame->m_Scheduler.Enroll(Task_SpriteOnBone(pExhaustFlame->pev, this, FX_BONES_IDX[AIR_STRIKE][0], g_vecZero, 2.f, 220.f, 3.f, true), TASK_ANIMATION);

		// The rest 3 should be... exhaust cloud?
		m_Scheduler.Enroll(Task_Miniature_AirStrike(), TASK_MINIATURE, true);

		break;
	}

	case CLUSTER_BOMB:
		// All 13 would be explo point.
		m_Scheduler.Enroll(Task_Miniature_ClusterBomb(Sprites::MINOR_EXPLO, 20), TASK_MINIATURE, true);
		break;

	case CARPET_BOMBARDMENT:
		// All 8 would be explo point.
		m_Scheduler.Enroll(Task_Miniature_ClusterBomb(Sprites::CARPET_FRAGMENT_EXPLO, 12, true, { -8, 0, 0 }), TASK_MINIATURE, true);
		break;

	case GUNSHIP_STRIKE:
		// The first one is muzzle, the rests are dests.
		m_Scheduler.Enroll(Task_Miniature_Gunship(), TASK_MINIATURE, true);
		break;

	case FUEL_AIR_BOMB:
	{
		static auto const FRAME_COUNT = g_rgiSpriteFrameCount.at(Sprites::LIFTED_DUST);

		// the poison cloud fades in and out nearby.
		for (auto&& [pCloud, iBone] : std::views::zip(m_rgpVisualFxSpr, FX_BONES_IDX[FUEL_AIR_BOMB]))
		{
			pCloud = CSpriteDisplay::Create(pev->origin, Sprites::LIFTED_DUST, m_pPlayer);
			pCloud->pev->scale = UTIL_Random(0.4f, 0.8f);
			pCloud->pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);
			pCloud->m_Scheduler.Enroll(Task_SpriteOnBone(pCloud->pev, this, iBone, { -32, 0, 0 }, 0, 0, 3.f, false), TASK_ANIMATION);
			pCloud->m_Scheduler.Enroll(Task_SineOpaque(pCloud->pev, this, UTIL_Random(0.4f, 0.8f), UTIL_Random(0.f, 3.14f), 0.f, 220.f, 3.f), TASK_ANIMATION);
		}
		break;
	}

	case PHOSPHORUS_MUNITION:

		// Fire burning on the sphere with occational spark.
		for (auto&& [pSphere, iBone] : std::views::zip(m_rgpVisualFxSpr, FX_BONES_IDX[PHOSPHORUS_MUNITION]))
		{
			pSphere = CSpriteDisplay::Create(pev->origin, Sprites::FLAME[1], m_pPlayer);	// flame2.spr is smaller thus fits better.
			pSphere->pev->renderamt = 0;
			pSphere->pev->scale = 0.2f;
			pSphere->m_Scheduler.Enroll(Task_SpriteOnBone(pSphere->pev, this, iBone, { -8, 0, 0 }, 2.f, 220.f, 3.f, false), TASK_ANIMATION);
			pSphere->m_Scheduler.Enroll(Task_SpriteEnterLoopOut(pSphere->pev, this, 3, 20, 24, 24), TASK_ANIMATION);
		}

		// Sparks
		m_Scheduler.Enroll(Task_Miniature_Phosphorus(), TASK_MINIATURE, true);

		break;

	default:
		break;
	}
}

void CDynamicTarget::Spawn() noexcept
{
#ifdef _DEBUG
	// Normally it should be enought, as the largest should be cluster, which is 13.
	static auto const MAX_BONE_COUNT =
		std::ranges::max_element(FX_BONES_IDX, {}, &std::ranges::range_value_t<decltype(FX_BONES_IDX)>::size);
	assert(MAX_BONE_COUNT->size() <= m_rgpVisualFxSpr.size());
#endif

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
	m_Scheduler.Enroll(Task_AngleInterpol());
	m_Scheduler.Enroll(Task_Remove());

	UpdateEvalMethod();
}

CDynamicTarget *CDynamicTarget::Create(CBasePlayer *pPlayer, CPrefabWeapon *pRadio) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CDynamicTarget>();

	pEdict->v.angles = Angles::Upwards();	// determind my model.

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
	auto pbonecontroller = (mstudiobonecontroller_t *)((uint8_t *)pstudiohdr + pstudiohdr->bonecontrollerindex);

	// fetch controller

	memcpy(&ARROW_CONTROLLER, pbonecontroller, sizeof(ARROW_CONTROLLER));
	memcpy(&MINIATURE_CONTROLLER, ++pbonecontroller, sizeof(MINIATURE_CONTROLLER));

	// fetch bones

	static constexpr std::array FX_BONES_NAME{
		"PrecisionGuide_"sv,
		"ClusterMunitions_"sv,
		"CarpetBombing_"sv,
		"GunshipStrike_"sv,
		"FuelAirExplosive_"sv,
		"PhosphorusMunition_"sv,
	};
	static_assert(FX_BONES_NAME.size() == AIRSUPPORT_TYPES);

	for (auto const pbones = (mstudiobone_t*)((uint8_t*)pstudiohdr + pstudiohdr->boneindex);
		auto&& [szName, rgiIndeces] : std::views::zip(FX_BONES_NAME, FX_BONES_IDX))
	{
		rgiIndeces.clear();
		rgiIndeces.reserve(4);

		unsigned iCount = 0;
		for (auto pbone = pbones; iCount < pstudiohdr->numbones; ++iCount, ++pbone)
		{
			if (!std::ranges::starts_with(pbone->name, szName))
				continue;

			rgiIndeces.push_back(iCount);
		}
	}

	g_engfuncs.pfnRemoveEntity(pEdict);
	bDone = true;
}

//
// CFixedTarget
//

CFixedTarget::~CFixedTarget() noexcept
{
	// A missile binding to this entity indicates a jet had spawned and at least had one projectile thrown.
	if (m_pMissile)
		return;

	// In that case, the ownership (i.e. who's responsibility to delete beacon entity) transfer to the jet and missiles.
	// The target entity will not release beacon entity.

	for (auto&& pBeam : m_rgpBeacons)
	{
		if (pBeam)
			pBeam->pev->flags |= FL_KILLME;
	}
}

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

		if (!(bool)CVar::gs_beacon_fx)
			continue;

		if (m_pTargeting && m_pTargeting->IsAlive())
			continue;

		UTIL_Shockwave(pev->origin, (float)CVar::gs_radius, Sprites::m_rgLibrary[Sprites::SHOCKWAVE], 0, 0, 1.f, 6.f, 0, Color::Team[pev->team], 192, 0);
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

			if (!vecCandidates.empty() && (vecCandidates.front()->pev->origin - pev->origin).Make2D().LengthSquared() < (CVar::gs_radius->value * CVar::gs_radius->value))
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
		gmsgTextMsg::Send(m_pPlayer->edict(), (uint8_t)4, Localization::REJECT_NO_JET_SPAWN);
		pev->flags |= FL_KILLME;
		co_return;
	}

	EHANDLE<CJet> pJet = CJet::Create(m_pPlayer, this, m_vecJetSpawn);

	for (;;)
	{
		[[unlikely]]
		if (!pJet)	// Jet found no way to launch missile
		{
			gmsgTextMsg::Send(m_pPlayer->edict(), (uint8_t)4, Localization::REJECT_NO_VALID_TRACELINE);
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
		co_await (float)CVar::gs_holding;
		gmsgTextMsg::Send(m_pPlayer->edict(), (uint8_t)4, Localization::GUNSHIP_DESPAWNING);
		break;

	default:
		co_await 10.f;
		gmsgTextMsg::Send(m_pPlayer->edict(), (uint8_t)4, Localization::REJECT_TIME_OUT);
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
	pev->renderfx = (kRenderFx)((std::underlying_type_t<kRenderFx>)std::roundf(CVar::target_render_fx->value) % 21);	// kRenderFxDistort(15) is the default value. Change it at Hook.cpp
	pev->renderamt = 0;
	pev->skin = Models::targetmdl::SKIN_BLUE;
	pev->nextthink = 0.1f;
	pev->team = m_pPlayer->m_iTeam;

	if ((bool)CVar::target_illumination)
		pev->effects &= ~EF_DIMLIGHT;

	m_vecPlayerPosWhenCalled = m_pPlayer->pev->origin;

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

	pEdict->v.angles = (pDynamicTarget->m_qNormRotatingTo * pDynamicTarget->m_qPseudoanim).Euler();
	pEdict->v.angles.pitch = -pEdict->v.angles.pitch;	// fucking quake.
	pEdict->v.origin = pDynamicTarget->pev->origin;
	pEdict->v.body = pDynamicTarget->pev->body;
	pEdict->v.sequence = pDynamicTarget->pev->sequence;
	pEdict->v.frame = pDynamicTarget->pev->frame;
	pEdict->v.controller[0] = pDynamicTarget->pev->controller[0];
	pEdict->v.controller[1] = pDynamicTarget->pev->controller[1];

	pPrefab->m_rgpBeacons = pDynamicTarget->m_rgpBeacons;
	pDynamicTarget->m_rgpBeacons.fill(nullptr);	// Transfer the ownership of these entities to the fixed targets.

	pPrefab->m_pTargeting = pDynamicTarget->m_pTargeting;
	pPrefab->m_pPlayer = pDynamicTarget->m_pPlayer;
	pPrefab->m_AirSupportType = g_rgiAirSupportSelected[pDynamicTarget->m_pPlayer->entindex()];
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

CFixedTarget* CFixedTarget::Create(CBasePlayer* pPlayer, CBaseEntity* pTargeting) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CFixedTarget>();

	pEdict->v.angles = Angles::Upwards();
	pEdict->v.origin = pTargeting->pev->origin;
	pEdict->v.body = 0;
	pEdict->v.sequence = 0;
	pEdict->v.frame = 0;
	pEdict->v.controller[0] = 0;
	pEdict->v.controller[1] = 0;

	//pPrefab->m_rgpBeacons = pDynamicTarget->m_rgpBeacons;
	//pDynamicTarget->m_rgpBeacons.fill(nullptr);	// Transfer the ownership of these entities to the fixed targets.

	pPrefab->m_pTargeting = pTargeting;
	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_AirSupportType = g_rgiAirSupportSelected[pPlayer->entindex()];
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
