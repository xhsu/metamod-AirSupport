import <algorithm>;
import <array>;
import <numbers>;

import progdefs;

import CBase;
import Entity;
import Math;
import Message;
import Resources;
import Task;

import UtlRandom;

using std::array;

extern "C++" namespace Missile
{
	Task Task_RemoveBeam(short iEntIndex) noexcept
	{
		co_await 1.f;

		// Ent gets invalided.
		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_KILLBEAM);
		WriteData(iEntIndex);
		MsgEnd();
	}

	Task Task_TravelSFX(EHANDLE<CBaseEntity> pEntity) noexcept
	{
		auto const iEntIndex = ent_cast<short>(pEntity.Get());

		for (; pEntity;)
		{
			g_engfuncs.pfnEmitSound(pEntity.Get(), CHAN_WEAPON, Sounds::TRAVEL, VOL_NORM, ATTN_NORM, 0, UTIL_Random(94, 112));
			co_await 1.f;
		}

		TaskScheduler::Enroll(Task_RemoveBeam(iEntIndex));
	};

	edict_t *Create(CBasePlayer *pPlayer, Vector const &vecSpawnOrigin, Vector const &vecTargetOrigin) noexcept
	{
		auto const pEdict = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));
		if (pev_valid(&pEdict->v) != 2)
			return nullptr;

		g_engfuncs.pfnSetOrigin(pEdict, vecSpawnOrigin);
		g_engfuncs.pfnSetModel(pEdict, Models::PROJECTILE[AIR_STRIKE]);
		g_engfuncs.pfnSetSize(pEdict, Vector(-2, -2, -2), Vector(2, 2, 2));

		pEdict->v.classname = MAKE_STRING(Classname::MISSILE);
		pEdict->v.owner = pPlayer->edict();
		pEdict->v.solid = SOLID_BBOX;
		pEdict->v.movetype = MOVETYPE_TOSS;
		pEdict->v.velocity = (vecTargetOrigin - vecSpawnOrigin).Normalize() * 1000;
		g_engfuncs.pfnVecToAngles(pEdict->v.velocity, pEdict->v.angles);
		pEdict->v.nextthink = 0.1f;

		MsgPVS(SVC_TEMPENTITY, vecSpawnOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecSpawnOrigin);
		WriteData((short)Sprite::m_rgLibrary[Sprite::FIRE]);
		WriteData((byte)5);
		WriteData((byte)255);
		MsgEnd();

		pEdict->v.effects = EF_LIGHT | EF_BRIGHTLIGHT;

		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_BEAMFOLLOW);
		WriteData(ent_cast<short>(pEdict));
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_TRAIL]);
		WriteData((byte)10);
		WriteData((byte)3);
		WriteData((byte)255);
		WriteData((byte)255);
		WriteData((byte)255);
		WriteData((byte)255);
		MsgEnd();

		g_engfuncs.pfnMakeVectors(pEdict->v.angles);

		auto const qRot = Quaternion::Rotate(Vector(0, 0, 1), gpGlobals->v_forward);

		array const rgvecPericoord =
		{
			get_spherical_coord(pEdict->v.origin, qRot, 24.f, 120.f, 0.f),
			get_spherical_coord(pEdict->v.origin, qRot, 24.f, 120.f, 72.f),
			get_spherical_coord(pEdict->v.origin, qRot, 24.f, 120.f, 144.f),
			get_spherical_coord(pEdict->v.origin, qRot, 24.f, 120.f, 216.f),
			get_spherical_coord(pEdict->v.origin, qRot, 24.f, 120.f, 288.f),
		};

		for (auto &&vecPos : rgvecPericoord)
		{
			MsgPVS(SVC_TEMPENTITY, vecPos);
			WriteData(TE_SPRITE);
			WriteData(vecPos);
			WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE]);
			WriteData((byte)10);
			WriteData((byte)50);
			MsgEnd();
		}

		TaskScheduler::Enroll(Task_TravelSFX(pEdict));

		return pEdict;
	}

	void Think(CBaseEntity *pEntity) noexcept
	{
		pEntity->pev->nextthink = gpGlobals->time + UTIL_Random(0.015f, 0.05f);

		pEntity->pev->angles += Vector(
			UTIL_Random(-0.35f, 0.35f),
			UTIL_Random(-0.35f, 0.35f),
			UTIL_Random(-0.35f, 0.35f)
		);

		// GoldSrc Mystery #1: The fucking v_angle and angles.
		g_engfuncs.pfnMakeVectors(Vector(
			-pEntity->pev->angles.x,
			pEntity->pev->angles.y,
			pEntity->pev->angles.z)
		);

		pEntity->pev->velocity = gpGlobals->v_forward * 1000;

		Vector vecOrigin = pEntity->pev->origin + gpGlobals->v_forward * -48;

		MsgPVS(SVC_TEMPENTITY, vecOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecOrigin);
		WriteData((short)Sprite::m_rgLibrary[Sprite::FIRE2]);
		WriteData((byte)3);
		WriteData((byte)255);
		MsgEnd();

		MsgPVS(SVC_TEMPENTITY, vecOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecOrigin);

		switch (UTIL_Random(0, 2))
		{
		case 0:
			WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE]);
			break;

		case 1:
			WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_1]);
			break;

		default:
			WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_2]);
			break;
		}

		WriteData((byte)UTIL_Random<short>(1, 10));
		WriteData((byte)UTIL_Random<short>(50, 255));
		MsgEnd();
	}
}
