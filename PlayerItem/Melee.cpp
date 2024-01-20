/*

Created Date: Jan 19 2024

Modern Warfare Dev Team
Programmer - Luna the Reborn

*/

import event_flags;

import Uranus;
import Weapons;

import UtlRandom;


enum EVKnife
{
	KNIFE_IDLE,
	KNIFE_ATTACK1HIT,
	KNIFE_ATTACK2HIT,
	KNIFE_DRAW,
	KNIFE_STABHIT,
	KNIFE_STABMISS,
	KNIFE_MIDATTACK1HIT,
	KNIFE_MIDATTACK2HIT,
};

enum EVShieldKnife
{
	KNIFE_SHIELD_IDLE,
	KNIFE_SHIELD_SLASH,
	KNIFE_SHIELD_ATTACKHIT,
	KNIFE_SHIELD_DRAW,
	KNIFE_SHIELD_UPIDLE,
	KNIFE_SHIELD_UP,
	KNIFE_SHIELD_DOWN,
};


void CKnife2::Spawn() noexcept
{
	Precache();

	m_iId = WEAPON_KNIFE;
	g_engfuncs.pfnSetModel(edict(), "models/w_knife.mdl");

	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_iClip = WEAPON_NOCLIP;

	// Get ready to fall down
	FallInit();

	// extend
	__super::Spawn();
}

void CKnife2::Precache() noexcept
{
	g_engfuncs.pfnPrecacheModel("models/v_knife.mdl");
	g_engfuncs.pfnPrecacheModel("models/shield/v_shield_knife.mdl");
	g_engfuncs.pfnPrecacheModel("models/w_knife.mdl");

	g_engfuncs.pfnPrecacheSound("weapons/knife_deploy1.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_hit1.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_hit2.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_hit3.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_hit4.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_slash1.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_slash2.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_stab.wav");
	g_engfuncs.pfnPrecacheSound("weapons/knife_hitwall1.wav");

	m_usKnife = g_engfuncs.pfnPrecacheEvent(1, "events/knife.sc");
}

int CKnife2::GetItemInfo(ItemInfo* p) noexcept
{
	p->pszName = &CLASSNAME[0];
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = 0;

	return true;
}

qboolean CKnife2::Deploy() noexcept
{
	g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3f, 2.4f, 0, PITCH_NORM);

	m_iSwing = 0;
	m_fMaxSpeed = KNIFE_MAX_SPEED;

	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->HasShield())
	{
		return DefaultDeploy("models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", UseDecrement() != false);
	}
	else
		return DefaultDeploy("models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", UseDecrement() != false);
}

void CKnife2::Holster(int skiplocal) noexcept
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void FindHullIntersection(const Vector& vecSrc, TraceResult& trOut, const Vector& mins, const Vector& maxs, edict_t* pEntity) noexcept
{
	TraceResult tmpTrace;
	auto const vecHullEnd = vecSrc + ((trOut.vecEndPos - vecSrc) * 2);
	g_engfuncs.pfnTraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);

	if (tmpTrace.flFraction < 1.0f)
	{
		trOut = tmpTrace;
		return;
	}

	Vector vecEnd;
	Vector const minmaxs[2] = { mins, maxs };
	auto distance = 1e12;

	for (auto i = 0; i < 2; i++)
	{
		for (auto j = 0; j < 2; j++)
		{
			for (auto k = 0; k < 2; k++)
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);

				if (tmpTrace.flFraction < 1.0f)
				{
					auto const thisDistance = (tmpTrace.vecEndPos - vecSrc).LengthSquared();

					if (thisDistance < distance)
					{
						trOut = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CKnife2::PrimaryAttack() noexcept
{
	Swing();
}

void CKnife2::SetPlayerShieldAnim() noexcept
{
	if (!m_pPlayer->HasShield())
		return;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		strcpy(m_pPlayer->m_szAnimExtention, "shield");
	}
	else
	{
		strcpy(m_pPlayer->m_szAnimExtention, "shieldknife");
	}
}

void CKnife2::ResetPlayerShieldAnim() noexcept
{
	if (!m_pPlayer->HasShield())
		return;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		strcpy(m_pPlayer->m_szAnimExtention, "shieldknife");
	}
}

bool CKnife2::ShieldSecondaryFire(int iUpAnim, int iDownAnim) noexcept
{
	if (!m_pPlayer->HasShield())
	{
		return false;
	}

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

		SendWeaponAnim(iDownAnim, UseDecrement() != false);

		strcpy(m_pPlayer->m_szAnimExtention, "shieldknife");

		m_fMaxSpeed = KNIFE_MAX_SPEED;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(iUpAnim, UseDecrement() != false);

		strcpy(m_pPlayer->m_szAnimExtention, "shielded");

		m_fMaxSpeed = KNIFE_MAX_SPEED_SHIELD;
		m_pPlayer->m_bShieldDrawn = true;
	}

	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) != WPNSTATE_SHIELD_DRAWN);
	m_pPlayer->ResetMaxSpeed();

	m_flNextPrimaryAttack = GetNextAttackDelay(0.4f);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6f;

	return true;
}

void CKnife2::SecondaryAttack() noexcept
{
	if (!ShieldSecondaryFire(KNIFE_SHIELD_UP, KNIFE_SHIELD_DOWN))
	{
		Stab();
		pev->nextthink = UTIL_WeaponTimeBase() + 0.35f;
	}
}

void CKnife2::WeaponIdle() noexcept
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector((float)AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_bShieldDrawn)
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0f;

	// only idle if the slid isn't back
	SendWeaponAnim(KNIFE_IDLE, UseDecrement() != false);
}

