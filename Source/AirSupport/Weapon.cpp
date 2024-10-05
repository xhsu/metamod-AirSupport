#ifdef __INTELLISENSE__
#include <ranges>
#endif

#include <cassert>

import metamod_api;

import Configurations;
import Effects;
import Jet;
import Localization;
import Menu;
import Message;
import Query;
import Resources;
import Target;
import Task;
import Uranus;
import Weapon;

import UtlHook;
import UtlRandom;



// Hook Forwards

void __fastcall OrpheuF_FireBullets(CBaseEntity *pThis, int edx, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) noexcept
{
	g_bIsSomeoneShooting = true;
	HookInfo::FireBullets(pThis, edx, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage, pevAttacker);
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
	g_bIsSomeoneShooting = false;
	return ret;
}

void __cdecl OrpheuF_W_Precache() noexcept
{
	HookInfo::W_Precache();

	gUranusCollection.pfnUTIL_PrecacheOtherWeapon(CRadio::CLASSNAME);
}

// CRadio

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
	auto const iMode = g_rgiAirSupportSelected[m_pPlayer->entindex()];

	if (iMode == GUNSHIP_STRIKE && m_rgbTeamCooldown[m_pPlayer->m_iTeam] && !HavingGunshipRunning()) [[unlikely]]
	{
		if (!m_Scheduler.Exist(TASK_RADIO_FORCED_HOLSTER))
		{
			m_Scheduler.Enroll(
				Task_SequencedHolster(),
				TASK_RADIO_FORCED_HOLSTER
			);
		}
	}
	else if (m_pPlayer->m_afButtonPressed & IN_ATTACK && m_pPlayer->m_flNextAttack <= 0 && m_bSoundSeqFinished) [[unlikely]]
	{
		switch (iMode)
		{
		case CARPET_BOMBARDMENT:

			// The first coord must be legit as well.
			if (m_pTarget->pev->skin == Models::targetmdl::SKIN_GREEN)
			{
				m_pTarget->EnableBeacons();

				[[unlikely]]
				if (!m_bHintPressAndHold)
				{
					gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::HINT_PRESS_AND_HOLD);
					m_bHintPressAndHold = true;
				}
			}

			break;

		case GUNSHIP_STRIKE:
			if (CGunship::s_bInstanceExists)
			{
				gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::GUNSHIP_ENTITY_MUTUALLY_EXCLUSIVE);
				break;
			}

			[[fallthrough]];

		default:
			auto const bAccepted = (m_pTarget->pev->skin == Models::targetmdl::SKIN_GREEN);
			auto const iFlag = bAccepted ? TASK_RADIO_ACCEPTED : TASK_RADIO_REJECTED;

			m_Scheduler.Enroll(Task_CallAnimation(bAccepted), iFlag);
			m_Scheduler.Enroll(Task_CallSoundFx(bAccepted), iFlag);

			if (bAccepted)
				m_Scheduler.Enroll(Task_FixedTargetCalling(), TASK_RADIO_TARGET);

			break;
		}
	}
	else if (m_pPlayer->m_afButtonReleased & IN_ATTACK) [[unlikely]]
	{
		switch (iMode)
		{
		default:
			break;

		case CARPET_BOMBARDMENT:

			// Only if you already entered aiming mode can you start the attack.
			[[likely]]
			if (m_pTarget->m_bFreezed)
			{
				auto const bAccepted = (m_pTarget->pev->skin == Models::targetmdl::SKIN_GREEN);
				auto const iFlag = bAccepted ? TASK_RADIO_ACCEPTED : TASK_RADIO_REJECTED;

				m_Scheduler.Enroll(Task_CallAnimation(bAccepted), iFlag);
				m_Scheduler.Enroll(Task_CallSoundFx(bAccepted), iFlag);

				if (bAccepted)
					m_Scheduler.Enroll(Task_FixedTargetCalling(), TASK_RADIO_TARGET);
				else
					m_pTarget->DisableBeacons();
			}

			break;
		}
	}
	else if (m_pPlayer->m_afButtonPressed & IN_ATTACK2) [[unlikely]]
	{
		// One can only select gunship mode in this case.
		if (HavingGunshipRunning())
			return;

		UTIL_ShowMenu(
			m_pPlayer->edict(),
			Menu::Key::AIRSUPPORT,
			std::format(Menu::Text::AIRSUPPORT_TEMPLATE,
				iMode == AIR_STRIKE ? "\\d" : "\\w", iMode == AIR_STRIKE ? Menu::Text::SELECTED : "",
				iMode == CLUSTER_BOMB ? "\\d" : "\\w", iMode == CLUSTER_BOMB ? Menu::Text::SELECTED : "",
				iMode == CARPET_BOMBARDMENT ? "\\d" : "\\w", iMode == CARPET_BOMBARDMENT ? Menu::Text::SELECTED : "",
				iMode == GUNSHIP_STRIKE ? "\\d" : "\\w", iMode == GUNSHIP_STRIKE ? Menu::Text::SELECTED : "",
				iMode == FUEL_AIR_BOMB ? "\\d" : "\\w", iMode == FUEL_AIR_BOMB ? Menu::Text::SELECTED : "",
				iMode == PHOSPHORUS_MUNITION ? "\\d" : "\\w", iMode == PHOSPHORUS_MUNITION ? Menu::Text::SELECTED : ""
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
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters | dont_ignore_glass, m_pPlayer->edict(), &tr);
		g_engfuncs.pfnTraceLine(m_pPlayer->pev->origin, m_pPlayer->pev->origin + Vector(0, 0, 4096), ignore_monsters | ignore_glass, m_pPlayer->edict(), &tr2);

		if (EHANDLE<CBaseEntity> aiming{ tr.pHit }; aiming && aiming->IsPlayer())
		{
			auto pTarget = CFixedTarget::Create(m_pPlayer, aiming);
			pTarget->Activate();
		}
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

	if (m_Scheduler.Delist(TASK_RADIO_ACCEPTED) > 0 && !m_bSoundSeqFinished)
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

	// sync flag reset
	m_bSoundSeqFinished = true;

	// Resume shield protection
	if (m_pPlayer->m_bOwnsShield)
		m_pPlayer->pev->gamestate = 0;

	m_pPlayer->m_flNextAttack = -1;

	// DO NOT CLEAR the task list!!!
	// since we are having Task_FixedTargetCalling() await every 0.1 second, it is likely that
	// the radio holstered without the CFixedTarget entity activated!
	//assert(m_Scheduler.m_List.size() == 0);

	__super::Holster(param);
}

Task CRadio::Task_Deploy() noexcept
{
	m_iClientPredictionId = WEAPON_KNIFE;	// because that doesn't have any ammo ui etc.
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->m_bOwnsShield)
		m_pPlayer->pev->gamestate = 1;	// pause shield protection

	gUranusCollection.pfnDefaultDeploy(this,
		Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw,
		"knife", false
	);	// Enforce to play the anim.

	[[unlikely]]
	if (m_pTarget)
		m_pTarget->pev->flags |= FL_KILLME;	// Depose the old one.

	// Must have a one-frame delay for crosshair hiding due to
	// CBasePlayer::SelectLastItem() put UpdateShieldCrosshair() behind CBasePlayerItem::Deploy().
	m_Scheduler.Enroll(
		[](CBasePlayer* player) noexcept -> Task { co_await TaskScheduler::NextFrame::Rank[0]; player->m_iHideHUD |= HIDEHUD_CROSSHAIR; }(m_pPlayer),
		TASK_RADIO_DEPLOY
	);

	// Disable client prediction after a bit.
	m_Scheduler.Enroll(
		[](CPrefabWeapon* self) noexcept -> Task { co_await 0.1f; self->m_iClientPredictionId = WEAPON_NONE; }(this),
		TASK_RADIO_DEPLOY
	);

	co_await Models::v_radio::time::draw;

	auto const bHavingGunship = HavingGunshipRunning();

	if (m_rgbTeamCooldown[m_pPlayer->m_iTeam] && !bHavingGunship)
	{
		// Cooling down. Holster the weapon!
		gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::REJECT_COOLING_DOWN);

		m_Scheduler.Enroll(
			Task_SequencedHolster(),
			TASK_RADIO_DEPLOY | TASK_RADIO_FORCED_HOLSTER	// weird, but it's actually making sense.
		);
	}
	else
	{
		if (bHavingGunship)
			g_rgiAirSupportSelected[m_pPlayer->entindex()] = GUNSHIP_STRIKE;	// locked in this mode.

		SendWeaponAnim((int)Models::v_radio::seq::idle, false);

		// Create the new one on every draw.
		m_pTarget = CDynamicTarget::Create(m_pPlayer, this);
	}
}

