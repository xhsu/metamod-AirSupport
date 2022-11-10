#include <fmt/core.h>

import Entity;
import Hook;
import Resources;

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

int HamF_Item_Deploy(CBasePlayerItem *pItem) noexcept
{
	auto const pThis = (CBasePlayerWeapon *)pItem;	// The actual class of this one is ... CKnife, but anyway.

	pThis->m_iSwing = 0;
	pThis->m_fMaxSpeed = KNIFE_MAX_SPEED;

	pThis->m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	pThis->m_pPlayer->m_bShieldDrawn = false;

	if (pThis->pev->weapons == RADIO_KEY)
		return g_pfnDefaultDeploy(pThis, Models::V_RADIO, Models::P_RADIO, (int)Models::v_radio::seq::draw, "knife", false);	// Enforce to play the anim.
	else
	{
		g_engfuncs.pfnEmitSound(ent_cast<edict_t *>(pThis->pev), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3f, 2.4f, 0, PITCH_NORM);

		if (pThis->m_pPlayer->m_bOwnsShield)
			return g_pfnDefaultDeploy(pThis, "models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_SHIELD_DRAW, "shieldknife", pThis->UseDecrement() != 0);
		else
			return g_pfnDefaultDeploy(pThis, "models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", pThis->UseDecrement() != 0);
	}

	// #UNDONE clear AS_CARPET_BOMBING src origin
}

void HamF_Item_PostFrame(CBasePlayerItem *pThis) noexcept
{
	if (pThis->pev->weapons != RADIO_KEY || !pThis->m_pPlayer || !pThis->m_pPlayer->IsAlive())
		return g_pfnItemPostFrame(pThis);

	[[unlikely]]
	if (pThis->m_pPlayer->m_afButtonPressed & IN_ATTACK)
	{
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
		pEdict->v.groupinfo = MISSILE_GROUPINFO;

		MsgPVS(SVC_TEMPENTITY, tr.vecEndPos);
		WriteData(TE_SPRITE);
		WriteData(tr.vecEndPos);
		WriteData((short)Sprite::m_rgLibrary[Sprite::FIRE]);
		WriteData((byte)5);
		WriteData((byte)255);
		MsgEnd();

		pEdict->v.effects = EF_LIGHT | EF_BRIGHTLIGHT;

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
	}
}

void HamF_Weapon_PrimaryAttack(CBasePlayerWeapon *pThis) noexcept { return g_pfnWeaponPrimaryAttack(pThis); }
void HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis) noexcept { return g_pfnWeaponSecondaryAttack(pThis); }
void HamF_Item_Holster(CBasePlayerItem *pThis, int skiplocal) noexcept { return g_pfnItemHolster(pThis, skiplocal); }
