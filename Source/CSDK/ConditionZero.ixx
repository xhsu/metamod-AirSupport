module;

#ifdef _DEBUG
#include <cassert>
#endif

#include <cstring>	// _stricmp

export module ConditionZero;

import std;
import hlsdk;

import CBase;
import GameRules;	// only used in CBase extensions.
import Message;		// only used in CBase extensions.
import Platform;
import Query;		// for each player, only used in CBase extensions.
import Uranus;		// runtime hook compatibility

import UtlHook;
import UtlRandom;


using std::strcpy;	// #MSVC_BUG_STDCOMPAT


export inline constexpr std::ptrdiff_t ITEM_INFO_ARRAY_OFS_8684 = 0x100CDC09 - 0x100CDC00;	// offset from W_Precache
export inline constexpr std::ptrdiff_t ITEM_INFO_ARRAY_OFS_9899 = 0x100C1A78 - 0x100C1A70;
export inline constexpr std::ptrdiff_t AMMO_INFO_ARRAY_OFS_8684 = 0x100CDC15 - 0x100CDC00;	// offset from W_Precache
export inline constexpr std::ptrdiff_t AMMO_INFO_ARRAY_OFS_9899 = 0x100C1A89 - 0x100C1A70;
export inline constexpr std::ptrdiff_t GI_AMMO_INDEX_OFS_8684 = 0x100CDC21 - 0x100CDC00;	// offset from W_Precache
export inline constexpr std::ptrdiff_t GI_AMMO_INDEX_OFS_9899 = 0x100C1A99 - 0x100C1A70;

export inline constexpr unsigned char ITEM_INFO_ARRAY_8684_PATTERN[] = "\xBF\x2A\x2A\x2A\x2A\xF3\xAB\xB9\x2A\x2A\x2A\x2A\xBF\x2A\x2A\x2A\x2A\xF3\xAB\x68\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8";
export inline constexpr unsigned char ITEM_INFO_ARRAY_9899_PATTERN[] = "\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xC7\x05";
export inline constexpr unsigned char AMMO_INFO_ARRAY_8684_PATTERN[] = "\xBF\x2A\x2A\x2A\x2A\xF3\xAB\x68\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68";
export inline constexpr unsigned char AMMO_INFO_ARRAY_9899_PATTERN[] = "\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x00\x00\x00\x00";
export inline constexpr unsigned char GI_AMMO_INDEX_8684_PATTERN[] = "\xA3\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8";
export inline constexpr unsigned char GI_AMMO_INDEX_9899_PATTERN[] = "\x05\x2A\x2A\x2A\x2A\x00\x00\x00\x00\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8";

export inline std::int32_t* gpiAmmoIndex = nullptr;


// ServerActivate_Post
export void RetrieveConditionZeroVar() noexcept
{
	auto const pItemInfo = UTIL_RetrieveGlobalVariable<ItemInfo>(
		UTIL_SearchPattern("mp.dll", 0, ITEM_INFO_ARRAY_8684_PATTERN, ITEM_INFO_ARRAY_9899_PATTERN),
		1	// skip the instructing operator, just hold on the address
	);

	auto const pAmmoInfo = UTIL_RetrieveGlobalVariable<AmmoInfo>(
		UTIL_SearchPattern("mp.dll", 0, AMMO_INFO_ARRAY_8684_PATTERN, AMMO_INFO_ARRAY_9899_PATTERN),
		1
	);

	gpiAmmoIndex = UTIL_RetrieveGlobalVariable<int32_t>(
		UTIL_SearchPattern("mp.dll", 0, GI_AMMO_INDEX_8684_PATTERN, GI_AMMO_INDEX_9899_PATTERN),
		1
	);

	CBasePlayerItem::ItemInfoArray = std::span{ pItemInfo, MAX_WEAPONS };
	CBasePlayerItem::AmmoInfoArray = std::span{ pAmmoInfo, MAX_AMMO_SLOTS };

#ifdef _DEBUG
	assert(pItemInfo && pAmmoInfo && gpiAmmoIndex);
#else
	[[unlikely]]
	if (!pItemInfo)
		UTIL_Terminate("Global variable \"CBasePlayerItem::m_ItemInfoArray[]\" no found!");
	[[unlikely]]
	if (!pAmmoInfo)
		UTIL_Terminate("Global variable \"CBasePlayerItem::m_AmmoInfoArray[]\" no found!");
	[[unlikely]]
	if (!gpiAmmoIndex)
		UTIL_Terminate("Global variable \"::giAmmoIndex\" no found!");
#endif
}

