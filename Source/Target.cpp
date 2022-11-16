#include <cassert>

import progdefs;
import util;

import Beam;
import Entity;
import Resources;
import Waypoint;

import UtlRandom;

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

		pTarget->v.classname = MAKE_STRING(Classname::AIM);
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

		pTarget->v.classname = MAKE_STRING(Classname::FIXED_TARGET);
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
