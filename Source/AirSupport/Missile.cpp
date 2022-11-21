#include <cassert>

import <array>;

import Missile;
import Resources;
import Math;

import UtlRandom;

using std::array;

// Explosion.cpp
extern void RangeDamage(CBasePlayer *pAttacker, const Vector &vecOrigin, float const flRadius, float const flDamage) noexcept;
extern void ScreenEffects(const Vector &vecOrigin, float const flRadius, float const flPunchMax, float const flKnockForce) noexcept;
extern Task VisualEffects(const Vector vecOrigin, float const flRadius) noexcept;
extern void Impact(CBasePlayer *pAttacker, CBaseEntity *pProjectile, float flDamage) noexcept;
//

CPrecisionAirStrike::~CPrecisionAirStrike() noexcept
{
	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_KILLBEAM);
	WriteData(ent_cast<short>(pev));
	MsgEnd();
}

Task CPrecisionAirStrike::Task_SFX() noexcept
{
	for (;;)
	{
		g_engfuncs.pfnEmitSound(edict(), CHAN_WEAPON, Sounds::TRAVEL, VOL_NORM, ATTN_NORM, 0, UTIL_Random(94, 112));
		co_await 1.f;
	}
}

Task CPrecisionAirStrike::Task_Trail() noexcept
{
	for (;;)
	{
		co_await gpGlobals->frametime;

		pev->angles += Vector(
			UTIL_Random(-0.35f, 0.35f),
			UTIL_Random(-0.35f, 0.35f),
			UTIL_Random(-0.35f, 0.35f)
		);

		// GoldSrc Mystery #1: The fucking v_angle and angles.
		g_engfuncs.pfnMakeVectors(Vector(
			-pev->angles.x,
			pev->angles.y,
			pev->angles.z)
		);

		pev->velocity = gpGlobals->v_forward * SPEED;

		Vector vecOrigin = pev->origin + gpGlobals->v_forward * -48;

		MsgPVS(SVC_TEMPENTITY, vecOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecOrigin);
		WriteData((short)Sprites::m_rgLibrary[Sprites::FIRE2]);
		WriteData((byte)3);
		WriteData((byte)255);
		MsgEnd();

		MsgPVS(SVC_TEMPENTITY, vecOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecOrigin);

		switch (UTIL_Random(0, 2))
		{
		case 0:
			WriteData((short)Sprites::m_rgLibrary[Sprites::SMOKE]);
			break;

		case 1:
			WriteData((short)Sprites::m_rgLibrary[Sprites::SMOKE_1]);
			break;

		default:
			WriteData((short)Sprites::m_rgLibrary[Sprites::SMOKE_2]);
			break;
		}

		WriteData((byte)UTIL_Random<short>(1, 10));
		WriteData((byte)UTIL_Random<short>(50, 255));
		MsgEnd();
	}
}

void CPrecisionAirStrike::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE[AIR_STRIKE]);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = (m_vecTarget - pev->origin).Normalize() * SPEED;
	g_engfuncs.pfnVecToAngles(pev->velocity, pev->angles);

	MsgPVS(SVC_TEMPENTITY, pev->origin);
	WriteData(TE_SPRITE);
	WriteData(pev->origin);
	WriteData((short)Sprites::m_rgLibrary[Sprites::FIRE]);
	WriteData((byte)5);
	WriteData((byte)255);
	MsgEnd();

	pev->effects = EF_LIGHT | EF_BRIGHTLIGHT;

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(ent_cast<short>(pev));
	WriteData((short)Sprites::m_rgLibrary[Sprites::SMOKE_TRAIL]);
	WriteData((byte)10);
	WriteData((byte)3);
	WriteData((byte)255);
	WriteData((byte)255);
	WriteData((byte)255);
	WriteData((byte)255);
	MsgEnd();

	g_engfuncs.pfnMakeVectors(pev->angles);

	auto const qRot = Quaternion::Rotate(Vector(0, 0, 1), gpGlobals->v_forward);

	array const rgvecPericoord =
	{
		get_spherical_coord(pev->origin, qRot, 24.f, 120.f, 0.f),
		get_spherical_coord(pev->origin, qRot, 24.f, 120.f, 72.f),
		get_spherical_coord(pev->origin, qRot, 24.f, 120.f, 144.f),
		get_spherical_coord(pev->origin, qRot, 24.f, 120.f, 216.f),
		get_spherical_coord(pev->origin, qRot, 24.f, 120.f, 288.f),
	};

	for (auto &&vecPos : rgvecPericoord)
	{
		MsgPVS(SVC_TEMPENTITY, vecPos);
		WriteData(TE_SPRITE);
		WriteData(vecPos);
		WriteData((short)Sprites::m_rgLibrary[Sprites::SMOKE]);
		WriteData((byte)10);
		WriteData((byte)50);
		MsgEnd();
	}

	m_Scheduler.Enroll(Task_SFX());
	m_Scheduler.Enroll(Task_Trail());
}

void CPrecisionAirStrike::Touch(CBaseEntity *pOther) noexcept
{
	if (g_engfuncs.pfnPointContents(pev->origin) == CONTENTS_SKY)
	{
		pev->flags |= FL_KILLME;
		return;
	}

	g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pev), CHAN_WEAPON, UTIL_GetRandomOne(Sounds::EXPLOSION), VOL_NORM, 0.3f, 0, UTIL_Random(92, 116));

	Impact(m_pPlayer, this, 125.f);
	RangeDamage(m_pPlayer, pev->origin, 350.f, 275.f);
	ScreenEffects(pev->origin, 700.f, 12.f, 2048.f);
	TaskScheduler::Enroll(VisualEffects(pev->origin, 700.f));

	pev->flags |= FL_KILLME;
}

CPrecisionAirStrike *CPrecisionAirStrike::Create(CBasePlayer *pPlayer, Vector const &vecOrigin, Vector const &vecTarget) noexcept
{
	auto const [pEdict, pPrefab] = UTIL_CreateNamedPrefab<CPrecisionAirStrike>();

	pEdict->v.origin = vecOrigin;

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_vecTarget = vecTarget;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
