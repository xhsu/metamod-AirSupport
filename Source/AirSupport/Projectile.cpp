import <cassert>;

import <array>;
import <list>;
import <ranges>;

import mathlib;

import Effects;
import Hook;
import Math;
import Menu;
import Projectile;
import Ray;
import Resources;
import Uranus;

import UtlRandom;

using std::array;
using std::list;

// Explosion.cpp
extern void RangeDamage(CBasePlayer *pAttacker, const Vector &vecOrigin, float const flRadius, float const flDamage) noexcept;
extern void ScreenEffects(const Vector &vecOrigin, float const flRadius, float const flPunchMax, float const flKnockForce) noexcept;
extern Task VisualEffects(const Vector vecOrigin, float const flRadius) noexcept;
extern TraceResult Impact(CBasePlayer *pAttacker, CBaseEntity *pProjectile, float flDamage) noexcept;
//

//
// CPrecisionAirStrike
//

Task CPrecisionAirStrike::Task_Deviation() noexcept
{
	TraceResult tr{};
	trace_hull_functor_t fnTraceHull{ Vector(-3, -3, -3), Vector(3, 3, 3) };

	for (Vector vecDir, vecCurDir, vecOrigin, vecEstVel;;)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		if (m_pEnemy && m_pEnemy->IsAlive())
		{
			vecDir = (m_pEnemy->pev->origin - this->pev->origin).Normalize();
			vecCurDir = this->pev->velocity.Normalize();
			vecEstVel = arithmetic_lerp(vecDir, vecCurDir, 0.55).Normalize() * CVar::PAS_ProjSpeed->value;

			fnTraceHull(pev->origin, pev->origin + vecEstVel, dont_ignore_monsters, edict(), &tr);

			// If the correction is leading to hit a wall, skip it.
			if (EHANDLE<CBaseEntity> pHit{ tr.pHit }; pHit && pHit->IsBSPModel())
				goto LAB_SKIP_CALIBRATION;

			this->pev->velocity = vecEstVel;
			this->pev->angles = vecEstVel.VectorAngles();
		}

	LAB_SKIP_CALIBRATION:;

		pev->angles += Angles{
			UTIL_Random(-0.2f, 0.2f),
			UTIL_Random(-0.2f, 0.2f),
			UTIL_Random(-0.2f, 0.2f),
		};

		// GoldSrc Mystery #1: The fucking v_angle and angles.
		pev->v_angle = Angles(
			-pev->angles.pitch,
			pev->angles.yaw,
			pev->angles.roll
		);

		pev->velocity = pev->v_angle.Front() * CVar::PAS_ProjSpeed->value;

		vecOrigin = pev->origin + pev->v_angle.Front() * -48;

		MsgPVS(SVC_TEMPENTITY, vecOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecOrigin);
		WriteData((short)Sprites::m_rgLibrary[Sprites::ROCKET_EXHAUST_FLAME]);
		WriteData((byte)3);
		WriteData((byte)255);
		MsgEnd();
	}
}

Task CPrecisionAirStrike::Task_EmitExhaust() noexcept
{
	pev->effects = EF_LIGHT | EF_BRIGHTLIGHT;

	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::TRAVEL, VOL_NORM, 0.3f, 0, UTIL_Random(94, 112));

	co_await 0.25f;	// The entity must flying into the world before the beam can correctly displayed.

	MsgAll(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());	// short (entity:attachment to follow)
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);	// short (sprite index)
	WriteData((byte)UTIL_Random(20, 40));	// byte (life in 0.1's) 
	WriteData((byte)3);		// byte (line width in 0.1's) 
	WriteData((byte)255);	// r
	WriteData((byte)255);	// g
	WriteData((byte)191);	// b
	WriteData((byte)255);	// byte (brightness)
	MsgEnd();

	for (;;)
	{
		co_await UTIL_Random(0.04f, 0.08f);

		auto const vecOrigin = pev->origin + pev->v_angle.Front() * -48;

		auto pSpark = CSpriteDisplay::Create(vecOrigin, kRenderTransAdd, Sprites::ROCKET_TRAIL_SMOKE[0]);
		pSpark->pev->renderamt = UTIL_Random(50.f, 255.f);
		pSpark->pev->rendercolor = Vector(255, 255, UTIL_Random(192, 255));
		pSpark->pev->frame = (float)UTIL_Random(17, 22);
		pSpark->pev->scale = UTIL_Random(0.3f, 1.1f);
		pSpark->m_Scheduler.Enroll(Task_FadeOut(pSpark->pev, /*STAY*/ 0.2f, /*DECAY*/ UTIL_Random(0.1f, 0.5f), /*ROLL*/ 0, /*SCALE_INC*/ UTIL_Random(0.35f, 0.55f)), TASK_ANIMATION);
	}
}

void CPrecisionAirStrike::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = (m_vecTarget - pev->origin).Normalize() * CVar::PAS_ProjSpeed->value;
	pev->angles = pev->velocity.VectorAngles();
	pev->body = AIR_STRIKE;

	m_Scheduler.Enroll(Task_Deviation());
	m_Scheduler.Enroll(Task_EmitExhaust());
}

