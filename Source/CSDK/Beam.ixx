export module Beam;

import <algorithm>;

/* AMX Mod X
*    Beam entities include by KORD_12.7.
*
* Version 1.3 (last update: 4, may (05), 2013)
*
* http://aghl.ru/forum/ - Russian Half-Life and Adrenaline Gamer Community
*
* This file is provided as is (no warranties)
*/

// These functions are here to show the way beams are encoded as entities.
// Encoding beams as entities simplifies their management in the client/server architecture.

export import const_;
export import customentity;	// Beam types & flags
export import eiface;
import util;


/* stock Beam_SetType(const iBeamEntity, const iType)
	return set_pev(iBeamEntity, pev_rendermode, (pev(iBeamEntity, pev_rendermode) & 0xF0) | iType & 0x0F); */
export inline void Beam_SetType(entvars_t *pev, EBeamTypes iType) noexcept { pev->rendermode = (pev->rendermode & 0xF0 | iType & 0x0F); }

/* stock Beam_SetFlags(const iBeamEntity, const iType)
	return set_pev(iBeamEntity, pev_rendermode, (pev(iBeamEntity, pev_rendermode) & 0x0F) | iType & 0xF0); */
export inline void Beam_SetFlags(entvars_t *pev, EBeamFlags iType) noexcept { pev->rendermode = (pev->rendermode & 0x0F | iType & 0xF0); }

/* stock Beam_SetStartPos(const iBeamEntity, const Float: flVecStart[3])
	return set_pev(iBeamEntity, pev_origin, flVecStart); */
export inline void Beam_SetStartPos(entvars_t *pev, const Vector &vecSrc) noexcept { pev->origin = vecSrc; }

/* stock Beam_SetEndPos(const iBeamEntity, const Float: flVecEnd[3])
	return set_pev(iBeamEntity, pev_angles, flVecEnd); */
export inline void Beam_SetEndPos(entvars_t *pev, const Vector &vecEnd) noexcept { pev->angles = vecEnd; }

/* #define Beam_SetStartEntity(%0,%1) \
	set_pev(%0, pev_sequence, (%1 & 0x0FFF) | ((pev(%0, pev_sequence) & 0xF000) << 12)); \
	set_pev(%0, pev_owner, %1) */
/* stock Beam_SetStartEntity(const iBeamEntity, const iEntityIndex) */
export inline void Beam_SetStartEntity(entvars_t *pev, short iEntIndex) noexcept
{
	pev->sequence = (iEntIndex & 0x0FFF) | ((pev->sequence & 0xF000) << 12);
	pev->owner = g_engfuncs.pfnPEntityOfEntIndex(iEntIndex);
}

/* #define Beam_SetEndEntity(%0,%1) \
	set_pev(%0, pev_skin, (%1 & 0x0FFF) | ((pev(%0, pev_skin) & 0xF000) << 12)); \
	set_pev(%0, pev_aiment, %1) */
/* stock Beam_SetEndEntity(const iBeamEntity, const iEntityIndex) */
export inline void Beam_SetEndEntity(entvars_t *pev, short iEntIndex) noexcept
{
	pev->skin = (iEntIndex & 0x0FFF) | ((pev->sequence & 0xF000) << 12);
	pev->aiment = g_engfuncs.pfnPEntityOfEntIndex(iEntIndex);
}

/* stock Beam_SetStartAttachment(const iBeamEntity, const iAttachment)
	return set_pev(iBeamEntity, pev_sequence, (pev(iBeamEntity, pev_sequence) & 0x0FFF) | ((iAttachment & 0xF) << 12)); */
export inline void Beam_SetStartAttachment(entvars_t *pev, int iAttachment) noexcept { pev->sequence = pev->sequence & 0x0FFF | ((iAttachment & 0xF) << 12); }

