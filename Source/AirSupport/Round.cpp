import util;

import Effects;
import GameRules;
import Jet;
import Projectile;
import Query;
import Round;
import Target;
import Task.Const;

import Task;

void OrpheuF_CleanUpMap(CHalfLifeMultiplay *pThis) noexcept
{
	g_pfnCleanUpMap(pThis);

	// Reset Entities

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CBullet::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CCarpetBombardment::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CClusterBomb::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CDebris::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	// CDynamicTarget does not required to be clear up

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CFixedTarget::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CFlame::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CFloatingDust::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CFuelAirCloud::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CFuelAirExplosive::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CGroundedDust::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	// CGunship does not required to be clear up

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CGunshotSmoke::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CIncendiaryMunition::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CJet::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CPhosphorus::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CPrecisionAirStrike::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSmoke::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	{
		for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CThickStaticSmoke::CLASSNAME))
			pEnt->v.flags |= FL_KILLME;

		for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CThinSmoke::CLASSNAME))
			pEnt->v.flags |= FL_KILLME;

		{
			for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CToxicSmoke::CLASSNAME))
				pEnt->v.flags |= FL_KILLME;
		}
	}

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSparkMdl::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSparkSpr::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSpriteDisplay::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	// Reset tasks

	TaskScheduler::Delist(TASK_SUFFOCATION);
	TaskScheduler::Delist(TASK_ENTITY_ON_FIRE);
	TaskScheduler::Delist(TASK_FLAME_ON_PLAYER);
}

void Task_GetWorld(void) noexcept
{
	g_pEdictWorld = g_engfuncs.pfnPEntityOfEntIndex(0);
	g_pevWorld = &g_pEdictWorld->v;
}

Task Task_UpdateTeams(void) noexcept
{
	g_rgpPlayersOfCT.reserve(gpGlobals->maxClients + 1);
	g_rgpPlayersOfTerrorist.reserve(gpGlobals->maxClients + 1);

	for (;;)
	{
		co_await TaskScheduler::NextFrame::Rank[1];

		g_rgpPlayersOfCT.clear();
		g_rgpPlayersOfTerrorist.clear();

		for (auto &&pPlayer : Query::all_players())
		{
			switch (pPlayer->m_iTeam)
			{
			case TEAM_TERRORIST:
				g_rgpPlayersOfTerrorist.emplace_back(pPlayer->edict());
				break;

			case TEAM_CT:
				g_rgpPlayersOfCT.emplace_back(pPlayer->edict());
				break;

			default:
				continue;
			}
		}
	}
}
