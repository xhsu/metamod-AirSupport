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

import UtlRandom;


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

	for (auto&& pEnt : FIND_ENTITY_BY_CLASSNAME(CClusterCharge::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(CDebris::CLASSNAME))
		pEnt->v.flags |= FL_KILLME;

	// CDynamicTarget does not required to be clear up

	for (auto&& pEnt : Query::all_nonplayer_entities() | Query::with_classname_of(CDynamicTarget::BEAM_CLASSNAME))
		pEnt->pev->flags |= FL_KILLME;	// Fixing the but when player calls a carpet bombardment during round ending.

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

Task Task_TeamwiseAI(cvar_t const* pcvarEnable, std::vector<edict_t*> const* pTeamMembers, std::vector<edict_t*> const* pEnemies) noexcept
{
	std::vector<CBasePlayer*> rgpPotentialTargets{}, rgpCandidates{};
	rgpPotentialTargets.reserve(gpGlobals->maxClients + 1);

	for (TraceResult tr{};;)
	{
		co_await UTIL_Random(5.f, 10.f);

		if (pcvarEnable->value <= 0)
			continue;

		rgpPotentialTargets.clear();
		rgpCandidates =
			(*pEnemies)
			| Query::as<CBasePlayer>()
			| std::views::filter(&CBasePlayer::IsAlive)
			| std::ranges::to<std::vector>();

		for (auto&& pPlayer : rgpCandidates)
		{
			auto const vecSky = pPlayer->pev->origin + Vector{ 0, 0, 4096 };
			g_engfuncs.pfnTraceLine(pPlayer->pev->origin, vecSky, ignore_monsters, nullptr, &tr);

			if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
				continue;

			for (auto&& pPlayer2 : rgpCandidates)
			{
				if ((pPlayer->pev->origin - pPlayer2->pev->origin).LengthSquared() < (300 * 300))
					rgpPotentialTargets.push_back(pPlayer);
			}
		}

		if (rgpPotentialTargets.empty())
			continue;

		// If failed on same target over and over again, let's try another one.
		UTIL_Shuffle(rgpPotentialTargets);

		for (auto&& pPlayer :
			(*pTeamMembers)
			| Query::as<CBasePlayer>()
			| std::views::filter(&CBasePlayer::IsAlive)
			| std::views::filter(&CBasePlayer::IsBot)
			)
		{
			for (auto&& pTarget : rgpPotentialTargets)
			{
				auto const vecDir = (pTarget->pev->origin - pPlayer->pev->origin).Normalize();
				auto const vecFwd = pPlayer->pev->v_angle.Front();
				auto const flAngle = std::acos(DotProduct(vecDir, vecFwd)/* No div len required, both len are 1. */) / std::numbers::pi * 180.0;

				if (flAngle > 120)
					continue;

				g_engfuncs.pfnTraceLine(pPlayer->pev->origin, pTarget->pev->origin, ignore_monsters, nullptr, &tr);

				if (tr.flFraction < 0.99f)
					continue;

				CFixedTarget::Create(pPlayer, pTarget)->Activate();
				goto LAB_CONTINUE;
			}
		}

	LAB_CONTINUE:;
	}
}
