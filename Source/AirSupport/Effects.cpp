import <numbers>;

import edict;
import util;

import Effects;
import Math;
import Resources;

import UtlRandom;

//
// CFlame
//

Task CFlame::Task_Animation() noexcept
{
	// Consider this as initialization
	m_LastAnimUpdate = std::chrono::high_resolution_clock::now();

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		auto const CurTime = std::chrono::high_resolution_clock::now();
		auto const flTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(CurTime - m_LastAnimUpdate).count() / 1'000'000'000.0;

		pev->framerate = float(30.0 * flTimeDelta);
		pev->frame += pev->framerate;
		pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pev->frame < 0 || pev->frame >= m_iMaxFrame)
			pev->frame -= float((pev->frame / m_iMaxFrame) * m_iMaxFrame);

		m_LastAnimUpdate = CurTime;
	}
}

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

Task CFlame::Task_Remove() noexcept
{
	co_await UTIL_Random(9.f, 14.f);

	pev->flags |= FL_KILLME;
}

void CFlame::Spawn() noexcept
{
	m_iFlameSprIndex = UTIL_Random(0u, Sprites::FLAME.size() - 1);
	m_iMaxFrame = Sprites::Frames::FLAME[m_iFlameSprIndex];

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(192.f, 255.f);
	pev->frame = UTIL_Random<float>(0, m_iMaxFrame);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_TOSS;
	pev->gravity = 2.f;
	pev->scale = UTIL_Random(0.6f, 0.85f);

	g_engfuncs.pfnSetModel(edict(), Sprites::FLAME[m_iFlameSprIndex]);
	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 64.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_Scheduler.Enroll(Task_Animation());
	//m_Scheduler.Enroll(Task_DetectGround());
	m_Scheduler.Enroll(Task_EmitLight());
	m_Scheduler.Enroll(Task_Remove());

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

Task CSmoke::Task_EmitSmoke() noexcept
{
	for (;;)
	{
		co_await UTIL_Random(0.1f, 0.3f);

		Vector const vecNoise = get_spherical_coord(m_flRadius, UTIL_Random(70.0, 80.0), UTIL_Random(0.0, 359.9));

		MsgPVS(SVC_TEMPENTITY, pev->origin + vecNoise);
		WriteData(TE_SPRITE);
		WriteData(pev->origin + vecNoise);
		WriteData((short)Sprites::m_rgLibrary[Sprites::PERSISTENT_SMOKE]);
		WriteData((byte)UTIL_Random<short>(45, 55));
		WriteData((byte)UTIL_Random<short>(40, 60));
		MsgEnd();
	}
}

Task CSmoke::Task_Remove() noexcept
{
	for (;;)
	{
		co_await 0.1f;

		bool bAnySurvived = false;

		for (const auto &flame : m_rgpFlamesDependent)
		{
			[[likely]]
			if (flame)
			{
				bAnySurvived = true;
				break;
			}
		}

		[[unlikely]]
		if (!bAnySurvived)
		{
			pev->flags |= FL_KILLME;
			co_return;
		}
	}
}

void CSmoke::Spawn() noexcept
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	g_engfuncs.pfnSetSize(edict(), Vector(-0.2, -0.2, -0.2), Vector(0.2, 0.2, 0.2));
	g_engfuncs.pfnDropToFloor(edict());

	m_Scheduler.Enroll(Task_EmitSmoke());
	m_Scheduler.Enroll(Task_Remove());
}