Task CRadio::Task_CallAnimation(bool bGotoHolster) noexcept
{
	m_bCanHolster = false;
	SendWeaponAnim((int)Models::v_radio::seq::use);
	m_pPlayer->m_flNextAttack = Models::v_radio::time::use;

	co_await Models::v_radio::time::use;

	m_bCanHolster = true;

	if (bGotoHolster)
	{
		// Even if holster is ordered, it must have another weapon that can switch out.
		bGotoHolster = false;

		for (auto&& pWeapon : Query::all_weapons_belongs_to(m_pPlayer))
		{
			if (pWeapon->pev == this->pev)
				continue;

			bGotoHolster = true;
			break;
		}
	}

	if (!bGotoHolster)	// No any other weapon, not even a knife. Pathetic.
	{
		SendWeaponAnim((int)Models::v_radio::seq::idle);
		co_return;
	}

	// goto holster

	m_bCanHolster = false;
	SendWeaponAnim((int)Models::v_radio::seq::holster);
	m_pPlayer->m_flNextAttack = Models::v_radio::time::holster;

	co_await Models::v_radio::time::holster;

	// wait until all audio played (synchronization)
	while (!m_bSoundSeqFinished)
	{
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	m_bCanHolster = true;
	g_engfuncs.pfnClientCommand(m_pPlayer->edict(), "lastinv\n");

	if (g_rgiAirSupportSelected[m_pPlayer->entindex()] == GUNSHIP_STRIKE)
		gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::HINT_RESEL_TARGET);
}

