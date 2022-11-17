#include <cassert>

import progdefs;
import util;

import Beam;
import Entity;
import Missile;
import Resources;
import Waypoint;

import UtlRandom;
/*
extern "C++" namespace Laser
{
	void Create(CBasePlayerWeapon *pWeapon) noexcept
	{
		auto const pBeam = Beam_Create(Sprite::BEAM, 32.f);

		Beam_SetFlags(&pBeam->v, BEAM_FSHADEOUT);	// fade out on rear end.
		Beam_PointsInit(pBeam, pWeapon->m_pPlayer->pev->origin, pWeapon->m_pPlayer->GetGunPosition());

		pBeam->v.classname = MAKE_STRING(Classname::BEAM);
		pBeam->v.effects |= EF_NODRAW;
		pBeam->v.renderfx = kRenderFxNone;
		pBeam->v.nextthink = 0.1f;
		pBeam->v.euser1 = pWeapon->m_pPlayer->edict();	// pev->owner gets occupied by 'starting ent'
		pBeam->v.team = pWeapon->m_pPlayer->m_iTeam;

		//auto const pSprite = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("cycler_sprite"));
		//g_engfuncs.pfnSetModel(pSprite, Sprite::AIM);
		//pSprite->v.framerate = 10;
		//pSprite->v.renderfx = kRenderFxStrobeSlow;
		//pSprite->v.rendermode = kRenderNormal;
		//pSprite->v.renderamt = 128;
		//pSprite->v.rendercolor = Vector(255, 255, 255);

		//gpGamedllFuncs->dllapi_table->pfnSpawn(pSprite);

		//pSprite->v.classname = MAKE_STRING(Classname::AIM);
		//pSprite->v.solid = SOLID_NOT;
		//pSprite->v.takedamage = DAMAGE_NO;
		//pSprite->v.effects |= EF_NODRAW;
		//pSprite->v.euser1 = ent_cast<edict_t *>(pWeapon->m_pPlayer->pev);

		pWeapon->pev->euser1 = pBeam;
		//pWeapon->pev->euser2 = pSprite;
		//pSprite->v.euser2 = pBeam;
		//pBeam->v.euser2 = pSprite;
	}

	void Think(CBaseEntity *pEntity) noexcept
	{
		[[unlikely]]
		if (pev_valid(pEntity->pev->euser1) != 2 || !((CBasePlayer *)pEntity->pev->euser1->pvPrivateData)->IsAlive())
		{
			pEntity->pev->flags |= FL_KILLME;
			//pEntity->pev->euser2->v.flags |= FL_KILLME;
			return;
		}

		pEntity->pev->nextthink = gpGlobals->time + 0.4f;

		if (pEntity->pev->effects & EF_NODRAW)
			return;

		auto const pPlayer = (CBasePlayer *)pEntity->pev->euser1->pvPrivateData;
		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		Vector vecSrc = pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		TraceResult tr{};

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pEntity->pev->euser1, &tr);

		Beam_SetStartPos(pEntity->pev, tr.vecEndPos);
		Beam_SetEndPos(pEntity->pev, tr.vecEndPos + tr.vecPlaneNormal * 96);

		//auto const pSprite = (CBaseEntity *)pEntity->pev->euser2->pvPrivateData;

		//pSprite->pev->effects &= ~EF_NODRAW;
		//pSprite->pev->rendermode = kRenderTransAdd;
		//pSprite->pev->renderfx = kRenderFxNone;

		//g_engfuncs.pfnSetOrigin(pEntity->pev->euser2, tr.vecEndPos + tr.vecPlaneNormal);
		//g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pEntity->pev->euser2->v.angles);
	}
};

extern "C++" namespace Target
{
	void Create(CBasePlayerWeapon *pWeapon) noexcept
	{
		auto const pTarget = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));

		g_engfuncs.pfnSetOrigin(pTarget, pWeapon->pev->origin);
		g_engfuncs.pfnSetModel(pTarget, Models::TARGET);
		g_engfuncs.pfnSetSize(pTarget, Vector::Zero(), Vector::Zero());

		pTarget->v.classname = MAKE_STRING(CDynamicTarget::CLASSNAME);
		pTarget->v.solid = SOLID_NOT;
		pTarget->v.movetype = MOVETYPE_NOCLIP;
		pTarget->v.effects |= EF_NODRAW;
		pTarget->v.rendermode = kRenderTransAdd;
		pTarget->v.renderfx = kRenderFxPulseFastWide;
		pTarget->v.renderamt = 128;
		pTarget->v.nextthink = 0.1f;
		pTarget->v.euser1 = pWeapon->m_pPlayer->edict();	// pev->owner was not occupied, but just keep the usage sync with Laser type.
		pTarget->v.team = pWeapon->m_pPlayer->m_iTeam;

		pWeapon->pev->euser1 = pTarget;

		TaskScheduler::Enroll(Task_ScanJetSpawn(pTarget));
	}

	void Think(CBaseEntity *pEntity) noexcept
	{
		[[unlikely]]
		if (EHANDLE<CBasePlayer> pPlayer = pEntity->pev->euser1;
			!pPlayer || !pPlayer->IsAlive() || !pPlayer->m_rgpPlayerItems[3] || pev_valid(pPlayer->m_rgpPlayerItems[3]->pev) != 2)
		{
			pEntity->pev->flags |= FL_KILLME;
			return;
		}

		pEntity->pev->nextthink = 0.1f;	// This think needs to be called every frame, so we are not making it a task.

		if (pEntity->pev->effects & EF_NODRAW)
			return;

		// Calc where does player aiming

		auto const pPlayer = (CBasePlayer *)pEntity->pev->euser1->pvPrivateData;
		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		Vector const vecSrc = pPlayer->GetGunPosition();
		Vector const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		TraceResult tr{};

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pEntity->pev->euser1, &tr);

		g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pEntity->pev->angles);
		pEntity->pev->angles.x += 270.f;	// don't know why, but this is the deal.

		g_engfuncs.pfnSetOrigin(pEntity->edict(), tr.vecEndPos);

		// Determind model color

		pEntity->pev->team = pPlayer->m_iTeam;	// ZR, ZR.

		if ((pEntity->pev->vuser1 - pEntity->pev->origin).LengthSquared() > 24.0 * 24.0)
		{
			pEntity->pev->vuser1 = pEntity->pev->origin;	// Moving too far from last aiming
			pEntity->pev->vuser2 = Vector::Zero();	// Clear last est. jet spawn.
			pEntity->pev->fuser1 = gpGlobals->time + 0.1f;	// Next think regards color change.
			pEntity->pev->skin = Models::targetmdl::SKIN_RED;
		}
		else if (pEntity->pev->fuser1 < gpGlobals->time)
			pEntity->pev->skin = pEntity->pev->vuser2 != Vector::Zero() ? Models::targetmdl::SKIN_GREEN : Models::targetmdl::SKIN_RED;

		// Model Animation

		pEntity->pev->framerate = float(Models::targetmdl::FPS * gpGlobals->frametime);
		pEntity->pev->frame += pEntity->pev->framerate;
		pEntity->pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pEntity->pev->frame < 0 || pEntity->pev->frame >= 256)
			pEntity->pev->frame -= float((pEntity->pev->frame / 256.0) * 256.0);
	}

	Task Task_ScanJetSpawn(EHANDLE<CBaseEntity> pTarget) noexcept
	{
		TraceResult tr{};
		EHANDLE<CBasePlayer> pPlayer = pTarget->pev->euser1;

		for (; pTarget;)
		{
			size_t iCounter = 0;

			if (pTarget->pev->effects & EF_NODRAW)
			{
				co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
				continue;
			}

			if (pTarget->pev->vuser2 != Vector::Zero())	// result yielded. skip this round.
			{
				co_await (gpGlobals->frametime * 2.f);
				continue;
			}

			for (const auto &vec : g_WaypointMgr.m_rgvecOrigins)
			{
				++iCounter;
				g_engfuncs.pfnTraceLine(pTarget->pev->vuser1, vec, ignore_monsters | ignore_glass, nullptr, &tr);

				[[unlikely]]
				if (tr.flFraction > 0.99f)
				{
					pTarget->pev->vuser2 = vec;
					break;
				}

				[[unlikely]]
				if (!(iCounter % 256))
				{
					co_await (gpGlobals->frametime / 2.f);	// gurentee resume next frame.

					if (!pTarget)
						co_return;
				}
			}

			[[unlikely]]
			if (pTarget->pev->vuser2 == Vector::Zero())
			{
				[[likely]]
				if (pPlayer && pPlayer->IsAlive())
				{
					// Try to get a temp spawn location above player.
					Vector const vecSrc = pPlayer->GetGunPosition();
					Vector const vecEnd { vecSrc.x, vecSrc.y, 8192.f };
	
					g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters | ignore_glass, nullptr, &tr);
	
					if (Vector const vecSavedCandidate = tr.vecEndPos; g_engfuncs.pfnPointContents(vecSavedCandidate) == CONTENTS_SKY)
					{
						g_engfuncs.pfnTraceLine(vecSavedCandidate, pTarget->pev->vuser1, ignore_monsters | ignore_glass, nullptr, &tr);
	
						if (tr.flFraction > 0.99f)
							pTarget->pev->vuser2 = vecSavedCandidate;
					}
				}

				// Must have a break here, otherwise it would cause FS due to infinite loop.
				co_await (gpGlobals->frametime * 2.f);
			}
		}

		co_return;
	}
};

extern "C++" namespace FixedTarget
{
	edict_t *Create(Vector const &vecOrigin, Vector const &vecAngles, CBasePlayer *const pPlayer) noexcept
	{
		auto const pTarget = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));

		pTarget->v.angles = vecAngles;

		g_engfuncs.pfnSetOrigin(pTarget, vecOrigin);
		g_engfuncs.pfnSetModel(pTarget, Models::TARGET);
		g_engfuncs.pfnSetSize(pTarget, Vector::Zero(), Vector::Zero());

		pTarget->v.classname = MAKE_STRING(CFixedTarget::CLASSNAME);
		pTarget->v.solid = SOLID_NOT;
		pTarget->v.movetype = MOVETYPE_NOCLIP;
		pTarget->v.rendermode = kRenderTransAdd;
		pTarget->v.renderfx = kRenderFxDistort;
		pTarget->v.renderamt = 0;
		pTarget->v.skin = Models::targetmdl::SKIN_BLUE;
		pTarget->v.nextthink = 0.1f;
		pTarget->v.euser1 = pPlayer->edict();	// pev->owner was not occupied, but just keep the usage sync with Laser type.
		pTarget->v.team = pPlayer->m_iTeam;

		pTarget->v.vuser1 = vecOrigin;		// Targeting origin input.
		pTarget->v.vuser2 = Vector::Zero();	// Clearing jet spawn origin output.

		TaskScheduler::Enroll(Target::Task_ScanJetSpawn(pTarget));	// This function is cross-usage-allowed.

		return pTarget;
	}

	void Start(CBaseEntity *pTarget) noexcept
	{
		TaskScheduler::Enroll(FixedTarget::Task_RecruitJet(pTarget));
	}

	Task Task_RecruitJet(EHANDLE<CBaseEntity> pFixedTarget) noexcept
	{
		auto const pPlayer = (CBasePlayer *)pFixedTarget->pev->euser1->pvPrivateData; assert(pPlayer != nullptr);
		auto const &vecJetSpawn = pFixedTarget->pev->vuser2;

		if (vecJetSpawn == Vector::Zero())
		{
			g_engfuncs.pfnClientPrintf(pPlayer->edict(), print_center, "The pilot found nowhere to approach your location.");
			pFixedTarget->pev->flags |= FL_KILLME;
			co_return;
		}

		auto const pJet = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));
		auto const vecDir = Vector(pFixedTarget->pev->origin.x, pFixedTarget->pev->origin.y, vecJetSpawn.z) - vecJetSpawn;

		g_engfuncs.pfnVecToAngles(vecDir, pJet->v.angles);
		g_engfuncs.pfnSetOrigin(pJet, vecJetSpawn);
		g_engfuncs.pfnSetModel(pJet, Models::PLANE[AIR_STRIKE]);
		g_engfuncs.pfnSetSize(pJet, Vector::Zero(), Vector::Zero());

		pJet->v.classname = MAKE_STRING(Classname::JET);
		pJet->v.solid = SOLID_NOT;
		pJet->v.movetype = MOVETYPE_NOCLIP;
		pJet->v.velocity = vecDir.Normalize() * 4096;
		pJet->v.euser1 = pPlayer->edict();	// pev->owner was not occupied, but just keep the usage sync with Laser type.
		pJet->v.euser2 = pFixedTarget->edict();

		TaskScheduler::Enroll(Jet::Task_Jet(pJet));

		for (EHANDLE<CBaseEntity> pJetEntity = pJet; pFixedTarget;)
		{
			[[unlikely]]
			if (!pJetEntity)	// Jet found no way to launch missile
			{
				g_engfuncs.pfnClientPrintf(pPlayer->edict(), print_center, "The pilot have no clear sight.");
				pFixedTarget->pev->flags |= FL_KILLME;
				co_return;
			}

			if (pev_valid(pFixedTarget->pev->euser2) == 2)	// Waiting for a missile binding to it.
				break;

			co_await 0.01f;
		}

		for (; pFixedTarget;)
		{
			if (pev_valid(pFixedTarget->pev->euser2) != 2)	// Missile entity despawned.
			{
				pFixedTarget->pev->flags |= FL_KILLME;	// So should this.
				break;
			}

			co_await 0.1f;
		}

		co_return;
	}
};
*/
//
// CDynamicTarget
// Representing the "only I can see" target model when player holding radio.
//