/////////////////
// CBaseEntity //
/////////////////

Vector CBaseEntity::FireBullets3(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand)
{
	Vector ret{};
	gUranusCollection.pfnFireBullets3(
		this, nullptr, &ret,
		vecSrc, vecDirShooting, flSpread, flDistance,
		iPenetration, iBulletType, iDamage, flRangeModifier,
		pevAttacker,
		bPistol,
		shared_rand
	);

	return ret;
}

CBaseEntity* CBaseEntity::Create(char const* szName, const Vector& vecOrigin, const Angles& vecAngles, edict_t* pentOwner) noexcept
{
	return gUranusCollection.pfnCreate(szName, vecOrigin, vecAngles, pentOwner);
}

////////////////
// CBaseDelay //
////////////////

void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value) noexcept
{
	gUranusCollection.pfnSUB_UseTargets(this, pActivator, useType, value);
}

/////////////////////
// CBasePlayerItem //
/////////////////////

void CBasePlayerItem::DestroyItem(void) noexcept
{
	if (m_pPlayer)
		m_pPlayer->RemovePlayerItem(this);

	Kill();
}

void CBasePlayerItem::DefaultTouch(CBaseEntity* pOther) noexcept
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	if (pPlayer->m_bIsVIP
		&& m_iId != WEAPON_USP
		&& m_iId != WEAPON_GLOCK18
		&& m_iId != WEAPON_P228
		&& m_iId != WEAPON_DEAGLE
		&& m_iId != WEAPON_KNIFE)
	{
		return;
	}

	if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
	{
		return;
	}

	if (pOther->AddPlayerItem(this))
	{
		AttachToPlayer(pPlayer);
		g_engfuncs.pfnEmitSound(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM, SND_FL_NONE, PITCH_NORM);
	}

	SUB_UseTargets(pOther, USE_TOGGLE, 0);
}

void CBasePlayerItem::FallThink(void) noexcept
{
	pev->nextthink = gpGlobals->time + 0.1f;

	if (pev->flags & FL_ONGROUND)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if (pev_valid(pev->owner) != EValidity::Full)
		{
			g_engfuncs.pfnEmitSound(edict(), CHAN_VOICE, "items/weapondrop1.wav", VOL_NORM, ATTN_NORM, SND_FL_NONE, UTIL_Random(0, 29) + 95);
		}

		// lie flat
		pev->angles.pitch = 0.0f;
		pev->angles.roll = 0.0f;

		Materialize();
	}
}

void CBasePlayerItem::Materialize() noexcept
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		if (g_pGameRules->IsMultiplayer())
		{
			g_engfuncs.pfnEmitSound(edict(), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, SND_FL_NONE, 150);
		}

		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = SOLID_TRIGGER;

	// link into world.
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	SetTouch(&CBasePlayerItem::DefaultTouch);

	if (g_pGameRules->IsMultiplayer())
	{
		if (!CanDrop())
		{
			SetTouch(nullptr);
		}

		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	else
	{
		SetThink(nullptr);
	}
}

