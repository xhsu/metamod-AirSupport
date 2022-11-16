import edict;
import util;

import Effects;
import Resources;

import UtlRandom;

Task CFlame::Task_Animation() noexcept
{
	for (;;)
	{
		co_await gpGlobals->frametime;

		pev->framerate = float(18.0 * gpGlobals->frametime);
		pev->frame += pev->framerate;
		pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pev->frame < 0 || pev->frame >= m_iMaxFrame)
			pev->frame -= float((pev->frame / m_iMaxFrame) * m_iMaxFrame);
	}
}

Task CFlame::Task_DetectGround() noexcept
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
		WriteData((short)Sprite::m_rgLibrary[Sprite::BLACK_SMOKE]);
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
	co_await UTIL_Random(8.f, 14.f);

	pev->flags |= FL_KILLME;
}

void CFlame::Spawn() noexcept
{
	m_iFlameSprIndex = UTIL_Random(0u, Sprite::FLAME.size() - 1);
	m_iMaxFrame = Sprite::Frames::FLAME[m_iFlameSprIndex];

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = UTIL_Random(192.f, 255.f);
	pev->frame = UTIL_Random<float>(0, m_iMaxFrame);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_TOSS;
	pev->gravity = 2.f;
	pev->scale = UTIL_Random(0.6f, 0.85f);

	g_engfuncs.pfnSetModel(edict(), Sprite::FLAME[m_iFlameSprIndex]);
	g_engfuncs.pfnSetSize(edict(), Vector(-32, -32, -64) * pev->scale, Vector(32, 32, 64) * pev->scale);	// it is still required for pfnTraceMonsterHull

	// Doing this is to prevent spawning on slope and the spr just stuck and sink into ground.
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), Vector(pev->origin.x, pev->origin.y, pev->origin.z + 64.0), Vector(pev->origin.x, pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnTraceMonsterHull(edict(), tr.vecEndPos, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);	// pfnSetOrigin includes the abssize setting, restoring our hitbox.

	m_Scheduler.Enroll(Task_Animation());
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
	g_engfuncs.pfnTraceMonsterHull(edict(), pev->origin, pev->origin + pev->velocity.Normalize(), ignore_monsters | ignore_glass, nullptr, &tr);

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
