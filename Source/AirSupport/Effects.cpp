import <array>;
import <chrono>;
import <numbers>;

import edict;
import shake;
import util;

import Effects;
import Math;
import Projectile;
import Query;
import Resources;

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

Task Task_FadeOut(entvars_t *const pev, float const DECAY, float const ROLL) noexcept
{
	for (; pev->renderamt > 0;)
	{
		co_await TaskScheduler::NextFrame::Rank.back();

		pev->renderamt -= DECAY;
		pev->angles.roll += ROLL;
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
			WriteData((byte)UTIL_GetRandomOne(Decal::SCORCH).m_Index);
			MsgEnd();

			pev->view_ofs = pev->origin + Vector(0, 0, 64.0 * pev->scale);

			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_NONE;

			g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// Set size is required if pev->solid changed.

			m_Scheduler.Enroll(Task_EmitSmoke());

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
		WriteData((byte)UTIL_Random(12, 14));	// rad in 10's
		WriteData((byte)UTIL_Random(0xC3, 0xCD));	// r
		WriteData((byte)UTIL_Random(0x3E, 0x46));	// g
		WriteData((byte)UTIL_Random(0x05, 0x10));	// b
		WriteData((byte)2);	// brightness
		WriteData((byte)0);	// life in 10's
		WriteData((byte)1);	// decay in 10's
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
		WriteData((byte)UTIL_Random(10, 20));	// (scale in 0.1's)
		WriteData((byte)UTIL_Random(15, 20));	// (framerate)
		MsgEnd();

		//MsgPVS(SVC_TEMPENTITY, pev->view_ofs);
		//WriteData(TE_FIREFIELD);
		//WriteData(pev->view_ofs + vecNoise);
		//WriteData((short)UTIL_Random(-32.0 * pev->scale, 32.0 * pev->scale));
		//WriteData((short)Sprite::m_rgLibrary[Sprite::BLACK_SMOKE]);
		//WriteData((byte)UTIL_Random(2, 4));
		//WriteData((byte)(TEFIRE_FLAG_SOMEFLOAT | TEFIRE_FLAG_ALPHA));
		//WriteData((byte)25);
		//MsgEnd();
	}
}

void CFlame::Spawn() noexcept
{
	auto const iFlameSprIndex = UTIL_Random(0u, Sprites::FLAME.size() - 1);
	auto const iFrameCount = Sprites::Frames::FLAME[iFlameSprIndex];

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(192.f, 255.f);
	pev->frame = (float)UTIL_Random(0, iFrameCount);

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

	m_Scheduler.Enroll(Task_SpriteLoop(pev, iFrameCount, 30));
	//m_Scheduler.Enroll(Task_DetectGround());
	m_Scheduler.Enroll(Task_EmitLight());
	m_Scheduler.Enroll(Task_Remove(pev, UTIL_Random(9.f, 14.f)));

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
		WriteData((byte)UTIL_GetRandomOne(Decal::SCORCH).m_Index);
		MsgEnd();
	}

	pev->view_ofs = pev->origin + Vector(0, 0, 64.0 * pev->scale);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;

	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// Set size is required if pev->solid changed.

	m_Scheduler.Enroll(Task_EmitSmoke());

	SetTouch(&CFlame::Touch_DealBurnDmg);
}