void CPrecisionAirStrike::Touch(CBaseEntity *pOther) noexcept
{
	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::TRAVEL, VOL_NORM, 0.3f, SND_STOP, UTIL_Random(94, 112));

	if (g_engfuncs.pfnPointContents(pev->origin) == CONTENTS_SKY)
	{
		pev->flags |= FL_KILLME;
		return;
	}

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::EXPLOSION), VOL_NORM, 0.3f, 0, UTIL_Random(92, 116));

	Impact(m_pPlayer, this, CVar::PAS_DmgImpact->value);
	RangeDamage(m_pPlayer, pev->origin, CVar::PAS_DmgRadius->value, CVar::PAS_DmgExplo->value);
	ScreenEffects(pev->origin, CVar::PAS_FxRadius->value, CVar::PAS_FxPunchMax->value, CVar::PAS_FxKnock->value);
	TaskScheduler::Enroll(VisualEffects(pev->origin, 700.f));	// this one is not the same as the fx radus, though with same default value.

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_KILLBEAM);
	WriteData(ent_cast<short>(pev));
	MsgEnd();

	pev->flags |= FL_KILLME;
}

CPrecisionAirStrike *CPrecisionAirStrike::Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget, CBaseEntity* pEnemy) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CPrecisionAirStrike>();

	pEdict->v.origin = vecOrigin;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_vecTarget = vecTarget;
	pPrefab->m_pEnemy = pEnemy;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CClusterCharge
//

Task CClusterCharge::Task_Explo() noexcept
{
	TraceResult tr{};

	co_await m_flTotalFuseTime;

	g_engfuncs.pfnTraceLine(pev->origin, Vector(pev->origin.x, pev->origin.y, pev->origin.z - 48.0), ignore_monsters | ignore_glass, nullptr, &tr);

	if (tr.flFraction == 1.f)
	{
		// Is it flying to a wall?

		g_engfuncs.pfnTraceLine(pev->origin, pev->origin + pev->velocity, ignore_monsters | ignore_glass, nullptr, &tr);
	}
	else
	{
		// If this is somewhere near gound, cause fire.
		// But the fire should not be flying out, this is not 'burning debris'

		auto const pFlame = Prefab_t::Create<CFlame>(tr.vecEndPos + tr.vecPlaneNormal * 72.0);
		pFlame->pev->velocity = Vector::Zero();
	}

	if (tr.pHit && tr.pHit->v.solid == SOLID_BSP)
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::SMALL_SCORCH).m_Index);

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_EXPLOSION);
	WriteData(pev->origin);
	WriteData(Sprites::m_rgLibrary[Sprites::MINOR_EXPLO]);
	WriteData((byte)UTIL_Random(10, 20));
	WriteData((byte)20);
	WriteData(TE_EXPLFLAG_NONE);
	MsgEnd();

	co_await TaskScheduler::NextFrame::Rank[1];

	RangeDamage(m_pPlayer, pev->origin, 120.f, 210.f);
	ScreenEffects(pev->origin, 180.f, 4.f, 256.f);

	if ((s_iCounter++) % 2 == 0)
		Prefab_t::Create<CFloatingDust>(pev->origin, Angles(0, 0, UTIL_Random(0.0, 359.9)));
	else if (pev->flags & FL_ONGROUND)
		Prefab_t::Create<CGroundedDust>(pev->origin);

	co_await TaskScheduler::NextFrame::Rank[1];
	pev->flags |= FL_KILLME;
}

Task CClusterCharge::Task_VisualEffects() noexcept
{
	for (;;)
	{
		co_await 0.075f;

		if (!IsInWorld())
		{
			pev->flags |= FL_KILLME;
			co_return;
		}

		if (pev->waterlevel != 0)
		{
			pev->velocity = pev->velocity * 0.5f;
			pev->framerate = 0.2f;
		}

		MsgPVS(SVC_TEMPENTITY, pev->origin);
		WriteData(TE_SPARKS);
		WriteData(pev->origin);
		MsgEnd();

		MsgPVS(SVC_TEMPENTITY, pev->origin);
		WriteData(TE_SMOKE);
		WriteData(pev->origin);
		WriteData((short)Sprites::m_rgLibrary[UTIL_GetRandomOne(Sprites::BLACK_SMOKE)]);
		WriteData((byte)UTIL_Random(5, 10));	// (scale in 0.1's)
		WriteData((byte)UTIL_Random(15, 20));	// (framerate)
		MsgEnd();
	}
}

void CClusterCharge::Spawn() noexcept
{
	pev->velocity = get_spherical_coord(250.0, UTIL_Random(45.0, 180.0), UTIL_Random(0.0, 360.0));
	pev->gravity = 0.55f;
	pev->friction = 0.7f;

	auto const flSpeed = pev->velocity.Length();

	pev->angles = Angles(UTIL_Random(0.0, 180.0), UTIL_Random(0.0, 360.0), UTIL_Random(0.0, 360.0));
	pev->avelocity = Angles(flSpeed, UTIL_Random(-flSpeed, flSpeed), 0);

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_BOUNCE;

	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->framerate = 1.f;
	pev->body = 5;	// small charge
	pev->scale = 2.f;

	m_Scheduler.Enroll(Task_Explo());
	m_Scheduler.Enroll(Task_VisualEffects());
}

void CClusterCharge::Touch(CBaseEntity *pOther) noexcept
{
	if (FClassnameIs(pOther->pev, "func_breakable") && pOther->pev->rendermode != kRenderNormal)
	{
		pev->velocity = pev->velocity * -2.0f;
		return;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8f;
	}
	else
	{
		if (m_iBounceCount < 5)
		{
			// Regular sound
			if (gpGlobals->time < (pev->dmgtime - m_flTotalFuseTime * 0.65f))
				g_engfuncs.pfnEmitSound(edict(), CHAN_VOICE, Sounds::GRENADE_BOUNCE[1], VOL_NORM, ATTN_NORM, 0, UTIL_Random(96, 108));

			// Damger sound
			else
				g_engfuncs.pfnEmitSound(edict(), CHAN_VOICE, Sounds::GRENADE_BOUNCE[0], VOL_NORM, ATTN_NORM, 0, UTIL_Random(96, 108));
		}

		if (m_iBounceCount >= 10)
		{
			pev->groundentity = ent_cast<edict_t *>(0);
			pev->flags |= FL_ONGROUND;
			pev->velocity = Vector::Zero();
		}

		m_iBounceCount++;
	}

	pev->framerate = float(pev->velocity.Length() / 200.0);

	if (pev->framerate > 1)
	{
		pev->framerate = 1.0f;
	}
	else if (pev->framerate < 0.5f)
	{
		pev->framerate = 0.0f;
	}
}