Task CDynamicTarget::Task_Animation() noexcept
{
	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await TaskScheduler::NextFrame::Rank[1];	// behind coord update.

		pev->framerate = float(Models::targetmdl::FPS * gpGlobals->frametime);
		pev->frame += pev->framerate;
		pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pev->frame < 0 || pev->frame >= 256)
			pev->frame -= float((pev->frame / 256.0) * 256.0);	// model sequence is different from SPRITE, no matter now many frame you have, it will stretch/squeeze into 256.
	}
}

Task CDynamicTarget::Task_DeepEvaluation() noexcept
{
	size_t iCounter = 0;
	TraceResult tr{};

	for (;;)
	{
		co_await(gpGlobals->frametime / 3.f);

		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		[[unlikely]]
		if (pev->skin == Models::targetmdl::SKIN_GREEN)
		{
			co_return;	// It's done, stop current evaluation.
		}

		// Try to get a temp spawn location above player.
		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd{ vecSrc.x, vecSrc.y, 8192.f };

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters | ignore_glass, nullptr, &tr);

		if (Vector const vecSavedCandidate = tr.vecEndPos; g_engfuncs.pfnPointContents(vecSavedCandidate) == CONTENTS_SKY)
		{
			g_engfuncs.pfnTraceLine(vecSavedCandidate, pev->origin, ignore_monsters | ignore_glass, nullptr, &tr);

			if (tr.flFraction > 0.99f)
			{
				co_await 0.1f;	// avoid red-green flashing.

				pev->skin = Models::targetmdl::SKIN_GREEN;
				co_return;
			}
		}

		iCounter = 0;

		for (const auto &vec : g_WaypointMgr.m_rgvecOrigins)
		{
			++iCounter;
			g_engfuncs.pfnTraceLine(pev->origin, vec, ignore_monsters | ignore_glass, nullptr, &tr);

			[[unlikely]]
			if (tr.flFraction > 0.99f)
			{
				co_await 0.1f;	// avoid red-green flashing.

				pev->skin = Models::targetmdl::SKIN_GREEN;
				co_return;
			}

			[[unlikely]]
			if (!(iCounter % 256))
			{
				co_await(gpGlobals->frametime / 3.f);	// gurentee resume next frame. div by 3 is to ensure priority
			}
		}
	}
}

