import std;

import CBase;
import Hook;
import Menu;
import Plugin;
import Resources;
import Task;
import Weapon;
import Models;
import FileSystem;

import UtlHook;	// debug command.

using std::string;


META_RES OnClientCommand(CBasePlayer *pPlayer, const string &szCommand) noexcept
{
	auto const bAlive = pPlayer->IsAlive();

	[[unlikely]]
	if (bAlive && szCommand == "takeradio")
	{
		//auto pRadio = Prefab_t::Create<CRadio>();
		//pRadio->DefaultTouch(pPlayer);

		// This is just a modified version of CBP::GiveNamedItem()

		auto const pEdict = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING(CRadio::CLASSNAME));
		pEdict->v.origin = pPlayer->pev->origin;
		pEdict->v.spawnflags |= SF_NORESPAWN;

		gpGamedllFuncs->dllapi_table->pfnSpawn(pEdict);

		auto const iSolid = pEdict->v.solid;
		gpGamedllFuncs->dllapi_table->pfnTouch(pEdict, pPlayer->edict());

		if (iSolid != pEdict->v.solid)
			return MRES_SUPERCEDE;

		pEdict->v.flags |= FL_KILLME;
		return MRES_SUPERCEDE;
	}
	else if (szCommand == "showmetargetorigin")
	{
		auto const pEdict = ent_cast<edict_t *>(pPlayer->pev);

		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		auto const vecSrc = pPlayer->GetGunPosition();
		auto const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters | ignore_glass, pEdict, &tr);

		g_engfuncs.pfnClientPrintf(pEdict, print_console, std::format("[AIMING AT]:\n\t{}\n\t{}\n\t{}\n", tr.vecEndPos.x, tr.vecEndPos.y, tr.vecEndPos.z).c_str());
		return MRES_SUPERCEDE;
	}
	else if (bAlive && szCommand == "menuselect" && pPlayer->m_iMenu > EMenu::Menu_LastItem)
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
		g_engfuncs.pfnClientPrintf(pPlayer->edict(), print_console, std::format(
			"\tCompiled with MSVC {0} and C++ {1}L\n"
			"\tPlugin version {2}\n",

			_MSC_FULL_VER, __cplusplus,
			PLID->version
		).c_str());

		return MRES_SUPERCEDE;
	}

#ifdef _DEBUG
	else if (szCommand == "sys_timescale")
	{
		//static constexpr unsigned char FN[] = "\xCC\x55\x8B\xEC\x83\xEC\x5C\xA1\x2A\x2A\x2A\x2A\x33\xC5\x89\x45\xFC\x83\x3D\x2A\x2A\x2A\x2A\x2A\x8B";
		//static constexpr std::ptrdiff_t OFS = 0x101FE4DE - 0x101FE3E0;
		//auto const addr = UTIL_SearchPattern("hw.dll", 1, FN);
		//auto const pSysSpeed = UTIL_RetrieveGlobalVariable<float>(addr, OFS);

		static auto pSysTimeScale = reinterpret_cast<float*>(UTIL_GetModuleBase("hw.dll") + 0x31B5C4);

		if (auto const fl = std::atof(g_engfuncs.pfnCmd_Argv(1)); fl > 0)
			*pSysTimeScale = (float)fl;

		return MRES_SUPERCEDE;
	}
#endif

	return MRES_IGNORED;
}
