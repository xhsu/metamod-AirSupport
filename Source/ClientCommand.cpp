#include <fmt/core.h>

import <string>;

import meta_api;

import CBase;
import Entity;
import Hook;
import Resources;

using std::string;

META_RES OnClientCommand(CBasePlayer *pPlayer, const string &szCommand) noexcept
{
	if (!pPlayer->IsAlive())
		return MRES_IGNORED;

	if (szCommand == "takeradio")
	{
		if (auto const pWeapon = pPlayer->m_pActiveItem; pWeapon->m_iId == WEAPON_KNIFE)
		{
			if (pWeapon->pev->weapons == RADIO_KEY)
				return MRES_SUPERCEDE;

			pPlayer->pev->viewmodel = MAKE_STRING(Models::V_RADIO);
			pPlayer->pev->weaponmodel = MAKE_STRING(Models::P_RADIO);

			pPlayer->m_flNextAttack = Models::v_radio::time::draw;
			gmsgWeaponAnim::Send(ent_cast<edict_t *>(pPlayer->pev), (byte)Models::v_radio::seq::draw, 0);

			pWeapon->pev->weapons = RADIO_KEY;
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

			pWeapon->pev->weapons = 0;

			g_pfnItemDeploy(pWeapon);

			//remove_task(iEntity);
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

	return MRES_IGNORED;
}