bool CClusterCharge::ShouldCollide(EHANDLE<CBaseEntity> pOther) noexcept
{
	if (pOther.Is<CClusterCharge>() || pOther.Is<CClusterBomb>())
		return false;

	return true;
}

CClusterCharge *CClusterCharge::Create(CBasePlayer *pPlayer, Vector const &vecSpawn, float const flFuseTime) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CClusterCharge>();

	pEdict->v.origin = vecSpawn;
	pEdict->v.dmgtime = gpGlobals->time + flFuseTime;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_flTotalFuseTime = flFuseTime;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CClusterBomb
//

Task CClusterBomb::Task_ClusterBomb() noexcept
{
	for (; pev->origin.z > m_flDetonationHeight;)
	{
		co_await 0.01f;
	}

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_EXPLOSION);
	WriteData(pev->origin);
	WriteData(Sprites::m_rgLibrary[Sprites::AIRBURST]);
	WriteData((byte)35);
	WriteData((byte)12);
	WriteData(TE_EXPLFLAG_NONE);
	MsgEnd();

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_DLIGHT);
	WriteData(pev->origin);
	WriteData((byte)70);
	WriteData((byte)255);
	WriteData((byte)0);
	WriteData((byte)0);
	WriteData((byte)2);
	WriteData((byte)0);
	MsgEnd();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::CLUSTER_BOMB_DROP, VOL_NORM, 0, SND_STOP, UTIL_Random(92, 112));

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_KILLBEAM);
	WriteData(ent_cast<short>(pev));
	MsgEnd();

	for (int i = 0; i < 8; ++i)
	{
		Prefab_t::Create<CDebris>(pev->origin);
	}

	UTIL_ExplodeModel(
		pev->origin,
		UTIL_Random() ? -750.f : 750.f,
		Models::m_rgLibrary[Models::GIBS_METAL],
		UTIL_Random(16, 24),
		UTIL_Random(8.f, 12.f)
	);

	co_await TaskScheduler::NextFrame::Rank[0];

	RangeDamage(m_pPlayer, pev->origin, 180.f, 100.f);
	ScreenEffects(pev->origin, 360.f, 5.f, 512.f);

	co_await 1.f;

	size_t iCounter = 0;
	TraceResult tr{};

	for (auto &&vec : m_rgvecExploOrigins)
	{
		// Is this near ground?
		g_engfuncs.pfnTraceLine(vec, Vector(vec.x, vec.y, vec.z - 48.0), ignore_monsters | ignore_glass, nullptr, &tr);

		if (tr.flFraction == 1.0)
		{
			// Is this near a wall?
			auto const vecCenter = Vector(m_vecTargetGround.x, m_vecTargetGround.y, vec.z);
			auto const vecDir = (vec - vecCenter).Normalize();
			g_engfuncs.pfnTraceLine(vecCenter, vec + vecDir * 48, ignore_monsters | ignore_glass, nullptr, &tr);
		}
		else
		{
			// If this is somewhere near gound, cause fire.
			// But the fire should not be flying out, this is not 'burning debris'

			auto const pFlame = Prefab_t::Create<CFlame>(vec);
			pFlame->pev->velocity = Vector::Zero();
		}

		if (tr.flFraction < 1.f)
			UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::SCORCH).m_Index);

		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_EXPLOSION);
		WriteData(vec);
		WriteData(Sprites::m_rgLibrary[Sprites::MINOR_EXPLO]);
		WriteData((byte)UTIL_Random(5, 15));
		WriteData((byte)24);
		WriteData(TE_EXPLFLAG_NONE);
		MsgEnd();

		co_await TaskScheduler::NextFrame::Rank[0];

		RangeDamage(m_pPlayer, vec, 120.f, 210.f);
		ScreenEffects(vec, 180.f, 4.f, 256.f);

		if (iCounter % 3 == 0)
		{
			Prefab_t::Create<CFloatingDust>(vec, Angles(0, 0, UTIL_Random(0.0, 359.9)));
		}

		++iCounter;
		co_await UTIL_Random(0.08f, 0.11f);
	}

	pev->flags |= FL_KILLME;
	co_return;
}

