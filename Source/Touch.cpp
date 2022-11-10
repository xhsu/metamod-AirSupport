import <cmath>;

import <array>;
import <numbers>;
import <ranges>;

import meta_api;

import UtlRandom;

import CBase;
import Entity;
import Hook;
import Resources;
import Task;

using std::array;

float GetAmountOfPlayerVisible(const Vector& vecSrc, CBaseEntity *pEntity) noexcept
{
	float retval = 0.0f;
	TraceResult tr{};

	static constexpr float topOfHead = 25.0f;
	static constexpr float standFeet = 34.0f;
	static constexpr float crouchFeet = 14.0f;
	static constexpr float edgeOffset = 13.0f;

	static constexpr float damagePercentageChest = 0.40f;
	static constexpr float damagePercentageHead = 0.20f;
	static constexpr float damagePercentageFeet = 0.20f;
	static constexpr float damagePercentageRightSide = 0.10f;
	static constexpr float damagePercentageLeftSide = 0.10f;

	if (!pEntity->IsPlayer())
	{
		// the entity is not a player, so the damage is all or nothing.
		g_engfuncs.pfnTraceLine(vecSrc, pEntity->Center(), ignore_monsters, nullptr, &tr);

		if (tr.flFraction == 1.0f)
			retval = 1.0f;

		return retval;
	}

	// check chest
	Vector vecChest = pEntity->pev->origin;
	g_engfuncs.pfnTraceLine(vecSrc, vecChest, ignore_monsters, nullptr, &tr);

	if (tr.flFraction == 1.0f)
		retval += damagePercentageChest;

	// check top of head
	Vector vecHead = pEntity->pev->origin + Vector(0, 0, topOfHead);
	g_engfuncs.pfnTraceLine(vecSrc, vecHead, ignore_monsters, nullptr, &tr);

	if (tr.flFraction == 1.0f)
		retval += damagePercentageHead;

	// check feet
	Vector vecFeet = pEntity->pev->origin;
	vecFeet.z -= (pEntity->pev->flags & FL_DUCKING) ? crouchFeet : standFeet;

	g_engfuncs.pfnTraceLine(vecSrc, vecFeet, ignore_monsters, nullptr, &tr);

	if (tr.flFraction == 1.0f)
		retval += damagePercentageFeet;

	Vector2D dir = (pEntity->pev->origin - vecSrc).Make2D().Normalize();

	Vector2D perp(-dir.y * edgeOffset, dir.x * edgeOffset);
	Vector vecRightSide = pEntity->pev->origin + Vector(perp.x, perp.y, 0);
	Vector vecLeftSide = pEntity->pev->origin - Vector(perp.x, perp.y, 0);

	// check right "edge"
	g_engfuncs.pfnTraceLine(vecSrc, vecRightSide, ignore_monsters, nullptr, &tr);

	if (tr.flFraction == 1.0f)
		retval += damagePercentageRightSide;

	// check left "edge"
	g_engfuncs.pfnTraceLine(vecSrc, vecLeftSide, ignore_monsters, nullptr, &tr);

	if (tr.flFraction == 1.0f)
		retval += damagePercentageLeftSide;

	return retval;
}

