import <format>;
import <string>;

import CBase;
import Hook;
import Menu;
import Plugin;
import Resources;
import Task;
import Weapon;

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
	if (szCommand == "takeradio")
	{
		auto pRadio = Prefab_t::Create<CRadio>();
		pRadio->DefaultTouch(pPlayer);

		return MRES_SUPERCEDE;
	}
	else if (szCommand == "showmetargetorigin")
	{
		auto const pEdict = ent_cast<edict_t *>(pPlayer->pev);

		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		auto const vecSrc = pPlayer->GetGunPosition();
		auto const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pEdict, &tr);

		g_engfuncs.pfnClientPrintf(pEdict, print_console, std::format("[AIMING AT]:\n\t{}\n\t{}\n\t{}\n", tr.vecEndPos.x, tr.vecEndPos.y, tr.vecEndPos.z).c_str());
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
	else if (szCommand == "menuselect" && pPlayer->m_iMenu > EMenu::Menu_LastItem)
	{
		if (auto const iSlot = std::atoi(g_engfuncs.pfnCmd_Argv(1)); iSlot > 0 && iSlot < 10)
		{
			OnMenuSelection(pPlayer, iSlot);
			pPlayer->m_iMenu = EMenu::Menu_OFF;

			return MRES_SUPERCEDE;
		}
	}
	else if (szCommand == "edict")
	{
		g_engfuncs.pfnServerPrint(std::format("{}/{}\n", g_engfuncs.pfnNumberOfEntities(), gpGlobals->maxEntities).c_str());
		return MRES_SUPERCEDE;
	}
	else if (szCommand == "airsupport_extra_info")
	{
#ifndef _MSC_FULL_VER
#error "This project is exclusive to MSVC. Fvck clang, GCC and Apple stuff."
#endif
		g_engfuncs.pfnServerPrint(std::format(
			"\tCompiled with MSVC {0} and C++ {1}L\n"
			"\tPlugin build number {2}\n",

			_MSC_FULL_VER, __cplusplus,
			Engine::LocalBuildNumber()
		).c_str());

		return MRES_SUPERCEDE;
	}

	return MRES_IGNORED;
}