void CBasePlayerItem::AttemptToMaterialize() noexcept
{
	float const time = g_pGameRules->FlWeaponTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

void CBasePlayerItem::FallInit(void) noexcept
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	g_engfuncs.pfnSetOrigin(edict(), pev->origin);

	// pointsize until it lands on the ground.
	g_engfuncs.pfnSetSize(edict(), Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(&CBasePlayerItem::FallThink);

	pev->nextthink = gpGlobals->time + 0.1f;
}

///////////////////////
// CBasePlayerWeapon //
///////////////////////

export constexpr float UTIL_WeaponTimeBase() noexcept { return 0.f; }	// or return gpGlobals->time; if not CLIENT_WEAPONS
export constexpr bool CanAttack(float attack_time, float curtime, bool isPredicted) noexcept
{
	if (!isPredicted)
		return attack_time <= curtime;

	return attack_time <= 0;
}
export constexpr bool IsSecondaryWeapon(int id) noexcept
{
	switch (id)
	{
	case WEAPON_P228:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
	case WEAPON_USP:
	case WEAPON_GLOCK18:
	case WEAPON_DEAGLE:
		return true;
	default:
		break;
	}

	return false;
}
export constexpr void DecalGunshot(TraceResult* pTrace, int iBulletType, bool ClientOnly, entvars_t* pShooter, bool bHitMetal) noexcept {}
export constexpr float GetBaseAccuracy(WeaponIdType id) noexcept
{
	switch (id)
	{
	case WEAPON_M4A1:
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_SG552:
	case WEAPON_FAMAS:
	case WEAPON_GALIL:
	case WEAPON_M249:
	case WEAPON_P90:
	case WEAPON_TMP:
		return 0.2f;
	case WEAPON_MAC10:
		return 0.15f;
	case WEAPON_UMP45:
	case WEAPON_MP5N:
		return 0.0f;
	}

	return 0.0f;
}

bool CBasePlayerWeapon::DefaultDeploy(char const* szViewModel, char const* szWeaponModel, int iAnim, char const* szAnimExt, int skiplocal) noexcept
{
	if (!CanDeploy())
		return false;

	m_pPlayer->TabulateAmmo();

	m_pPlayer->pev->viewmodel = MAKE_STRING_UNSAFE(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING_UNSAFE(szWeaponModel);

	model_name = m_pPlayer->pev->viewmodel;
	strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
	SendWeaponAnim(iAnim, skiplocal);

	m_pPlayer->m_flNextAttack = 0.75f;
	m_flTimeWeaponIdle = 1.5f;
	m_flLastFireTime = 0.0f;
	m_flDecreaseShotsFired = gpGlobals->time;

	m_pPlayer->m_iFOV = DEFAULT_FOV;
	m_pPlayer->pev->fov = DEFAULT_FOV;
	m_pPlayer->m_iLastZoom = DEFAULT_FOV;
	m_pPlayer->m_bResumeZoom = false;

	return true;
}

qboolean CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay, int body) noexcept
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return false;

	auto const j = std::min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
	if (!j)
	{
		return false;
	}

	m_pPlayer->m_flNextAttack = fDelay;

	ReloadSound();
	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0);

	m_fInReload = true;
	m_flTimeWeaponIdle = fDelay + 0.5f;

	return true;
}

void CBasePlayerWeapon::ReloadSound(void) noexcept
{
	static constexpr auto MAX_DIST_RELOAD_SOUND = 512.0 * 512.0;

	for (CBasePlayer* pPlayer : Query::all_players())
	{
		if (pPlayer == m_pPlayer)
			continue;

		auto const distance = (m_pPlayer->pev->origin - pPlayer->pev->origin).LengthSquared();
		if (distance <= MAX_DIST_RELOAD_SOUND)
		{
			gmsgReloadSound::Send(
				pPlayer->edict(),
				std::uint8_t((1.0f - (distance / MAX_DIST_RELOAD_SOUND)) * 255.0f),
				!strcmp(STRING(pev->classname), "weapon_m3") || !strcmp(STRING(pev->classname), "weapon_xm1014")
			);
		}
	}
}

