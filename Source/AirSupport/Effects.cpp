#include <cassert>	// #UPDATE_AT_CPP26 replace all with contract

import std;
import hlsdk;

import Configurations;
import DamageOverTime;
import Effects;
import Math;
import Message;
import Projectile;
import Query;
import Resources;
import Target;
import Task;

import UtlRandom;

using std::array;
using std::pair;
using std::string;

//
// Componental function
//

Task Task_SpriteLoop(entvars_t *const pev, short const FRAME_COUNT, double const FPS) noexcept
{
	short iFrame = (short)std::clamp(pev->frame, 0.f, float(FRAME_COUNT - 1));

	for (;;)
	{
		co_await float(1.f / FPS);

		iFrame = (iFrame + 1) % FRAME_COUNT;

		pev->framerate = float(1.f / FPS);
		pev->frame = iFrame;
		pev->animtime = gpGlobals->time;
	}
}

Task Task_SpriteLoop(entvars_t* const pev, uint16_t const STARTS_AT, uint16_t const FRAME_COUNT, double const FPS) noexcept
{
	uint16_t iFrame = STARTS_AT + UTIL_Random<uint16_t>(0, FRAME_COUNT - 1);

	for (;;)
	{
		co_await float(1.f / FPS);

		iFrame = (iFrame - STARTS_AT + 1) % FRAME_COUNT + STARTS_AT;

		pev->framerate = float(1.f / FPS);
		pev->frame = iFrame;
		pev->animtime = gpGlobals->time;
	}
}

Task Task_SpritePlayOnce(entvars_t *const pev, short const FRAME_COUNT, double const FPS) noexcept
{
	short iFrame = (short)std::clamp(pev->frame, 0.f, float(FRAME_COUNT - 1));

	for (; iFrame < FRAME_COUNT;)
	{
		co_await float(1.f / FPS);

		pev->framerate = float(1.f / FPS);
		pev->frame = ++iFrame;
		pev->animtime = gpGlobals->time;
	}

	pev->flags |= FL_KILLME;
}

Task Task_SpritePlayOnce(entvars_t *const pev, uint16_t const FRAME_COUNT, double const FPS, float const AWAIT, float const DECAY, float const ROLL, float const SCALE_INC) noexcept
{
	for (auto iFrame = (uint16_t)std::clamp(pev->frame, 0.f, float(FRAME_COUNT - 1));
		iFrame < FRAME_COUNT;)
	{
		co_await float(1.f / FPS);

		pev->framerate = float(1.f / FPS);
		pev->frame = ++iFrame;
		pev->animtime = gpGlobals->time;
	}

	if (AWAIT > 0)
		co_await AWAIT;

	// Save the scale here to compatible with other behaviour.
	auto const flOriginalScale = pev->scale;

	for (auto flPercentage = (255.f - pev->renderamt) / 255.f;
		pev->renderamt > 0;
		flPercentage = (255.f - pev->renderamt) / 255.f)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DECAY;
		pev->angles.roll += ROLL;
		pev->scale = flOriginalScale * (1.f + flPercentage * SCALE_INC);	// fade out by zooming the SPR.
	}

	pev->flags |= FL_KILLME;
}

Task Task_SpriteLoopOut(entvars_t* const pev, uint16_t const LOOP_STARTS_AT, uint16_t const LOOP_FRAME_COUNT, uint16_t const OUT_ENDS_AT, float const TIME, double const FPS) noexcept
{
	uint16_t const OUT_STARTS_AT = LOOP_STARTS_AT + LOOP_FRAME_COUNT;
	uint16_t const& LOOP_ENDS_AT = OUT_STARTS_AT;
	auto const FRAME_INTERVAL = float(1.f / FPS);

	uint16_t iFrame = LOOP_STARTS_AT + UTIL_Random<uint16_t>(0, LOOP_FRAME_COUNT - 1);

	for (auto const flTimeUp = gpGlobals->time + TIME; flTimeUp > gpGlobals->time;)
	{
		co_await FRAME_INTERVAL;

		iFrame = (iFrame - LOOP_STARTS_AT + 1) % LOOP_FRAME_COUNT + LOOP_STARTS_AT;

		pev->framerate = FRAME_INTERVAL;
		pev->frame = iFrame;
		pev->animtime = gpGlobals->time;
	}

	// Play all to the end of loop so we can concat to end frames.
	for (; iFrame < LOOP_ENDS_AT;)
	{
		co_await FRAME_INTERVAL;

		pev->framerate = FRAME_INTERVAL;
		pev->frame = ++iFrame;
		pev->animtime = gpGlobals->time;
	}

	assert(iFrame == OUT_STARTS_AT);

	for (; iFrame < OUT_ENDS_AT;)
	{
		co_await FRAME_INTERVAL;

		pev->framerate = FRAME_INTERVAL;
		pev->frame = ++iFrame;
		pev->animtime = gpGlobals->time;
	}

	pev->flags |= FL_KILLME;
}

Task Task_SqueezeDieout(entvars_t* const pev, float const AWAIT, float const DECAY, float const SQUEEZE) noexcept
{
	Vector const vecOriginalMin = pev->mins / pev->scale;
	Vector const vecOriginalMax = pev->maxs / pev->scale;

	co_await AWAIT;

	for (; pev->scale > 0.01f && pev->renderamt > 0;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DECAY;
		pev->scale -= SQUEEZE;

		g_engfuncs.pfnSetSize(ent_cast<edict_t*>(pev), vecOriginalMin * pev->scale, vecOriginalMax * pev->scale);
		g_engfuncs.pfnDropToFloor(ent_cast<edict_t*>(pev));
	}

	pev->flags |= FL_KILLME;
}

Task Task_FadeOut(entvars_t *const pev, float const AWAIT, float const DECAY, float const ROLL) noexcept
{
	if (AWAIT > 0)
		co_await AWAIT;

	for (; pev->renderamt > 0;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DECAY;
		pev->angles.roll += ROLL;
	}

	pev->flags |= FL_KILLME;
}

Task Task_FadeOut(entvars_t *const pev, float const AWAIT, float const DECAY, float const ROLL, float const SCALE_INC) noexcept
{
	if (AWAIT > 0)
		co_await AWAIT;

	// Save the scale here to compatible with other behaviour.
	auto const flOriginalScale = pev->scale;

	for (auto flPercentage = (255.f - pev->renderamt) / 255.f;
		pev->renderamt > 0;
		flPercentage = (255.f - pev->renderamt) / 255.f)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DECAY;
		pev->angles.roll += ROLL;
		pev->scale = flOriginalScale * (1.f + flPercentage * SCALE_INC);	// fade out by zooming the SPR.
	}

	pev->flags |= FL_KILLME;
}

Task Task_Remove(entvars_t *const pev, float const TIME) noexcept
{
	co_await TIME;

	pev->flags |= FL_KILLME;
}

Task Task_FadeIn(entvars_t *const pev, float const TRANSPARENT_INC, float const FINAL_VAL, float const ROLL) noexcept
{
	for (; pev->renderamt < FINAL_VAL;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt += TRANSPARENT_INC;
		pev->angles.roll += ROLL;
	}

	[[unlikely]]
	if (abs(ROLL) <= std::numeric_limits<float>::epsilon())
		co_return;

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->angles.roll += ROLL;
	}
}

Task Task_Fade(entvars_t *const pev, float const INC, float const DEC, float const PEAK, float const ROLL) noexcept
{
	for (; pev->renderamt < PEAK;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt += INC;
		pev->angles.roll += ROLL;
	}

	for (; pev->renderamt > 0;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DEC;
		pev->angles.roll += ROLL;
	}

	pev->flags |= FL_KILLME;
}

