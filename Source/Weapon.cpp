#include <fmt/core.h>

import <array>;
import <numbers>;

import meta_api;

import Beam;
import Entity;
import Hook;
import Resources;
import Task;

import UtlRandom;

using std::array;

enum EWeaponState
{
	WPNSTATE_USP_SILENCED = (1 << 0),
	WPNSTATE_GLOCK18_BURST_MODE = (1 << 1),
	WPNSTATE_M4A1_SILENCED = (1 << 2),
	WPNSTATE_ELITE_LEFT = (1 << 3),
	WPNSTATE_FAMAS_BURST_MODE = (1 << 4),
	WPNSTATE_SHIELD_DRAWN = (1 << 5),
};

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

inline constexpr float KNIFE_MAX_SPEED = 250.0f;
inline constexpr float KNIFE_MAX_SPEED_SHIELD = 180.0f;

int HamF_Item_AddToPlayer(CBasePlayerItem *pThis, CBasePlayer *pPlayer) noexcept
{
	g_pfnItemAddToPlayer(pThis, pPlayer);

	if (pPlayer->IsBot())
		return true;

	gmsgWeaponList::Send(ent_cast<edict_t *>(pPlayer->pev),
		HUD::RADIO,
		(byte)-1,
		(byte)-1,
		(byte)-1,
		(byte)-1,
		4,	// slot
		2,	// pos
		WEAPON_NIL,
		0
	);

	pPlayer->pev->weapons |= (1 << WEAPON_NIL);

	gmsgWeapPickup::Send(pPlayer->pev->pContainingEntity, WEAPON_NIL);

	return true;
}

int HamF_Item_Deploy(CBasePlayerItem *pItem) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;	// The actual class of this one is ... CKnife, but anyway.

	[[unlikely]]
	if (pev_valid(pThis->pev->euser1) != 2)
		Target::Create(pThis);

	pThis->m_iSwing = 0;
	pThis->m_fMaxSpeed = KNIFE_MAX_SPEED;

	pThis->m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	pThis->m_pPlayer->m_bShieldDrawn = false;

	if (pThis->pev->weapons == RADIO_KEY)
	{
		TimedFnMgr::Enroll(Weapon::Task_RadioDeploy(pThis));
		return true;
	}

	g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pThis->pev), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3f, 2.4f, 0, PITCH_NORM);

	if (pThis->m_pPlayer->m_bOwnsShield)
		return g_pfnDefaultDeploy(pThis, "models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", pThis->UseDecrement() != 0);
	else
		return g_pfnDefaultDeploy(pThis, "models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", pThis->UseDecrement() != 0);

	// #UNDONE clear AS_CARPET_BOMBING src origin
}

void HamF_Item_PostFrame(CBasePlayerItem *pItem) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;

	if (pThis->pev->weapons != RADIO_KEY || !pThis->m_pPlayer || !pThis->m_pPlayer->IsAlive())
		return g_pfnItemPostFrame(pThis);

	[[unlikely]]
	if (pThis->m_pPlayer->m_afButtonPressed & IN_ATTACK)
	{
		g_engfuncs.pfnMakeVectors(pThis->m_pPlayer->pev->v_angle);

		Vector vecSrc = pThis->m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, ent_cast<edict_t *>(pThis->m_pPlayer->pev), &tr);

		vecSrc = tr.vecEndPos;
		vecEnd = Vector(vecSrc.x, vecSrc.y, 8192.f);
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, tr.pHit, &tr);

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
		{
			g_engfuncs.pfnClientPrintf(pThis->m_pPlayer->edict(), print_center, "You must target an outdoor location.");
			return;
		}

		TimedFnMgr::Enroll(Weapon::Task_RadioUse(pThis));
	}
}

void HamF_Weapon_PrimaryAttack(CBasePlayerWeapon *pThis) noexcept { return g_pfnWeaponPrimaryAttack(pThis); }
void HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis) noexcept { return g_pfnWeaponSecondaryAttack(pThis); }

qboolean HamF_Item_CanHolster(CBasePlayerItem *pThis) noexcept
{
	return g_pfnItemCanHolster(pThis) & pThis->has_disconnected;
}