Task CDynamicTarget::Task_QuickEvaluation() noexcept
{
	for (;;)
	{
		if (m_pPlayer->m_pActiveItem != m_pRadio || m_pRadio->pev->weapons != RADIO_KEY)
		{
			co_await Models::v_radio::time::draw;	// the model will be hidden for this long, at least.
			continue;
		}

		co_await TaskScheduler::NextFrame::Rank[0];

		// Update team info so we can hide from proper player group.

		pev->team = m_pPlayer->m_iTeam;

		// Calc where does player aiming

		g_engfuncs.pfnMakeVectors(m_pPlayer->pev->v_angle);

		Vector const vecSrc = m_pPlayer->GetGunPosition();
		Vector const vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;

		TraceResult tr{};
		UTIL_TraceLine(vecSrc, vecEnd, m_pPlayer->edict(), m_pPlayer->m_iTeam == TEAM_CT ? g_rgpCTs : g_rgpTers, &tr);

		if (pev_valid(tr.pHit) != 2 && m_flLastValidTracking < gpGlobals->time - 0.5f)	// Compensenting bad aiming
			m_pTargeting = tr.pHit;
		else if (pev_valid(tr.pHit) == 2)
		{
			m_pTargeting = tr.pHit;
			m_flLastValidTracking = gpGlobals->time;
		}

		if (m_pTargeting && !m_pTargeting->IsBSPModel() && m_pTargeting->IsAlive())
		{
			pev->angles = Vector::Zero();	// facing up.

			Vector const vecCenter = m_pTargeting->Center();
			g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));	// attach to target.
		}
		else
		{
			g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pev->angles);
			pev->angles.x += 270.f;	// don't know why, but this is the deal.

			g_engfuncs.pfnSetOrigin(edict(), tr.vecEndPos);
		}

		// Quick Evaluation

		if ((pev->origin - m_vecLastAiming).LengthSquared() > 24.0 * 24.0)
		{
			// Remove old deep think
			m_Scheduler.Delist(DETAIL_ANALYZE_KEY);

			// Is it under sky?
			g_engfuncs.pfnTraceLine(
				pev->origin,
				Vector(pev->origin.x, pev->origin.y, 8192),
				ignore_monsters | ignore_glass,
				nullptr, &tr
			);

			if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
			{
				// Start on deep analyze
				m_Scheduler.Enroll(Task_DeepEvaluation(), DETAIL_ANALYZE_KEY);

				pev->skin = Models::targetmdl::SKIN_RED;
			}
			else
			{
				pev->skin = Models::targetmdl::SKIN_GREEN;
			}

			m_vecLastAiming = pev->origin;
		}
	}
}