Task CClusterBomb::Task_ClusterBomb2() noexcept
{
	for (; pev->origin.z > m_flDetonationHeight;)
	{
		co_await 0.01f;
	}

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_EXPLOSION);
	WriteData(pev->origin);
	WriteData(Sprites::m_rgLibrary[Sprites::AIRBURST]);
	WriteData((byte)35);
	WriteData((byte)12);
	WriteData(TE_EXPLFLAG_NONE);
	MsgEnd();

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_DLIGHT);
	WriteData(pev->origin);
	WriteData((byte)70);
	WriteData((byte)255);
	WriteData((byte)0);
	WriteData((byte)0);
	WriteData((byte)2);
	WriteData((byte)0);
	MsgEnd();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::CLUSTER_BOMB_DROP, VOL_NORM, 0, SND_STOP, UTIL_Random(92, 112));

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_KILLBEAM);
	WriteData(ent_cast<short>(pev));
	MsgEnd();

	for (float flFuseTime = 1.f; flFuseTime < 3.4f; flFuseTime += 0.1f)
	{
		CClusterCharge::Create(
			m_pPlayer,
			pev->origin + Vector(UTIL_Random(-10.0, 10.0), UTIL_Random(-10.0, 10.0), UTIL_Random(-10.0, 10.0)),
			flFuseTime
		);
	}

	UTIL_ExplodeModel(
		pev->origin,
		UTIL_Random() ? -750.f : 750.f,
		Models::m_rgLibrary[Models::GIBS_METAL],
		UTIL_Random(16, 24),
		UTIL_Random(8.f, 12.f)
	);

	co_await TaskScheduler::NextFrame::Rank[0];

	RangeDamage(m_pPlayer, pev->origin, 180.f, 100.f);
	ScreenEffects(pev->origin, 360.f, 5.f, 512.f);

	co_await 1.f;

	pev->flags |= FL_KILLME;
	co_return;
}

void CClusterBomb::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = Vector::Down() * SPEED;
	pev->gravity = 1.f;
	g_engfuncs.pfnVecToAngles(pev->velocity, pev->angles);

	pev->body = CLUSTER_BOMB;
	pev->effects = EF_DIMLIGHT;

	// This is just a bomb drop. No energy post-apply onto the projectile, therefore no complex VFX.

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);
	WriteData((byte)20);
	WriteData((byte)3);
	WriteData((byte)255);
	WriteData((byte)255);
	WriteData((byte)255);
	WriteData((byte)255);
	MsgEnd();

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::CLUSTER_BOMB_DROP, VOL_NORM, 0, 0, UTIL_Random(92, 112));

	m_Scheduler.Enroll(Task_ClusterBomb2());
	// Calculate everything, including all those detonation spots and where is the first detonation.

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(m_vecTargetGround, Vector(m_vecTargetGround.x, m_vecTargetGround.y, 8192), ignore_glass | ignore_monsters, nullptr, &tr);

	auto const &flMaxAbsHeight = tr.vecEndPos.z;
	auto const flHeightDiff = (flMaxAbsHeight - m_vecTargetGround.z);
	static constexpr auto flStep = 96.0;

	m_rgvecExploOrigins.reserve(64);

	for (double fl = 0, flMax = 16; fl < (flHeightDiff + flStep) && flMax >= 1.0; fl += flStep, flMax *= 0.5)
	{
		for (auto i = 0; i < (decltype(i))std::floor(flMax); ++i)
		{
			m_rgvecExploOrigins.emplace_back(
				m_vecTargetGround + get_cylindrical_coord(UTIL_Random(10, 250), UTIL_Random(0.0, 359.9), UTIL_Random(fl, fl + flStep))
			);
		}
	}

	if (m_rgvecExploOrigins.empty())
	{
		pev->flags |= FL_KILLME;
		return;
	}

	m_flDetonationHeight = std::min<double>(m_vecTargetGround.z + flStep * 5, flMaxAbsHeight) - 2;
}

CClusterBomb *CClusterBomb::Create(CBasePlayer *pPlayer, Vector const &vecSpawn, Vector const &vecTargetOrigin) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CClusterBomb>();

	pEdict->v.origin = vecSpawn;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_vecTargetGround = vecTargetOrigin;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CCarpetBombardment
//

Task CCarpetBombardment::Task_Touch() noexcept
{
	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_EXPLOSION);
	WriteData(pev->origin + Vector(0, 0, 70));
	WriteData(Sprites::m_rgLibrary[Sprites::CARPET_FRAGMENT_EXPLO]);
	WriteData((byte)UTIL_Random(20, 30));
	WriteData((byte)12);
	WriteData(TE_EXPLFLAG_NOSOUND);
	MsgEnd();

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::EXPLOSION_SHORT), VOL_NORM, 0.3f, 0, UTIL_Random(92, 116));

	co_await TaskScheduler::NextFrame::Rank[0];

	auto const tr = Impact(m_pPlayer, this, 125.f);
	RangeDamage(m_pPlayer, pev->origin, 250.f, 200.f);
	ScreenEffects(pev->origin, 500.f, 15.f, 1024.f);

	if (tr.pHit != nullptr)
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::SCORCH).m_Index);

	co_await TaskScheduler::NextFrame::Rank[0];

	auto const iFlameCount = UTIL_Random(1, 3);
	for (int i = 0; i < iFlameCount; ++i)
	{
		auto const pFlame = Prefab_t::Create<CFlame>(tr.vecEndPos + Vector(0, 0, 8));
		pFlame->pev->velocity = get_spherical_coord(350.f, UTIL_Random(30.0, 45.0), UTIL_Random(0.0, 359.9));
		pFlame->pev->gravity = 1.f;
	}

	auto const pSmoke = Prefab_t::Create<CSmoke>(tr.vecEndPos + Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), UTIL_Random(0, 72)));
	pSmoke->LitByFlame(false);

	co_await TaskScheduler::NextFrame::Rank[0];

	auto const qRotation = Quaternion::Rotate(Vector(0, 0, 1), tr.vecPlaneNormal);

	array const rgvecVelocitys =
	{
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 0),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 120),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 240),
	};

	for (auto &&vecVelocity : rgvecVelocitys)
	{
		auto const flScale = UTIL_Random(0.65f, 1.f);

		UTIL_BreakModel(
			tr.vecEndPos, Vector(flScale, flScale, flScale), vecVelocity * UTIL_Random(300.f, 500.f),
			UTIL_Random(0.8f, 2.f),
			Models::m_rgLibrary[Models::GIBS_CONCRETE],
			UTIL_Random(4, 12),
			UTIL_Random(8.f, 20.f),
			0x40
		);

		Prefab_t::Create<CDebris>(tr.vecEndPos)->pev->velocity = vecVelocity;
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	pev->flags |= FL_KILLME;
}