Task Task_SpriteOnAttachment_NotOwned(entvars_t *const pev, EHANDLE<CBaseEntity> pAttachTo, uint16_t const ATTACHMENT, Vector const vecOfs, float const INC, float const PEAK, float const DECAY) noexcept
{
	Vector vecAttOrigin{}, vecTransformedOfs{};
	auto const me = ent_cast<edict_t *>(pev);

	for (; pAttachTo;)	// Must be a post-awaiting. Otherwise the validity of entity is subject to change after the check.
	{
		auto&& [f, r, u] = pAttachTo->pev->angles.AngleVectors();
		vecTransformedOfs = vecOfs.x * f + vecOfs.y * r + vecOfs.z * u;
		vecAttOrigin = UTIL_GetAttachment(pAttachTo.Get(), ATTACHMENT);	// LUNA: DO NOT use the engine version, it's buggy.

		g_engfuncs.pfnSetOrigin(me, vecAttOrigin + vecTransformedOfs);

		if (pev->renderamt < PEAK)
			pev->renderamt = std::min(pev->renderamt + INC, PEAK);

		co_await TaskScheduler::NextFrame::Rank[0];
	}

	for (; pev->renderamt > 0; pev->renderamt -= DECAY)
	{
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	// We are not owning this entity, DO NOT remove it!
	co_return;
}

Task Task_SpriteOnBone(entvars_t *const pev, EHANDLE<CBaseEntity> pAttachTo, uint16_t const BONE, Vector const vecOfs, float const INC, float const PEAK, float const DECAY, bool const OWNED) noexcept
{
	Vector vecBoneOrigin{}, vecTransformedOfs{};
	auto const me = ent_cast<edict_t *>(pev);

	for (; pAttachTo;)	// Must be a post-awaiting. Otherwise the validity of entity is subject to change after the check.
	{
		auto&& [f, r, u] = pAttachTo->pev->angles.AngleVectors();
		vecTransformedOfs = vecOfs.x * f + vecOfs.y * r + vecOfs.z * u;
		vecBoneOrigin = UTIL_GetBonePosition(pAttachTo.Get(), BONE);	// LUNA: DO NOT use the engine version, it's buggy.

		g_engfuncs.pfnSetOrigin(me, vecBoneOrigin + vecTransformedOfs);

		if (pev->renderamt < PEAK)
			pev->renderamt = std::min(pev->renderamt + INC, PEAK);

		co_await TaskScheduler::NextFrame::Rank[0];
	}

	for (; pev->renderamt > 0; pev->renderamt -= DECAY)
	{
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	// In case we over-subtracted.
	pev->renderamt = 0;

	// Don't remove it unless we own it!
	if (OWNED)
		pev->flags |= FL_KILLME;

	co_return;
}

Task Task_SpriteEnterLoopOut(entvars_t *const pev, EHANDLE<CBaseEntity> pExistanceRelyOn, uint16_t const LOOP_START_POS, uint16_t const LOOP_END_POS_EXC, uint16_t const FRAME_COUNT, float const FPS) noexcept
{
	uint16_t iFrame = 0;

	co_await float(1.0 / FPS);

	for (; pExistanceRelyOn && iFrame < LOOP_START_POS;)
	{
		pev->framerate = float(1.0 / FPS);
		pev->frame = ++iFrame;
		pev->animtime = gpGlobals->time;

		co_await float(1.0 / FPS);
	}

	auto const iLoopFrameCount = LOOP_END_POS_EXC - LOOP_START_POS;

	for (; pExistanceRelyOn;)
	{
		iFrame = (iFrame + 1) % iLoopFrameCount;

		pev->framerate = float(1.0 / FPS);
		pev->frame = float(LOOP_START_POS + iFrame);
		pev->animtime = gpGlobals->time;

		co_await float(1.0 / FPS);
	}

	for (iFrame += LOOP_START_POS; iFrame < FRAME_COUNT; ++iFrame)
	{
		pev->framerate = float(1.0 / FPS);
		pev->frame = iFrame;
		pev->animtime = gpGlobals->time;

		co_await float(1.0 / FPS);
	}

	pev->flags |= FL_KILLME;
}

Task Task_SineOpaque(entvars_t* pev, EHANDLE<CBaseEntity> pExistanceRelyOn, double OMEGA, double PHI, float LOWER_BOUND, float UPPER_BOUND, float DECAY) noexcept
{
	auto const RANGE = UPPER_BOUND - LOWER_BOUND;

	for (; pExistanceRelyOn;)
	{
		auto const flPercentage = (std::sin(OMEGA * gpGlobals->time + PHI) + 1.0) / 2.0;
		pev->renderamt = float(LOWER_BOUND + RANGE * flPercentage);

		co_await TaskScheduler::NextFrame::Rank[5];
	}

	for (; pev->renderamt > 0; pev->renderamt -= DECAY)
	{
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	pev->flags |= FL_KILLME;
	co_return;
}

//
// CFlame
//

Task CFlame::Task_DetectGround() noexcept	// Deprecated
{
	for (;;)
	{
		co_await 0.07f;

		if (pev->waterlevel != 0)
		{
			pev->flags |= FL_KILLME;
			co_return;
		}

		if (pev->flags & FL_ONGROUND)
		{
			TraceResult tr{};
			g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, -8192), ignore_monsters | ignore_glass, nullptr, &tr);

			MsgBroadcast(SVC_TEMPENTITY);
			WriteData(TE_WORLDDECAL);
			WriteData(tr.vecEndPos);
			WriteData((uint8_t)UTIL_GetRandomOne(Decal::SCORCH).m_Index);
			MsgEnd();

			pev->view_ofs = pev->origin + Vector(0, 0, 64.0 * pev->scale);

			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_NONE;

			g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// Set size is required if pev->solid changed.

			m_Scheduler.Enroll(Task_EmitSmoke(), TASK_ACTION);

			SetTouch(&CFlame::Touch_DealBurnDmg);
			co_return;
		}
	}
}

Task CFlame::Task_EmitLight() noexcept
{
	for (;;)
	{
		co_await UTIL_Random(0.1f, 0.2f);

		Vector const vecNoise = pev->scale * Vector(
			UTIL_Random(-24.0, 24.0),
			UTIL_Random(-24.0, 24.0),
			UTIL_Random(8.0, 12.0)
		);

		MsgPVS(SVC_TEMPENTITY, pev->view_ofs);
		WriteData(TE_DLIGHT);
		WriteData(pev->origin + vecNoise);	// pos
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

Task CFlame::Task_EmitSmoke() noexcept
{
	for (;;)
	{
		co_await UTIL_Random(0.5f, 0.8f);

		Vector const vecNoise = pev->scale * Vector(
			UTIL_Random(-24.0, 24.0),
			UTIL_Random(-24.0, 24.0),
			UTIL_Random(-60.0, -36.0)
		);

		MsgPVS(SVC_TEMPENTITY, pev->view_ofs);
		WriteData(TE_SMOKE);
		WriteData(pev->view_ofs + vecNoise);
		WriteData((short)Sprites::m_rgLibrary[UTIL_GetRandomOne(Sprites::BLACK_SMOKE)]);
		WriteData((uint8_t)UTIL_Random(10, 20));	// (scale in 0.1's)
		WriteData((uint8_t)UTIL_Random(15, 20));	// (framerate)
		MsgEnd();

		//MsgPVS(SVC_TEMPENTITY, pev->view_ofs);
		//WriteData(TE_FIREFIELD);
		//WriteData(pev->view_ofs + vecNoise);
		//WriteData((short)UTIL_Random(-32.0 * pev->scale, 32.0 * pev->scale));
		//WriteData((short)Sprite::m_rgLibrary[Sprite::BLACK_SMOKE]);
		//WriteData((uint8_t)UTIL_Random(2, 4));
		//WriteData((uint8_t)(TEFIRE_FLAG_SOMEFLOAT | TEFIRE_FLAG_ALPHA));
		//WriteData((uint8_t)25);
		//MsgEnd();
	}
}

void CFlame::Spawn() noexcept
{
	auto const iFlameSprIndex = UTIL_Random(0u, Sprites::FLAME.size() - 1);
	auto const& iFrameCount = Sprites::Frames::FLAME[iFlameSprIndex];
	static constexpr uint16_t FLAME_START_FRAME[] = { 4, 4 };
	static constexpr uint16_t FLAME_OUT_ENDS_AT[] = { 25, 24 };

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(192.f, 255.f);
	pev->rendercolor = Vector(255, UTIL_Random(220.0, 255.0), UTIL_Random(220.0, 255.0));	// Little variation from one to another.
	// No more frame assignment here, moved to Task_SpriteLoopOut().

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_TOSS;
	pev->gravity = 2.f;
	pev->scale = UTIL_Random(0.6f, 0.85f);

	g_engfuncs.pfnSetModel(edict(), Sprites::FLAME[iFlameSprIndex]);
	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 64.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	//m_Scheduler.Enroll(Task_SpriteLoop(pev, FLAME_START_FRAME[iFlameSprIndex], iFrameCount, 30));
	//m_Scheduler.Enroll(Task_DetectGround());
	m_Scheduler.Enroll(Task_EmitLight(), TASK_ACTION);
	//m_Scheduler.Enroll(Task_SqueezeDieout(pev, 3.f, 3.f, 0.01f));
	m_Scheduler.Enroll(Task_SpriteLoopOut(pev, FLAME_START_FRAME[iFlameSprIndex], iFrameCount, FLAME_OUT_ENDS_AT[iFlameSprIndex], UTIL_Random(9.f, 14.f), 30), TASK_ANIMATION);

	SetTouch(&CFlame::Touch_AttachingSurface);
}

void CFlame::Touch_AttachingSurface(CBaseEntity *pOther) noexcept
{
	if (!pOther || ent_cast<int>(pOther) != 0)
		return;

	if (pev->waterlevel != 0)
	{
		pev->flags |= FL_KILLME;
		return;
	}

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pev->origin,
		pev->origin + pev->velocity.Normalize() * 64 * std::numbers::sqrt3,	// for the edge case. the vertix-center dist of a cube is root 3.
		ignore_monsters | ignore_glass, nullptr, &tr);

	if (tr.flFraction < 1.f)
	{
		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_WORLDDECAL);
		WriteData(tr.vecEndPos);
		WriteData((uint8_t)UTIL_GetRandomOne(Decal::SCORCH).m_Index);
		MsgEnd();
	}

	pev->view_ofs = pev->origin + Vector(0, 0, 64.0 * pev->scale);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;

	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// Set size is required if pev->solid changed.

	m_Scheduler.Enroll(Task_EmitSmoke(), TASK_ACTION);

	SetTouch(&CFlame::Touch_DealBurnDmg);
}

void CFlame::Touch_DealBurnDmg(CBaseEntity *pOther) noexcept
{
	if (!pOther || pev_valid(pOther->pev) != EValidity::Full)
		return;

	if (pOther->pev->takedamage == DAMAGE_NO)
		return;

	auto const iEntIndex = ent_cast<int>(pOther->pev);
	if (m_rgflDamageInterval[iEntIndex] >= gpGlobals->time)
		return;

	pOther->TakeDamage(
		m_pOwner ? m_pOwner->pev : ent_cast<entvars_t *>(0),
		m_pOwner ? m_pOwner->pev : ent_cast<entvars_t *>(0),
		UTIL_Random(5.f, 12.f),
		DMG_SLOWBURN
	);

	m_rgflDamageInterval[iEntIndex] = gpGlobals->time + 0.5f;

	if (pOther->IsPlayer())
		Gas::TryCough((CBasePlayer *)pOther);
}

//
// CSmoke
//

Task CSmoke::Task_DriftColor(Vector const vecTargetColor) noexcept
{
	auto const vecDeltaColor = vecTargetColor - pev->rendercolor;
	auto LastTime = std::chrono::high_resolution_clock::now();

	for (; (pev->rendercolor - vecTargetColor).LengthSquared() > 1; )
	{
		co_await TaskScheduler::NextFrame::Rank[1];

		auto const CurTime = std::chrono::high_resolution_clock::now();
		auto const flTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(CurTime - LastTime).count() / 1'000'000'000.0;

		pev->rendercolor += vecDeltaColor * flTimeDelta;

		pev->rendercolor.x = std::clamp(pev->rendercolor.x, 0.f, 255.f);
		pev->rendercolor.y = std::clamp(pev->rendercolor.y, 0.f, 255.f);
		pev->rendercolor.z = std::clamp(pev->rendercolor.z, 0.f, 255.f);

		LastTime = CurTime;
	}

	pev->rendercolor = vecTargetColor;
	co_return;
}

Task CSmoke::Task_ReflectingFlame() noexcept
{
	static constexpr auto SEARCH_RADIUS = 192.0;

	TraceResult tr{};
	double flPercentage{}, flContribution{};

	for (;;)
	{
		co_await 0.05f;

		flPercentage = 0;
		g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, pev->absmin.z - 36.0), ignore_glass | ignore_monsters, nullptr, &tr);

		for (edict_t *pEdict : FIND_ENTITY_IN_SPHERE(tr.vecEndPos, (float)SEARCH_RADIUS))
		{
			flContribution = 0;

			if (pEdict->v.classname == MAKE_STRING(CFlame::CLASSNAME) || pEdict->v.classname == MAKE_STRING(CPhosphorus::CLASSNAME))
				flContribution = std::clamp(1.0 - (pEdict->v.origin - tr.vecEndPos).Length() / SEARCH_RADIUS, 0.0, 0.4);
			else if (pEdict->v.classname == MAKE_STRING(CFuelAirCloud::CLASSNAME))
			{
				if (CFuelAirCloud *pCloud = (CFuelAirCloud *)pEdict->pvPrivateData; pCloud->m_bIgnited)
					flContribution = 1;	// It's exploding!!
			}

			flPercentage = std::clamp(flPercentage + flContribution, 0.0, 1.0);
		}

		DriftToWhite(1.0 - flPercentage);
	}
}