void HamF_Item_Holster(CBasePlayerItem *pThis, int skiplocal) noexcept
{
	Weapon::OnRadioHolster(pThis);
	g_pfnItemHolster(pThis, skiplocal);
}

extern "C++" namespace Weapon
{
	TimedFn Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept
	{
		pThis->pev->weapons = RADIO_KEY;
		pThis->m_pPlayer->m_iHideHUD |= HIDEHUD_CROSSHAIR;
		pThis->has_disconnected = true;	// BORROWED MEMBER: allow holster.

		g_pfnDefaultDeploy(pThis, Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw, "knife", false);	// Enforce to play the anim.

		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::draw;
		pThis->m_flNextPrimaryAttack = Models::v_radio::time::draw;
		pThis->m_flNextSecondaryAttack = Models::v_radio::time::draw;
		pThis->m_flTimeWeaponIdle = Models::v_radio::time::draw;
		co_await Models::v_radio::time::draw;

		if (!pThis || pThis->m_pPlayer->m_pActiveItem != pThis || pThis->pev->weapons != RADIO_KEY)
			co_return;

		pThis->SendWeaponAnim((int)Models::v_radio::seq::idle, false);
		pThis->pev->euser1->v.effects &= ~EF_NODRAW;
	}

	TimedFn Task_RadioUse(EHANDLE<CBasePlayerWeapon> pThis) noexcept
	{
		auto const pTarget = (CBaseEntity *)pThis->pev->euser1->pvPrivateData;
		pTarget->pev->effects |= EF_NODRAW;
		FixedTarget::Create(pTarget->pev->origin, pTarget->pev->angles, pThis->m_pPlayer->edict());

		pThis->has_disconnected = false;	// BORROWED MEMBER: forbid holster.
		pThis->SendWeaponAnim((int)Models::v_radio::seq::use);
		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::use;
		pThis->m_flNextPrimaryAttack = Models::v_radio::time::use;
		pThis->m_flNextSecondaryAttack = Models::v_radio::time::use;
		pThis->m_flTimeWeaponIdle = Models::v_radio::time::use;
		co_await Models::v_radio::time::use;

		if (!pThis || pThis->m_pPlayer->m_pActiveItem != pThis || pThis->pev->weapons != RADIO_KEY)
			co_return;

		pThis->SendWeaponAnim((int)Models::v_radio::seq::holster);
		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::holster;
		pThis->m_flNextPrimaryAttack = Models::v_radio::time::holster;
		pThis->m_flNextSecondaryAttack = Models::v_radio::time::holster;
		pThis->m_flTimeWeaponIdle = Models::v_radio::time::holster;
		co_await Models::v_radio::time::holster;

		if (!pThis || pThis->m_pPlayer->m_pActiveItem != pThis || pThis->pev->weapons != RADIO_KEY)
			co_return;

		for (auto &&pItem : pThis->m_pPlayer->m_rgpPlayerItems)
		{
			if (!pItem || pev_valid(pItem->pev) != 2)
				continue;

			pThis->has_disconnected = true;	// BORROWED MEMBER: allow holster.

			if (pItem == pThis)	// this guy only got his knife.
			{
				// Clear radio events
				Weapon::OnRadioHolster(pThis);

				// Take out knife.
				g_pfnItemDeploy(pThis);
			}
			else
			{
				// By cl_command is actually better for net game.
				//g_pfnSwitchWeapon(pThis->m_pPlayer, pItem);
				g_engfuncs.pfnClientCommand(pThis->m_pPlayer->edict(), "%s\n", STRING(pItem->pev->classname));
			}

			break;
		}
	}

	void OnRadioHolster(CBasePlayerItem *pThis) noexcept
	{
		pThis->pev->weapons = 0;
		pThis->m_pPlayer->m_iHideHUD &= ~HIDEHUD_CROSSHAIR;	// #TODO use CurWeapon to fix crosshair?

		[[likely]]
		if (pThis->pev->euser1)
			pThis->pev->euser1->v.effects |= EF_NODRAW;
	}
};