void CFlame::Touch_DealBurnDmg(CBaseEntity *pOther) noexcept
{
	if (!pOther || pev_valid(pOther->pev) != 2)
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

			if (pEdict->v.classname == MAKE_STRING(CFlame::CLASSNAME))
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
	const char *pszModel = nullptr;
	array rgiValidFrame{ 0, 1 };

	switch (UTIL_Random(0, 2))
	{
	case 0:
		pszModel = Sprites::SMOKE;
		rgiValidFrame = { 2, 6 };
		break;

	case 1:
		pszModel = Sprites::SMOKE_1;
		rgiValidFrame = { 7, 10 };
		break;

	default:
		pszModel = Sprites::SMOKE_2;
		rgiValidFrame = { 16, 24 };
	}

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(64.f, 128.f);
	pev->rendercolor = Vector(255, 255, 255);
	pev->frame = (float)UTIL_Random(rgiValidFrame[0], rgiValidFrame[1]);

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * 12;
	pev->scale = UTIL_Random(3.f, 5.f);

	g_engfuncs.pfnSetModel(edict(), pszModel);
	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -32) * pev->scale, Vector(32, 32, 32) * pev->scale);	// it is still required for pfnTraceMonsterHull

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

	m_Scheduler.Enroll(Task_FadeOut(pev, 0.055f, 0.07f));
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

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->gravity = 0;
	pev->velocity = Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), 0).Normalize() * UTIL_Random(12.0, 18.0);
	pev->scale = UTIL_Random(2.f, 3.f);

	g_engfuncs.pfnSetModel(edict(), Sprites::LIFTED_DUST);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.
	g_engfuncs.pfnSetSize(edict(), Vector(-128, -128, -128) * pev->scale, Vector(128, 128, 128) * pev->scale);

	m_Scheduler.Enroll(Task_SpriteLoop(pev, FRAME_COUNT, FPS));
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.07f, 0.07f));
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
		WriteData((byte)UTIL_Random(5, 10));	// (scale in 0.1's)
		WriteData((byte)UTIL_Random(15, 20));	// (framerate)
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

	m_Scheduler.Enroll(Task_Debris());
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

	m_Scheduler.Enroll(Task_Remove(pev, HOLD_TIME));
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
		(m_tr.vecPlaneNormal - Vector::Up()).LengthSquared() < std::numeric_limits<float>::epsilon() ? Vector::Forward() : Vector::Up()	// #INVESTIGATE why will consteval fail here?
	);

	pev->velocity = vecDir.Normalize() * UTIL_Random(72.0, 96.0);

	g_engfuncs.pfnSetOrigin(edict(), m_tr.vecEndPos + m_tr.vecPlaneNormal * 24.0 * pev->scale);	// The actual SPR size will be 36 on radius. Clip the outter plain black part and it will be 24.

	m_Scheduler.Enroll(Task_SpritePlayOnce(pev, Sprites::Frames::BLACK_SMOKE, FPS));
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.055f, 0.07f));
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

	m_Scheduler.Enroll(Task_SpritePlayOnce(pev, FRAME_COUNT, FPS));
	m_Scheduler.Enroll(Task_FadeOut(pev, 0.06f, 0.f));
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

	m_Scheduler.Enroll(Task_Remove(pev, HOLD_TIME));
}

//
// PlayerCough
//

[[nodiscard]]
pair<bool/*ShouldCough*/, bool/*Toxic*/> UTIL_AnySmokeNear(Vector const &vecSrc) noexcept
{
	for (auto &&pSmoke :
		FIND_ENTITY_IN_SPHERE(vecSrc, float(32.0 * std::numbers::sqrt3)) |
		std::views::filter([](edict_t *pEdcit) noexcept { return pEdcit->v.classname == MAKE_STRING(CSmoke::CLASSNAME); })
		)
	{
		// Any CSmoke entity near the head?

		if (pSmoke->v.renderamt < 32)
			continue;

		return { true, false };
	}

	for (auto &&pFloatingDust :
		FIND_ENTITY_IN_SPHERE(vecSrc, float(72.0 * std::numbers::sqrt3)) |
		std::views::filter([](edict_t *pEdcit) noexcept { return pEdcit->v.classname == MAKE_STRING(CFloatingDust::CLASSNAME); })
		)
	{
		// Any CFloatingDust entity near the head?

		if (pFloatingDust->v.renderamt < 32)
			continue;

		return { true, false };
	}

	for (auto &&pFlame :
		FIND_ENTITY_IN_SPHERE(vecSrc, 72.f) |
		std::views::filter([](edict_t *pEdcit) noexcept { return pEdcit->v.classname == MAKE_STRING(CFlame::CLASSNAME); })
		)
	{
		// Any CFlame entity near the head?

		return { true, false };
	}

	for (auto &&pCloud :
		FIND_ENTITY_IN_SPHERE(vecSrc, 72.f) |
		std::views::filter([](edict_t *pEdcit) noexcept { return pEdcit->v.classname == MAKE_STRING(CFuelAirCloud::CLASSNAME); })
		)
	{
		// Any CFuelAirCloud entity near the head?
		// It's quite toxic so the transparency doesn't matter.

		return { true, true };
	}

	return { false, false };
}