/* stock Beam_SetEndAttachment(const iBeamEntity, const iAttachment)
	return set_pev(iBeamEntity, pev_skin, (pev(iBeamEntity, pev_skin) & 0x0FFF) | ((iAttachment & 0xF) << 12)); */
export inline void Beam_SetEndAttachment(entvars_t *pev, int iAttachment) noexcept { pev->skin = pev->skin & 0x0FFF | ((iAttachment & 0xF) << 12); }

/* stock Beam_SetTexture(const iBeamEntity, const iSpriteIndex)
	return set_pev(iBeamEntity, pev_modelindex, iSpriteIndex); */
export inline void Beam_SetTexture(entvars_t *pev, int iSpriteIndex) noexcept { pev->modelindex = iSpriteIndex; }

/* stock Beam_SetWidth(const iBeamEntity, const Float: flWidth)
	return set_pev(iBeamEntity, pev_scale, flWidth); */
export inline void Beam_SetWidth(entvars_t *pev, float flWidth) noexcept { pev->scale = flWidth; }

/* stock Beam_SetNoise(const iBeamEntity, const iNoise)
	return set_pev(iBeamEntity, pev_body, iNoise); */
export inline void Beam_SetNoise(entvars_t *pev, int iNoise) noexcept { pev->body = iNoise; }

/* stock Beam_SetColor(const iBeamEntity, const Float: flColor[3])
	return set_pev(iBeamEntity, pev_rendercolor, flColor); */
export inline void Beam_SetColor(entvars_t *pev, const Vector &flColor) noexcept { pev->rendercolor = flColor; }

/* stock Beam_SetBrightness(const iBeamEntity, const Float: flBrightness)
	return set_pev(iBeamEntity, pev_renderamt, flBrightness); */
export inline void Beam_SetBrightness(entvars_t *pev, float flBrightness) noexcept { pev->renderamt = flBrightness; }

/* stock Beam_SetFrame(const iBeamEntity, const Float: flFrame)
	return set_pev(iBeamEntity, pev_frame, flFrame); */
export inline void Beam_SetFrame(entvars_t *pev, float flFrame) noexcept { pev->frame = flFrame; }

/* stock Beam_SetScrollRate(const iBeamEntity, const Float: flSpeed)
	return set_pev(iBeamEntity, pev_animtime, flSpeed); */
export inline void Beam_SetScrollRate(entvars_t *pev, float flSpeed) noexcept { pev->animtime = flSpeed; }

/* stock Beam_GetType(const iBeamEntity)
	return pev(iBeamEntity, pev_rendermode) & 0x0F; */
export inline EBeamTypes Beam_GetType(entvars_t *pev) noexcept { return static_cast<EBeamTypes>(pev->rendermode & 0x0F); }

/* stock Beam_GetFlags(const iBeamEntity)
	return pev(iBeamEntity, pev_rendermode) & 0xF0; */
export inline EBeamFlags Beam_GetFlags(entvars_t *pev) noexcept { return static_cast<EBeamFlags>(pev->rendermode & 0xF0); }

/* stock Beam_GetStartEntity(const iBeamEntity)
	return pev(iBeamEntity, pev_sequence) & 0xFFF; */
export inline auto Beam_GetStartEntity(entvars_t *pev) noexcept { return pev->sequence & 0xFFF; }

/* stock Beam_GetEndEntity(const iBeamEntity)
	return pev(iBeamEntity, pev_skin) & 0xFFF; */
export inline auto Beam_GetEndEntity(entvars_t *pev) noexcept { return pev->skin & 0xFFF; }