Task CDynamicTarget::Task_Remove() noexcept
{
	for (;;)
	{
		co_await gpGlobals->frametime;

		[[unlikely]]
		if (!m_pPlayer->IsAlive()	// Including "disconnection", since client drop out will cause pev->deadflag == DEAD_DEAD
			|| !m_pRadio
			|| !(m_pPlayer->pev->weapons & (1 << WEAPON_NIL))	// for some reason, player no longer hold radio.
			)
		{
			pev->flags |= FL_KILLME;
			co_return;
		}
	}
}

void CDynamicTarget::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), m_pPlayer->pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::TARGET);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->effects |= EF_NODRAW;
	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxPulseFastWide;
	pev->renderamt = 128;
	pev->team = m_pPlayer->m_iTeam;

	m_Scheduler.Enroll(Task_Animation());
	m_Scheduler.Enroll(Task_QuickEvaluation());
	m_Scheduler.Enroll(Task_Remove());
}

CDynamicTarget *CDynamicTarget::Create(CBasePlayer *pPlayer, CBasePlayerWeapon *pRadio) noexcept
{
	auto const pEdict = g_engfuncs.pfnCreateEntity();

	assert(pEdict != nullptr);
	assert(pEdict->pvPrivateData == nullptr);

	auto const pPrefab = new(pEdict) CDynamicTarget;

	pPrefab->pev = &pEdict->v;

	assert(pPrefab->pev != nullptr);
	assert(pEdict->pvPrivateData != nullptr);
	assert(pEdict->v.pContainingEntity == pEdict);

	pEdict->v.classname = MAKE_STRING(CDynamicTarget::CLASSNAME);

	pPrefab->m_pPlayer = pPlayer;
	pPrefab->m_pRadio = pRadio;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}