Task Task_GlobalCoughThink() noexcept
{
	array<float, 33> rgflTimeNextCough{};
	array<float, 33> rgflTimeNextInhale{};
	auto const pevWorld = &g_engfuncs.pfnPEntityOfEntIndex(0)->v;

	co_await 0.1f;

	for (;;)
	{
		for (CBasePlayer *pPlayer : Query::all_living_players())	// #INVESTIGATE #POTENTIAL_BUG awaiting in this loop can cause CTD in release mod. WHY???
		{
			auto const [bShouldCough, bIsToxic] = UTIL_AnySmokeNear(pPlayer->EyePosition());

			if (bShouldCough)
			{
				if (auto const iIndex = pPlayer->entindex(); rgflTimeNextCough[iIndex] < gpGlobals->time)
				{
					rgflTimeNextCough[iIndex] = gpGlobals->time + UTIL_Random(3.f, 3.5f);
					g_engfuncs.pfnEmitSound(pPlayer->edict(), CHAN_AUTO, UTIL_GetRandomOne(Sounds::PLAYER_COUGH), VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 116));
				}
			}

			if (bIsToxic && pPlayer->pev->takedamage != DAMAGE_NO)
			{
				if (auto const iIndex = pPlayer->entindex(); rgflTimeNextInhale[iIndex] < gpGlobals->time)
				{
					rgflTimeNextInhale[iIndex] = gpGlobals->time + UTIL_Random(1.5f, 2.5f);
					pPlayer->TakeDamage(pevWorld, pevWorld, 5.f, DMG_POISON);
				}
			}
		}

		co_await 0.1f;
	}
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
	m_Scheduler.Enroll(Task_SpritePlayOnce(pev, iFrameCount, 20));
	m_Scheduler.Enroll(Task_EmitLight());

	g_engfuncs.pfnEmitSound(edict(), CHAN_WEAPON, UTIL_GetRandomOne(Sounds::FuelAirBomb::GAS_EXPLO), VOL_NORM, UTIL_Random(ATTN_NORM / 2.f, ATTN_NORM), 0, UTIL_Random(88, 116));

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
		WriteData((byte)UTIL_Random(40, 50));	// rad in 10's
		WriteData((byte)UTIL_Random(0xC3, 0xCD));	// r
		WriteData((byte)UTIL_Random(0x3E, 0x46));	// g
		WriteData((byte)UTIL_Random(0x05, 0x10));	// b
		WriteData((byte)2);	// brightness
		WriteData((byte)0);	// life in 10's
		WriteData((byte)1);	// decay in 10's
		MsgEnd();
	}
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

	m_Scheduler.Enroll(Task_SpriteLoop(pev, FRAME_COUNT, FPS), TASK_ANIMATION);	// This should be removed as well, we are not going to loop on flame SPR.
	m_Scheduler.Enroll(Task_FadeIn(0.05f, 32.f, 0.07f), TASK_FADE_IN);

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

			pOther->TakeDamage(pev, m_pPlayer->pev, pev->renderamt, DMG_SLOWBURN);
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

	uint64_t const iPlayerTaskId = TASK_HB_AND_ER | (1ull << uint64_t(pPlayer->entindex() + 32ull));

	if (!TaskScheduler::Exist(iPlayerTaskId))
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
			[](EHANDLE<CFuelAirCloud> const &pCloud) noexcept -> Vector const &{ return pCloud->pev->origin; }
		);

		if (iIgnitedCount <= 0)
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
			flSpeed = std::clamp<double>(4096.0 - flDistance, 0.0, 256.0);

			if (flSpeed > 1)
				pPlayer->pev->velocity += vecDir.Normalize() * flSpeed;

			if (flDistance < 1024)
				ApplySuffocation(pPlayer);	// #FIXME #POTENTIAL_BUG sometimes won't work??
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

	co_await float(12.5 - flTimeDelta);	// 13 is the length of audio file Sounds::PLAYER_HB_AND_ER
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
			g_engfuncs.pfnEmitSound(pEdict, CHAN_STATIC, UTIL_GetRandomOne(Sounds::HIT_METAL), VOL_NORM, ATTN_NORM, 0, UTIL_Random(96, 108));

			Angles vecAngles{};
			g_engfuncs.pfnVecToAngles(-tr.vecPlaneNormal, vecAngles);
			Prefab_t::Create<CSparkMdl>(tr.vecEndPos, vecAngles);
		}
		else if ((pPlayer->pev->origin - pCloud->pev->origin).LengthSquared() < (72.0 * 72.0))
		{
			Vector vecAttachmentPos{};
			Angles vecAngles{};
			g_engfuncs.pfnGetAttachment(pPlayer->edict(), 0, vecAttachmentPos, vecAngles);

			auto const pEdict = Prefab_t::Create<CSparkSpr>(vecAttachmentPos)->edict();
			g_engfuncs.pfnEmitSound(pEdict, CHAN_STATIC, UTIL_GetRandomOne(Sounds::HIT_METAL), VOL_NORM, ATTN_NORM, 0, UTIL_Random(96, 108));
		}
	}
}