qboolean CBasePlayerWeapon::AddPrimaryAmmo(int iCount, const char* szName, int iMaxClip, int iMaxCarry) noexcept
{
	int iIdAmmo{};

	if (iMaxClip < 1)
	{
		m_iClip = WEAPON_NOCLIP;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, (char*)szName, iMaxCarry);
	}
	else if (m_iClip == 0)
	{
		auto const i = std::min(m_iClip + iCount, iMaxClip);
		m_iClip += i;

		iIdAmmo = m_pPlayer->GiveAmmo(iCount - i, (char*)szName, iMaxCarry);
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, (char*)szName, iMaxCarry);
	}

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (m_pPlayer->HasPlayerItem(this))
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			g_engfuncs.pfnEmitSound(edict(), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM, SND_FL_NONE, PITCH_NORM);
		}
	}

	return iIdAmmo > 0 ? true : false;
}

qboolean CBasePlayerWeapon::AddSecondaryAmmo(int iCount, const char* szName, int iMaxCarry) noexcept
{
	auto const iIdAmmo = m_pPlayer->GiveAmmo(iCount, (char*)szName, iMaxCarry);

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		g_engfuncs.pfnEmitSound(edict(), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM, SND_FL_NONE, PITCH_NORM);
	}

	return iIdAmmo > 0 ? true : false;
}

void CBasePlayerWeapon::EjectBrassLate(void) noexcept
{
	auto&& [fwd, right, up] = (m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle).AngleVectors();

	auto const vecUp = UTIL_Random<float>(100, 150) * up;
	auto const vecRight = UTIL_Random<float>(50, 70) * right;

	auto const vecShellVelocity = (m_pPlayer->pev->velocity + vecRight + vecUp) + fwd * 25;
	auto const soundType = (m_iId == WEAPON_XM1014 || m_iId == WEAPON_M3) ? TE_BOUNCE_SHOTSHELL : TE_BOUNCE_SHELL;

	auto const vecOrigin = pev->origin + m_pPlayer->pev->view_ofs + up * -9 + fwd * 16;

	gmsgBrass::Region<MSG_PVS>(vecOrigin,
		vecOrigin,
		vecShellVelocity,
		pev->angles.yaw,
		m_iShellId,
		soundType,
		(std::uint8_t)m_pPlayer->entindex()
	);
}

void CBasePlayerWeapon::FireRemaining(int& shotsFired, float& shootTime, bool bIsGlock) noexcept
{
	if (--m_iClip < 0)
	{
		m_iClip = 0;
		shotsFired = 3;
		shootTime = 0;
		return;
	}

	g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	constexpr auto flag = FEV_NOTHOST;

	if (bIsGlock)
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.05f, 8192, 1, BULLET_PLAYER_9MM, 18, 0.9f, m_pPlayer->pev, true, m_pPlayer->random_seed);
		--m_pPlayer->ammo_9mm;

		g_engfuncs.pfnPlaybackEvent(
			flag, m_pPlayer->edict(), m_usFireGlock18, 0,
			(float*)&g_vecZero, (float*)&g_vecZero,
			vecDir.x, vecDir.y,
			int(m_pPlayer->pev->punchangle.pitch * 10000), int(m_pPlayer->pev->punchangle.yaw * 10000),
			m_iClip == 0, false
		);
	}
	else
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, 30, 0.96f, m_pPlayer->pev, false, m_pPlayer->random_seed);
		--m_pPlayer->ammo_556nato;

		g_engfuncs.pfnPlaybackEvent(
			flag, m_pPlayer->edict(), m_usFireFamas, 0,
			(float*)&g_vecZero, (float*)&g_vecZero,
			vecDir.x, vecDir.y,
			int(m_pPlayer->pev->punchangle.pitch * 10000000), int(m_pPlayer->pev->punchangle.yaw * 10000000),
			false, false
		);
	}

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (++shotsFired != 3)
	{
		shootTime = gpGlobals->time + 0.1f;
	}
	else
		shootTime = 0;
}

bool CBasePlayerWeapon::HasSecondaryAttack(void) const noexcept
{
	if (m_pPlayer && m_pPlayer->HasShield())
	{
		return true;
	}

	switch (m_iId)
	{
	case WEAPON_AK47:
	case WEAPON_XM1014:
	case WEAPON_MAC10:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
	case WEAPON_MP5N:
	case WEAPON_UMP45:
	case WEAPON_M249:
	case WEAPON_M3:
	case WEAPON_TMP:
	case WEAPON_DEAGLE:
	case WEAPON_P228:
	case WEAPON_P90:
	case WEAPON_C4:
	case WEAPON_GALIL:
		return false;
	default:
		break;
	}

	return true;
}

