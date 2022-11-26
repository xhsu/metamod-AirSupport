#include <cassert>

import <array>;
import <ranges>;

import Effects;
import Math;
import Menu;
import Missile;
import Resources;

import UtlRandom;

using std::array;

// Explosion.cpp
extern void RangeDamage(CBasePlayer *pAttacker, const Vector &vecOrigin, float const flRadius, float const flDamage) noexcept;
extern void ScreenEffects(const Vector &vecOrigin, float const flRadius, float const flPunchMax, float const flKnockForce) noexcept;
extern Task VisualEffects(const Vector vecOrigin, float const flRadius) noexcept;
extern TraceResult Impact(CBasePlayer *pAttacker, CBaseEntity *pProjectile, float flDamage) noexcept;
//

//
// CPrecisionAirStrike
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
	WriteData(entindex());
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);
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

	Impact(m_pPlayer, this, 500.f);
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
	WriteData((byte)24);
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
		UTIL_Random(-300.f, 300.f),
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
			Prefab_t::Create<CSmoke>(vec, Vector(0, 0, UTIL_Random(0.0, 359.9)));
		}

		++iCounter;
		co_await UTIL_Random(0.08f, 0.11f);
	}

	pev->flags |= FL_KILLME;
	co_return;
}

void CClusterBomb::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE[CLUSTER_BOMB]);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = Vector::Down() * SPEED;
	pev->gravity = 1.f;
	g_engfuncs.pfnVecToAngles(pev->velocity, pev->angles);

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

	m_Scheduler.Enroll(Task_ClusterBomb());
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
	WriteData(TE_EXPLFLAG_NONE);
	MsgEnd();

	co_await TaskScheduler::NextFrame::Rank[0];

	auto const tr = Impact(m_pPlayer, this, 125.f);
	RangeDamage(m_pPlayer, pev->origin, 250.f, 200.f);
	ScreenEffects(pev->origin, 500.f, 15.f, 1024.f);

	if (tr.pHit != nullptr)
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::SCORCH).m_Index);

	co_await TaskScheduler::NextFrame::Rank[0];

	auto const pFieldSmoke = Prefab_t::Create<CFieldSmoke>();
	auto const iFlameCount = UTIL_Random(1, 3);
	for (int i = 0; i < iFlameCount; ++i)
	{
		auto const pFlame = Prefab_t::Create<CFlame>(tr.vecEndPos + Vector(0, 0, 8));
		pFlame->pev->velocity = get_spherical_coord(350.f, UTIL_Random(30.0, 45.0), UTIL_Random(0.0, 359.9));
		pFlame->pev->gravity = 1.f;

		auto const pSmoke = Prefab_t::Create<CSmoke>(tr.vecEndPos + Vector(UTIL_Random(-96, 96), UTIL_Random(-96, 96), UTIL_Random(0, 72)));
		pSmoke->LitByFlame();

		pFieldSmoke->EnrollFlame(pFlame);
		pFieldSmoke->EnrollSmoke(pSmoke);
	}

	pFieldSmoke->Activate();

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
	g_engfuncs.pfnSetModel(edict(), Models::PROJECTILE[CARPET_BOMBARDMENT]);
	g_engfuncs.pfnSetSize(edict(), Vector(-2, -2, -2), Vector(2, 2, 2));

	pev->owner = m_pPlayer->edict();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = Vector::Zero();	// No init speed needed. Gravity is good enough.
	pev->gravity = UTIL_Random(0.8f, 1.1f);
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
		UTIL_Decal(tr.pHit, tr.vecEndPos, UTIL_GetRandomOne(Decal::GUNSHOT).m_Index);

	if (tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
	{
		EHANDLE<CBaseEntity> pVictim = tr.pHit;

		g_pfnClearMultiDamage();
		pVictim->TraceAttack(m_pShooter->pev, 100.f, pev->velocity.Normalize(), &tr, DMG_BULLET);
		g_pfnApplyMultiDamage(pev, m_pShooter->pev);

		pev->flags |= FL_KILLME;
		co_return;
	}

	Vector vecAngles{};
	g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, vecAngles);
	vecAngles.x += 270.f;	// it seems like all MDL requires a += 270 shift.

	Prefab_t::Create<CSpark>(tr.vecEndPos, vecAngles);
	Prefab_t::Create<CGunshotSmoke>(tr);

	co_await TaskScheduler::NextFrame::Rank[1];

	auto const flScale = UTIL_Random(0.05f, 0.075f);
	UTIL_BreakModel(
		tr.vecEndPos, Vector(flScale, flScale, flScale), tr.vecPlaneNormal * UTIL_Random(75, 100),
		UTIL_Random(0.8f, 1.2f),
		Models::m_rgLibrary[Models::GIBS_CONCRETE],
		UTIL_Random(4, 12),
		UTIL_Random(8.f, 12.f),
		0x40
	);

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_GUNSHOT);
	WriteData(tr.vecEndPos);
	MsgEnd();

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
			EHANDLE<CBaseEntity> pVictim = tr.pHit;

			g_pfnClearMultiDamage();
			pVictim->TraceAttack(m_pShooter->pev, 100.f, pev->velocity.Normalize(), &tr, DMG_BULLET);
			g_pfnApplyMultiDamage(pev, m_pShooter->pev);
		}

		m_vecLastTraceSrc = pev->origin;
	}
}

void CBullet::Spawn() noexcept
{
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;
	pev->gravity = 0;

	g_engfuncs.pfnSetModel(edict(), "models/rshell.mdl");
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetSize(edict(), Vector(-0.1, -0.1, -0.1), Vector(0.1, 0.1, 0.1));

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_BEAMFOLLOW);
	WriteData(entindex());
	WriteData(Sprites::m_rgLibrary[Sprites::TRAIL]);
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