void CSmoke::LitByFlame(bool const bShouldStartingWhite) noexcept
{
	if (!bShouldStartingWhite)
		pev->rendercolor = m_vecStartingLitClr + m_vecFlameClrToWhite * 0.4;

	if (!m_Scheduler.Exist(TASK_REFLECTING_FLAME))
		m_Scheduler.Enroll(Task_ReflectingFlame(), TASK_REFLECTING_FLAME);
}

void CSmoke::DriftToWhite(double const flPercentage) noexcept
{
	m_Scheduler.Delist(TASK_COLOR_DRIFT);
	m_Scheduler.Enroll(
		Task_DriftColor(m_vecStartingLitClr + m_vecFlameClrToWhite * std::clamp(flPercentage, 0.0, 1.0)),
		TASK_COLOR_DRIFT
	);
}

void CSmoke::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 0;
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(0, 1);//(float)UTIL_Random(0, 5);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * 12;
	pev->scale = UTIL_Random(1.f, 2.0f);

	g_engfuncs.pfnSetModel(edict(), Sprites::STATIC_SMOKE_THICK);
	g_engfuncs.pfnSetSize(edict(), MIN_SIZE * pev->scale, MAX_SIZE * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 32.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	if ((tr.vecEndPos - pev->origin).LengthSquared() < (64.0 * 64.0))	// Just in case it teleport to some random roof.
		g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_vecStartingLitClr = Vector(
		UTIL_Random(0xC3, 0xCD),	// r
		UTIL_Random(0x3E, 0x46),	// g
		UTIL_Random(0x05, 0x10)		// b
	);
	m_vecFlameClrToWhite = Vector(255, 255, 255) - m_vecStartingLitClr;

	m_Scheduler.Enroll(Task_Fade(pev, 1.5f, 0.055f, UTIL_Random(96.f, 128.f), UTIL_Random() ? 0.05f : -0.05f), TASK_FADE_IN | TASK_FADE_OUT);
}

void CSmoke::Touch(CBaseEntity *pOther) noexcept
{
	if (!pOther->IsPlayer())
		return;

	auto const pPlayer = (CBasePlayer *)pOther;

	Gas::TryCough(pPlayer);
}

void CThinSmoke::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 0;
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(0, 5);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * 12;
	pev->scale = UTIL_Random(0.9f, 1.3f);

	g_engfuncs.pfnSetModel(edict(), Sprites::STATIC_SMOKE_THIN);
	g_engfuncs.pfnSetSize(edict(), MIN_SIZE * pev->scale, MAX_SIZE * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 32.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_vecStartingLitClr = Vector(
		UTIL_Random(0xC3, 0xCD),	// r
		UTIL_Random(0x3E, 0x46),	// g
		UTIL_Random(0x05, 0x10)		// b
	);
	m_vecFlameClrToWhite = Vector(255, 255, 255) - m_vecStartingLitClr;

	// No default fading method.
}

Task CToxicSmoke::Task_InFloatOut() noexcept
{
	auto const PEAK = UTIL_Random(64.f, 96.f);
	auto const INC = 0.5f;
	auto const DEC = 0.055f;
	auto const ROLL = 0.06f;

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, -8192), ignore_glass | ignore_monsters, nullptr, &tr);

	pev->velocity = CrossProduct(tr.vecPlaneNormal, Vector(Vector2D(1, 0).Rotate(UTIL_Random(0, 359)), 0)) * UTIL_Random(3.f, 6.f);

	for (; pev->renderamt < PEAK;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt += INC;
		pev->angles.roll += ROLL;

		if (pev->velocity.Length() < 30)
			pev->velocity *= UTIL_Random(1.01, 1.1);
	}

	for (; pev->renderamt > 0;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DEC;
		pev->angles.roll += ROLL;
	}

	pev->flags |= FL_KILLME;
}

