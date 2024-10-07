export module Transpiler;

export struct Transpiler_t
{
	int		(*pfnPrecacheModel)		(const char *s);
	int		(*pfnPrecacheSound)		(const char *s);
	int		(*pfnPrecacheGeneric)	(const char *s);
	int		(*pfnDecalIndex)		(const char *s);
};

export inline Transpiler_t g_engfuncs;
