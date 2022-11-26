import <array>;
import <format>;
import <numbers>;

import meta_api;

import Hook;
import Localization;
import Menu;
import Missile;
import Resources;
import Target;
import Task;
import Weapon;

import UtlHook;
import UtlRandom;

using std::array;

import Effects;

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

qboolean __fastcall HamF_Item_AddToPlayer(CBasePlayerItem *pThis, int, CBasePlayer *pPlayer) noexcept
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

qboolean __fastcall HamF_Item_Deploy(CBasePlayerItem *pItem, int) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;	// The actual class of this one is ... CKnife, but anyway.

	pThis->m_iSwing = 0;
	pThis->m_fMaxSpeed = KNIFE_MAX_SPEED;

	pThis->m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	pThis->m_pPlayer->m_bShieldDrawn = false;

	if (pThis->pev->weapons == RADIO_KEY)
	{
		TaskScheduler::Enroll(Weapon::Task_RadioDeploy(pThis));
		return true;
	}

	g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pThis->pev), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3f, 2.4f, 0, PITCH_NORM);

	if (pThis->m_pPlayer->m_bOwnsShield)
		return g_pfnDefaultDeploy(pThis, "models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", pThis->UseDecrement() != 0);
	else
		return g_pfnDefaultDeploy(pThis, "models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", pThis->UseDecrement() != 0);
}

void __fastcall HamF_Item_PostFrame(CBasePlayerItem *pItem, int) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;

	if (pThis->pev->weapons != RADIO_KEY || !pThis->m_pPlayer || !pThis->m_pPlayer->IsAlive())
		return g_pfnItemPostFrame(pThis);

	if (pThis->m_pPlayer->m_afButtonPressed & IN_ATTACK) [[unlikely]]
	{
		if (g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()] != CARPET_BOMBARDMENT)
		{
			if (DYN_TARGET(pThis)->v.skin != Models::targetmdl::SKIN_GREEN)
				TaskScheduler::Enroll(Weapon::Task_RadioRejected(pThis));
			else
				TaskScheduler::Enroll(Weapon::Task_RadioAccepted(pThis));
		}
		else
		{
			auto const pTarget = (CDynamicTarget *)DYN_TARGET(pThis)->pvPrivateData;
			pTarget->EnableBeacons();
		}
	}
	else if (pThis->m_pPlayer->m_afButtonReleased & IN_ATTACK) [[unlikely]]
	{
		if (g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()] == CARPET_BOMBARDMENT)
		{
			if (DYN_TARGET(pThis)->v.skin != Models::targetmdl::SKIN_GREEN)
			{
				TaskScheduler::Enroll(Weapon::Task_RadioRejected(pThis));

				auto const pTarget = (CDynamicTarget *)DYN_TARGET(pThis)->pvPrivateData;
				pTarget->DisableBeacons();
			}
			else
				TaskScheduler::Enroll(Weapon::Task_RadioAccepted(pThis));
		}
	}
	else if (pThis->m_pPlayer->m_afButtonPressed & IN_ATTACK2) [[unlikely]]
	{
		auto const &iIndex = g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()];

		UTIL_ShowMenu(
			pThis->m_pPlayer->edict(),
			Menu::Key::AIRSUPPORT,
			std::format(Menu::Text::AIRSUPPORT_TEMPLATE,
				iIndex == AIR_STRIKE ? "\\d" : "\\w", iIndex == AIR_STRIKE ? " - Selected" : "",
				iIndex == CLUSTER_BOMB ? "\\d" : "\\w", iIndex == CLUSTER_BOMB ? " - Selected" : "",
				iIndex == CARPET_BOMBARDMENT ? "\\d" : "\\w", iIndex == CARPET_BOMBARDMENT ? " - Selected" : "",
				iIndex == GUNSHIP_STRIKE ? "\\d" : "\\w", iIndex == GUNSHIP_STRIKE ? " - Selected" : "",
				iIndex == FUEL_AIR_BOMB ? "\\d" : "\\r", iIndex == FUEL_AIR_BOMB ? " - Selected" : ""
			)
		);

		pThis->m_pPlayer->m_iMenu = EMenu::Menu_AirSupport;
	}
	else if (pThis->m_pPlayer->m_afButtonPressed & IN_USE) [[unlikely]]
	{
		g_engfuncs.pfnMakeVectors(pThis->m_pPlayer->pev->v_angle);

		UTIL_ExplodeModel(pThis->m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 256, UTIL_Random() ? -200.f : 200.f, Models::m_rgLibrary[Models::GIBS_CONCRETE], 5, 3.f);
	}
}