void CToxicSmoke::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 0;
	pev->rendercolor = Vector(255, 255, 191); // pale yellow
	pev->frame = (float)UTIL_Random(0, 5);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * 12;
	pev->scale = UTIL_Random(3.f, 5.f);

	g_engfuncs.pfnSetModel(edict(), Sprites::STATIC_SMOKE_THIN);
	g_engfuncs.pfnSetSize(edict(), MIN_SIZE * pev->scale, MAX_SIZE * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 32.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_vecStartingLitClr = Vector(
		UTIL_Random(0xC3, 0xCD),	// r
		UTIL_Random(0x3E, 0x46),	// g
		UTIL_Random(0x05, 0x10)		// b
	);
	m_vecFlameClrToWhite = Vector(255, 255, 255) - m_vecStartingLitClr;

	m_Scheduler.Enroll(Task_InFloatOut(), TASK_FADE_IN | TASK_FADE_OUT);
}

void CToxicSmoke::Touch(CBaseEntity *pOther) noexcept
{
	if (!pOther->IsPlayer())
		return;

	auto const pPlayer = (CBasePlayer *)pOther;

	Gas::TryCough(pPlayer);
	Gas::Intoxicate(pPlayer, this->pev, nullptr, (float)CVar::pim_toxic_dmg, (float)CVar::pim_toxic_inv);
}

Task CThickStaticSmoke::Task_Dispatch() noexcept
{
	// Simulating the smoke spreading process.

	for (;;)
	{
		Vector const vecOrigin{ pev->origin.x + UTIL_Random(-72.f, 72.f), pev->origin.y + UTIL_Random(-72.f, 72.f), pev->origin.z };

		auto pChild = Prefab_t::Create<CThinSmoke>(vecOrigin, Angles(0, 0, UTIL_Random(0, 360)));
		pChild->LitByFlame(true);
		pChild->m_Scheduler.Enroll(Task_Fade(pChild->pev, 1.5f, FADEOUT_SPEED, this->pev->renderamt / 2.f, 0.05f), TASK_FADE_IN | TASK_FADE_OUT);

		co_await UTIL_Random(2.f, 3.5f);
	}
}

Task CThickStaticSmoke::Task_Scaling() noexcept
{
	auto const flOriginalScale = pev->scale;

	for (auto flPercentage = (255.f - pev->renderamt) / 255.f;; flPercentage = (255.f - pev->renderamt) / 255.f)
	{
		co_await TaskScheduler::NextFrame::Rank[1];

		pev->scale = flOriginalScale * (1.f + flPercentage * 0.4f);	// fade out by zooming the SPR.
	}
}

void CThickStaticSmoke::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(0, 1);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->scale = UTIL_Random(1.3f, 1.7f);

	g_engfuncs.pfnSetModel(edict(), Sprites::STATIC_SMOKE_THICK);
	g_engfuncs.pfnSetSize(edict(), MIN_SIZE * pev->scale, MAX_SIZE * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 32.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_vecStartingLitClr = Vector(
		UTIL_Random(0xC3, 0xCD),	// r
		UTIL_Random(0x3E, 0x46),	// g
		UTIL_Random(0x05, 0x10)		// b
	);
	m_vecFlameClrToWhite = Vector(255, 255, 255) - m_vecStartingLitClr;

	m_Scheduler.Enroll(Task_FadeOut(pev, 0.f, FADEOUT_SPEED, 0.f), TASK_FADE_OUT);
	m_Scheduler.Enroll(Task_Dispatch(), TASK_ANIMATION);
	m_Scheduler.Enroll(Task_Scaling(), TASK_SCALING);
}

//
// CRaisedDust
//

void CFloatingDust::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(64.f, 128.f);
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * UTIL_Random(12.0, 18.0);
	pev->scale = UTIL_Random(2.f, 3.f);

	g_engfuncs.pfnSetModel(edict(), Sprites::LIFTED_DUST);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.
	g_engfuncs.pfnSetSize(edict(), MIN_SIZE * pev->scale, MAX_SIZE * pev->scale);

	m_Scheduler.Enroll(Task_SpriteLoop(pev, FRAME_COUNT, FPS), TASK_ANIMATION);
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.f, 0.07f, 0.07f), TASK_FADE_OUT);
}

void CFloatingDust::Touch(CBaseEntity *pOther) noexcept
{
	if (!pOther->IsPlayer())
		return;

	auto const pPlayer = (CBasePlayer *)pOther;

	Gas::TryCough(pPlayer);
}

//
// CDebris
//

Task CDebris::Task_Debris() noexcept
{
	for (; pev->renderamt > 0;)
	{
		co_await UTIL_Random(0.07f, 0.1f);

		MsgPVS(SVC_TEMPENTITY, pev->origin);
		WriteData(TE_SMOKE);
		WriteData(pev->origin);
		WriteData((short)Sprites::m_rgLibrary[UTIL_GetRandomOne(Sprites::BLACK_SMOKE)]);
		WriteData((uint8_t)UTIL_Random(5, 10));	// (scale in 0.1's)
		WriteData((uint8_t)UTIL_Random(15, 20));	// (framerate)
		MsgEnd();

		pev->renderamt -= 0.1f;
	}

	pev->flags |= FL_KILLME;
}

void CDebris::Spawn() noexcept
{
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = UTIL_Random(192.f, 255.f);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_TOSS;
	pev->gravity = 1.f;
	pev->velocity = get_spherical_coord(400, UTIL_Random(45, 135), UTIL_Random(0.0, 359.9));
	pev->avelocity = { 400, UTIL_Random(-400.0, 400.0), 0 };

	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::GIBS_METAL);
	g_engfuncs.pfnSetSize(edict(), Vector(-1, -1, -1), Vector(1, 1, 1));

	m_Scheduler.Enroll(Task_Debris(), TASK_ACTION);
}

void CDebris::Touch(CBaseEntity *pOther) noexcept
{
	if (pOther->pev->solid == SOLID_BSP)
		pev->flags |= FL_KILLME;
}

//
// CSpark
//

void CSparkMdl::Spawn() noexcept
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->gravity = 0;
	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxNone;
	pev->renderamt = UTIL_Random(192.f, 255.f);

	auto const iValue = UTIL_Random(0, 5);
	switch (iValue)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		pev->body = 0;
		pev->skin = iValue;
		break;

	case 4:
		pev->body = 1;
		break;

	case 5:
		pev->body = 2;
		break;

	default:
		std::unreachable();
	}

	g_engfuncs.pfnSetModel(edict(), Models::SPARK);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	m_Scheduler.Enroll(Task_Remove(pev, HOLD_TIME), TASK_TIME_OUT);
}

//
// CGunshotSmoke
//

void CGunshotSmoke::Spawn() noexcept
{
	auto const flLightness = UTIL_Random(1.0, 48.0);

	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = UTIL_Random(128.f, 192.f);	// Alpha?
	pev->rendercolor = Vector(0xD1, 0xC5, 0x9F);	// Color. Cannot be 0x000000
	pev->frame = 0;

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->gravity = 0;
	pev->scale = UTIL_Random(1.5f, 2.5f);

	g_engfuncs.pfnSetModel(edict(), UTIL_GetRandomOne(Sprites::BLACK_SMOKE));

	Vector const vecDir = m_tr.vecPlaneNormal + CrossProduct(m_tr.vecPlaneNormal,
		(m_tr.vecPlaneNormal - Vector::Up()).LengthSquared() < std::numeric_limits<float>::epsilon() ? Vector::Front() : Vector::Up()	// #INVESTIGATE why will consteval fail here?
	);

	pev->velocity = vecDir.Normalize() * UTIL_Random(72.0, 96.0);

	g_engfuncs.pfnSetOrigin(edict(), m_tr.vecEndPos + m_tr.vecPlaneNormal * 24.0 * pev->scale);	// The actual SPR size will be 36 on radius. Clip the outter plain black part and it will be 24.

	m_Scheduler.Enroll(Task_SpritePlayOnce(pev, Sprites::Frames::BLACK_SMOKE, FPS), TASK_ANIMATION);
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.f, 0.055f, 0.07f), TASK_FADE_OUT);
}

CGunshotSmoke *CGunshotSmoke::Create(const TraceResult &tr) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CGunshotSmoke>();

	pEdict->v.origin = tr.vecEndPos;

	pPrefab->m_tr = tr;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CGroundedDust
//

