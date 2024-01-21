import <cassert>;

import <array>;
import <format>;
import <numbers>;

import meta_api;

import DamageOverTime;	// optional because it's in debug.
import Hook;
import Jet;
import Localization;
import Math;	// optional because it's in debug.
import Menu;
import Projectile;
import Query;
import Resources;
import Target;
import Task;
import Uranus;
import Weapon;

import UtlHook;
import UtlRandom;

using std::array;

import Effects;

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
/*
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
		TaskScheduler::Enroll(Weapon::Task_RadioDeploy(pThis), UTIL_CombineTaskAndPlayerIndex(TASK_RADIO_DEPLOY, pThis->m_pPlayer->entindex()));
		return true;
	}

	g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pThis->pev), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3f, 2.4f, 0, PITCH_NORM);

	if (pThis->m_pPlayer->m_bOwnsShield)
		return gUranusCollection.pfnDefaultDeploy(pThis, "models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", pThis->UseDecrement() != 0);
	else
		return gUranusCollection.pfnDefaultDeploy(pThis, "models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", pThis->UseDecrement() != 0);
}

void __fastcall HamF_Item_PostFrame(CBasePlayerItem *pItem, int) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;

	if (pThis->pev->weapons != RADIO_KEY || !pThis->m_pPlayer || !pThis->m_pPlayer->IsAlive())
		return g_pfnItemPostFrame(pThis);

	if (pThis->m_pPlayer->m_afButtonPressed & IN_ATTACK) [[unlikely]]
	{
		uint64_t const iPlayer = pThis->m_pPlayer->entindex();

		switch (g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()])
		{
		case CARPET_BOMBARDMENT:
		{
			auto const pTarget = (CDynamicTarget *)DYN_TARGET(pThis)->pvPrivateData;
			pTarget->EnableBeacons();
			break;
		}

		case GUNSHIP_STRIKE:
			if (CGunship::s_bInstanceExists)
			{
				gmsgTextMsg::Send(pThis->m_pPlayer->edict(), 4, Localization::GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE);
				return;
			}

			[[fallthrough]];

		default:
			auto const bAccepted = (DYN_TARGET(pThis)->v.skin == Models::targetmdl::SKIN_GREEN);

			TaskScheduler::Enroll(
				bAccepted ? Weapon::Task_RadioAccepted(pThis) : Weapon::Task_RadioRejected(pThis),
				UTIL_CombineTaskAndPlayerIndex(bAccepted ? TASK_RADIO_ACCEPTED : TASK_RADIO_REJECTED, iPlayer)
			);
			break;
		}
	}
	else if (pThis->m_pPlayer->m_afButtonReleased & IN_ATTACK) [[unlikely]]
	{
		uint64_t const iPlayer = pThis->m_pPlayer->entindex();

		switch (g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()])
		{
		case CARPET_BOMBARDMENT:
		{
			if (DYN_TARGET(pThis)->v.skin != Models::targetmdl::SKIN_GREEN)
			{
				TaskScheduler::Enroll(Weapon::Task_RadioRejected(pThis), UTIL_CombineTaskAndPlayerIndex(TASK_RADIO_REJECTED, iPlayer));

				auto const pTarget = (CDynamicTarget *)DYN_TARGET(pThis)->pvPrivateData;
				pTarget->DisableBeacons();
			}
			else
				TaskScheduler::Enroll(Weapon::Task_RadioAccepted(pThis), UTIL_CombineTaskAndPlayerIndex(TASK_RADIO_ACCEPTED, iPlayer));
		}
			break;

		default:
			break;
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
				iIndex == FUEL_AIR_BOMB ? "\\d" : "\\w", iIndex == FUEL_AIR_BOMB ? " - Selected" : "",
				iIndex == PHOSPHORUS_MUNITION ? "\\d" : "\\w", iIndex == PHOSPHORUS_MUNITION ? " - Selected" : ""
			)
		);

		pThis->m_pPlayer->m_iMenu = EMenu::Menu_AirSupport;
	}

#ifdef _DEBUG
	else if (pThis->m_pPlayer->m_afButtonPressed & IN_USE) [[unlikely]]
	{
		g_engfuncs.pfnMakeVectors(pThis->m_pPlayer->pev->v_angle);

		[[maybe_unused]] TraceResult tr{}, tr2{};
		auto const vecSrc = pThis->m_pPlayer->GetGunPosition();
		auto const vecEnd = vecSrc + gpGlobals->v_forward * 4096.0;
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, pThis->m_pPlayer->edict(), &tr);
		g_engfuncs.pfnTraceLine(pThis->m_pPlayer->pev->origin, pThis->m_pPlayer->pev->origin + Vector(0, 0, 4096), ignore_monsters, pThis->m_pPlayer->edict(), &tr2);

		if (EHANDLE<CBaseEntity> pHit{tr.pHit}; pHit && pHit->IsPlayer())
			Burning::ByPhosphorus(pHit.As<CBasePlayer>(), pThis->m_pPlayer);
	}
#endif
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

		gUranusCollection.pfnDefaultDeploy(pThis, Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw, "knife", false);	// Enforce to play the anim.

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

		co_await Sounds::Length::Radio::REQUESTING;
		RESUME_CHECK;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_VOICE, UTIL_GetRandomOne(Sounds::REJECTING), 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));
		gmsgTextMsg::Send(pThis->m_pPlayer->edict(), 4, Localization::REJECT_COVERED_LOCATION);

		static_assert(Models::v_radio::time::use - TIME_PRESS_TALK - Sounds::Length::Radio::REQUESTING > 0);
		co_await (Models::v_radio::time::use - TIME_PRESS_TALK - Sounds::Length::Radio::REQUESTING);
		RESUME_CHECK;

		pThis->SendWeaponAnim((int)Models::v_radio::seq::idle, false);
		pThis->has_disconnected = true;	// BORROWED MEMBER: allow holster.
	}

	Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept
	{
		EHANDLE<CDynamicTarget> pTarget = DYN_TARGET(pThis);
		EHANDLE<CFixedTarget> pFixedTarget = CFixedTarget::Create(pTarget);
		auto const iTaskIndex = UTIL_CombineTaskAndPlayerIndex(TASK_RADIO_ACCEPTED, pThis->m_pPlayer->entindex());

		DYN_TARGET(pThis) = nullptr;
		pTarget->pev->flags |= FL_KILLME;

		pThis->has_disconnected = false;	// BORROWED MEMBER: forbid holster.
		pThis->SendWeaponAnim((int)Models::v_radio::seq::use);
		pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::use;

		g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_STATIC, Sounds::NOISE, VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));

		static constexpr auto SubRoutine_Sound = [](EHANDLE<CBasePlayerWeapon> pThis, EHANDLE<CFixedTarget> pFixedTarget) noexcept -> Task
		{
			auto const iAirsupportType = g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()];

			static constexpr float TIME_PRESS_TALK = 19.f / 45.f;
			co_await TIME_PRESS_TALK;
			RESUME_CHECK;

			g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_STATIC, Sounds::REQUESTING, 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

			co_await Sounds::Length::Radio::REQUESTING;
			RESUME_CHECK;

			g_engfuncs.pfnEmitSound(pThis.Get(), CHAN_STATIC, Sounds::ACCEPTING[iAirsupportType], 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

			co_await Sounds::Length::Radio::ACCEPTING[iAirsupportType];
			RESUME_CHECK;

			[[likely]]
			if (pFixedTarget)
				pFixedTarget->Activate();
			else
				co_return;	// Round ended or something???
		};

		static constexpr auto SubRoutine_Animation = [](EHANDLE<CBasePlayerWeapon> pThis) noexcept -> Task
		{
			co_await Models::v_radio::time::use;
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
		};

		TaskScheduler::Enroll(SubRoutine_Animation(pThis), iTaskIndex);
		TaskScheduler::Enroll(SubRoutine_Sound(pThis, pFixedTarget), iTaskIndex);

		co_return;
	}

	void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept
	{
		pThis->m_flNextPrimaryAttack = 0;	// Because we use a essentially infinity at the draw.
		pThis->m_flNextSecondaryAttack = 0;
		pThis->m_flTimeWeaponIdle = 0;

		pThis->pev->weapons = 0;
		pThis->m_pPlayer->m_iHideHUD &= ~HIDEHUD_CROSSHAIR;	// #GIVEN_UP use CurWeapon to fix crosshair?

		[[likely]]
		if (DYN_TARGET(pThis))
		{
			DYN_TARGET(pThis)->v.flags |= FL_KILLME;
			DYN_TARGET(pThis) = nullptr;
		}

		if (TaskScheduler::Delist(UTIL_CombineTaskAndPlayerIndex(TASK_RADIO_ACCEPTED, pThis->m_pPlayer->entindex())) > 0)
		{
			for (CFixedTarget *pFixedTarget :
				FIND_ENTITY_BY_CLASSNAME(CFixedTarget::CLASSNAME) | Query::as<CFixedTarget>() |
				std::views::filter([&](CFixedTarget *p) noexcept { return p->m_pPlayer == pThis->m_pPlayer; }) |	// Called by this player
				std::views::filter([&](CFixedTarget *p) noexcept { return !p->m_Scheduler.Exist(TASK_ACTION); })	// Not yet activated.
				)
			{
				pFixedTarget->pev->flags |= FL_KILLME;
				gmsgTextMsg::Send(pThis->m_pPlayer->edict(), 4, Localization::REJECT_TIME_OUT);
			}
		}

		// Resume shield protection
		if (pThis->m_pPlayer->m_bOwnsShield)
			pThis->m_pPlayer->pev->gamestate = 0;
	}
};
*/
void __fastcall OrpheuF_FireBullets(CBaseEntity *pThis, int edx, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) noexcept
{
	g_bIsSomeoneShooting = true;
	HookInfo::FireBullets(pThis, edx, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage, pevAttacker);
	//UTIL_UndoPatch(g_pfnFireBullets, HookInfo::FireBullets.m_OriginalBytes);
	//g_pfnFireBullets(pThis, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage, pevAttacker);
	//UTIL_DoPatch(g_pfnFireBullets, HookInfo::FireBullets.m_PatchedBytes);
	g_bIsSomeoneShooting = false;
}

