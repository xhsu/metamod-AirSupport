import <ranges>;

import util;

import Effects;
import GameRules;
import Jet;
import Projectile;
import Round;
import Target;

void OrpheuF_CleanUpMap(CHalfLifeMultiplay *pThis) noexcept
{
	g_pfnCleanUpMap(pThis);

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

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CJet::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CPrecisionAirStrike::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSmoke::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSparkMdl::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSparkSpr::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;
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

		for (auto &&pPlayer :
			std::views::iota(1, gpGlobals->maxClients) |
			std::views::transform([](int idx) noexcept { return g_engfuncs.pfnPEntityOfEntIndex(idx); }) |
			std::views::filter([](edict_t *pent) noexcept { return pent != nullptr && pent->pvPrivateData != nullptr; }) |
			std::views::transform([](edict_t *pent) noexcept { return (CBasePlayer *)pent->pvPrivateData; }) |
			std::views::filter([](CBasePlayer *pPlayer) noexcept { return !pPlayer->has_disconnected && !(pPlayer->pev->flags & FL_DORMANT); })
			)
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
