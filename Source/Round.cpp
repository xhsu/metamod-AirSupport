import <ranges>;

import util;

import Effects;
import Entity;
import GameRules;

void OrpheuF_CleanUpMap(CHalfLifeMultiplay *pThis) noexcept
{
	g_pfnCleanUpMap(pThis);

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::FIXED_TARGET))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::JET))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::MISSILE))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CFlame::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CSmoke::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;
}

Task Task_UpdateTeams(void) noexcept
{
	for (;;)
	{
		co_await 0.1f;

		g_rgpCTs.clear();
		g_rgpTers.clear();

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
				g_rgpTers.emplace_back(pPlayer->edict());
				break;

			case TEAM_CT:
				g_rgpCTs.emplace_back(pPlayer->edict());
				break;

			default:
				continue;
			}
		}
	}
}
