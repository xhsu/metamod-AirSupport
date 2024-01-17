module;

#ifdef _DEBUG
#include <cassert>
#endif

export module ConditionZero;

import <span>;

import event_flags;

import CBase;
import GameRules;	// only used in CBase extensions.
import Message;	// only used in CBase extensions.
import Platform;

import UtlHook;
import UtlRandom;

export using fnCBaseCreate_t = CBaseEntity * (__cdecl*)(const char* pszName, Vector const& vecOrigin, Angles const& vecAngles, edict_t* pentOwner) noexcept;
export inline fnCBaseCreate_t g_pfnCSCZCBaseCreate = nullptr;
export inline constexpr unsigned char CSCZ_CBASE_CREATE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x83\xEC\x0C\x56\x8B\x75\x08\x2B\xB0\x2A\x2A\x2A\x2A\x57\x56\xFF\x15\x2A\x2A\x2A\x2A\x8B\xF8\x83\xC4\x04";

export using fnSUB_UseTargets_t = void(__thiscall*)(CBaseDelay*, CBaseEntity* pActivator, USE_TYPE useType, float value) noexcept;
export inline fnSUB_UseTargets_t g_pfnSUB_UseTargets = nullptr;
export inline constexpr unsigned char CSCZ_SUB_USE_TARGETS_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x57\x8B\xF9\x8B\x4F\x04\x83\xB9\x2A\x2A\x2A\x2A\x2A\x75\x0D\x83\xBF\x2A\x2A\x2A\x2A\x2A\x0F\x84\x2A\x2A\x2A\x2A\xF3\x0F\x10\x8F";

export inline constexpr unsigned char PRECACHE_OTHER_WPN_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x83\xEC\x2C\x8B\x4D\x08\x2B\x88\x2A\x2A\x2A\x2A\x56\x51\xE8\x2A\x2A\x2A\x2A\x8B\xF0\x83\xC4\x04";
export inline constexpr std::ptrdiff_t ITEM_INFO_ARRAY_OFS = 0x100C18FC - 0x100C1860;
export inline std::span<ItemInfo> g_rgItemInfo;

export inline constexpr unsigned char ADD_AMMO_REG_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x53\x8B\x1D\x2A\x2A\x2A\x2A\x56\x57\x8B\x7D\x08\xBE\x2A\x2A\x2A\x2A\x8B\x06\x85\xC0\x74\x0B\x57\x50\xFF\xD3\x83\xC4\x08";
export inline constexpr std::ptrdiff_t AMMO_INFO_ARRAY_OFS = 0x100BDF30 - 0x100BDF20;
export inline constexpr std::ptrdiff_t GI_AMMO_INDEX_OFS = 0x100BDF51 - 0x100BDF20;
export inline std::span<AmmoInfo> g_rgAmmoInfo;
export inline std::int32_t* gpiAmmoIndex = nullptr;

export using fnEmptyEntityHashTable_t = void(__cdecl*)(void) noexcept;
export inline fnEmptyEntityHashTable_t g_pfnEmptyEntityHashTable = nullptr;
export inline constexpr unsigned char EMPTY_ENT_HASH_TABLE_FN_ANNIV_PATTERN[] = "\xCC\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x8E\x2A\x2A\x2A\x2A\x53\x57\x8B\x3D\x2A\x2A\x2A\x2A\x33\xDB\x66\x0F\x1F\x44\x00";

export using fnAddEntityHashValue_t = void(__cdecl*)(entvars_t* pev, const char* pszClassname, int32_t) noexcept;
export inline fnAddEntityHashValue_t g_pfnAddEntityHashValue = nullptr;
export inline constexpr unsigned char ADD_ENT_HASH_TABLE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x51\x83\x7D\x10\x00\x0F\x85\x2A\x2A\x2A\x2A\x53\x8B\x5D\x08\x83\x3B\x00\x0F\x84\x2A\x2A\x2A\x2A\xA1";