void CCarpetBombardment::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = Vector::Zero();	// No init speed needed. Gravity is good enough.
	pev->gravity = UTIL_Random(0.8f, 1.1f);
	pev->body = CARPET_BOMBARDMENT;
}

void CCarpetBombardment::Touch(CBaseEntity *pOther) noexcept
{
	m_Scheduler.Enroll(Task_Touch());

	if (m_pCorrespondingBeacon)
		m_pCorrespondingBeacon->pev->flags |= FL_KILLME;
}

CCarpetBombardment *CCarpetBombardment::Create(CBasePlayer *pPlayer, Vector const &vecSpawn, CBeam *pBeacon) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CCarpetBombardment>();

	g_engfuncs.pfnVecToAngles(Vector::Down(), pEdict->v.angles);
	pEdict->v.origin = vecSpawn;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_pCorrespondingBeacon = pBeacon;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CBullet
//

Task CBullet::Task_Touch() noexcept
{
	TraceResult tr{};
	g_engfuncs.pfnTraceLine(m_vecLastTraceSrc, pev->origin + pev->velocity, dont_ignore_monsters, edict(), &tr);

	if (tr.pHit && tr.pHit->v.solid == SOLID_BSP)
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::BIGSHOT).m_Index);

	if (tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
	{
		EHANDLE<CBaseEntity> pVictim{ tr.pHit };

		gUranusCollection.pfnClearMultiDamage();
		pVictim->TraceAttack(m_pShooter->pev, 100.f, pev->velocity.Normalize(), &tr, DMG_BULLET);
		gUranusCollection.pfnApplyMultiDamage(pev, m_pShooter->pev);

		pev->flags |= FL_KILLME;
		co_return;
	}

	Angles vecAngles{};
	g_engfuncs.pfnVecToAngles(-tr.vecPlaneNormal, vecAngles);	// netagive is due to we want to rotate the "front" of that model towards into the surface it hits.

	Prefab_t::Create<CSparkMdl>(tr.vecEndPos, vecAngles);
	Prefab_t::Create<CGroundedDust>(tr.vecEndPos);
	CGunshotSmoke::Create(tr);

	co_await TaskScheduler::NextFrame::Rank[1];

	UTIL_BreakModel(
		tr.vecEndPos, Vector(1, 1, 1) /* Invalid Arg? */, tr.vecPlaneNormal * UTIL_Random(75, 100),
		UTIL_Random(0.8f, 1.2f),
		Models::m_rgLibrary[Models::GIBS_CONCRETE],
		UTIL_Random(4, 8),
		UTIL_Random(8.f, 12.f),
		0x40
	);

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_GUNSHOT);
	WriteData(tr.vecEndPos);
	MsgEnd();

	Prefab_t::Create<CSparkSpr>(tr.vecEndPos);

	pev->flags |= FL_KILLME;
}

Task CBullet::Task_Fly() noexcept
{
	TraceResult tr{};

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		for (auto &&pEdict :
			std::views::iota(1, gpGlobals->maxClients) |
			std::views::transform([](int idx) noexcept { return g_engfuncs.pfnPEntityOfEntIndex(idx); }) |
			std::views::filter([](edict_t *pEdict) noexcept { return pEdict != nullptr && pEdict->pvPrivateData != nullptr; }) |

			// Only player who is alive.
			std::views::filter([](edict_t *pEdict) noexcept { return pEdict->v.deadflag == DEAD_NO && pEdict->v.takedamage != DAMAGE_NO; }) |

			// Too far from us.
			std::views::filter([&](edict_t *pEdict) noexcept { return (pEdict->v.origin - pev->origin).LengthSquared() < (WHIZZ_RADIUS * WHIZZ_RADIUS); }) |

			// Can't be the one who shoots the bullet.
			std::views::filter([&](edict_t *pEdict) noexcept { return pEdict != pev->owner; })
			)
		{
			g_engfuncs.pfnClientCommand(pEdict, "spk %s\n", UTIL_GetRandomOne(Sounds::WHIZZ));
		}

		g_engfuncs.pfnTraceLine(m_vecLastTraceSrc, pev->origin, dont_ignore_monsters, edict(), &tr);

		if (tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
		{
			EHANDLE<CBaseEntity> pVictim{ tr.pHit };

			gUranusCollection.pfnClearMultiDamage();
			pVictim->TraceAttack(m_pShooter->pev, 100.f, pev->velocity.Normalize(), &tr, DMG_BULLET);
			gUranusCollection.pfnApplyMultiDamage(pev, m_pShooter->pev);
		}

		m_vecLastTraceSrc = pev->origin;
	}
}

void CBullet::Spawn() noexcept
{
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;
	pev->body = GUNSHIP_STRIKE;

	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetSize(edict(), Vector(-0.1, -0.1, -0.1), Vector(0.1, 0.1, 0.1));

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());
	WriteData(Sprites::m_rgLibrary[Sprites::BEAM]);
	WriteData((byte)1);
	WriteData((byte)1);
	WriteData((byte)255);
	WriteData((byte)200);
	WriteData((byte)120);
	WriteData((byte)UTIL_Random(192, 255));
	MsgEnd();

	m_Scheduler.Enroll(Task_Fly());
}