float CBasePlayerWeapon::GetNextAttackDelay(float delay) noexcept
{
	if (m_flLastFireTime == 0.0f || m_flNextPrimaryAttack == -1.0f)
	{
		// At this point, we are assuming that the client has stopped firing
		// and we are going to reset our book keeping variables.
		m_flPrevPrimaryAttack = delay;
		m_flLastFireTime = gpGlobals->time;
	}

	// TODO: Build 6xxx
	// at build 6153 beta this removed
	// maybe it was initiated due to the delay of the shot

	// calculate the time between this shot and the previous
	float flTimeBetweenFires = gpGlobals->time - m_flLastFireTime;
	float flCreep = 0.0f;

	if (flTimeBetweenFires > 0)
	{
		flCreep = flTimeBetweenFires - m_flPrevPrimaryAttack;
	}

	float flNextAttack = UTIL_WeaponTimeBase() + delay - flCreep;

	// save the last fire time
	m_flLastFireTime = gpGlobals->time;

	// we need to remember what the m_flNextPrimaryAttack time is set to for each shot,
	// store it as m_flPrevPrimaryAttack.
	m_flPrevPrimaryAttack = flNextAttack - UTIL_WeaponTimeBase();

	return flNextAttack;
}

/////////////////
// CBasePlayer //
/////////////////

void CBasePlayer::SetAnimation(PLAYER_ANIM playerAnim) noexcept
{
	gUranusCollection.pfnSetAnimation(this, 0, playerAnim);
}

qboolean CBasePlayer::HasPlayerItem(CBasePlayerItem* pCheckItem) const noexcept
{
	auto item = m_rgpPlayerItems[pCheckItem->iItemSlot()];
	while (item)
	{
		if (FClassnameIs(item->pev, STRING(pCheckItem->pev->classname)))
			return true;

		item = item->m_pNext;
	}

	return false;
}

int CBasePlayer::GetAmmoIndex(const char* psz) noexcept
{
	if (!psz)
		return -1;

	for (auto i = 0; i <= *gpiAmmoIndex; ++i)
	{
		[[unlikely]]
		if (CBasePlayerItem::AmmoInfoArray[i].pszName == nullptr)
			continue;

		if (!_stricmp(psz, CBasePlayerItem::AmmoInfoArray[i].pszName))
			return i;
	}

	return -1;
}

int CBasePlayer::AmmoInventory(int iAmmoIndex) const noexcept
{
	if (iAmmoIndex == -1)
		return -1;

	return m_rgAmmo[iAmmoIndex];
}

void CBasePlayer::TabulateAmmo(void) noexcept
{
	ammo_buckshot = AmmoInventory(GetAmmoIndex("buckshot"));
	ammo_9mm = AmmoInventory(GetAmmoIndex("9mm"));
	ammo_556nato = AmmoInventory(GetAmmoIndex("556Nato"));
	ammo_556natobox = AmmoInventory(GetAmmoIndex("556NatoBox"));
	ammo_762nato = AmmoInventory(GetAmmoIndex("762Nato"));
	ammo_45acp = AmmoInventory(GetAmmoIndex("45acp"));
	ammo_50ae = AmmoInventory(GetAmmoIndex("50AE"));
	ammo_338mag = AmmoInventory(GetAmmoIndex("338Magnum"));
	ammo_57mm = AmmoInventory(GetAmmoIndex("57mm"));
	ammo_357sig = AmmoInventory(GetAmmoIndex("357SIG"));
}

void CBasePlayer::UpdateShieldCrosshair(bool bShieldDrawn) noexcept
{
	if (bShieldDrawn)
		m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	else
		m_iHideHUD |= HIDEHUD_CROSSHAIR;
}
