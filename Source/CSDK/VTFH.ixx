export module VTFH;

export import hlsdk;

export import CBase;
export import Platform;

import UtlHook;

export inline constexpr size_t VFTIDX_CBASE_TRACEATTACK = 11;
export inline constexpr size_t VFTIDX_CBASE_TAKEDAMAGE = 12;
export inline constexpr size_t VFTIDX_CBASE_KILLED = 14;
export inline constexpr size_t VFTIDX_CBASE_TRACEBLEED = 16;
export inline constexpr size_t VFTIDX_CBASE_DAMAGEDECAL = 29;
export inline constexpr size_t VFTIDX_CBASE_GETNEXTTARGET = 43;
export inline constexpr size_t VFTIDX_CBASE_TOUCH = 45;

export using fnEntityTraceAttack_t = void(__thiscall*)(CBaseEntity*, entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) noexcept;
export using fnEntityTakeDamage_t = qboolean(__thiscall*)(CBaseEntity*, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) noexcept;
export using fnEntityKilled_t = void(__thiscall*)(CBaseEntity*, entvars_t* pevAttacker, int iGib) noexcept;
export using fnEntityTraceBleed_t = void(__thiscall*)(CBaseEntity*, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) noexcept;
export using fnEntityDamageDecal_t = int(__thiscall*)(CBaseEntity*, int bitsDamageType) noexcept;
export using fnEntityGetNextTarget_t = CBaseEntity * (__thiscall*)(CBaseEntity*) noexcept;
export using fnEntityTouch_t = void(__thiscall*)(CBaseEntity*, CBaseEntity*) noexcept;

export inline fnEntityTraceAttack_t g_pfnEntityTraceAttack = nullptr;
export inline fnEntityTakeDamage_t g_pfnEntityTakeDamage = nullptr;
export inline fnEntityKilled_t g_pfnEntityKilled = nullptr;
export inline fnEntityTraceBleed_t g_pfnEntityTraceBleed = nullptr;
export inline fnEntityDamageDecal_t g_pfnEntityDamageDecal = nullptr;
export inline fnEntityGetNextTarget_t g_pfnEntityGetNextTarget = nullptr;

export void RetrieveCBaseVirtualFn() noexcept
{
	auto const pEnt = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));	// Technically this is not CBaseEntity, but it is the closest one. It overrides Spawn() and ObjectCaps(), so it is still pure enough.

	[[unlikely]]
	if (!pEnt || !pEnt->pvPrivateData)
	{
		if (pEnt)
			g_engfuncs.pfnRemoveEntity(pEnt);

		UTIL_Terminate("Failed to retrieve classtype for \"info_target\".");
	}

	g_pfnEntityTraceAttack = (fnEntityTraceAttack_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_TRACEATTACK);
	g_pfnEntityTakeDamage = (fnEntityTakeDamage_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_TAKEDAMAGE);
	g_pfnEntityKilled = (fnEntityKilled_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_KILLED);
	g_pfnEntityTraceBleed = (fnEntityTraceBleed_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_TRACEBLEED);
	g_pfnEntityDamageDecal = (fnEntityDamageDecal_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_DAMAGEDECAL);
	g_pfnEntityGetNextTarget = (fnEntityGetNextTarget_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_GETNEXTTARGET);

	g_engfuncs.pfnRemoveEntity(pEnt);
}
