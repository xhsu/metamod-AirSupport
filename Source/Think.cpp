import meta_api;

import Entity;

META_RES OnThink(CBaseEntity *pEntity) noexcept
{
	[[unlikely]]
	if (pEntity->pev->classname == MAKE_STRING(Classname::MISSILE))
	{
		Missile::Think(pEntity);
		return MRES_HANDLED;
	}
	else if (pEntity->pev->classname == MAKE_STRING(Classname::BEAM))
	{
		Laser::Think(pEntity);
		return MRES_HANDLED;
	}
	else if (pEntity->pev->classname == MAKE_STRING(Classname::AIM))
	{
		Target::Think(pEntity);
		return MRES_HANDLED;
	}
	//else if (pEntity->pev->classname == MAKE_STRING(Classname::FIXED_TARGET))
	//{
	//	pEntity->pev->rendermode = kRenderTransAdd;
	//	pEntity->pev->renderfx = kRenderFxDistort;
	//	pEntity->pev->renderamt = 128;
	//	pEntity->pev->nextthink = 0.1f;
	//	return MRES_HANDLED;
	//}

	return MRES_IGNORED;
}
