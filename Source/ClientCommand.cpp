#include <fmt/core.h>

import <string>;

import meta_api;

import Entity;
import Hook;
import Resources;
import Query;

using std::string;

// Waypoint.cpp
extern Task Waypoint_Scan(void) noexcept;
extern void Waypoint_Read(void) noexcept;
//

META_RES OnClientCommand(CBasePlayer *pPlayer, const string &szCommand) noexcept
{
	if (!pPlayer->IsAlive())
		return MRES_IGNORED;

	[[unlikely]]
	if (szCommand == "takeradio" || szCommand == HUD::RADIO)
	{
		if (auto const pWeapon = pPlayer->m_pActiveItem; pWeapon->m_iId == WEAPON_KNIFE)
		{
			if (pWeapon->pev->weapons == RADIO_KEY)
				return MRES_SUPERCEDE;

			TaskScheduler::Enroll(Weapon::Task_RadioDeploy((CBasePlayerWeapon *)pWeapon));
		}
		else if (auto const pWeapon = pPlayer->m_rgpPlayerItems[3])
		{
			pWeapon->pev->weapons = RADIO_KEY;
			g_pfnSelectItem(pPlayer, "weapon_knife");
		}

		return MRES_SUPERCEDE;
	}
	else if (szCommand == "weapon_knife")
	{
		if (auto const pWeapon = pPlayer->m_pActiveItem; pWeapon->m_iId == WEAPON_KNIFE)
		{
			if (pWeapon->pev->weapons != RADIO_KEY)
				return MRES_IGNORED;

			// Clear radio events
			Weapon::OnRadioHolster((CBasePlayerWeapon *)pWeapon);

			// Take out knife.
			g_pfnItemDeploy(pWeapon);

			return MRES_SUPERCEDE;
		}
	}
	else if (szCommand == "showmetargetorigin")
	{
		auto const pEdict = ent_cast<edict_t *>(pPlayer->pev);

		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		auto const vecSrc = pPlayer->GetGunPosition();
		auto const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pEdict, &tr);

		g_engfuncs.pfnClientPrintf(pEdict, print_console, fmt::format("[AIMING AT]:\n\t{}\n\t{}\n\t{}\n", tr.vecEndPos.x, tr.vecEndPos.y, tr.vecEndPos.z).c_str());
		return MRES_SUPERCEDE;
	}
	else if (szCommand == "scanjetspawn")
	{
		TaskScheduler::Enroll(Waypoint_Scan());
		return MRES_SUPERCEDE;
	}
	else if (szCommand == "readjetspawn")
	{
		Waypoint_Read();
		return MRES_SUPERCEDE;
	}
	//else if (szCommand == "showjetspawn")
	//{
	//	if (!TimedFnMgr::Exist(RADIO_KEY * 2))
	//		TimedFnMgr::Enroll(Waypoint_Show(pPlayer), RADIO_KEY * 2);
	//	else
	//		TimedFnMgr::Delist(RADIO_KEY * 2);
	//
	//	return MRES_SUPERCEDE;
	//}

	return MRES_IGNORED;
}