/*
stock Beam_GetStartPos(const iBeamEntity, Float: vecStartPos[3])
{
	static iEntity; iEntity = Beam_GetStartEntity(iBeamEntity);

	if (Beam_GetType(iBeamEntity) == BEAM_ENTS && pev_valid(iEntity))
	{
		pev(iEntity, pev_origin, vecStartPos);
		return;
	}

	pev(iBeamEntity, pev_origin, vecStartPos);
}*/
export inline Vector &Beam_GetStartPos(entvars_t *pev) noexcept
{
	auto const iStartEnt = Beam_GetStartEntity(pev);

	if (auto const pevStartEnt = ent_cast<entvars_t *>(iStartEnt); pev_valid(pevStartEnt) == 2 && Beam_GetType(pevStartEnt) == BEAM_ENTS)
	{
		return pevStartEnt->origin;
	}

	return pev->origin;
}

/*
stock Beam_GetEndPos(const iBeamEntity, Float: vecEndPos[3])
{
	static iType;
	static iEntity;

	iType = Beam_GetType(iBeamEntity);

	if (iType == BEAM_POINTS || iType == BEAM_HOSE)
	{
		pev(iBeamEntity, pev_angles, vecEndPos);
		return;
	}

	iEntity = Beam_GetEndEntity(iBeamEntity);

	if (pev_valid(iEntity))
	{
		pev(iEntity, pev_origin, vecEndPos);
		return;
	}

	pev(iBeamEntity, pev_angles, vecEndPos);
}*/
export inline Vector &Beam_GetEndPos(entvars_t *pev) noexcept
{
	auto const iType = Beam_GetType(pev);

	switch (iType)
	{
	case BEAM_POINTS:
	case BEAM_HOSE:
		return pev->angles;

	default:
		auto const iEndEnt = Beam_GetEndEntity(pev);

		if (auto const pevEndEnt = ent_cast<entvars_t *>(iEndEnt); pev_valid(pevEndEnt) == 2)
			return pevEndEnt->origin;

		return pev->angles;
	}
}

/* stock Beam_GetTexture(const iBeamEntity)
	return pev(iBeamEntity, pev_modelindex); */
export inline int &Beam_GetTexture(entvars_t *pev) noexcept { return pev->modelindex; }

/* stock Float: Beam_GetWidth(const iBeamEntity)
	return entity_get_float(iBeamEntity, EV_FL_scale); */
export inline float &Beam_GetWidth(entvars_t *pev) noexcept { return pev->scale; }

/* stock Beam_GetNoise(const iBeamEntity)
	return pev(iBeamEntity, pev_body); */
export inline int &Beam_GetNoise(entvars_t *pev) noexcept { return pev->body; }

/* stock Beam_GetColor(const iBeamEntity, Float: flRGB[3])
	return pev(iBeamEntity, pev_rendercolor, flRGB); */
export inline Vector &Beam_GetColor(entvars_t *pev) noexcept { return pev->rendercolor; }

/* stock Float: Beam_GetBrightness(const iBeamEntity)
	return entity_get_float(iBeamEntity, EV_FL_renderamt); */
export inline float &Beam_GetBrightness(entvars_t *pev) noexcept { return pev->renderamt; }

/* stock Float: Beam_GetFrame(const iBeamEntity)
	return entity_get_float(iBeamEntity, EV_FL_frame); */
export inline float &Beam_GetFrame(entvars_t *pev) noexcept { return pev->frame; }

/* stock Float: Beam_GetScrollRate(const iBeamEntity)
	return entity_get_float(iBeamEntity, EV_FL_animtime); */
export inline float &Beam_GetScrollRate(entvars_t *pev) noexcept { return pev->animtime; }

// Pre-declaration
extern inline void Beam_Init(edict_t *pEdict, const char *pszSpriteName, float flWidth) noexcept;
extern inline void Beam_RelinkBeam(edict_t *pEdict) noexcept;
//

export inline edict_t *Beam_Create(const char *pszSpriteName, float flWidth) noexcept
{
	auto const pEntity = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("beam"));

	if (pev_valid(pEntity) != 2)
		return nullptr;

	Beam_Init(pEntity, pszSpriteName, flWidth);
	return pEntity;
}