Vector *__fastcall OrpheuF_FireBullets3(CBaseEntity* pThis, void* edx, Vector* pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) noexcept
{
	// LUNA: for any C++ function, the return value was already prepared at caller side,
	// with its value passed into the callee as if it were an argument.
	// for example:
	// Vector fn(void) => Vector* fn(Vector* pret)
	// where the return value and the pret argument have exactly same address.
	// edx register must be pass on, or it might invokes content loss.

	g_bIsSomeoneShooting = true;
	auto const ret = HookInfo::FireBullets3(pThis, edx, pret, vecSrc, vecDirShooting, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, pevAttacker, bPistol, shared_rand);
	//UTIL_UndoPatch(gUranusCollection.pfnFireBullets3, HookInfo::FireBullets3.m_OriginalBytes);
	//auto const ret = gUranusCollection.pfnFireBullets3(pThis, edx, pret, vecSrc, vecDirShooting, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, pevAttacker, bPistol, shared_rand);
	//UTIL_DoPatch(gUranusCollection.pfnFireBullets3, HookInfo::FireBullets3.m_PatchedBytes);
	g_bIsSomeoneShooting = false;
	return ret;
}

void __cdecl OrpheuF_W_Precache() noexcept
{
	HookInfo::W_Precache();

	gUranusCollection.pfnUTIL_PrecacheOtherWeapon(CRadio::CLASSNAME);
}