void CGroundedDust::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(64.f, 128.f);
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = 0;

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * UTIL_Random(12.0, 18.0);
	pev->scale = UTIL_Random(1.f, 1.2f);

	g_engfuncs.pfnSetModel(edict(), Sprites::GROUNDED_DUST);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_Scheduler.Enroll(Task_SpritePlayOnce(pev, FRAME_COUNT, FPS), TASK_ANIMATION);
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.f, 0.06f, 0.f), TASK_FADE_OUT);
}

//
// CSparkSpr
//

void CSparkSpr::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255.f;
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->velocity = Vector::Zero();
	pev->scale = UTIL_Random(0.35f, 0.45f);

	g_engfuncs.pfnSetModel(edict(), Sprites::SPARK);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.
	g_engfuncs.pfnSetSize(edict(), Vector(-0.2, -0.2, -0.2), Vector(0.2, 0.2, 0.2));	// such that we can collide with fuel-air cloud.

	m_Scheduler.Enroll(Task_Remove(pev, HOLD_TIME), TASK_TIME_OUT);
}

//
// CFuelAirCloud
//

Task CFuelAirCloud::Task_FadeIn(float const TRANSPARENT_INC, float const FINAL_VAL, float const ROLL) noexcept
{
	for (; pev->renderamt < FINAL_VAL;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt += TRANSPARENT_INC;
		pev->angles.roll += ROLL;
	}

	m_bFadeInDone = true;

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->angles.roll += ROLL;
	}
}

Task CFuelAirCloud::Task_Ignite(void) noexcept
{
	if (m_bIgnited)
		co_return;

	co_await 0.05f;

	// Enable the ability to ignite others
	m_bIgnited = true;

	pev->renderamt = 255.f;	// maximize brightness
	pev->rendercolor = Vector(255, 255, 255);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->velocity = Vector::Zero();
	pev->scale = UTIL_Random(1.f, 2.f);

	pev->angles = Angles();
	pev->framerate = 0;
	pev->frame = 0;
	pev->animtime = 0;

	auto const iSelectedFlame = UTIL_Random(0u, Sprites::GAS_EXPLO.size() - 1);
	auto const iFrameCount = Sprites::Frames::GAS_EXPLO[iSelectedFlame];

	g_engfuncs.pfnSetModel(edict(), Sprites::GAS_EXPLO[iSelectedFlame]);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetSize(edict(), Vector(-128, -128, -128) * pev->scale, Vector(128, 128, 128) * pev->scale);
	g_engfuncs.pfnDropToFloor(edict());

	m_Scheduler.Delist(TASK_FADE_IN | TASK_ANIMATION);
	m_Scheduler.Enroll(Task_SpritePlayOnce(pev, iFrameCount, 25), TASK_ANIMATION);	// 2 secs
	m_Scheduler.Enroll(Task_EmitLight(), TASK_ACTION);

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::FuelAirBomb::GAS_EXPLO), VOL_NORM, UTIL_Random(ATTN_NORM / 2.f, ATTN_NORM), SND_FL_NONE, UTIL_Random(88, 116));

	co_await 0.1f;

	if (!UTIL_Random(0, 3))
		Prefab_t::Create<CSmoke>(pev->origin + Vector(0, 0, UTIL_Random(-64.0, 64.0)), Angles(0, 0, UTIL_Random(0, 360)))->LitByFlame(false);

	if (!UTIL_Random(0, 4))
		Prefab_t::Create<CFlame>(pev->origin)->pev->velocity = get_spherical_coord(128, UTIL_Random(30.0, 60.0), UTIL_Random(0.0, 360.0));
}

Task CFuelAirCloud::Task_EmitLight(void) noexcept
{
	for (;;)
	{
		co_await UTIL_Random(0.1f, 0.2f);

		Vector const vecNoise = pev->scale * Vector(
			UTIL_Random(-36.0, 36.0),
			UTIL_Random(-36.0, 36.0),
			UTIL_Random(-36.0, 36.0)
		);

		MsgPVS(SVC_TEMPENTITY, pev->origin);
		WriteData(TE_DLIGHT);
		WriteData(pev->origin + vecNoise);	// pos
		WriteData((uint8_t)UTIL_Random(40, 50));	// rad in 10's
		WriteData((uint8_t)UTIL_Random(0xC3, 0xCD));	// r
		WriteData((uint8_t)UTIL_Random(0x3E, 0x46));	// g
		WriteData((uint8_t)UTIL_Random(0x05, 0x10));	// b
		WriteData((uint8_t)2);	// brightness
		WriteData((uint8_t)0);	// life in 10's
		WriteData((uint8_t)1);	// decay in 10's
		MsgEnd();
	}
}

Task CFuelAirCloud::Task_TimeOut(void) noexcept
{
	co_await 60.f;

	m_Scheduler.Delist(TASK_FADE_IN);
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.f, 0.06f, 0.07f), TASK_FADE_OUT);
}

void CFuelAirCloud::Spawn() noexcept
{
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 0.f;
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->velocity = Vector::Zero();
	pev->scale = UTIL_Random(2.f, 3.f);

	g_engfuncs.pfnSetModel(edict(), Sprites::LIFTED_DUST);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.
	g_engfuncs.pfnSetSize(edict(), Vector(-128, -128, -128) * pev->scale, Vector(128, 128, 128) * pev->scale);

	m_Scheduler.Enroll(Task_SpriteLoop(pev, FRAME_COUNT, FPS), TASK_ANIMATION);	// This should be removed as well, we are not going to loop on flame SPR later on.
	m_Scheduler.Enroll(Task_FadeIn(0.05f, 32.f, 0.07f), TASK_FADE_IN);
	m_Scheduler.Enroll(Task_TimeOut(), TASK_TIME_OUT);

	s_rgpAeroClouds.emplace_back(this);
}

void CFuelAirCloud::Touch(CBaseEntity *pOther) noexcept
{
	if (!pOther || pOther->pev->solid == SOLID_BSP)
		return;

	if (!m_bIgnited)
	{
		auto const &iClassName = pOther->pev->classname;

		if (iClassName == MAKE_STRING(CSparkSpr::CLASSNAME) ||
			iClassName == MAKE_STRING(CFlame::CLASSNAME) ||
			iClassName == MAKE_STRING(CDebris::CLASSNAME))
		{
			this->Ignite();
		}
		else if (auto const pGrenade = EHANDLE{ pOther }.As<CGrenade>();
			pGrenade != nullptr && !pGrenade->m_bIsC4)
		{
			// Save for the explosion think.
			pGrenade->m_pBombDefuser = this;
		}
		else if (pOther->IsPlayer())
		{
			auto const pPlayer = (CBasePlayer *)pOther;
			Gas::TryCough(pPlayer);
			Gas::Intoxicate(pPlayer, this->pev, m_pPlayer->pev, (float)CVar::fab_toxic_dmg, (float)CVar::fab_toxic_inv);
		}

		return;
	}

	// Was ignited.
	else
	{
		// Chain reaction.
		if (m_iIgnitedCounts < 3 && pOther->pev->classname == MAKE_STRING(CFuelAirCloud::CLASSNAME))
		{
			if (CFuelAirCloud *pCloud = (CFuelAirCloud *)pOther; !pCloud->m_bIgnited)
			{
				pCloud->Ignite();
				++m_iIgnitedCounts;
			}
		}

		// Hurting entities.
		else if (pOther->pev->takedamage != DAMAGE_NO)
		{
			if (pOther->IsPlayer())
			{
				ApplySuffocation((CBasePlayer *)pOther);

				if (CBasePlayer *pPlayer = (CBasePlayer *)pOther; pPlayer->m_iTeam == m_pPlayer->m_iTeam && gcvarFriendlyFire->value < 1)
					return;
			}

			pOther->TakeDamage(pev, m_pPlayer->pev, pev->renderamt * (float)CVar::fab_burning_dmg_mul, DMG_SLOWBURN);
		}
	}
}

CFuelAirCloud *CFuelAirCloud::Create(CBasePlayer *pPlayer, Vector const &vecOrigin) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CFuelAirCloud>();

	pEdict->v.angles = Angles(0, 0, UTIL_Random(0.0, 359.9));
	pEdict->v.origin = vecOrigin;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

void CFuelAirCloud::ApplySuffocation(CBasePlayer *pPlayer) noexcept
{
	// Cannot be static - the address will change when map changes.
	auto const pevWorld = &g_engfuncs.pfnPEntityOfEntIndex(0)->v;

	uint64_t const iPlayerTaskId = TASK_SUFFOCATION | (1ull << uint64_t(pPlayer->entindex() + 32ull));

	if (!TaskScheduler::Exist(iPlayerTaskId, false))
		TaskScheduler::Enroll(Task_PlayerSuffocation(pPlayer, pevWorld), iPlayerTaskId);
}

