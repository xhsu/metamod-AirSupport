import <algorithm>;
import <array>;
import <numbers>;

import progdefs;

import CBase;
import Entity;
import Message;
import Resources;
import Task;

import UtlRandom;

using std::array;

extern "C++" namespace Missile
{
	TimedFn Task_TravelSFX(EHANDLE<CBaseEntity> pEntity) noexcept
	{
		for (; pEntity;)
		{
			g_engfuncs.pfnEmitSound(pEntity.Get(), CHAN_WEAPON, Sounds::TRAVEL, VOL_NORM, ATTN_NORM, 0, UTIL_Random(94, 112));
			co_await 1.f;
		}
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
		//pEdict->v.groupinfo = MISSILE_GROUPINFO; // #POTENTIAL_BUG
		pEdict->v.nextthink = 0.1f;

		MsgPVS(SVC_TEMPENTITY, vecSpawnOrigin);
		WriteData(TE_SPRITE);
		WriteData(vecSpawnOrigin);
		WriteData((short)Sprite::m_rgLibrary[Sprite::FIRE]);
		WriteData((byte)5);
		WriteData((byte)255);
		MsgEnd();

		pEdict->v.effects = EF_LIGHT | EF_BRIGHTLIGHT;

		// #INVESTIGATE why won't this work?
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

		static constexpr auto get_spherical_coord = [](float radius, float inclination, float azimuth) noexcept
		{
			radius = std::clamp(radius, 0.f, 8192.f);	// r ∈ [0, ∞)
			inclination = (float)std::clamp(inclination * std::numbers::pi / 180.0, 0.0, std::numbers::pi);	// θ ∈ [0, π]
			azimuth = (float)std::clamp(azimuth * std::numbers::pi / 180.0, 0.0, std::numbers::pi * 2.0);	// φ ∈ [0, 2π)

			auto const length = radius * sin(inclination);

			return Vector(
				length * cos(azimuth),
				length * sin(azimuth),
				radius * cos(inclination)
			);
		};

		g_engfuncs.pfnMakeVectors(pEdict->v.angles);

		auto const qRot = Quaternion::Rotate(Vector(0, 0, 1), gpGlobals->v_forward);

		array const rgvecPericoord =
		{
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 0.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 72.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 144.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 216.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 288.f),
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

		TimedFnMgr::Enroll(Task_TravelSFX(pEdict));

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