//
// CFixedTarget
//

Task CFixedTarget::Task_PrepareJetSpawn() noexcept
{
	TraceResult tr{};
	size_t iCounter = 0;

	for (;;)
	{
		[[unlikely]]
		if (m_vecJetSpawn != Vector::Zero())
		{
			co_return;	// Nothing to do here, mate.
		}

		iCounter = 0;

		for (const auto &vec : g_WaypointMgr.m_rgvecOrigins)
		{
			++iCounter;
			g_engfuncs.pfnTraceLine(pev->origin, vec, ignore_monsters | ignore_glass, nullptr, &tr);

			[[unlikely]]
			if (tr.flFraction > 0.99f)
			{
				m_vecJetSpawn = vec;
				co_return;
			}

			[[unlikely]]
			if (!(iCounter % 128))
				co_await TaskScheduler::NextFrame::Rank[0];
		}

		co_await TaskScheduler::NextFrame::Rank[0];

		g_engfuncs.pfnTraceLine(m_vecTempSpawn, Vector(m_vecTempSpawn.x, m_vecTempSpawn.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
		g_engfuncs.pfnTraceLine(pev->origin, tr.vecEndPos, ignore_monsters | ignore_glass, nullptr, &tr);

		[[unlikely]]
		if (tr.flFraction > 0.99f)
		{
			m_vecJetSpawn = tr.vecEndPos;
			co_return;
		}

		g_engfuncs.pfnTraceLine(m_pPlayer->pev->origin, Vector(m_pPlayer->pev->origin.x, m_pPlayer->pev->origin.y, 8192), ignore_monsters | ignore_glass, nullptr, &tr);
		g_engfuncs.pfnTraceLine(pev->origin, tr.vecEndPos, ignore_monsters | ignore_glass, nullptr, &tr);

		[[unlikely]]
		if (tr.flFraction > 0.99f)
		{
			m_vecJetSpawn = tr.vecEndPos;
			co_return;
		}

		co_await TaskScheduler::NextFrame::Rank[0];
	}
}

Task CFixedTarget::Task_RecruitJet() noexcept
{
	co_await TaskScheduler::NextFrame::Rank[1];	// one last chance.

	if (m_vecJetSpawn == Vector::Zero())
	{
		g_engfuncs.pfnClientPrintf(m_pPlayer->edict(), print_center, "The pilot found nowhere to approach your location.");
		pev->flags |= FL_KILLME;
		co_return;
	}

	auto const pJet = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));
	auto const vecDir = Vector(pev->origin.x, pev->origin.y, m_vecJetSpawn.z) - m_vecJetSpawn;

	g_engfuncs.pfnVecToAngles(vecDir, pJet->v.angles);
	g_engfuncs.pfnSetOrigin(pJet, m_vecJetSpawn);
	g_engfuncs.pfnSetModel(pJet, Models::PLANE[AIR_STRIKE]);
	g_engfuncs.pfnSetSize(pJet, Vector::Zero(), Vector::Zero());

	pJet->v.classname = MAKE_STRING(Classname::JET);
	pJet->v.solid = SOLID_NOT;
	pJet->v.movetype = MOVETYPE_NOCLIP;
	pJet->v.velocity = vecDir.Normalize() * 4096;
	pJet->v.euser1 = m_pPlayer->edict();	// pev->owner was not occupied, but just keep the usage sync with Laser type.
	pJet->v.euser2 = edict();

	TaskScheduler::Enroll(Jet::Task_Jet(pJet));

	for (EHANDLE<CBaseEntity> pJetEntity = pJet;;)
	{
		[[unlikely]]
		if (!pJetEntity)	// Jet found no way to launch missile
		{
			g_engfuncs.pfnClientPrintf(m_pPlayer->edict(), print_center, "The pilot have no clear sight.");
			pev->flags |= FL_KILLME;
			co_return;
		}

		[[unlikely]]
		if (m_pMissile)	// Waiting for a missile binding to it.
			break;

		co_await TaskScheduler::NextFrame::Rank[1];
	}

	for (;;)
	{
		if (!m_pMissile)	// Missile entity despawned.
		{
			pev->flags |= FL_KILLME;	// So should this.
			co_return;
		}

		co_await TaskScheduler::NextFrame::Rank[1];
	}

	co_return;
}