export using fnRemoveEntityHashValue_t = void(__cdecl*)(entvars_t* pev, const char* pszClassname, int32_t) noexcept;
export inline fnRemoveEntityHashValue_t g_pfnRemoveEntityHashValue = nullptr;
export inline constexpr unsigned char REMOVE_ENT_HASH_TABLE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x8B\x4D\x0C\x33\xD2\x56\x57\x8A\x01\x84\xC0\x74\x18\x0F\xBE\xF0\x03\xD2\x2C\x41\x3C\x19\x77\x03\x83\xC2\x20\x8A\x41\x01";


export void RetrieveConditionZeroFn() noexcept
{
	g_pfnCSCZCBaseCreate = (fnCBaseCreate_t)UTIL_SearchPattern("mp.dll", 1, CSCZ_CBASE_CREATE_FN_ANNIV_PATTERN);
	g_pfnSUB_UseTargets = (fnSUB_UseTargets_t)UTIL_SearchPattern("mp.dll", 1, CSCZ_SUB_USE_TARGETS_FN_ANNIV_PATTERN);
	g_pfnEmptyEntityHashTable = (fnEmptyEntityHashTable_t)UTIL_SearchPattern("mp.dll", 1, EMPTY_ENT_HASH_TABLE_FN_ANNIV_PATTERN);
	g_pfnAddEntityHashValue = (fnAddEntityHashValue_t)UTIL_SearchPattern("mp.dll", 1, ADD_ENT_HASH_TABLE_FN_ANNIV_PATTERN);
	g_pfnRemoveEntityHashValue = (fnRemoveEntityHashValue_t)UTIL_SearchPattern("mp.dll", 1, REMOVE_ENT_HASH_TABLE_FN_ANNIV_PATTERN);

	auto const pItemInfo = UTIL_RetrieveGlobalVariable<ItemInfo>(
		UTIL_SearchPattern("mp.dll", 1, PRECACHE_OTHER_WPN_FN_ANNIV_PATTERN),
		ITEM_INFO_ARRAY_OFS
	);

	auto const pAmmoInfo = UTIL_RetrieveGlobalVariable<AmmoInfo>(
		UTIL_SearchPattern("mp.dll", 1, ADD_AMMO_REG_FN_ANNIV_PATTERN),
		AMMO_INFO_ARRAY_OFS
	);

	gpiAmmoIndex = UTIL_RetrieveGlobalVariable<int32_t>(
		UTIL_SearchPattern("mp.dll", 1, ADD_AMMO_REG_FN_ANNIV_PATTERN),
		GI_AMMO_INDEX_OFS
	);

	CBasePlayerItem::ItemInfoArray = g_rgItemInfo = std::span{ pItemInfo, MAX_WEAPONS };
	CBasePlayerItem::AmmoInfoArray = g_rgAmmoInfo = std::span{ pAmmoInfo, MAX_AMMO_SLOTS };

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

CBaseEntity* CBaseEntity::Create(char const* szName, const Vector& vecOrigin, const Angles& vecAngles, edict_t* pentOwner) noexcept
{
	return g_pfnCSCZCBaseCreate(szName, vecOrigin, vecAngles, pentOwner);
}

////////////////
// CBaseDelay //
////////////////

void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value) noexcept
{
	g_pfnSUB_UseTargets(this, pActivator, useType, value);
}

/////////////////////
// CBasePlayerItem //
/////////////////////

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
		g_engfuncs.pfnEmitSound(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	SUB_UseTargets(pOther, USE_TOGGLE, 0);
}

void CBasePlayerItem::Materialize() noexcept
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		if (g_pGameRules->IsMultiplayer())
		{
			g_engfuncs.pfnEmitSound(edict(), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);
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

///////////////////////
// CBasePlayerWeapon //
///////////////////////

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
			g_engfuncs.pfnEmitSound(edict(), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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
		g_engfuncs.pfnEmitSound(edict(), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
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
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.05, 8192, 1, BULLET_PLAYER_9MM, 18, 0.9, m_pPlayer->pev, true, m_pPlayer->random_seed);
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
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, 30, 0.96, m_pPlayer->pev, false, m_pPlayer->random_seed);
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

/////////////////
// CBasePlayer //
/////////////////

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
