import <cmath>;

import <array>;
import <bit>;
import <numbers>;
import <ranges>;
import <vector>;

import meta_api;
import shake;

import UtlRandom;

import Effects;
import Hook;
import Math;
import Query;
import Resources;
import Task;

using std::array;
using std::bit_cast;
using std::vector;

float GetAmountOfPlayerVisible(const Vector& vecSrc, CBaseEntity *pEntity) noexcept
{
	float retval = 0.0f;
	TraceResult tr{};

	static constexpr float topOfHead = 25.0f;
	static constexpr float standFeet = 34.0f;
	static constexpr float crouchFeet = 14.0f;
	static constexpr double edgeOffset = 13.0;	// Compiler happyness.

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

void RangeDamage(CBasePlayer *pAttacker, const Vector &vecOrigin, float const flRadius, float const flDamage) noexcept
{
	bool const bInWater = g_engfuncs.pfnPointContents(vecOrigin) == CONTENTS_WATER;

	for (auto &&pEntity : Query::all_entities_in_radius(vecOrigin, flRadius) |
		std::views::filter([](CBaseEntity *p) noexcept { return p->pev->takedamage != DAMAGE_NO; })
		)
	{
		// blast's don't tavel into or out of water
		if (bInWater && pEntity->pev->waterlevel == 0)
			continue;

		if (!bInWater && pEntity->pev->waterlevel == 3)
			continue;

		float const flDistance = (vecOrigin - pEntity->Center()).Length();
		float const flModifer = (flRadius - flDistance) * (flRadius - flDistance) * 1.25f / (flRadius * flRadius) * 1.5f;
		float const flAdjustedDmg = flModifer * GetAmountOfPlayerVisible(vecOrigin, pEntity) * flDamage;

		if (flAdjustedDmg < 1.f)
			continue;

		if (pEntity->IsPlayer())
		{
			auto const pVictim = (CBasePlayer *)pEntity;
			if (pVictim->IsAlive() && gcvarFriendlyFire->value < 1 && pVictim->m_iTeam == pAttacker->m_iTeam)
				continue;
		}

		pEntity->TakeDamage(pAttacker->pev, pAttacker->pev, flAdjustedDmg, DMG_EXPLOSION);
	}
}

void ScreenEffects(const Vector &vecOrigin, float const flRadius, float const flPunchMax, float const flKnockForce) noexcept
{
	for (auto &&pPlayer : FIND_ENTITY_IN_SPHERE(vecOrigin, flRadius)
		| std::views::filter([](edict_t *pEdict) noexcept { return pEdict->v.takedamage != DAMAGE_NO; })
		| std::views::transform([](edict_t *pEdict) noexcept { return (CBaseEntity *)pEdict->pvPrivateData; })
		| std::views::transform([](CBaseEntity *pEntity) noexcept { return (pEntity->IsPlayer() && pEntity->IsAlive()) ? (CBasePlayer *)pEntity : nullptr; })
		| std::views::filter([](CBasePlayer *pPlayer) noexcept { return pPlayer != nullptr; })
		)
	{
		Vector const vecDiff = pPlayer->pev->origin - vecOrigin;
		float const flDistance = vecDiff.Length();
		float const flModifer = (flRadius - flDistance) * (flRadius - flDistance) * 1.25f / (flRadius * flRadius) * 1.5f;

		gmsgScreenShake::Send(pPlayer->edict(),
			ScaledFloat<1 << 12>(35.0 * flModifer),	// amp
			ScaledFloat<1 << 12>(5.0 * flModifer),	// dur
			ScaledFloat<1 << 8>(12)					// freq
		);

		gmsgScreenFade::Send(pPlayer->edict(),
			ScaledFloat<1 << 12>(1.0 * flModifer),	// phase time
			ScaledFloat<1 << 12>(0.1),	// color hold
			FFADE_IN,	// flags
			255,		// r
			255,		// g
			255,		// b
			255			// a
		);

		float const flPunch = flPunchMax * flModifer;
		pPlayer->pev->punchangle += Angles(
			flPunch * (UTIL_Random() ? 1.0 : -1.0),
			flPunch * (UTIL_Random() ? 1.0 : -1.0),
			flPunch * (UTIL_Random() ? 1.0 : -1.0)
		);

		float const flSpeed = flModifer * flKnockForce;
		if (pPlayer->pev->maxspeed > 1)
			pPlayer->pev->velocity += vecDiff.Normalize() * flSpeed;

		if (flModifer > 0.65f)
			g_engfuncs.pfnClientCommand(pPlayer->edict(), "spk %s\n", Sounds::PLAYER_EAR_RINGING);
	}
}

Task VisualEffects(const Vector vecOrigin, float const flRadius) noexcept	// The parameter must pass by copy. This is a coroutine.
{
	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_SPRITE);
	WriteData(Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z + 200.0));
	WriteData((short)Sprites::m_rgLibrary[Sprites::ROCKET_EXPLO]);
	WriteData((byte)20);
	WriteData((byte)100);
	MsgEnd();

	MsgBroadcast(SVC_TEMPENTITY);
	WriteData(TE_SPRITE);
	WriteData(Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z + 70.0));
	WriteData((short)Sprites::m_rgLibrary[Sprites::ROCKET_EXPLO2]);
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
	WriteData((byte)70);
	WriteData((byte)255);
	WriteData((byte)0);
	WriteData((byte)0);
	WriteData((byte)2);
	WriteData((byte)0);
	MsgEnd();

	co_await gpGlobals->frametime;

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(vecOrigin, Vector(vecOrigin.x, vecOrigin.y, 8192.f), ignore_monsters, nullptr, &tr);
	g_engfuncs.pfnTraceLine(tr.vecEndPos, Vector(vecOrigin.x, vecOrigin.y, -8192.f), ignore_monsters, nullptr, &tr);

	for (int i = 0; i < 3; ++i)
	{
		auto pEdict = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("spark_shower"));
		g_engfuncs.pfnSetOrigin(pEdict, vecOrigin);
		pEdict->v.angles = bit_cast<Angles>(tr.vecPlaneNormal);	// This class, CShower, use pev->angles as a vector not an angle.

		pEdict->v.absmin = vecOrigin - Vector(1, 1, 1);
		pEdict->v.absmax = vecOrigin + Vector(1, 1, 1);

		gpGamedllFuncs->dllapi_table->pfnSpawn(pEdict);
	}

	if (g_engfuncs.pfnPointContents(vecOrigin) == CONTENTS_WATER)
		co_return;

	auto const qRotation = Quaternion::Rotate(Vector::Up(), tr.vecPlaneNormal);

	array const rgvecVelocitys =
	{
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 0),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 45),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 90),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 135),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 180),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 225),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 270),
		get_spherical_coord(Vector::Zero(), qRotation, UTIL_Random(300.f, 500.f), UTIL_Random(20.f, 30.f), 315),
	};

	for (auto &&vecVelocity : rgvecVelocitys)
	{
		// The scale parameter seemes no-op

		UTIL_BreakModel(
			vecOrigin, Vector(1, 1, 1), vecVelocity,
			UTIL_Random(0.8f, 2.f),
			Models::m_rgLibrary[Models::GIBS_CONCRETE],
			UTIL_Random(4, 12),
			UTIL_Random(8.f, 20.f),
			0x40
		);
	}

	UTIL_ExplodeModel(
		Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z + 70.0),
		UTIL_Random(300.f, 500.f),
		Models::m_rgLibrary[Models::GIBS_CONCRETE],
		UTIL_Random(6, 9),
		UTIL_Random(12.f, 15.f)
	);

	co_await gpGlobals->frametime;

	array const rgvecPericoords =
	{
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 0),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 45),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 90),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 135),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 180),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 225),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 270),
		get_spherical_coord(vecOrigin, qRotation, 160.f, UTIL_Random(85.f, 95.f), 315),
	};

	for (auto &&vecPeri : rgvecPericoords)
		Prefab_t::Create<CSmoke>(vecPeri)->LitByFlame(false);

	co_await gpGlobals->frametime;

	for (int i = 0; i < 8; ++i)
	{
		Prefab_t::Create<CFlame>(vecOrigin)->pev->velocity
			= get_spherical_coord(flRadius * 0.75, UTIL_Random(15.0, 25.0), UTIL_Random(0.0, 359.9));
	}
}