void __fastcall HamF_Weapon_PrimaryAttack(CBasePlayerWeapon *pThis, int) noexcept { return g_pfnWeaponPrimaryAttack(pThis); }
void __fastcall HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis, int) noexcept { return g_pfnWeaponSecondaryAttack(pThis); }

qboolean __fastcall HamF_Item_CanHolster(CBasePlayerItem *pThis, int) noexcept
{
	return g_pfnItemCanHolster(pThis) & pThis->has_disconnected;
}

void __fastcall HamF_Item_Holster(CBasePlayerItem *pThis, int, int skiplocal) noexcept
{
	if (pThis->pev->weapons == RADIO_KEY)
		Weapon::OnRadioHolster((CBasePlayerWeapon *)pThis);

	g_pfnItemHolster(pThis, skiplocal);
}

extern "C++" namespace Weapon
{
#define RESUME_CHECK	\
	if (!pThis || pThis->m_pPlayer->m_pActiveItem != pThis || pThis->pev->weapons != RADIO_KEY)	\
		co_return

	Task Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept
	{
		// Remove shield protection for now.
		if (pThis->m_pPlayer->m_bOwnsShield)
			pThis->m_pPlayer->pev->gamestate = 1;

		pThis->pev->weapons = RADIO_KEY;
		pThis->m_pPlayer->m_iHideHUD |= HIDEHUD_CROSSHAIR;
		pThis->has_disconnected = true;	// BORROWED MEMBER: allow holster.

		g_pfnDefaultDeploy(pThis, Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw, "knife", false);	// Enforce to play the anim.

		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::draw;
		pThis->m_flNextPrimaryAttack = std::numeric_limits<float>::max();
		pThis->m_flNextSecondaryAttack = std::numeric_limits<float>::max();
		pThis->m_flTimeWeaponIdle = std::numeric_limits<float>::max();

		co_await Models::v_radio::time::draw;
		RESUME_CHECK;

		pThis->SendWeaponAnim((int)Models::v_radio::seq::idle, false);

		[[unlikely]]
		if (DYN_TARGET(pThis))
			DYN_TARGET(pThis)->v.flags |= FL_KILLME;

		DYN_TARGET(pThis) = CDynamicTarget::Create(pThis->m_pPlayer, pThis)->edict();
	}

	Task Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept
	{
		pThis->has_disconnected = false;	// BORROWED MEMBER: forbid holster.
		pThis->SendWeaponAnim((int)Models::v_radio::seq::use);
		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::use;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_AUTO, Sounds::NOISE, VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));

		static constexpr float TIME_PRESS_TALK = 19.f / 45.f;
		co_await TIME_PRESS_TALK;
		RESUME_CHECK;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_VOICE, Sounds::REQUESTING, 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

		static constexpr float TIME_REQUESTING = 1.2f;
		co_await TIME_REQUESTING;
		RESUME_CHECK;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_VOICE, Sounds::REJECTING, 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));
		gmsgTextMsg::FreeBegin<MSG_ONE>(Vector::Zero(), pThis->m_pPlayer->edict(), (byte)4, Localization::REJECT_COVERED_LOCATION);


		static_assert(Models::v_radio::time::use - TIME_PRESS_TALK - TIME_REQUESTING > 0);
		co_await (Models::v_radio::time::use - TIME_PRESS_TALK - TIME_REQUESTING);
		RESUME_CHECK;

		pThis->SendWeaponAnim((int)Models::v_radio::seq::idle, false);
		pThis->has_disconnected = true;	// BORROWED MEMBER: allow holster.
	}

	Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept
	{
		EHANDLE<CDynamicTarget> pTarget = DYN_TARGET(pThis);
		EHANDLE<CFixedTarget> pFixedTarget = CFixedTarget::Create(pTarget);

		DYN_TARGET(pThis) = nullptr;
		pTarget->pev->flags |= FL_KILLME;

		pThis->has_disconnected = false;	// BORROWED MEMBER: forbid holster.
		pThis->SendWeaponAnim((int)Models::v_radio::seq::use);
		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::use;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_AUTO, Sounds::NOISE, VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));

		static constexpr float TIME_PRESS_TALK = 19.f / 45.f;
		co_await TIME_PRESS_TALK;
		RESUME_CHECK;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_VOICE, Sounds::REQUESTING, 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

		static constexpr float TIME_REQUESTING = 1.2f;
		co_await TIME_REQUESTING;
		RESUME_CHECK;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_VOICE, Sounds::RADIO[AIR_STRIKE], 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

		static constexpr float TIME_RESPOUNDING = 1.f;
		co_await TIME_RESPOUNDING;
		RESUME_CHECK;

		[[likely]]
		if (pFixedTarget)
			pFixedTarget->Activate();
		else
			co_return;	// Round ended or something???

		static_assert(Models::v_radio::time::use - TIME_PRESS_TALK - TIME_REQUESTING - TIME_RESPOUNDING > 0);
		co_await (Models::v_radio::time::use - TIME_PRESS_TALK - TIME_REQUESTING - TIME_RESPOUNDING);
		RESUME_CHECK;

		pThis->SendWeaponAnim((int)Models::v_radio::seq::holster);
		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::holster;

		co_await Models::v_radio::time::holster;
		RESUME_CHECK;

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

	void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept
	{
		pThis->m_flNextPrimaryAttack = 0;	// Because we use a essentially infinity at the draw.
		pThis->m_flNextSecondaryAttack = 0;
		pThis->m_flTimeWeaponIdle = 0;

		pThis->pev->weapons = 0;
		pThis->m_pPlayer->m_iHideHUD &= ~HIDEHUD_CROSSHAIR;	// #TODO use CurWeapon to fix crosshair?

		[[likely]]
		if (DYN_TARGET(pThis))
		{
			DYN_TARGET(pThis)->v.flags |= FL_KILLME;
			DYN_TARGET(pThis) = nullptr;
		}

		// Resume shield protection
		if (pThis->m_pPlayer->m_bOwnsShield)
			pThis->m_pPlayer->pev->gamestate = 0;
	}
};

qboolean SwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) noexcept
{
	if (pPlayer->m_pActiveItem && !pPlayer->m_pActiveItem->CanHolster())
		return false;

	UTIL_UndoPatch(g_pfnSwitchWeapon, HookInfo::SwitchWeapon.m_OriginalBytes);
	auto const ret = g_pfnSwitchWeapon(pPlayer, pWeapon);
	UTIL_DoPatch(g_pfnSwitchWeapon, HookInfo::SwitchWeapon.m_PatchedBytes);
	return ret;
}

void SelectItem(CBasePlayer *pPlayer, const char *pstr) noexcept
{
	if (pPlayer->m_pActiveItem && !pPlayer->m_pActiveItem->CanHolster())
		return;

	UTIL_UndoPatch(g_pfnSelectItem, HookInfo::SelectItem.m_OriginalBytes);
	g_pfnSelectItem(pPlayer, pstr);
	UTIL_DoPatch(g_pfnSelectItem, HookInfo::SelectItem.m_PatchedBytes);
}
