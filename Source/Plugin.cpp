import <cstring>;

import <concepts>;

import eiface;
import engine_api;
import Plugin;

// Hook.cpp
extern int HookEngineAPI(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion) noexcept;
extern int HookEngineAPI_Post(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion) noexcept;
extern int HookGameDLLNewFn(NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept;
extern int HookGameDLLExportedFn(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept;
extern int HookGameDLLExportedFn_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept;
//

// From SDK dlls/h_export.cpp:

//! Holds engine functionality callbacks
inline enginefuncs_t g_engfuncs = {};
inline globalvars_t *gpGlobals = nullptr;

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we do some setup operations here.
void __stdcall GiveFnptrsToDll(enginefuncs_t *pengfuncsFromEngine, globalvars_t *pGlobals) noexcept
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}

// Must provide at least one of these..
inline constexpr META_FUNCTIONS gMetaFunctionTable =
{
	.pfnGetEntityAPI			= nullptr,						// HL SDK; called before game DLL
	.pfnGetEntityAPI_Post		= nullptr,						// META; called after game DLL
	.pfnGetEntityAPI2			= &HookGameDLLExportedFn,		// HL SDK2; called before game DLL
	.pfnGetEntityAPI2_Post		= &HookGameDLLExportedFn_Post,	// META; called after game DLL
	.pfnGetNewDLLFunctions		= HookGameDLLNewFn,				// HL SDK2; called before game DLL
	.pfnGetNewDLLFunctions_Post	= nullptr,						// META; called after game DLL
	.pfnGetEngineFunctions		= &HookEngineAPI,				// META; called before HL engine
	.pfnGetEngineFunctions_Post	= &HookEngineAPI_Post,			// META; called after HL engine
};

// Metamod requesting info about this plugin:
//  ifvers			(given) interface_version metamod is using
//  pPlugInfo		(requested) struct with info about plugin
//  pMetaUtilFuncs	(given) table of utility functions provided by metamod
int Meta_Query(const char *pszInterfaceVersion, plugin_info_t const **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs) noexcept
{
	*pPlugInfo = &gPluginInfo;
	gpMetaUtilFuncs = pMetaUtilFuncs;

	return true;
}
static_assert(std::same_as<decltype(&Meta_Query), META_QUERY_FN>);

// Metamod attaching plugin to the server.
//  now				(given) current phase, ie during map, during changelevel, or at startup
//  pFunctionTable	(requested) table of function tables this plugin catches
//  pMGlobals		(given) global vars from metamod
//  pGamedllFuncs	(given) copy of function tables from game dll
int Meta_Attach(PLUG_LOADTIME iCurrentPhase, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) noexcept
{
	if (!pMGlobals) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'Meta_Attach' called with null 'pMGlobals' parameter.");
		return false;
	}

	gpMetaGlobals = pMGlobals;

	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'Meta_Attach' called with null 'pFunctionTable' parameter.");
		return false;
	}

	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
	gpGamedllFuncs = pGamedllFuncs;
	return true;
}
static_assert(std::same_as<decltype(&Meta_Attach), META_ATTACH_FN>);

// Metamod detaching plugin from the server.
// now		(given) current phase, ie during map, etc
// reason	(given) why detaching (refresh, console unload, forced unload, etc)
int Meta_Detach(PLUG_LOADTIME iCurrentPhase, PL_UNLOAD_REASON iReason) noexcept
{
	return true;
}
static_assert(std::same_as<decltype(&Meta_Detach), META_DETACH_FN>);