void CKnife2::Swing() noexcept
{
	g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

	auto vecSrc = m_pPlayer->GetGunPosition();
	auto vecEnd = vecSrc + gpGlobals->v_forward * KNIFE_SWING_DISTANCE;

	TraceResult tr;
	g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction >= 1.0f)
	{
		g_engfuncs.pfnTraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			EHANDLE<CBaseEntity> pHit{ tr.pHit };

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());

			// This is the point on the actual surface (the hull could have hit space)
			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction >= 1.0f)
	{
		if (!m_pPlayer->HasShield())
		{
			switch ((m_iSwing++) % 2)
			{
			case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != false); break;
			case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != false); break;
			}

			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.35f);
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		}
		else
		{
			SendWeaponAnim(KNIFE_SHIELD_ATTACKHIT, UseDecrement() != false);

			m_flNextPrimaryAttack = GetNextAttackDelay(1.f);
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.f;

		// play wiff or swish sound
		if (UTIL_Random())
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	}
	else
	{
		if (!m_pPlayer->HasShield())
		{
			switch ((m_iSwing++) % 2)
			{
			case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != false); break;
			case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != false); break;
			}

			m_flNextPrimaryAttack = GetNextAttackDelay(0.4f);
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		}
		else
		{
			SendWeaponAnim(KNIFE_SHIELD_ATTACKHIT, UseDecrement() != false);

			m_flNextPrimaryAttack = GetNextAttackDelay(1.f);
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;

		// play thwack, smack, or dong sound
		float flVol = 1.0f;
		bool bHitWorld = true;

		EHANDLE<CBaseEntity> pEntity{ tr.pHit };
		SetPlayerShieldAnim();

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		gUranusCollection.pfnClearMultiDamage();

		if (m_flNextPrimaryAttack + 0.4f < UTIL_WeaponTimeBase())
			pEntity->TraceAttack(m_pPlayer->pev, KNIFE_SWING_DAMAGE_FAST, gpGlobals->v_forward, &tr, (DMG_NEVERGIB | DMG_BULLET));
		else
			pEntity->TraceAttack(m_pPlayer->pev, KNIFE_SWING_DAMAGE, gpGlobals->v_forward, &tr, (DMG_NEVERGIB | DMG_BULLET));

		gUranusCollection.pfnApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		if (auto const iClass = pEntity->Classify(); iClass != CLASS_NONE && iClass != CLASS_MACHINE && iClass != CLASS_VEHICLE)
		{
			// play thwack or smack sound
			switch (UTIL_Random(0, 3))
			{
			case 0: g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			case 1: g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			case 2: g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			case 3: g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			}

			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

			if (!pEntity->IsAlive())
				return;

			flVol = 0.1f;
			bHitWorld = false;
		}

		if (!bHitWorld)
		{
			m_pPlayer->m_iWeaponVolume = int(flVol * KNIFE_WALLHIT_VOLUME);

			ResetPlayerShieldAnim();
		}
		else
		{
			// play texture hit sound
			// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line
			gUranusCollection.pfnTEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			// also play knife strike
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, UTIL_Random(0, 3) + 98);
		}
	}
}

void CKnife2::Stab() noexcept
{
	g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

	auto vecSrc = m_pPlayer->GetGunPosition();
	auto vecEnd = vecSrc + gpGlobals->v_forward * KNIFE_STAB_DISTANCE;

	TraceResult tr;
	g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction >= 1.0f)
	{
		g_engfuncs.pfnTraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			EHANDLE<CBaseEntity> pHit{ tr.pHit };

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());

			// This is the point on the actual surface (the hull could have hit space)
			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction >= 1.0f)
	{
		SendWeaponAnim(KNIFE_STABMISS, UseDecrement() != false);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;

		// play wiff or swish sound
		if (UTIL_Random())
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	}
	else
	{
		SendWeaponAnim(KNIFE_STABHIT, UseDecrement() != false);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
		m_flNextPrimaryAttack = GetNextAttackDelay(1.1f);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1f;

		// play thwack, smack, or dong sound
		float flVol = 1.0f;
		bool bHitWorld = true;

		EHANDLE<CBaseEntity> pEntity{ tr.pHit };

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		float flDamage = KNIFE_STAB_DAMAGE;

		if (pEntity && pEntity->IsPlayer())
		{
			auto const vec2LOS = gpGlobals->v_forward.Make2D().Normalize();

			g_engfuncs.pfnMakeVectors(pEntity->pev->angles);

			auto const flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

			//Triple the damage if we are stabbing them in the back.
			if (flDot > 0.80f)
				flDamage *= 3.0f;
		}

		g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);
		gUranusCollection.pfnClearMultiDamage();

		pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, (DMG_NEVERGIB | DMG_BULLET));
		gUranusCollection.pfnApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		if (auto const iClass = pEntity->Classify(); iClass != CLASS_NONE && iClass != CLASS_MACHINE && iClass != CLASS_VEHICLE)
		{
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

			if (!pEntity->IsAlive())
				return;
			else
				flVol = 0.1f;

			bHitWorld = false;
		}

		if (!bHitWorld)
		{
			m_pPlayer->m_iWeaponVolume = int(flVol * KNIFE_WALLHIT_VOLUME);

			ResetPlayerShieldAnim();
		}
		else
		{
			// play texture hit sound
			// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

			gUranusCollection.pfnTEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			// also play knife strike
			g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, UTIL_Random(0, 3) + 98);
		}
	}
}
