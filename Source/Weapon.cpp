#include <fmt/core.h>

import <array>;
import <numbers>;

import meta_api;

import Beam;
import Entity;
import Hook;
import Resources;
import Task;

import UtlRandom;

using std::array;

enum EWeaponState
{
	WPNSTATE_USP_SILENCED = (1 << 0),
	WPNSTATE_GLOCK18_BURST_MODE = (1 << 1),
	WPNSTATE_M4A1_SILENCED = (1 << 2),
	WPNSTATE_ELITE_LEFT = (1 << 3),
	WPNSTATE_FAMAS_BURST_MODE = (1 << 4),
	WPNSTATE_SHIELD_DRAWN = (1 << 5),
};

enum EVKnife
{
	KNIFE_IDLE,
	KNIFE_ATTACK1HIT,
	KNIFE_ATTACK2HIT,
	KNIFE_DRAW,
	KNIFE_STABHIT,
	KNIFE_STABMISS,
	KNIFE_MIDATTACK1HIT,
	KNIFE_MIDATTACK2HIT,
};

enum EVShieldKnife
{
	KNIFE_SHIELD_IDLE,
	KNIFE_SHIELD_SLASH,
	KNIFE_SHIELD_ATTACKHIT,
	KNIFE_SHIELD_DRAW,
	KNIFE_SHIELD_UPIDLE,
	KNIFE_SHIELD_UP,
	KNIFE_SHIELD_DOWN,
};

inline constexpr float KNIFE_MAX_SPEED = 250.0f;
inline constexpr float KNIFE_MAX_SPEED_SHIELD = 180.0f;

int HamF_Item_AddToPlayer(CBasePlayerItem *pThis, CBasePlayer *pPlayer) noexcept
{
	g_pfnItemAddToPlayer(pThis, pPlayer);

	if (pPlayer->IsBot())
		return true;

	gmsgWeaponList::Send(ent_cast<edict_t *>(pPlayer->pev),
		HUD::RADIO,
		(byte)-1,
		(byte)-1,
		(byte)-1,
		(byte)-1,
		4,	// slot
		2,	// pos
		WEAPON_NIL,
		0
	);

	pPlayer->pev->weapons |= (1 << WEAPON_NIL);

	gmsgWeapPickup::Send(pPlayer->pev->pContainingEntity, WEAPON_NIL);

	return true;
}

void CreateBeam(CBasePlayerWeapon *pWeapon) noexcept
{
	auto const pBeam = Beam_Create(Sprite::BEAM, 32.f);

	Beam_SetFlags(&pBeam->v, BEAM_FSHADEOUT);	// fade out on rear end.
	Beam_PointsInit(pBeam, pWeapon->m_pPlayer->pev->origin, pWeapon->m_pPlayer->GetGunPosition());

	pBeam->v.classname = MAKE_STRING(Classname::BEAM);
	pBeam->v.effects |= EF_NODRAW;
	pBeam->v.renderfx = kRenderFxNone;
	pBeam->v.nextthink = 0.1f;
	pBeam->v.euser1 = ent_cast<edict_t *>(pWeapon->m_pPlayer->pev);	// pev->owner gets occupied by 'starting ent'

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

int HamF_Item_Deploy(CBasePlayerItem *pItem) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;	// The actual class of this one is ... CKnife, but anyway.

	if (!pThis->pev->euser1)
		CreateBeam(pThis);

	pThis->m_iSwing = 0;
	pThis->m_fMaxSpeed = KNIFE_MAX_SPEED;

	pThis->m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	pThis->m_pPlayer->m_bShieldDrawn = false;

	if (pThis->pev->weapons == RADIO_KEY)
	{
		g_pfnDefaultDeploy(pThis, Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw, "knife", false);	// Enforce to play the anim.

		TimedFnMgr::Enroll(
			[](EHANDLE<CBasePlayerWeapon> pThis) noexcept -> TimedFn
			{
				pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::draw;
				pThis->m_flNextPrimaryAttack = Models::v_radio::time::draw;
				pThis->m_flNextSecondaryAttack = Models::v_radio::time::draw;
				pThis->m_flTimeWeaponIdle = Models::v_radio::time::draw;
				co_await Models::v_radio::time::draw;

				if (!pThis || pThis->m_pPlayer->m_pActiveItem != pThis || pThis->pev->weapons != RADIO_KEY)
					co_return;

				pThis->SendWeaponAnim((int)Models::v_radio::seq::idle, false);
				pThis->pev->euser1->v.effects &= ~EF_NODRAW;
			}(pThis)
		);

		return true;
	}
	else
	{
		pThis->pev->euser1->v.effects |= EF_NODRAW;

		g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pThis->pev), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3f, 2.4f, 0, PITCH_NORM);

		if (pThis->m_pPlayer->m_bOwnsShield)
			return g_pfnDefaultDeploy(pThis, "models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", pThis->UseDecrement() != 0);
		else
			return g_pfnDefaultDeploy(pThis, "models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", pThis->UseDecrement() != 0);
	}

	// #UNDONE clear AS_CARPET_BOMBING src origin
}