void CRadio::Spawn() noexcept
{
	pev->effects |= EF_NODRAW;

	m_iId = WEAPON_NIL;
	m_fMaxSpeed = 250.f;
}

qboolean CRadio::GetItemInfo(ItemInfo* p) noexcept
{
	p->pszName = &CLASSNAME[0];
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 2;
	p->iId = WEAPON_NIL;
	p->iFlags = 0;
	p->iWeight = 5;	// c4 == 3, knife == 0

	return true;
}

qboolean CRadio::Deploy() noexcept
{
	if (!CanDeploy())
		return false;

	m_Scheduler.Enroll(Task_Deploy(), TASK_RADIO_DEPLOY);
	return true;
}

void CRadio::ItemPostFrame() noexcept
{
	if (m_pPlayer->m_afButtonPressed & IN_ATTACK) [[unlikely]]
	{
		switch (g_rgiAirSupportSelected[m_pPlayer->entindex()])
		{
		case CARPET_BOMBARDMENT:
		{
			m_pTarget->EnableBeacons();
			break;
		}

		case GUNSHIP_STRIKE:
			if (CGunship::s_bInstanceExists)
			{
				gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE);
				return;
			}

			[[fallthrough]];

		default:
			auto const bAccepted = (m_pTarget->pev->skin == Models::targetmdl::SKIN_GREEN);

			m_Scheduler.Enroll(
				bAccepted ? Task_RadioAccepted() : Task_RadioRejected(),
				bAccepted ? TASK_RADIO_ACCEPTED : TASK_RADIO_REJECTED
			);
			break;
		}
	}
	else if (m_pPlayer->m_afButtonReleased & IN_ATTACK) [[unlikely]]
	{
		switch (g_rgiAirSupportSelected[m_pPlayer->entindex()])
		{
		case CARPET_BOMBARDMENT:
		{
			if (m_pTarget->pev->skin != Models::targetmdl::SKIN_GREEN)
			{
				m_Scheduler.Enroll(Task_RadioRejected(), TASK_RADIO_REJECTED);
				m_pTarget->DisableBeacons();
			}
			else
				m_Scheduler.Enroll(Task_RadioAccepted(), TASK_RADIO_ACCEPTED);
		}
			break;

		default:
			break;
		}
	}
	else if (m_pPlayer->m_afButtonPressed & IN_ATTACK2) [[unlikely]]
	{
		auto const &iIndex = g_rgiAirSupportSelected[m_pPlayer->entindex()];

		UTIL_ShowMenu(
			m_pPlayer->edict(),
			Menu::Key::AIRSUPPORT,
			std::format(Menu::Text::AIRSUPPORT_TEMPLATE,
				iIndex == AIR_STRIKE ? "\\d" : "\\w", iIndex == AIR_STRIKE ? " - Selected" : "",
				iIndex == CLUSTER_BOMB ? "\\d" : "\\w", iIndex == CLUSTER_BOMB ? " - Selected" : "",
				iIndex == CARPET_BOMBARDMENT ? "\\d" : "\\w", iIndex == CARPET_BOMBARDMENT ? " - Selected" : "",
				iIndex == GUNSHIP_STRIKE ? "\\d" : "\\w", iIndex == GUNSHIP_STRIKE ? " - Selected" : "",
				iIndex == FUEL_AIR_BOMB ? "\\d" : "\\w", iIndex == FUEL_AIR_BOMB ? " - Selected" : "",
				iIndex == PHOSPHORUS_MUNITION ? "\\d" : "\\w", iIndex == PHOSPHORUS_MUNITION ? " - Selected" : ""
			)
		);

		m_pPlayer->m_iMenu = EMenu::Menu_AirSupport;
	}