Task CFuelAirCloud::Task_AirPressure() noexcept
{
	array<double, 3> rgflPressureCenter{};
	Vector vecPressureCenter{}, vecDir{};
	int iIgnitedCount{};
	double flSpeed{}, flDistance{};

	for (;;)
	{
		co_await 0.1f;

		// Kick off all invalid clouds, especially after map reload.
		s_rgpAeroClouds.remove_if(
			[](EHANDLE<CFuelAirCloud> const &pCloud) noexcept -> bool { return !static_cast<bool>(pCloud); }
		);

		if (s_rgpAeroClouds.empty())
			continue;

		rgflPressureCenter.fill(0);
		iIgnitedCount = 0;

		std::ranges::for_each(s_rgpAeroClouds | std::views::filter([](EHANDLE<CFuelAirCloud> const &pCloud) noexcept { return pCloud->m_bIgnited; }),
			[&](Vector const &vecOrigin) noexcept { ++iIgnitedCount; rgflPressureCenter[0] += vecOrigin[0]; rgflPressureCenter[1] += vecOrigin[1]; rgflPressureCenter[2] += vecOrigin[2]; },
			[](EHANDLE<CFuelAirCloud> const &pCloud) noexcept -> Vector const& { return pCloud->pev->origin; }
		);

		if (iIgnitedCount < 10)
			continue;

		// equiv of 'vec /= fl'
		std::ranges::for_each(rgflPressureCenter,
			[flCount = (double)iIgnitedCount](double &val) noexcept { val /= flCount; }
		);

		vecPressureCenter = Vector(
			rgflPressureCenter[0],
			rgflPressureCenter[1],
			rgflPressureCenter[2]
		);

		for (CBasePlayer *pPlayer : Query::all_living_players())
		{
			vecDir = vecPressureCenter - pPlayer->pev->origin;
			flDistance = vecDir.Length();
			flSpeed = std::clamp<double>(2048.0 - flDistance, 0.0, 256.0);

			if (flSpeed > 1)
			{
				pPlayer->pev->velocity += vecDir.Normalize() * flSpeed;
				ApplySuffocation(pPlayer);	// #INVESTIGATE #SHOULD_DO_ON_FREE sometimes won't work??
			}
		}
	}
}

Task CFuelAirCloud::Task_PlayerSuffocation(CBasePlayer *pPlayer, entvars_t *pevWorld) noexcept
{
	auto LastTime = std::chrono::high_resolution_clock::now();

	g_engfuncs.pfnClientCommand(pPlayer->edict(), "spk %s\n", Sounds::PLAYER_HB_AND_ER);

	// 10 heart beats in the .wav
	for (auto i = 0; i < 10 && pPlayer->IsAlive(); ++i)
	{
		// Timer against the whole loop.
		LastTime = std::chrono::high_resolution_clock::now();

		pPlayer->TakeDamage(pevWorld, pevWorld, pPlayer->pev->health * 0.015f, DMG_DROWN);
		pPlayer->pev->punchangle.roll += UTIL_Random() ? -14.f : 14.f;

		gmsgScreenShake::Send(pPlayer->edict(),
			ScaledFloat<1 << 12>(15.0),	// amp
			ScaledFloat<1 << 12>(1.0),	// dur
			ScaledFloat<1 << 8>(12)		// freq
		);

		gmsgScreenFade::Send(pPlayer->edict(),
			ScaledFloat<1 << 12>(0.5),	// phase time
			ScaledFloat<1 << 12>(0.1),	// color hold
			FFADE_IN,	// flags
			160,		// r
			0,		// g
			0,		// b
			UTIL_Random(72, 128)			// a
		);

		int const iFOV = (i % 2) ? UTIL_Random(74, 79) : UTIL_Random(80, 85);

		for (; pPlayer->m_iFOV != iFOV && pPlayer->IsAlive();)
		{
			((iFOV - pPlayer->m_iFOV) > 0) ? ++pPlayer->m_iFOV : --pPlayer->m_iFOV;	// don't make the FOV-delta a constant and only eval once. It could cause compatibility issue.

			co_await TaskScheduler::NextFrame::Rank.back();
		}

		auto const CurTime = std::chrono::high_resolution_clock::now();
		auto const flTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(CurTime - LastTime).count() / 1'000'000'000.0;

		co_await float(0.77 - flTimeDelta);
	}

	for (; pPlayer->m_iFOV < 90 && pPlayer->IsAlive();)
	{
		co_await UTIL_GetRandomOne(TaskScheduler::NextFrame::Rank);
		++pPlayer->m_iFOV;
	}

	co_await 5.f;
}

Task CFuelAirCloud::Task_GlobalSuffocation() noexcept
{
	auto const StartingTime = std::chrono::high_resolution_clock::now();

	std::ranges::for_each(Query::all_living_players(), ApplySuffocation);

	auto const CurTime = std::chrono::high_resolution_clock::now();
	auto const flTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(CurTime - StartingTime).count() / 1'000'000'000.0;

	co_await float(g_rgflSoundTime.at(Sounds::PLAYER_HB_AND_ER) - flTimeDelta);
}

void CFuelAirCloud::OnTraceAttack(TraceResult const &tr, EHANDLE<CBaseEntity> pSkippedEntity) noexcept
{
	auto const pPlayer = pSkippedEntity.As<CBasePlayer>();

	[[unlikely]]
	if (pPlayer == nullptr)
		return;

	// Kick off all invalid clouds, especially after map reload.
	s_rgpAeroClouds.remove_if(
		[](EHANDLE<CFuelAirCloud> const &pCloud) noexcept -> bool { return !static_cast<bool>(pCloud); }
	);

	if (s_rgpAeroClouds.empty())
		return;

	if (auto pHitEntity = EHANDLE<CBaseEntity>(tr.pHit); pHitEntity.Is<CFuelAirExplosive>())
		return;

	for (auto &&pCloud : s_rgpAeroClouds |
		std::views::filter([](EHANDLE<CFuelAirCloud> const &pCloud) noexcept -> bool { return !pCloud->m_bIgnited; }))
	{
		if (((tr.vecEndPos + tr.vecPlaneNormal * 24.0) - pCloud->pev->origin).LengthSquared() < (72.0 * 72.0))
		{
			auto const pEdict = Prefab_t::Create<CSparkSpr>(tr.vecEndPos)->edict();
			g_engfuncs.pfnEmitSound(pEdict, CHAN_STATIC, UTIL_GetRandomOne(Sounds::HIT_METAL), VOL_NORM, ATTN_NORM, SND_FL_NONE, UTIL_Random(96, 108));

			Angles vecAngles{};
			g_engfuncs.pfnVecToAngles(-tr.vecPlaneNormal, vecAngles);
			Prefab_t::Create<CSparkMdl>(tr.vecEndPos, vecAngles);

			pCloud->Ignite();
		}
		else if ((pPlayer->pev->origin - pCloud->pev->origin).LengthSquared() < (72.0 * 72.0))
		{
			Vector vecAttachmentPos{};
			Angles vecAngles{};
			g_engfuncs.pfnGetAttachment(pPlayer->edict(), 0, vecAttachmentPos, vecAngles);

			auto const pEdict = Prefab_t::Create<CSparkSpr>(vecAttachmentPos)->edict();
			g_engfuncs.pfnEmitSound(pEdict, CHAN_STATIC, UTIL_GetRandomOne(Sounds::HIT_METAL), VOL_NORM, ATTN_NORM, SND_FL_NONE, UTIL_Random(96, 108));

			pCloud->Ignite();
		}
	}
}

//
// CSpriteDisplay
//

CSpriteDisplay *CSpriteDisplay::Create(Vector const &vecOrigin, std::string_view szModel, CBasePlayer* pPlayer) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CSpriteDisplay>();

	g_engfuncs.pfnSetOrigin(pEdict, vecOrigin);
	g_engfuncs.pfnSetModel(pEdict, szModel.data());
	g_engfuncs.pfnSetSize(pEdict, Vector::Zero(), Vector::Zero());

	pEdict->v.angles = Angles();

	pEdict->v.rendermode = kRenderTransAdd;	// For SPRITE, there is no other option, basically.
	pEdict->v.renderamt = 128.f;
	pEdict->v.rendercolor = Vector(255, 255, 255);
	pEdict->v.frame = 0;

	pEdict->v.solid = SOLID_NOT;
	pEdict->v.movetype = MOVETYPE_NONE;
	pEdict->v.gravity = 0;
	pEdict->v.velocity = Vector::Zero();
	pEdict->v.scale = 1.f;

	pPrefab->pev->nextthink = 0.1f;
	pPrefab->m_pPlayer = pPlayer;	// Only used in elective display

	return pPrefab;
}

