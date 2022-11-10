import meta_api;

import CBase;
import Entity;
import Message;
import Resources;

import UtlRandom;

META_RES OnThink(CBaseEntity *pEntity) noexcept
{
	[[unlikely]]
	if (pEntity->pev->classname != MAKE_STRING(Classname::MISSILE))
		return MRES_IGNORED;

	pEntity->pev->nextthink = gpGlobals->time + UTIL_Random(0.015f, 0.05f);

	pEntity->pev->angles += Vector(
		UTIL_Random(-0.35f, 0.35f),
		UTIL_Random(-0.35f, 0.35f),
		UTIL_Random(-0.35f, 0.35f)
	);

	g_engfuncs.pfnMakeVectors(pEntity->pev->angles);

	//pEntity->pev->velocity = gpGlobals->v_forward * 1000;

	Vector vecOrigin = pEntity->pev->origin + gpGlobals->v_forward * -100;

	MsgPVS(SVC_TEMPENTITY, vecOrigin);
	WriteData(TE_SPRITE);
	WriteData(vecOrigin);
	WriteData((short)Sprite::m_rgLibrary[Sprite::FIRE2]);
	WriteData((byte)3);
	WriteData((byte)255);
	MsgEnd();

	MsgPVS(SVC_TEMPENTITY, vecOrigin);
	WriteData(TE_SPRITE);
	WriteData(vecOrigin);

	switch (UTIL_Random(0, 2))
	{
	case 0:
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE]);
		break;

	case 1:
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_1]);
		break;

	default:
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_2]);
		break;
	}

	WriteData((byte)UTIL_Random<short>(1, 10));
	WriteData((byte)UTIL_Random<short>(50, 255));
	MsgEnd();

	return MRES_HANDLED;
}