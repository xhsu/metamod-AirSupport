export module Enforcer;

export struct Enforcer_t
{
	int		(*pfnPrecacheModel)		(const char* s);
	int		(*pfnPrecacheSound)		(const char* s);
	int		(*pfnDecalIndex)		(const char* s);
};

export inline Enforcer_t g_engfuncs;