#ifdef _DEBUG
	else if (m_pPlayer->m_afButtonPressed & IN_USE) [[unlikely]]
	{
		g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

		[[maybe_unused]] TraceResult tr{}, tr2{};
		auto const vecSrc = m_pPlayer->GetGunPosition();
		auto const vecEnd = vecSrc + gpGlobals->v_forward * 4096.0;
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);
		g_engfuncs.pfnTraceLine(m_pPlayer->pev->origin, m_pPlayer->pev->origin + Vector(0, 0, 4096), ignore_monsters, m_pPlayer->edict(), &tr2);

		if (EHANDLE<CBaseEntity> pHit{tr.pHit}; pHit && pHit->IsPlayer())
			Burning::ByPhosphorus(pHit.As<CBasePlayer>(), m_pPlayer);
	}
#endif
}

void CRadio::Holster(int param) noexcept
{
	m_pPlayer->m_iHideHUD &= ~HIDEHUD_CROSSHAIR;

	[[likely]]
	if (m_pTarget)
	{
		m_pTarget->pev->flags |= FL_KILLME;
		m_pTarget = nullptr;
	}

	if (m_Scheduler.Delist(TASK_RADIO_ACCEPTED) > 0)
	{
		for (CFixedTarget* pFixedTarget :
			Query::all_instances_of<CFixedTarget>()
			| std::views::filter([&](CFixedTarget* p) noexcept { return p->m_pPlayer == m_pPlayer; })	// Called by this player
			| std::views::filter([&](CFixedTarget* p) noexcept { return !p->m_Scheduler.Exist(TASK_ACTION); })	// Not yet activated.
			)
		{
			pFixedTarget->pev->flags |= FL_KILLME;
			gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::REJECT_TIME_OUT);
		}
	}

	if (m_Scheduler.Delist(TASK_RADIO_DEPLOY) > 0)
	{
		// not sure what to do here... a party maybe?
	}

	// Resume shield protection
	if (m_pPlayer->m_bOwnsShield)
		m_pPlayer->pev->gamestate = 0;

	m_pPlayer->m_flNextAttack = -1;
	assert(m_Scheduler.m_List.size() == 0);
	__super::Holster(param);
}