void Explosion(CBasePlayer *pAttacker, const Vector &vecOrigin, float flRadius, float flPunchMax, float flDamage, float flKnockForce) noexcept
{
	bool const bInWater = g_engfuncs.pfnPointContents(vecOrigin) == CONTENTS_WATER;

	for (auto &&pEntity : FIND_ENTITY_IN_SPHERE(vecOrigin, flRadius)
		| std::views::transform([](edict_t *pEdict) noexcept { return (CBaseEntity *)pEdict->pvPrivateData; })
		)
	{
		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->IsPlayer())
		{
			auto const pVictim = (CBasePlayer *)pEntity;
			if (pVictim->IsAlive() && gcvarFriendlyFire->value <= 0 && pVictim->m_iTeam == pAttacker->m_iTeam)
				continue;
		}

		// blast's don't tavel into or out of water
		if (bInWater && pEntity->pev->waterlevel == 0)
			continue;

		if (!bInWater && pEntity->pev->waterlevel == 3)
			continue;

		float flDistance = (vecOrigin - pEntity->Center()).Length();
		float flModifiedDmg = (flRadius - flDistance) * (flRadius - flDistance) * 1.25f / (flRadius * flRadius) * (GetAmountOfPlayerVisible(vecOrigin, pEntity) * flDamage) * 1.5f;

		if (flModifiedDmg < 1.f)
			continue;

		pEntity->TakeDamage(pAttacker->pev, pAttacker->pev, flModifiedDmg, DMG_EXPLOSION);

		if (!pEntity->IsAlive())
			continue;

		gmsgScreenShake::Send(ent_cast<edict_t *>(pEntity->pev), 1 << 13, 1 << 13, 1 << 13);
		gmsgScreenFade::Send(ent_cast<edict_t *>(pEntity->pev), 1 << 10, 0, 0x0000, 255, 255, 255, 255);

		pEntity->pev->punchangle += Vector(UTIL_Random(-flPunchMax, flPunchMax), UTIL_Random(-flPunchMax, flPunchMax), UTIL_Random(-flPunchMax, flPunchMax));

		Vector vecVelocity = (pEntity->Center() - vecOrigin).Normalize();
		float flSpeed = powf(flKnockForce, (flRadius - flDistance) / flRadius);

		if (pEntity->pev->maxspeed > 1)
			pEntity->pev->velocity += vecVelocity * flSpeed;
	}
}

void VisualEffects(const Vector &vecOrigin) noexcept
{
	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_SPRITE);
	WriteData(Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z + 200.f));
	WriteData((short)Sprite::m_rgLibrary[Sprite::ROCKET_EXPLO]);
	WriteData((byte)20);
	WriteData((byte)100);
	MsgEnd();

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_SPRITE);
	WriteData(Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z + 70.f));
	WriteData((short)Sprite::m_rgLibrary[Sprite::ROCKET_EXPLO2]);
	WriteData((byte)30);
	WriteData((byte)255);
	MsgEnd();

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_WORLDDECAL);
	WriteData(vecOrigin);
	WriteData((byte)UTIL_GetRandomOne(Decal::SCORCH).m_Index);
	MsgEnd();

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_DLIGHT);
	WriteData(vecOrigin);
	WriteData((byte)50);
	WriteData((byte)255);
	WriteData((byte)0);
	WriteData((byte)0);
	WriteData((byte)2);
	WriteData((byte)0);
	MsgEnd();

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(vecOrigin, Vector(vecOrigin.x, vecOrigin.y, 8192.f), ignore_monsters, nullptr, &tr);
	g_engfuncs.pfnTraceLine(tr.vecEndPos, Vector(vecOrigin.x, vecOrigin.y, -8192.f), ignore_monsters, nullptr, &tr);

	for (int i = 0; i < 3; ++i)
	{
		auto pEdict = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("spark_shower"));
		g_engfuncs.pfnSetOrigin(pEdict, vecOrigin);
		g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pEdict->v.angles);

		pEdict->v.absmin = vecOrigin - Vector(1, 1, 1);
		pEdict->v.absmax = vecOrigin + Vector(1, 1, 1);

		gpGamedllFuncs->dllapi_table->pfnSpawn(pEdict);
	}

	if (g_engfuncs.pfnPointContents(vecOrigin) == CONTENTS_WATER)
		return;

	static constexpr auto get_spherical_coord = [](const Vector &vecOrigin, const Quaternion &qRotation, float radius, float inclination, float azimuth) noexcept
	{
		radius = std::clamp(radius, 0.f, 8192.f);	// r ∈ [0, ∞)
		inclination = (float)std::clamp(inclination * std::numbers::pi / 180.0, 0.0, std::numbers::pi);	// θ ∈ [0, π]
		azimuth = (float)std::clamp(azimuth * std::numbers::pi / 180.0, 0.0, std::numbers::pi * 2.0);	// φ ∈ [0, 2π)

		auto const length = radius * sin(inclination);

		return vecOrigin + qRotation * Vector(
			length * cos(azimuth),
			length * sin(azimuth),
			radius * cos(inclination)
		);
	};

	static constexpr auto BreakModel = [](const Vector &vecOrigin, const Vector &vecScale, const Vector &vecVelocity, float flRandSpeedVar, short iModel, byte iCount, float flLife, byte bitsFlags) noexcept
	{
		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_BREAKMODEL);
		WriteData(vecOrigin);
		WriteData(vecScale);
		WriteData(vecVelocity);
		WriteData(static_cast<byte>(flRandSpeedVar * 10.f));
		WriteData(iModel);
		WriteData(iCount);
		WriteData(static_cast<byte>(flLife * 10.f));
		WriteData(bitsFlags);
		MsgEnd();
	};

	auto const qRotation = Quaternion::Rotate(Vector(0, 0, 1), tr.vecPlaneNormal);

	array const rgvecVelocitys =
	{
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 0),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 45),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 90),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 135),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 180),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 225),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 270),
		get_spherical_coord(Vector::Zero(), qRotation, 1.f, UTIL_Random(20.f, 30.f), 315),
	};

	for (auto &&vecVelocity : rgvecVelocitys)
	{
		auto const flScale = UTIL_Random(0.65f, 1.f);

		BreakModel(
			vecOrigin, Vector(flScale, flScale, flScale), vecVelocity * UTIL_Random(300.f, 500.f),
			UTIL_Random(0.8f, 2.f),
			Models::m_rgLibrary[Models::GIBS_WALL_BROWN],
			UTIL_Random(4, 12),
			UTIL_Random(8.f, 20.f),
			0x40
		);

		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_SPRITE);
		WriteData(vecOrigin + vecVelocity * UTIL_Random(200.f, 300.f));
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE]);
		WriteData((byte)25);
		WriteData((byte)225);
		MsgEnd();
	}

	array const rgvecPericoords =
	{
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 0),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 45),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 90),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 135),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 180),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 225),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 270),
		get_spherical_coord(vecOrigin, qRotation, 128.f, UTIL_Random(85.f, 95.f), 315),
	};

	for (auto &&vecPeri : rgvecPericoords)
	{
		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_SPRITE);
		WriteData(vecPeri);
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_2]);
		WriteData((byte)50);
		WriteData((byte)50);
		MsgEnd();
	}
}