export inline void Beam_Init(edict_t *pEdict, const char *pszSpriteName, float flWidth) noexcept
{
	pEdict->v.flags |= FL_CUSTOMENTITY;

	Beam_SetColor(&pEdict->v, Vector(255, 255, 255));
	Beam_SetBrightness(&pEdict->v, 255.0);
	Beam_SetNoise(&pEdict->v, 0);
	Beam_SetFrame(&pEdict->v, 0.0);
	Beam_SetScrollRate(&pEdict->v, 0.0);
	Beam_SetWidth(&pEdict->v, flWidth);

	g_engfuncs.pfnSetModel(pEdict, pszSpriteName);

	pEdict->v.skin = 0;
	pEdict->v.sequence = 0;
	pEdict->v.rendermode = 0;
}

export inline void Beam_PointsInit(edict_t *pEdict, const Vector& vecSrc, const Vector& vecEnd) noexcept
{
	Beam_SetType(&pEdict->v, BEAM_POINTS);
	Beam_SetStartPos(&pEdict->v, vecSrc);
	Beam_SetEndPos(&pEdict->v, vecEnd);
	Beam_SetStartAttachment(&pEdict->v, 0);
	Beam_SetEndAttachment(&pEdict->v, 0);
	Beam_RelinkBeam(pEdict);
}

export inline void Beam_HoseInit(edict_t *pEdict, const Vector &vecSrc, const Vector &vecDir) noexcept
{
	Beam_SetType(&pEdict->v, BEAM_HOSE);
	Beam_SetStartPos(&pEdict->v, vecSrc);
	Beam_SetEndPos(&pEdict->v, vecDir);
	Beam_SetStartAttachment(&pEdict->v, 0);
	Beam_SetEndAttachment(&pEdict->v, 0);
	Beam_RelinkBeam(pEdict);
}

export inline void Beam_PointEntInit(edict_t *pEdict, const Vector &vecSrc, short iEndIndex) noexcept
{
	Beam_SetType(&pEdict->v, BEAM_ENTPOINT);
	Beam_SetStartPos(&pEdict->v, vecSrc);
	Beam_SetEndEntity(&pEdict->v, iEndIndex);
	Beam_SetStartAttachment(&pEdict->v, 0);
	Beam_SetEndAttachment(&pEdict->v, 0);
	Beam_RelinkBeam(pEdict);
}

export inline void Beam_EntsInit(edict_t *pEdict, short iStartIndex, short iEndIndex) noexcept
{
	Beam_SetType(&pEdict->v, BEAM_ENTS);
	Beam_SetStartEntity(&pEdict->v, iStartIndex);
	Beam_SetEndEntity(&pEdict->v, iEndIndex);
	Beam_SetStartAttachment(&pEdict->v, 0);
	Beam_SetEndAttachment(&pEdict->v, 0);
	Beam_RelinkBeam(pEdict);
}

export inline void Beam_RelinkBeam(edict_t *pEdict) noexcept
{
	Vector &vecSrc = Beam_GetStartPos(&pEdict->v);
	Vector &vecEnd = Beam_GetEndPos(&pEdict->v);

	pEdict->v.mins.x = std::min(vecSrc.x, vecEnd.x);
	pEdict->v.mins.y = std::min(vecSrc.y, vecEnd.y);
	pEdict->v.mins.z = std::min(vecSrc.z, vecEnd.z);

	pEdict->v.maxs.x = std::max(vecSrc.x, vecEnd.x);
	pEdict->v.maxs.y = std::max(vecSrc.y, vecEnd.y);
	pEdict->v.maxs.z = std::max(vecSrc.z, vecEnd.z);

	pEdict->v.mins = pEdict->v.mins - pEdict->v.origin;
	pEdict->v.maxs = pEdict->v.maxs - pEdict->v.origin;

	g_engfuncs.pfnSetSize(pEdict, pEdict->v.mins, pEdict->v.maxs);
	g_engfuncs.pfnSetOrigin(pEdict, pEdict->v.origin);
}