void CBullet::Touch(CBaseEntity *pOther) noexcept
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	m_Scheduler.Enroll(Task_Touch());
}

CBullet *CBullet::Create(Vector const &vecOrigin, Vector const &vecVelocity, CBasePlayer *pShooter) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CBullet>();

	g_engfuncs.pfnVecToAngles(vecVelocity, pEdict->v.angles);
	pEdict->v.origin = vecOrigin;
	pEdict->v.velocity = vecVelocity;

	pPrefab->m_pShooter = pShooter;
	pPrefab->m_vecLastTraceSrc = vecOrigin;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CFuelAirExplosive
//

Task CFuelAirExplosive::Task_GasPropagate() noexcept
{
	m_bReleasingGas = true;
	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::FuelAirBomb::GAS_LEAK_LOOP, VOL_NORM, ATTN_NORM, 0, UTIL_Random(92, 112));

	TraceResult tr{};
	auto const vecTestSrc = pev->origin + Vector::Up() * 5.0;
	list<Vector> rgvecVarifiedLocations{};
	bool bGoodToSpawn = false;

	for (auto iCounter = 0ul; iCounter < 50; /* Increase the counter only when successed */)
	{
	LAB_CONTINUE:;
		co_await 0.02f;

		bGoodToSpawn = false;

		auto const flRangeMin = 10.0 + floor(iCounter / 10.0) * 100.0;
		auto const flRangeMax = 200.0 + floor(iCounter / 10.0) * 100.0;
		auto const vecCandidate = pev->origin + get_cylindrical_coord(UTIL_Random(flRangeMin, flRangeMax), UTIL_Random(0.0, 359.9), UTIL_Random(36, 96));

		for (auto &&vec : rgvecVarifiedLocations)
		{
			auto const flLenghSq = (vec - vecCandidate).LengthSquared();

			// Too close to each other.
			if (flLenghSq < 64.0 * 64.0)
				goto LAB_CONTINUE;
		}

		g_engfuncs.pfnTraceLine(vecTestSrc, vecCandidate, ignore_monsters | ignore_glass, nullptr, &tr);

		if (tr.flFraction < 1 || tr.fAllSolid)
		{
			rgvecVarifiedLocations.sort([&](Vector const &lhs, Vector const &rhs) noexcept {
				return (lhs - vecCandidate).LengthSquared() < (rhs - vecCandidate).LengthSquared();
			});

			for (auto &&vec : rgvecVarifiedLocations)
			{
				g_engfuncs.pfnTraceLine(vec, vecCandidate, ignore_glass | ignore_monsters, nullptr, &tr);
				if (tr.flFraction == 1.0 && !tr.fAllSolid)
				{
					bGoodToSpawn = true;
					break;
				}
			}
		}
		else
			bGoodToSpawn = true;

		if (!bGoodToSpawn)
			continue;

		m_rgpCloud.emplace_back(
			CFuelAirCloud::Create(m_pPlayer, vecCandidate)
		);

		rgvecVarifiedLocations.emplace_back(vecCandidate);
		++iCounter;
	}

	// all the gases are out.
	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::FuelAirBomb::GAS_LEAK_LOOP, VOL_NORM, ATTN_NORM, SND_STOP, UTIL_Random(92, 112));
	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::FuelAirBomb::GAS_LEAK_FADEOUT, VOL_NORM, ATTN_NORM, 0, UTIL_Random(92, 112));
	m_bReleasingGas = false;
	m_bGasAllOut = true;

LAB_WAIT_FOR_FADE_IN:;
	for (auto &&pCloud : m_rgpCloud)
	{
		co_await TaskScheduler::NextFrame::Rank[1];

		// Something / someone ignited it already.
		if (pCloud && pCloud->m_bIgnited)
			goto LAB_CO_RETURN;

		if (pCloud && !pCloud->m_bFadeInDone)
			goto LAB_WAIT_FOR_FADE_IN;
	}

LAB_CO_RETURN:;
	TakeDamage(pev, pev, 1.f, DMG_SHOCK);
}

Task CFuelAirExplosive::Task_StopSoundAndRemove() noexcept
{
	if (!m_bTouched)
		g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::CLUSTER_BOMB_DROP, VOL_NORM, 0, SND_STOP, UTIL_Random(92, 112));	// stop the flying sound!!

	if (m_bReleasingGas)
		g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::FuelAirBomb::GAS_LEAK_LOOP, VOL_NORM, ATTN_NORM, SND_STOP, UTIL_Random(92, 112));

	co_await TaskScheduler::NextFrame::Rank.back();
	pev->flags |= FL_KILLME;
}

void CFuelAirExplosive::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -36), Vector(2, 2, 36));

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = Vector::Zero();
	pev->gravity = 1.f;
	pev->effects = EF_DIMLIGHT;
	pev->takedamage = DAMAGE_YES;
	pev->body = FUEL_AIR_BOMB;

	// This is just a bomb drop. No energy post-apply onto the projectile, therefore no complex VFX.

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);
	WriteData((byte)20);
	WriteData((byte)3);
	WriteData((byte)255);
	WriteData((byte)255);
	WriteData((byte)255);
	WriteData((byte)255);
	MsgEnd();

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::CLUSTER_BOMB_DROP, VOL_NORM, 0, 0, UTIL_Random(92, 112));
}