void Impact(CBasePlayer *pAttacker, CBaseEntity *pProjectile, CBaseEntity *pOther, float flDamage) noexcept
{
	g_engfuncs.pfnMakeVectors(pProjectile->pev->angles);

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pProjectile->pev->origin, pProjectile->pev->origin + gpGlobals->v_forward * 4096.f, dont_ignore_monsters, ent_cast<edict_t *>(pProjectile->pev), &tr);

	if (&tr.pHit->v != pOther->pev)
		return;

	pOther->TraceAttack(pAttacker->pev, flDamage, gpGlobals->v_forward, &tr, DMG_BULLET);
	g_pfnApplyMultiDamage(pProjectile->pev, pAttacker->pev);
}

META_RES OnTouch(CBaseEntity *pEntity, CBaseEntity *pOther) noexcept
{
	if (pEntity->pev->classname == MAKE_STRING(Classname::JET) && pev_valid(pOther->pev) != 2)
	{
		pEntity->pev->flags |= FL_KILLME;
		return MRES_HANDLED;
	}

	else if (pEntity->pev->classname == MAKE_STRING(Classname::MISSILE))
	{
		if (g_engfuncs.pfnPointContents(pEntity->pev->origin) == CONTENTS_SKY)
		{
			g_engfuncs.pfnRemoveEntity(ent_cast<edict_t *>(pEntity->pev));
			return MRES_HANDLED;
		}

		CBasePlayer *pPlayer = (CBasePlayer *)pEntity->pev->owner->pvPrivateData;

		g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pEntity->pev), CHAN_WEAPON, UTIL_GetRandomOne(Sounds::EXPLOSION), VOL_NORM, 0.3f, 0, UTIL_Random(92, 116));

		if (pev_valid(pOther->pev) == 2)
			Impact(pPlayer, pEntity, pOther, 125.f);

		Explosion(pPlayer, pEntity->pev->origin, 350.f, 8.f, 275.f, 2048.f);
		VisualEffects(pEntity->pev->origin);

		TimedFnMgr::Delist(MISSILE_SOUND_CORO_KEY + ent_cast<unsigned long>(pEntity->pev));

		pEntity->pev->flags |= FL_KILLME;
	}

// #UNDONE else if (!strcmp(szClassName, "petrol_bomb"))

	return MRES_IGNORED;
}