TraceResult Impact(CBasePlayer *pAttacker, CBaseEntity *pProjectile, float flDamage) noexcept
{
	g_engfuncs.pfnMakeVectors(Angles{ -pProjectile->pev->angles.pitch, pProjectile->pev->angles.yaw, pProjectile->pev->angles.roll });

	TraceResult tr{};
	g_engfuncs.pfnTraceLine(pProjectile->pev->origin, pProjectile->pev->origin + gpGlobals->v_forward * 32.f, dont_ignore_monsters, ent_cast<edict_t *>(pProjectile->pev), &tr);

	if (pev_valid(tr.pHit) != 2)
		return tr;

	CBaseEntity *pOther = (CBaseEntity *)tr.pHit->pvPrivateData;

	if (pOther->pev->takedamage == DAMAGE_NO)
		return tr;

	if (pOther->IsPlayer())
	{
		auto const pVictim = (CBasePlayer *)pOther;
		if (pVictim->IsAlive() && gcvarFriendlyFire->value < 1 && pVictim->m_iTeam == pAttacker->m_iTeam)
			return tr;
	}

	pOther->TraceAttack(pAttacker->pev, flDamage, gpGlobals->v_forward, &tr, DMG_BULLET);
	g_pfnApplyMultiDamage(pProjectile->pev, pAttacker->pev);
	return tr;
}