Task CRadio::Task_Deploy() noexcept
{
	m_iMockedWeapon = WEAPON_KNIFE;	// because that doesn't have any ammo ui etc.
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->m_bOwnsShield)
		m_pPlayer->pev->gamestate = 1;

	gUranusCollection.pfnDefaultDeploy(this,
		Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw,
		"knife", false
	);	// Enforce to play the anim.

	// Must have a one-frame delay for crosshair hiding due to
	// CBasePlayer::SelectLastItem() put UpdateShieldCrosshair() behind CBasePlayerItem::Deploy().
	m_Scheduler.Enroll(
		[](CBasePlayer* player) noexcept -> Task { co_await TaskScheduler::NextFrame::Rank[0]; player->m_iHideHUD |= HIDEHUD_CROSSHAIR; }(m_pPlayer),
		TASK_RADIO_DEPLOY
	);

	// Disable client prediction after a bit.
	m_Scheduler.Enroll(
		[](CPrefabWeapon* self) noexcept -> Task { co_await 0.1f; self->m_iMockedWeapon = WEAPON_NONE; }(this),
		TASK_RADIO_DEPLOY
	);

	co_await Models::v_radio::time::draw;

	SendWeaponAnim((int)Models::v_radio::seq::idle, false);

	[[unlikely]]
	if (m_pTarget)
		m_pTarget->pev->flags |= FL_KILLME;

	m_pTarget = CDynamicTarget::Create(m_pPlayer, this);
}