void HamF_Item_PostFrame(CBasePlayerItem *pItem) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;

	if (pThis->pev->weapons != RADIO_KEY || !pThis->m_pPlayer || !pThis->m_pPlayer->IsAlive())
		return g_pfnItemPostFrame(pThis);

	[[unlikely]]
	if (pThis->m_pPlayer->m_afButtonPressed & IN_ATTACK)
	{
		TimedFnMgr::Enroll(
			[](EHANDLE<CBasePlayerWeapon> pThis) noexcept -> TimedFn
			{
				pThis->SendWeaponAnim((int)Models::v_radio::seq::use, false);
				pThis->m_pPlayer->m_flNextAttack = Models::v_radio::time::use;
				pThis->m_flNextPrimaryAttack = Models::v_radio::time::use;
				pThis->m_flNextSecondaryAttack = Models::v_radio::time::use;
				pThis->m_flTimeWeaponIdle = Models::v_radio::time::use;
				co_await Models::v_radio::time::use;

				if (!pThis || pThis->m_pPlayer->m_pActiveItem != pThis || pThis->pev->weapons != RADIO_KEY)
					co_return;

				pThis->SendWeaponAnim((int)Models::v_radio::seq::idle, false);
			}(pThis)
		);

		g_engfuncs.pfnMakeVectors(pThis->m_pPlayer->pev->v_angle);

		Vector vecSrc = pThis->m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 4096.f;

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, dont_ignore_monsters, ent_cast<edict_t *>(pThis->m_pPlayer->pev), &tr);

		vecSrc = tr.vecEndPos;
		vecEnd = Vector(vecSrc.x, vecSrc.y, 8192.f);
		g_engfuncs.pfnTraceLine(vecSrc, vecEnd, ignore_monsters, tr.pHit, &tr);

		if (g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
		{
			g_engfuncs.pfnClientPrintf(ent_cast<edict_t *>(pThis->m_pPlayer->pev), print_center, "You must target an outdoor location.");
			return;
		}

		auto const pEdict = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));
		if (pev_valid(&pEdict->v) != 2)
			return;

		g_engfuncs.pfnSetOrigin(pEdict, Vector(tr.vecEndPos.x, tr.vecEndPos.y, tr.vecEndPos.z - 2.5f));
		g_engfuncs.pfnSetModel(pEdict, Models::PROJECTILE[AIR_STRIKE]);
		g_engfuncs.pfnSetSize(pEdict, Vector(-2, -2, -2), Vector(2, 2, 2));

		pEdict->v.classname = MAKE_STRING(Classname::MISSILE);
		pEdict->v.owner = ent_cast<edict_t *>(pThis->m_pPlayer->pev);
		pEdict->v.solid = SOLID_BBOX;
		pEdict->v.movetype = MOVETYPE_FLY;
		pEdict->v.velocity = Vector(0, 0, -1000);
		g_engfuncs.pfnVecToAngles(pEdict->v.velocity, pEdict->v.angles);
		//pEdict->v.groupinfo = MISSILE_GROUPINFO; // #POTENTIAL_BUG
		pEdict->v.nextthink = 0.1f;

		MsgPVS(SVC_TEMPENTITY, tr.vecEndPos);
		WriteData(TE_SPRITE);
		WriteData(tr.vecEndPos);
		WriteData((short)Sprite::m_rgLibrary[Sprite::FIRE]);
		WriteData((byte)5);
		WriteData((byte)255);
		MsgEnd();

		pEdict->v.effects = EF_LIGHT | EF_BRIGHTLIGHT;

		// #INVESTIGATE why won't this work?
		MsgBroadcast(SVC_TEMPENTITY);
		WriteData(TE_BEAMFOLLOW);
		WriteData(ent_cast<short>(pEdict));
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_TRAIL]);
		WriteData((byte)10);
		WriteData((byte)3);
		WriteData((byte)255);
		WriteData((byte)255);
		WriteData((byte)255);
		WriteData((byte)255);
		MsgEnd();

		static constexpr auto get_spherical_coord = [](float radius, float inclination, float azimuth) noexcept
		{
			radius = std::clamp(radius, 0.f, 8192.f);	// r ∈ [0, ∞)
			inclination = (float)std::clamp(inclination * std::numbers::pi / 180.0, 0.0, std::numbers::pi);	// θ ∈ [0, π]
			azimuth = (float)std::clamp(azimuth * std::numbers::pi / 180.0, 0.0, std::numbers::pi * 2.0);	// φ ∈ [0, 2π)

			auto const length = radius * sin(inclination);

			return Vector(
				length * cos(azimuth),
				length * sin(azimuth),
				radius * cos(inclination)
			);
		};

		static constexpr auto get_aim_origin_vector = [](double fwd, double right, double up) noexcept
		{
			return gpGlobals->v_forward * fwd + gpGlobals->v_right * right + gpGlobals->v_up * up;
		};

		g_engfuncs.pfnMakeVectors(pEdict->v.angles);

		auto const qRot = Quaternion::Rotate(Vector(0, 0, 1), gpGlobals->v_forward);

		array const rgvecPericoord =
		{
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 0.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 72.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 144.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 216.f),
			pEdict->v.origin + qRot * get_spherical_coord(24.f, 120.f, 288.f),
		};

		for (auto &&vecPos : rgvecPericoord)
		{
			MsgPVS(SVC_TEMPENTITY, vecPos);
			WriteData(TE_SPRITE);
			WriteData(vecPos);
			WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE]);
			WriteData((byte)10);
			WriteData((byte)50);
			MsgEnd();
		}

		static constexpr auto fnMissileTravelSound = [](EHANDLE<CBaseEntity> pEntity) noexcept -> TimedFn
		{
			for (; pEntity;)
			{
				g_engfuncs.pfnEmitSound(pEntity.Get(), CHAN_WEAPON, Sounds::TRAVEL, VOL_NORM, ATTN_NORM, 0, UTIL_Random(94, 112));
				co_await 1.f;
			}
		};

		TimedFnMgr::Enroll(fnMissileTravelSound(pEdict), ent_cast<unsigned long>(pEdict) + MISSILE_SOUND_CORO_KEY);
	}
}

void HamF_Weapon_PrimaryAttack(CBasePlayerWeapon *pThis) noexcept { return g_pfnWeaponPrimaryAttack(pThis); }
void HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis) noexcept { return g_pfnWeaponSecondaryAttack(pThis); }

void HamF_Item_Holster(CBasePlayerItem *pThis, int skiplocal) noexcept
{
	g_pfnItemHolster(pThis, skiplocal);

	pThis->pev->weapons = 0;

	[[likely]]
	if (pThis->pev->euser1)
		pThis->pev->euser1->v.effects |= EF_NODRAW;
}