Task CRadio::Task_CallSoundFx(bool bAccepted) noexcept
{
	m_bSoundSeqFinished = false;
	g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, Sounds::NOISE, VOL_NORM, ATTN_STATIC, SND_FL_NONE, UTIL_Random(92, 108));

	static constexpr float TIME_PRESS_TALK = 19.f / 45.f;
	co_await TIME_PRESS_TALK;

	g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, Sounds::REQUESTING, 0.75f, ATTN_STATIC, SND_FL_NONE, UTIL_Random(92, 108));

	co_await (float)g_rgflSoundTime.at(Sounds::REQUESTING);

	std::string_view szSFX{};
	if (bAccepted)
	{
		auto const& iAirsupportType = g_rgiAirSupportSelected[m_pPlayer->entindex()];
		szSFX = Sounds::ACCEPTING[iAirsupportType];
		g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, szSFX.data(), 0.75f, ATTN_STATIC, SND_FL_NONE, UTIL_Random(92, 108));
	}
	else
	{
		szSFX = UTIL_GetRandomOne(Sounds::REJECTING);
		g_engfuncs.pfnEmitSound(edict(), CHAN_AUTO, szSFX.data(), 0.75f, ATTN_STATIC, SND_FL_NONE, UTIL_Random(92, 108));

		gmsgTextMsg::Send(m_pPlayer->edict(), 4, Localization::REJECT_COVERED_LOCATION);
	}

	co_await (float)g_rgflSoundTime.at(szSFX);

	// synchronization flag
	m_bSoundSeqFinished = true;
}

Task CRadio::Task_FixedTargetCalling() noexcept
{
	EHANDLE pFixedTarget{ CFixedTarget::Create(m_pTarget) };

	m_pTarget->pev->flags |= FL_KILLME;
	m_pTarget = nullptr;

	for (; !m_bSoundSeqFinished;)
	{
		co_await 0.1f;
	}

	m_rgbTeamCooldown[m_pPlayer->m_iTeam] = true;

	// The player interval was only enforced if calling is from a radio weapon.
	auto const iTaskId = UTIL_CombineTaskAndIndex(TASK_RADIO_TEAM_CD, m_pPlayer->m_iTeam);
	TaskScheduler::Delist(iTaskId);
	TaskScheduler::Enroll(
		[](int iTeam, EAirSupportTypes iType) -> Task
		{
			co_await (float)CVar::player_cd[(size_t)iType];
			m_rgbTeamCooldown[iTeam] = false;
		}
		(m_pPlayer->m_iTeam, pFixedTarget->m_AirSupportType),
		iTaskId
	);

	[[likely]]
	if (pFixedTarget)
		pFixedTarget->Activate();
	else
		co_return;	// Round ended or something???
}

Task CRadio::Task_SequencedHolster() noexcept
{
	while (!CanHolster())
	{
		co_await TaskScheduler::NextFrame::Rank[0];
	}

	// Even if holster is ordered, it must have another weapon that can switch out.
	auto bHasAnyOtherWeapon = false;

	for (auto&& pWeapon : Query::all_weapons_belongs_to(m_pPlayer))
	{
		if (pWeapon->pev == this->pev)
			continue;

		bHasAnyOtherWeapon = true;
		break;
	}

	if (!bHasAnyOtherWeapon)	// No any other weapon, not even a knife. Pathetic.
	{
		SendWeaponAnim((int)Models::v_radio::seq::idle);
		m_pPlayer->m_flNextAttack = 0;
		co_return;
	}

	SendWeaponAnim((int)Models::v_radio::seq::holster);
	m_pPlayer->m_flNextAttack = Models::v_radio::time::holster;

	co_await Models::v_radio::time::holster;

	g_engfuncs.pfnClientCommand(m_pPlayer->edict(), "lastinv\n");
}

bool CRadio::HavingGunshipRunning() noexcept
{
	for (auto&& pEntity :
		Query::all_instances_of<CFixedTarget>()
		| std::views::filter([&](CFixedTarget* pEntity) noexcept { return pEntity->m_pPlayer == m_pPlayer && pEntity->m_AirSupportType == GUNSHIP_STRIKE; })
		)
	{
		return true;
	}

	return false;
}