Task CRadio::Task_RadioRejected() noexcept
{
	m_bCanHolster = false;
	SendWeaponAnim((int)Models::v_radio::seq::use);
	m_pPlayer->m_flNextAttack = Models::v_radio::time::use;

	g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, Sounds::NOISE, VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));

	static constexpr float TIME_PRESS_TALK = 19.f / 45.f;
	co_await TIME_PRESS_TALK;

	g_engfuncs.pfnEmitSound(edict(), CHAN_VOICE, Sounds::REQUESTING, 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

	co_await Sounds::Length::Radio::REQUESTING;

	g_engfuncs.pfnEmitSound(edict(), CHAN_VOICE, UTIL_GetRandomOne(Sounds::REJECTING), 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));
	gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::REJECT_COVERED_LOCATION);

	static_assert(Models::v_radio::time::use - TIME_PRESS_TALK - Sounds::Length::Radio::REQUESTING > 0);
	co_await (Models::v_radio::time::use - TIME_PRESS_TALK - Sounds::Length::Radio::REQUESTING);

	SendWeaponAnim((int)Models::v_radio::seq::idle, false);
	m_bCanHolster = true;
}

Task CRadio::Task_RadioAccepted() noexcept
{
	auto const pFixedTarget = CFixedTarget::Create(m_pTarget);

	m_pTarget->pev->flags |= FL_KILLME;
	m_pTarget = nullptr;

	m_bTargetActivated = false;	// lock anim holster.
	m_bCanHolster = false;	// BORROWED MEMBER: forbid holster.
	SendWeaponAnim((int)Models::v_radio::seq::use);
	m_pPlayer->m_flNextAttack = Models::v_radio::time::use;

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::NOISE, VOL_NORM, ATTN_STATIC, 0, UTIL_Random(92, 108));

	// Continue as these two tasks.

	m_Scheduler.Enroll(Task_AcceptedAnim());	// this does not consider as the continuation of 'accept'.
	m_Scheduler.Enroll(Task_AcceptedSound(pFixedTarget), TASK_RADIO_ACCEPTED);

	co_return;
}

Task CRadio::Task_AcceptedAnim() noexcept
{
	// This task was expected to be executed after use anim.
	co_await Models::v_radio::time::use;

	bool bHasAnyOtherWeapon = false;
	for (auto&& pWeapon : Query::all_weapons_belongs_to(m_pPlayer))
	{
		if (pWeapon->pev == this->pev)
			continue;

		bHasAnyOtherWeapon = true;
		break;
	}

	[[likely]]
	if (!bHasAnyOtherWeapon)	// No any other weapon, not even a knife. Pathetic.
	{
		SendWeaponAnim((int)Models::v_radio::seq::idle);
		co_return;
	}

	m_bCanHolster = false;
	SendWeaponAnim((int)Models::v_radio::seq::holster);
	m_pPlayer->m_flNextAttack = Models::v_radio::time::holster;

	co_await Models::v_radio::time::holster;

	// No more checks.
	// It is unlikely to loose a weapon during the holster animation.

	// wait until all audio played
	while (!m_bTargetActivated)
	{
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	m_bCanHolster = true;	// BORROWED MEMBER: allow holster.
	m_pPlayer->m_flNextAttack = -1;
	g_engfuncs.pfnClientCommand(m_pPlayer->edict(), "lastinv\n");
}

Task CRadio::Task_AcceptedSound(EHANDLE<CFixedTarget> pFixedTarget) noexcept
{
	auto const iAirsupportType = g_rgiAirSupportSelected[m_pPlayer->entindex()];

	static constexpr float TIME_PRESS_TALK = 19.f / 45.f;
	co_await TIME_PRESS_TALK;

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::REQUESTING, 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

	co_await Sounds::Length::Radio::REQUESTING;

	g_engfuncs.pfnEmitSound(edict(), CHAN_STATIC, Sounds::ACCEPTING[iAirsupportType], 0.75f, ATTN_STATIC, 0, UTIL_Random(92, 108));

	co_await Sounds::Length::Radio::ACCEPTING[iAirsupportType];

	// sync with Task_AcceptedAnim()
	m_bTargetActivated = true;

	[[likely]]
	if (pFixedTarget)
		pFixedTarget->Activate();
	else
		co_return;	// Round ended or something???
}