//
// CPhosphorus
//

CPhosphorus::~CPhosphorus() noexcept
{
	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::Thermite::BURNING_LOOP, 0.75f, ATTN_STATIC, SND_STOP, UTIL_Random(88, 112));
	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::Thermite::BURNING_END, 0.75f, ATTN_STATIC, SND_FL_NONE, UTIL_Random(88, 112));
}

void CPhosphorus::Spawn() noexcept
{
	static const auto FRAME_COUNT = g_rgiSpriteFrameCount.at(Sprites::PHOSPHORUS_TRACE_HEAD);

	pev->scale = 1.f;

	g_engfuncs.pfnSetOrigin(edict(), pev->origin);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.
	g_engfuncs.pfnSetModel(edict(), Sprites::PHOSPHORUS_TRACE_HEAD);
	g_engfuncs.pfnSetSize(edict(), Vector(-4, -4, -4), Vector(4, 4, 4));	// LUNA: the size must be exactly 2*2*2. Tried 32^3 and it gets stuck on a ghost wall in the middle of nowhere.

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(192.f, 255.f);
	pev->rendercolor = Vector(255, UTIL_Random(220.0, 255.0), UTIL_Random(220.0, 255.0));
	pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);	// in case it looks too overlappy..

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->takedamage = DAMAGE_NO;

	m_Scheduler.Enroll(Task_SpriteLoop(pev, FRAME_COUNT, 30), TASK_ANIMATION);
	m_Scheduler.Enroll(Task_Remove(pev, UTIL_Random((float)CVar::pim_phos_burning_time - 5.f, (float)CVar::pim_phos_burning_time + 5.f)), TASK_TIME_OUT);
	m_Scheduler.Enroll(Task_Gravity(), TASK_FLYING);
	m_Scheduler.Enroll(Task_EmitExhaust(), TASK_FLYING | TASK_ACTION);

	SetTouch(&CPhosphorus::Touch_Flying);
}

void CPhosphorus::Touch_Flying(CBaseEntity *pOther) noexcept
{
	g_engfuncs.pfnTraceLine(pev->origin, pev->origin + pev->velocity.Normalize() * 32, ignore_monsters | ignore_glass, edict(), &m_tr);
	if (g_engfuncs.pfnPointContents(m_tr.vecEndPos) == CONTENTS_SKY)
	{
		pev->flags |= FL_KILLME;
		return;
	}

	if (!pOther->IsBSPModel())
		return;

	// Not a real spark, it's just a visual representation of 'too bright to properly see what's going on'
	auto pSpark = CSpriteDisplay::Create(Vector(pev->origin.x, pev->origin.y, pev->origin.z + 48.0), Sprites::PHOSPHORUS_MAJOR_SPARK);
	pSpark->pev->renderamt = UTIL_Random(192.f, 255.f);
	pSpark->pev->rendercolor = Vector(255, 255, UTIL_Random(192, 255));
	pSpark->pev->scale = UTIL_Random(1.f / 1.2f, 1.2f);
	pSpark->m_Scheduler.Enroll(Task_FadeOut(pSpark->pev, UTIL_Random(0.05f, 0.2f), 10.f, 0), TASK_FADE_OUT);

	static const auto FRAME_COUNT = g_rgiSpriteFrameCount.at(Sprites::PHOSPHORUS_FLAME);

	pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);	// in case it looks too overlappy..

	pev->velocity = Vector::Zero();
	pev->movetype = MOVETYPE_NONE;
	pev->view_ofs = pev->origin + Vector(0, 0, 64);

	g_engfuncs.pfnSetModel(edict(), Sprites::PHOSPHORUS_FLAME);
	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -108) * pev->scale, Vector(32, 32, 108) * pev->scale);
	g_engfuncs.pfnSetOrigin(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 108.f));	// compensate the difference in size.

	m_Scheduler.Delist(TASK_ANIMATION | TASK_FLYING);
	m_Scheduler.Enroll(Task_SpriteLoop(pev, FRAME_COUNT, 16), TASK_ANIMATION);
	m_Scheduler.Enroll(Task_EmitSmoke(), TASK_ACTION);
	m_Scheduler.Enroll(Task_EmitSpark(), TASK_ACTION);
	m_Scheduler.Enroll(Task_EmitLight(), TASK_ACTION);

	SetTouch(&CPhosphorus::Touch_Burning);

	MsgPVS(SVC_TEMPENTITY, pev->origin);	// the light for the spark
	WriteData(TE_DLIGHT);
	WriteData(pev->origin);	// pos
	WriteData((uint8_t)40);	// rad in 10's
	WriteData((uint8_t)255);	// r
	WriteData((uint8_t)255);	// g
	WriteData((uint8_t)255);	// b
	WriteData((uint8_t)8);		// life in 10's
	WriteData((uint8_t)60);	// decay in 10's
	MsgEnd();

	if (m_tr.pHit && m_tr.pHit->v.solid == SOLID_BSP)
		UTIL_Decal(m_tr.pHit, m_tr.vecEndPos, UTIL_GetRandomOne(Decal::SCORCH).m_Index);

	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::Thermite::BURNING_LOOP, 0.75f, ATTN_STATIC, SND_FL_NONE, UTIL_Random(88, 112));

	for (auto &&pPlayer :
		Query::all_living_players()
		| std::views::filter([&](CBasePlayer *pPlayer) noexcept { return (pev->origin - pPlayer->pev->origin).LengthSquared() < (150 * 150);})
		)
	{
		Burning::ByPhosphorus(pPlayer, m_pPlayer);
	}
}

void CPhosphorus::Touch_Burning(CBaseEntity *pOther) noexcept
{
	if (!pOther || pev_valid(pOther->pev) != EValidity::Full)
		return;

	if (pOther->pev->takedamage == DAMAGE_NO)
		return;

	auto const iEntIndex = ent_cast<int>(pOther->pev);
	if (m_rgflDamageInterval[iEntIndex] >= gpGlobals->time)
		return;

	pOther->TakeDamage(
		this->pev,
		m_pPlayer->pev,
		UTIL_Random((float)CVar::pim_touch_burning_dmg - 5.f, (float)CVar::pim_touch_burning_dmg + 5.f),
		DMG_SLOWBURN
	);

	m_rgflDamageInterval[iEntIndex] = gpGlobals->time + (float)CVar::pim_touch_burning_inv;	// Well, white phosphorus does the job...
}

Task CPhosphorus::Task_Gravity() noexcept
{
	auto const START_TIME = gpGlobals->time;

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[1];

		if (pev->gravity != 0)
			//pev->velocity.z = (gpGlobals->time - START_TIME) * (float)-386.08858267717 * pev->gravity;
			pev->velocity.z -= float(gpGlobals->frametime * 386.08858267717 * pev->gravity);

		pev->angles = pev->velocity.VectorAngles();

		// GoldSrc Mystery #1: The fucking v_angle and angles.
		pev->v_angle = Angles(
			-pev->angles.pitch,
			pev->angles.yaw,
			pev->angles.roll
		);
	}
}

