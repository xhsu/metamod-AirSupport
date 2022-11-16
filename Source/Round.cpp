import util;

import Entity;
import GameRules;
import Hook;

void OrpheuF_CleanUpMap(CHalfLifeMultiplay *pThis) noexcept
{
	g_pfnCleanUpMap(pThis);

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::FIXED_TARGET))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::JET))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::MISSILE))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::CFLAME))
		pEnt->v.flags |= FL_KILLME;

	for (auto &&pEnt : FIND_ENTITY_BY_CLASSNAME(Classname::CSMOKE))
		pEnt->v.flags |= FL_KILLME;
}