void CFuelAirExplosive::Touch(CBaseEntity *pOther) noexcept
{
	m_bTouched = true;
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::CLUSTER_BOMB_DROP, VOL_NORM, 0, SND_STOP, UTIL_Random(92, 112));
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::EXPLOSION_SHORT), VOL_NORM, 0, 0, UTIL_Random(92, 112));
	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::HIT_METAL), VOL_NORM, ATTN_NORM, 0, UTIL_Random(92, 112));

	pev->velocity = Vector::Zero();
	pev->gravity = 0;
	pev->effects = 0;

	g_engfuncs.pfnSetSize(edict(), Vector(-8, -8, -36), Vector(8, 8, 36));

	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), pev->origin + Vector(0, 0, 24), pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);
	g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);

	if (auto const tr = Impact(m_pPlayer, this, 180.f); tr.pHit && tr.pHit->v.solid == SOLID_BSP)
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::SMALL_SCORCH).m_Index);

	m_Scheduler.Enroll(Task_GasPropagate());
}

void CFuelAirExplosive::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) noexcept
{
	// Because if there are gas surrounding, the CFuelAirCloud::OnTraceAttack() fn will create one already.
	Prefab_t::Create<CSparkSpr>(ptr->vecEndPos);

	Angles vecAngles{};
	g_engfuncs.pfnVecToAngles(-ptr->vecPlaneNormal, vecAngles);
	Prefab_t::Create<CSparkMdl>(ptr->vecEndPos, vecAngles);

	TakeDamage(nullptr, pevAttacker, flDamage, bitsDamageType);
}

qboolean CFuelAirExplosive::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) noexcept
{
	// Only one of these can trigger.
	if (!(bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB | DMG_BURN | DMG_SHOCK | DMG_ENERGYBEAM | DMG_SLOWBURN | DMG_BLAST | DMG_EXPLOSION)))
		return false;

	// Therefore preventing inf loop
	pev->takedamage = DAMAGE_NO;

	static constexpr auto SCALE = 3;
	auto const vecExplo = pev->origin + Vector::Up() * 120 * SCALE;

	MsgPVS(SVC_TEMPENTITY, vecExplo);
	WriteData(TE_SPRITE);
	WriteData(vecExplo);
	WriteData(Sprites::m_rgLibrary[Sprites::GIGANTIC_EXPLO]);
	WriteData((byte)(SCALE * 10));
	WriteData((byte)255);
	MsgEnd();

	pev->effects = EF_NODRAW;
	UTIL_ExplodeModel(Vector(pev->origin.x, pev->origin.y, pev->absmax.z), 700.f, Models::m_rgLibrary[Models::GIBS_METAL], UTIL_Random(4, 6), UTIL_Random(10.f, 15.f));

	Prefab_t::Create<CSparkSpr>(
		Vector(
			UTIL_Random(pev->absmin.x, pev->absmax.x),
			UTIL_Random(pev->absmin.y, pev->absmax.y),
			UTIL_Random(pev->absmin.z, pev->absmax.z)
		)
	);

	// Huge-o-explo will only occurs when the gas isn't out.
	if (!m_bGasAllOut)
	{
		g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, UTIL_GetRandomOne(Sounds::EXPLOSION_BIG), VOL_NORM, ATTN_NONE, 0, UTIL_Random(92, 112));

		// Potential self-damaging, and cause inf-loop
		RangeDamage(m_pPlayer, pev->origin, 128.f * SCALE, 750.f);
	}

	g_engfuncs.pfnEmitSound(edict(), CHAN_BODY, UTIL_GetRandomOne(Sounds::HIT_METAL), VOL_NORM, ATTN_NORM, 0, UTIL_Random(92, 112));

	m_Scheduler.Enroll(Task_StopSoundAndRemove());
	return true;
}

CFuelAirExplosive *CFuelAirExplosive::Create(CBasePlayer *pPlayer, Vector const &vecOrigin) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CFuelAirExplosive>();

	pEdict->v.origin = vecOrigin;
	g_engfuncs.pfnVecToAngles(Vector::Down(), pEdict->v.angles);

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CIncendiaryMunition
//

Task CIncendiaryMunition::Task_Deviation() noexcept
{
	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[0];

		pev->angles += Angles(
			UTIL_Random(-0.4, 0.4),
			UTIL_Random(-0.4, 0.4),
			UTIL_Random(-0.4, 0.4)
		);

		// GoldSrc Mystery #1: The fucking v_angle and angles.
		pev->v_angle = Angles(
			-pev->angles.pitch,
			pev->angles.yaw,
			pev->angles.roll
		);

		pev->velocity = pev->v_angle.Front() * SPEED;
	}
}

Task CIncendiaryMunition::Task_EmitExhaust() noexcept
{
	pev->effects = EF_LIGHT | EF_BRIGHTLIGHT;

	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::TRAVEL, VOL_NORM, 0.3f, 0, UTIL_Random(94, 112));

	co_await 0.25f;	// The entity must flying into the world before the beam can correctly displayed.

	MsgAll(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());	// short (entity:attachment to follow)
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);	// short (sprite index)
	WriteData((byte)UTIL_Random(20, 40));	// byte (life in 0.1's) 
	WriteData((byte)3);		// byte (line width in 0.1's) 
	WriteData((byte)255);	// r
	WriteData((byte)255);	// g
	WriteData((byte)191);	// b
	WriteData((byte)255);	// byte (brightness)
	MsgEnd();

	co_await 0.25f;

	for (;;)
	{
		co_await UTIL_Random(0.04f, 0.08f);

		auto const vecOrigin = pev->origin + pev->v_angle.Front() * -48;

		auto pSpark = CSpriteDisplay::Create(vecOrigin, kRenderTransAdd, Sprites::ROCKET_TRAIL_SMOKE[0]);
		pSpark->pev->renderamt = UTIL_Random(50.f, 255.f);
		pSpark->pev->rendercolor = Vector(255, 255, UTIL_Random(192, 255));
		pSpark->pev->frame = (float)UTIL_Random(17, 22);
		pSpark->pev->scale = UTIL_Random(0.3f, 1.1f);
		pSpark->m_Scheduler.Enroll(Task_FadeOut(pSpark->pev, /*STAY*/ 0.2f, /*DECAY*/ UTIL_Random(0.1f, 0.5f), /*ROLL*/ 0, /*SCALE_INC*/ UTIL_Random(0.35f, 0.55f)), TASK_ANIMATION);
	}
}