Task CPhosphorus::Task_EmitExhaust() noexcept
{
	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());	// short (entity:attachment to follow)
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);	// short (sprite index)
	WriteData((uint8_t)UTIL_Random(20, 40));	// byte (life in 0.1's) 
	WriteData((uint8_t)3);		// byte (line width in 0.1's) 
	WriteData((uint8_t)255);	// r
	WriteData((uint8_t)255);	// g
	WriteData((uint8_t)191);	// b
	WriteData((uint8_t)255);	// byte (brightness)
	MsgEnd();

	// If the phosphorus gets spawned directly from the sky, then we actually needs to wait.
	//co_await 0.5f;

	for (;;)
	{
		co_await UTIL_Random(0.1f, 0.2f);

		auto const vecOrigin = pev->origin + pev->v_angle.Front() * -48;

		//MsgBroadcast(SVC_TEMPENTITY);
		//WriteData(TE_SPRITE);
		//WriteData(vecOrigin);
		//WriteData(Sprites::m_rgLibrary[UTIL_GetRandomOne(Sprites::ROCKET_TRAIL_SMOKE)]);
		//WriteData((byte)UTIL_Random<short>(1, 10));
		//WriteData((byte)UTIL_Random<short>(50, 255));
		//MsgEnd();

		auto pSpark = CSpriteDisplay::Create(vecOrigin, Sprites::ROCKET_TRAIL_SMOKE[0]);
		pSpark->pev->renderamt = UTIL_Random(50.f, 255.f);
		pSpark->pev->rendercolor = Vector(255, 255, UTIL_Random(192, 255));
		pSpark->pev->frame = (float)UTIL_Random(17, 22);
		pSpark->pev->scale = UTIL_Random(0.3f, 1.1f);
		pSpark->m_Scheduler.Enroll(Task_FadeOut(pSpark->pev, /*STAY*/ 0.2f, /*DECAY*/ UTIL_Random(0.1f, 0.5f), /*ROLL*/ 0, /*SCALE_INC*/ UTIL_Random(0.35f, 0.55f)), TASK_FADE_OUT);
	}
}

Task CPhosphorus::Task_EmitSmoke() noexcept
{
	if (UTIL_Random())
		co_await 4.f;

	for (;;)
	{
		co_await UTIL_Random(4.f, 8.f);

		if (UTIL_Random())
			Prefab_t::Create<CToxicSmoke>(pev->origin + Vector(0, 0, UTIL_Random(-64.0, 64.0)), Angles(0, 0, UTIL_Random(0, 360)))->LitByFlame(false);
	}
}

Task CPhosphorus::Task_EmitSpark() noexcept
{
	if (UTIL_Random())
		co_await UTIL_Random(4.f, 8.f);

	for (;;)
	{
		Vector const vecGround{
			pev->origin.x,
			pev->origin.y,
			pev->absmin.z + 16.0
		};

		Vector const vecVel = get_spherical_coord(1, UTIL_Random(30.0, 60.0), UTIL_Random(0, 360));

		CShower2::Create(m_pPlayer, vecGround, vecVel);

		co_await UTIL_Random(4.f, 8.f);
	}
}

Task CPhosphorus::Task_EmitLight() noexcept
{
	for (;;)
	{
		co_await UTIL_Random(0.1f, 0.2f);

		Vector const vecNoise{
			pev->origin.x + UTIL_Random(-24.0, 24.0),
			pev->origin.y + UTIL_Random(-24.0, 24.0),
			pev->absmin.z + 16.0
		};

		MsgPVS(SVC_TEMPENTITY, pev->view_ofs);
		WriteData(TE_DLIGHT);
		WriteData(vecNoise);	// pos
		WriteData((uint8_t)UTIL_Random(11, 13));	// rad in 10's
		WriteData((uint8_t)255);	// r
		WriteData((uint8_t)255);	// g
		WriteData((uint8_t)UTIL_Random(190, 210));	// b
		WriteData((uint8_t)2);	// brightness
		WriteData((uint8_t)0);	// life in 10's
		WriteData((uint8_t)1);	// decay in 10's
		MsgEnd();
	}
}

CPhosphorus *CPhosphorus::Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector2D const &vecInitVel) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CPhosphorus>();

	pEdict->v.origin = vecOrigin;
	pEdict->v.gravity = UTIL_Random(1.f / 1.1f, 1.1f);

	auto const flSpeed = vecInitVel.Length();

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->Spawn();
	pPrefab->pev->velocity = { vecInitVel, 0 };
	pPrefab->pev->avelocity = { flSpeed, UTIL_Random(-flSpeed, flSpeed), 0 };
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

CPhosphorus *CPhosphorus::Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CPhosphorus>();

	pEdict->v.origin = vecOrigin;
	pEdict->v.gravity = UTIL_Random(1.f / 1.1f, 1.1f);

	assert(vecTarget.z < vecOrigin.z);

	auto const vecDir = vecTarget.Make2D() - vecOrigin.Make2D();
	auto const H = vecOrigin.z - vecTarget.z;
	auto const S = vecDir.Length();
	auto const G = 386.08858267717 * pEdict->v.gravity;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->Spawn();
	pPrefab->pev->velocity = { vecDir.Normalize() * (S * std::sqrt(G / (2 * H))), 0 }; auto const flSpeed = pPrefab->pev->velocity.Length();
	pPrefab->pev->avelocity = { flSpeed, UTIL_Random(-flSpeed, flSpeed), 0 };
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

CPhosphorus *CPhosphorus::Create(CBasePlayer *pPlayer, Vector const &vecOrigin) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CPhosphorus>();

	pEdict->v.origin = vecOrigin;
	pEdict->v.gravity = UTIL_Random(1.f / 1.1f, 1.1f);

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CShower2
// Same as the original one, but with light.
//

Task CShower2::Task_Flashing() noexcept
{
	for (;;)
	{
		co_await 0.1f;

		MsgPVS(SVC_TEMPENTITY, pev->origin);
		WriteData(TE_SPARKS);
		WriteData(pev->origin);
		MsgEnd();

		MsgPVS(SVC_TEMPENTITY, pev->origin);
		WriteData(TE_DLIGHT);
		WriteData(pev->origin);	// pos
		WriteData((uint8_t)UTIL_Random(12, 14));	// rad in 10's
		WriteData((uint8_t)255);	// r
		WriteData((uint8_t)255);	// g
		WriteData((uint8_t)255);	// b
		WriteData((uint8_t)2);	// brightness
		WriteData((uint8_t)0);	// life in 10's
		WriteData((uint8_t)1);	// decay in 10's
		MsgEnd();

		pev->speed -= 0.1f;

		if (pev->speed <= 0)
			pev->flags |= FL_KILLME;

		pev->flags &= ~FL_ONGROUND;
	}
}

void CShower2::Touch(CBaseEntity *pOther) noexcept
{
	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pev->origin, pev->origin + pev->velocity.Normalize() * 16, ignore_monsters | ignore_glass, nullptr, &tr);

	if (tr.pHit && tr.pHit->v.solid == SOLID_BSP)
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::SMALL_SCORCH).m_Index);

	if (pOther->IsPlayer() && pOther->IsAlive())
		Burning::ByPhosphorus((CBasePlayer *)pOther, m_pPlayer);
}

CShower2 *CShower2::Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecDir) noexcept
{
	static const auto FRAME_COUNT = g_rgiSpriteFrameCount.at(Sprites::PHOSPHORUS_MINOR_SPARK);

	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CShower2>();

	pPrefab->m_pPlayer = pPlayer;

	pPrefab->pev->velocity = UTIL_Random(200, 300) * vecDir;
	pPrefab->pev->velocity.x += UTIL_Random(-100, 100);
	pPrefab->pev->velocity.y += UTIL_Random(-100, 100);

	if (pPrefab->pev->velocity.z >= 0)
		pPrefab->pev->velocity.z += 200;
	else
		pPrefab->pev->velocity.z -= 200;

	pPrefab->pev->movetype = MOVETYPE_BOUNCE;
	pPrefab->pev->gravity = 0.5f;
	pPrefab->pev->solid = SOLID_TRIGGER;

	g_engfuncs.pfnSetOrigin(pEdict, vecOrigin);
	g_engfuncs.pfnSetModel(pEdict, Sprites::PHOSPHORUS_MINOR_SPARK);	// Need a model to perform all sorts of movements
	g_engfuncs.pfnSetSize(pEdict, Vector::Zero(), Vector::Zero());

	pPrefab->pev->rendermode = kRenderTransAdd;
	pPrefab->pev->renderamt = UTIL_Random(192.f, 255.f);
	pPrefab->pev->rendercolor = Vector(255, UTIL_Random(220.0, 255.0), UTIL_Random(220.0, 255.0));
	pPrefab->pev->frame = (float)UTIL_Random(0, FRAME_COUNT - 1);	// in case it looks too overlappy..
	pPrefab->pev->scale = 0.3f;

	pPrefab->pev->speed = UTIL_Random(0.5f, 1.5f);
	pPrefab->pev->angles = pPrefab->pev->velocity.VectorAngles();

	pPrefab->pev->nextthink = 0.1f;
	pPrefab->m_Scheduler.Enroll(pPrefab->Task_Flashing(), TASK_ACTION);
	pPrefab->m_Scheduler.Enroll(Task_SpriteLoop(pPrefab->pev, FRAME_COUNT, 24), TASK_ANIMATION);

	return pPrefab;
}
