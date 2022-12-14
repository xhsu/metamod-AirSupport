import <array>;
import <format>;
import <numbers>;

import meta_api;

import Hook;
import Jet;
import Localization;
import Menu;
import Projectile;
import Query;
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
		TaskScheduler::Enroll(Weapon::Task_RadioDeploy(pThis), UTIL_CombineTaskAndPlayerIndex(TASK_RADIO_DEPLOY, pThis->m_pPlayer->entindex()));
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
				iIndex == FUEL_AIR_BOMB ? "\\d" : "\\w", iIndex == FUEL_AIR_BOMB ? " - Selected" : ""
			)
		);

		pThis->m_pPlayer->m_iMenu = EMenu::Menu_AirSupport;
	}
	else if (pThis->m_pPlayer->m_afButtonPressed & IN_USE) [[unlikely]]
	{
		g_engfuncs.pfnMakeVectors(pThis->m_pPlayer->pev->v_angle);

		//Prefab_t::Create<CSpriteDisplayment>(
		//	pThis->m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 256.0,
		//	(kRenderFn)g_rgiAirSupportSelected[pThis->m_pPlayer->entindex()]
		//);

		//Prefab_t::Create<CBullet>(
		//	pThis->m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 64.0,
		//	gpGlobals->v_forward * 2048,
		//	pThis->m_pPlayer
		//);

		TraceResult tr{};
		auto const vecSrc = pThis->m_pPlayer->GetGunPosition();
		auto const vecEnd = vecSrc + gpGlobals->v_forward * 4096.0;
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_glass | ignore_monsters, nullptr, &tr);
		//g_engfuncs.pfnTraceLine(tr.vecEndPos, Vector(tr.vecEndPos.x, tr.vecEndPos.y, 8192.0), ignore_glass | ignore_monsters, nullptr, &tr);

		//Prefab_t::Create<CFuelAirExplosive>(pThis->m_pPlayer, tr.vecEndPos + Vector::Down() * 3);
		Prefab_t::Create<CFlame>(tr.vecEndPos)->pev->velocity = Vector(0, 0, 300);
		//Prefab_t::Create<CFuelAirCloud>(tr.vecEndPos)->Ignite();

		//TaskScheduler::Enroll(CFuelAirCloud::Task_PlayerSuffocation(pThis->m_pPlayer, &g_engfuncs.pfnPEntityOfEntIndex(0)->v), TASK_HB_AND_ER);

		//Prefab_t::Create<CClusterCharge>(pThis->m_pPlayer, tr.vecEndPos + tr.vecPlaneNormal * 128.0, 2.f);
	}
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
		pThis->m_pPlayer->m_iHideHUD &= ~HIDEHUD_CROSSHAIR;	// #TODO use CurWeapon to fix crosshair?

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

void __fastcall OrpheuF_FireBullets(CBaseEntity *pThis, int, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) noexcept
{
	g_bIsSomeoneShooting = true;
	UTIL_UndoPatch(g_pfnFireBullets, HookInfo::FireBullets.m_OriginalBytes);
	g_pfnFireBullets(pThis, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage, pevAttacker);
	UTIL_DoPatch(g_pfnFireBullets, HookInfo::FireBullets.m_PatchedBytes);
	g_bIsSomeoneShooting = false;
}

Vector __fastcall OrpheuF_FireBullets3(long argument1, long argument2, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, bool bPistol, int shared_rand) noexcept
{
	// LUNA: this hook is VERY wierd and cannot be served as any other purpose
	// unlike other __fastcall, the 'this' pointer is still stuck in register ecx and the first two arguments remains unknown.
	// So does the return value, it's a array of constant mumbo-jumbo 4-byte data.
	// According to IDA, it consists of 16 arguments (excluding 'this' and 'edx') but only 15 arguments are meaningful.

	//_asm mov pThis, ecx;

	g_bIsSomeoneShooting = true;
	UTIL_UndoPatch(g_pfnFireBullets3, HookInfo::FireBullets3.m_OriginalBytes);
	auto const ret = g_pfnFireBullets3(argument1, argument2, vecSrc, vecDirShooting, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, pevAttacker, bPistol, shared_rand);
	UTIL_DoPatch(g_pfnFireBullets3, HookInfo::FireBullets3.m_PatchedBytes);
	g_bIsSomeoneShooting = false;
	return ret;
}