Task CIncendiaryMunition::Task_Fuse() noexcept
{
	auto const FUSE_DIST = (m_vecStartingPos - m_vecTarget).LengthSquared() * 0.375;

	for (;;)
	{
		co_await 0.1f;

		if ((pev->origin - m_vecStartingPos).LengthSquared() < FUSE_DIST)
			continue;

		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_EXPLOSION);
		WriteData(pev->origin);
		WriteData(Sprites::m_rgLibrary[Sprites::MINOR_EXPLO]);
		WriteData((byte)UTIL_Random(10, 20));
		WriteData((byte)20);
		WriteData(TE_EXPLFLAG_NONE);
		MsgEnd();

		auto pPhosphorus = CPhosphorus::Create(m_pPlayer, pev->origin);	// One of the shower guarantees to hit the goal.
		pPhosphorus->pev->gravity = 0.f;
		pPhosphorus->pev->velocity = (m_vecTarget - pev->origin).Normalize() * 500;

		for (int i = 0; i < 15; ++i)
		{
			pPhosphorus = CPhosphorus::Create(m_pPlayer, pev->origin);
			pPhosphorus->pev->gravity = UTIL_Random(0.1f, 0.25f);
			pPhosphorus->pev->velocity = ((m_vecTarget + get_cylindrical_coord(UTIL_Random(32, 256), UTIL_Random(0, 359), 0)) - pev->origin).Normalize() * 500;
		}

		g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::TRAVEL, VOL_NORM, 0.3f, SND_STOP, UTIL_Random(94, 112));
		pev->flags |= FL_KILLME;
	}
}

void CIncendiaryMunition::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_FLY;
	pev->velocity = (m_vecTarget - pev->origin).Normalize() * SPEED;
	pev->angles = pev->velocity.VectorAngles();
	pev->body = PHOSPHORUS_MUNITION;

	m_Scheduler.Enroll(Task_Deviation());
	m_Scheduler.Enroll(Task_EmitExhaust());
	m_Scheduler.Enroll(Task_Fuse());
}

void CIncendiaryMunition::Touch(CBaseEntity *pOther) noexcept
{
	TraceResult tr{};
	g_engfuncs.pfnTraceMonsterHull(edict(), pev->origin, pev->origin + pev->velocity, ignore_glass | ignore_monsters, nullptr, &tr);

	g_engfuncs.pfnEmitAmbientSound(edict(), pev->origin, Sounds::TRAVEL, VOL_NORM, 0.3f, SND_STOP, UTIL_Random(94, 112));
	pev->flags |= FL_KILLME;

	if (tr.flFraction == 1)
		return;

	for (int i = 0; i < 16; ++i)
	{
		Vector const vecNoise = CrossProduct(
			Vector{ UTIL_Random(-1.0, 1.0), UTIL_Random(-1.0, 1.0), UTIL_Random(-1.0, 1.0) },
			tr.vecPlaneNormal);

		auto pPhosphorus = CPhosphorus::Create(m_pPlayer, pev->origin + tr.vecPlaneNormal * 3);
		pPhosphorus->pev->velocity = (tr.vecPlaneNormal + vecNoise).Normalize() * UTIL_Random(450.0, 550.0);
	}
}

CIncendiaryMunition *CIncendiaryMunition::Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CIncendiaryMunition>();

	pEdict->v.origin = vecOrigin;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_vecTarget = vecTarget;
	pPrefab->m_vecStartingPos = vecOrigin;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

void CWPMunition_Explo(CBasePlayer* m_pPlayer, Vector const& vecOrigin) noexcept
{
	auto pThickSmoke = Prefab_t::Create<CThickStaticSmoke>(vecOrigin);
	pThickSmoke->LitByFlame(true);

	auto pSpr = CSpriteDisplay::Create(pThickSmoke->pev->origin, kRenderFn::kRenderTransAdd, Sprites::MINOR_EXPLO);
	pSpr->pev->scale = pThickSmoke->pev->scale * 2.f;
	pSpr->pev->renderamt = 255.f;
	pSpr->pev->frame = (float)3;
	pSpr->m_Scheduler.Enroll(Task_FadeOut(pSpr->pev, 0.f, 2.f, 0.f), TASK_FADE_OUT);

	//UTIL_ExplodeModel(Vector(pev->origin.x, pev->origin.y, pev->absmax.z), 700.f, Models::m_rgLibrary[Models::GIBS_METAL], UTIL_Random(4, 6), UTIL_Random(10.f, 15.f));
	UTIL_ExplodeModel(vecOrigin, 700.f, Models::m_rgLibrary[Models::GIBS_CONCRETE], UTIL_Random(10, 12), UTIL_Random(7.f, 12.f));

	for (auto i = 0; i < 360; i += 30)
	{
		auto pPhosphorus = CPhosphorus::Create(m_pPlayer, vecOrigin + Vector(0, 0, 32));

		pPhosphorus->pev->velocity = get_spherical_coord(UTIL_Random(300.f, 650.f), UTIL_Random(20.0, 30.0), i);
	}
}