Task CFixedTarget::Task_UpdateOrigin() noexcept
{
	for (; m_pTargeting && m_pTargeting->IsAlive();)
	{
		Vector const vecCenter = m_pTargeting->Center();
		g_engfuncs.pfnSetOrigin(edict(), Vector(vecCenter.x, vecCenter.y, m_pTargeting->pev->absmin.z + 1.0));

		co_await TaskScheduler::NextFrame::Rank[1];
	}
}

void CFixedTarget::Spawn() noexcept
{
	g_engfuncs.pfnSetOrigin(edict(), pev->origin);
	g_engfuncs.pfnSetModel(edict(), Models::TARGET);
	g_engfuncs.pfnSetSize(edict(), Vector::Zero(), Vector::Zero());

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;	// Fuck the useless MOVETYPE_FOLLOW
	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxDistort;
	pev->renderamt = 0;
	pev->skin = Models::targetmdl::SKIN_BLUE;
	pev->nextthink = 0.1f;
	pev->team = m_pPlayer->m_iTeam;

	m_vecTempSpawn = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;

	m_Scheduler.Enroll(Task_PrepareJetSpawn());
	m_Scheduler.Enroll(Task_UpdateOrigin());
}

void CFixedTarget::Activate() noexcept
{
	[[likely]]
	if (!m_Scheduler.Exist(RADIO_KEY))
		m_Scheduler.Enroll(Task_RecruitJet(), RADIO_KEY);
}

CFixedTarget *CFixedTarget::Create(Vector const &vecOrigin, Vector const &vecAngles, CBasePlayer *const pPlayer, CBaseEntity *const pTarget) noexcept
{
	auto const pEdict = g_engfuncs.pfnCreateEntity();

	assert(pEdict != nullptr);
	assert(pEdict->pvPrivateData == nullptr);

	auto const pPrefab = new(pEdict) CFixedTarget;

	pPrefab->pev = &pEdict->v;

	assert(pPrefab->pev != nullptr);
	assert(pEdict->pvPrivateData != nullptr);
	assert(pEdict->v.pContainingEntity == pEdict);

	pEdict->v.classname = MAKE_STRING(CFixedTarget::CLASSNAME);
	pEdict->v.angles = vecAngles;
	pEdict->v.origin = vecOrigin;

	pPrefab->m_pTargeting = pTarget;
	pPrefab->m_pPlayer = pPlayer;
	pPrefab->Spawn();
	pPrefab->pev->nextthink = 0.1f;

	return pPrefab;
}
