#include <cassert>

import progdefs;
import util;

import Beam;
import Entity;
import Resources;

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

		pWeapon->pev->euser1 = pTarget;
	}

	void Think(CBaseEntity *pEntity) noexcept
	{
		[[unlikely]]
		if (pev_valid(pEntity->pev->euser1) != 2 || !((CBasePlayer *)pEntity->pev->euser1->pvPrivateData)->IsAlive())
		{
			pEntity->pev->flags |= FL_KILLME;
			return;
		}

		pEntity->pev->nextthink = 0.1f;

		if (pEntity->pev->effects & EF_NODRAW)
			return;

		// Calc where does player aiming

		auto const pPlayer = (CBasePlayer *)pEntity->pev->euser1->pvPrivateData;
		g_engfuncs.pfnMakeVectors(pPlayer->pev->v_angle);

		Vector vecSrc = pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;
		TraceResult tr{};

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pEntity->pev->euser1, &tr);

		g_engfuncs.pfnVecToAngles(tr.vecPlaneNormal, pEntity->pev->angles);
		pEntity->pev->angles.x += 270.f;	// don't know why, but this is the deal.

		g_engfuncs.pfnSetOrigin(pEntity->edict(), tr.vecEndPos);

		// Model Animation

		pEntity->pev->framerate = float(Models::targetmdl::FPS * gpGlobals->frametime);
		pEntity->pev->frame += pEntity->pev->framerate;
		pEntity->pev->animtime = gpGlobals->time;

		[[unlikely]]
		if (pEntity->pev->frame < 0 || pEntity->pev->frame >= 256)
			pEntity->pev->frame -= float((pEntity->pev->frame / 256.0) * 256.0);
	}
};

extern "C++" namespace FixedTarget
{
	void Create(Vector const &vecOrigin, Vector const &vecAngles, edict_t* const pPlayer) noexcept
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
		pTarget->v.nextthink = 0.1f;
		pTarget->v.euser1 = pPlayer;	// pev->owner was not occupied, but just keep the usage sync with Laser type.

		TimedFnMgr::Enroll(FixedTarget::Think(pTarget));
	}

	TimedFn Think(EHANDLE<CBaseEntity> pTarget) noexcept
	{
		auto const pPlayer = (CBasePlayer *)pTarget->pev->euser1->pvPrivateData; assert(pPlayer != nullptr);

		Vector const vecSrc = pPlayer->GetGunPosition();
		Vector const vecEnd{ vecSrc.x, vecSrc.y, 8192.f };
		TraceResult tr{};

		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, pPlayer->edict(), &tr);

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
		{
			g_engfuncs.pfnClientPrintf(pPlayer->edict(), print_center, "No valid jet spawn point found.");
			pTarget->pev->flags |= FL_KILLME;
			co_return;
		}

		auto const pJet = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));

		Vector const vecDir = Vector(pTarget->pev->origin.x, pTarget->pev->origin.y, tr.vecEndPos.z) - tr.vecEndPos;

		g_engfuncs.pfnVecToAngles(vecDir, pJet->v.angles);
		g_engfuncs.pfnSetOrigin(pJet, tr.vecEndPos);
		g_engfuncs.pfnSetModel(pJet, Models::PLANE[AIR_STRIKE]);
		g_engfuncs.pfnSetSize(pJet, Vector::Zero(), Vector::Zero());

		pJet->v.classname = MAKE_STRING(Classname::JET);
		pJet->v.solid = SOLID_NOT;
		pJet->v.movetype = MOVETYPE_NOCLIP;
		pJet->v.velocity = vecDir.Normalize() * 4096;
		pJet->v.euser1 = pPlayer->edict();	// pev->owner was not occupied, but just keep the usage sync with Laser type.
		pJet->v.euser2 = pTarget->edict();

		TimedFnMgr::Enroll(Jet::Think(pJet));

		for (; pTarget;)
		{
			if (pev_valid(pTarget->pev->euser2) == 2)	// Waiting for a missile binding to it.
				break;

			co_await 0.01f;
		}

		for (; pTarget;)
		{
			if (pev_valid(pTarget->pev->euser2) != 2)	// Missile entity despawned.
			{
				pTarget->pev->flags |= FL_KILLME;	// So should this.
				break;
			}

			co_await 0.1f;
		}

		co_return;
	}
};
