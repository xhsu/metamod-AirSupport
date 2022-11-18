#include <cassert>

import <cstring>;

import <string>;

import progdefs;	// edict_t
import util;

import UtlHook;

import Entity;
import GameRules;
import Hook;
import Plugin;
import Prefab;
import Task;

// Resources.cpp
extern void Precache(void) noexcept;
//

// Weapon.cpp
extern int HamF_Item_AddToPlayer(CBasePlayerItem *pThis, CBasePlayer *pPlayer) noexcept;
extern int HamF_Item_Deploy(CBasePlayerItem *pThis) noexcept;
extern void HamF_Item_PostFrame(CBasePlayerItem *pThis) noexcept;
extern void HamF_Weapon_PrimaryAttack(CBasePlayerWeapon *pThis) noexcept;
extern void HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis) noexcept;
extern qboolean HamF_Item_CanHolster(CBasePlayerItem *pThis) noexcept;
extern void HamF_Item_Holster(CBasePlayerItem *pThis, int skiplocal) noexcept;
extern qboolean SwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) noexcept;
extern void SelectItem(CBasePlayer *pPlayer, const char *pstr) noexcept;
//

// Round.cpp
extern void OrpheuF_CleanUpMap(CHalfLifeMultiplay *pThis) noexcept;
//

// Waypoint.cpp
extern Task Waypoint_Scan(void) noexcept;
extern void Waypoint_Read(void) noexcept;
//

inline bool g_bShouldPrecache = true;

void DeployHooks(void) noexcept
{
	static bool bHooksPerformed = false;

	[[likely]]
	if (bHooksPerformed)
		return;

	edict_t *pEnt = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("weapon_knife"));

	if (!pEnt || !pEnt->pvPrivateData) [[unlikely]]
	{
		if (pEnt)
			g_engfuncs.pfnRemoveEntity(pEnt);

		LOG_ERROR("Failed to retrieve classtype for \"weapon_knife\".");
		return;
	}

	auto const rgpfnCKnife = UTIL_RetrieveVirtualFunctionTable(pEnt->pvPrivateData);

	g_engfuncs.pfnRemoveEntity(pEnt);
	pEnt = nullptr;

	if (rgpfnCKnife == nullptr) [[unlikely]]
	{
		LOG_ERROR("Failed to retrieve vtable for \"weapon_knife\".");
		return;
	}

	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_ADDTOPLAYER, UTIL_CreateTrampoline(true, 1, &HamF_Item_AddToPlayer), (void **)&g_pfnItemAddToPlayer);
	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_DEPLOY, UTIL_CreateTrampoline(true, 0, &HamF_Item_Deploy), (void **)&g_pfnItemDeploy);
	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_POSTFRAME, UTIL_CreateTrampoline(true, 0, &HamF_Item_PostFrame), (void **)&g_pfnItemPostFrame);
//	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_WEAPON_PRIMARYATTACK, UTIL_CreateTrampoline(true, 0, &HamF_Weapon_PrimaryAttack), (void **)&g_pfnWeaponPrimaryAttack);
//	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_WEAPON_SECONDARYATTACK, UTIL_CreateTrampoline(true, 0, &HamF_Weapon_SecondaryAttack), (void **)&g_pfnWeaponSecondaryAttack);
//	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_CANHOLSTER, UTIL_CreateTrampoline(true, 0, &HamF_Item_CanHolster), (void **)&g_pfnItemCanHolster);
	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_HOLSTER, UTIL_CreateTrampoline(true, 1, &HamF_Item_Holster), (void **)&g_pfnItemHolster);

	g_pfnRadiusFlash = (fnRadiusFlash_t)UTIL_SearchPattern("mp.dll", RADIUS_FLASH_FN_PATTERN, 1);
	g_pfnSelectItem = (fnSelectItem_t)UTIL_SearchPattern("mp.dll", SELECT_ITEM_FN_PATTERN, 1);
	g_pfnApplyMultiDamage = (fnApplyMultiDamage_t)UTIL_SearchPattern("mp.dll", APPLY_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnClearMultiDamage = (fnClearMultiDamage_t)UTIL_SearchPattern("mp.dll", CLEAR_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnAddMultiDamage = (fnAddMultiDamage_t)UTIL_SearchPattern("mp.dll", ADD_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnDefaultDeploy = (fnDefaultDeploy_t)UTIL_SearchPattern("mp.dll", DEFAULT_DEPLOY_FN_PATTERN, 1);
	g_pfnSwitchWeapon = (fnSwitchWeapon_t)UTIL_SearchPattern("mp.dll", SWITCH_WEAPON_FN_PATTERN, 1);

	pEnt = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));	// Technically this is not CBaseEntity, but it is the closest one. It overrides Spawn() and ObjectCaps(), so it is still pure enough.

	if (!pEnt || !pEnt->pvPrivateData) [[unlikely]]
	{
		if (pEnt)
			g_engfuncs.pfnRemoveEntity(pEnt);

		LOG_ERROR("Failed to retrieve classtype for \"info_target\".");
		return;
	}

	g_pfnEntityTraceAttack = (fnEntityTraceAttack_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_TRACEATTACK);
	g_pfnEntityTakeDamage = (fnEntityTakeDamage_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_TAKEDAMAGE);
	g_pfnEntityKilled = (fnEntityKilled_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_KILLED);
	g_pfnEntityTraceBleed = (fnEntityTraceBleed_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_TRACEBLEED);
	g_pfnEntityDamageDecal = (fnEntityDamageDecal_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_DAMAGEDECAL);
	g_pfnEntityGetNextTarget = (fnEntityGetNextTarget_t)UTIL_RetrieveVirtualFunction(pEnt->pvPrivateData, VFTIDX_CBASE_GETNEXTTARGET);

	g_engfuncs.pfnRemoveEntity(pEnt);
	pEnt = nullptr;

#ifdef _DEBUG
	assert(g_pfnRadiusFlash != nullptr);
	assert(g_pfnSelectItem != nullptr);
	assert(g_pfnApplyMultiDamage != nullptr);
	assert(g_pfnClearMultiDamage != nullptr);
	assert(g_pfnAddMultiDamage != nullptr);
	assert(g_pfnDefaultDeploy != nullptr);
	assert(g_pfnSwitchWeapon != nullptr);
#else
	[[unlikely]]
	if (!g_pfnRadiusFlash)
		LOG_ERROR("Function \"::RadiusFlash\" no found!");
	[[unlikely]]
	if (!g_pfnSelectItem)
		LOG_ERROR("Function \"CBasePlayer::SelectItem\" no found!");
	[[unlikely]]
	if (!g_pfnApplyMultiDamage)
		LOG_ERROR("Function \"::ApplyMultiDamage\" no found!");
	[[unlikely]]
	if (!g_pfnClearMultiDamage)
		LOG_ERROR("Function \"::ClearMultiDamage\" no found!");
	[[unlikely]]
	if (!g_pfnAddMultiDamage)
		LOG_ERROR("Function \"::AddMultiDamage\" no found!");
	[[unlikely]]
	if (!g_pfnDefaultDeploy)
		LOG_ERROR("Function \"CBasePlayerWeapon::DefaultDeploy\" no found!");
	[[unlikely]]
	if (!g_pfnSwitchWeapon)
		LOG_ERROR("Function \"CBasePlayer::SwitchWeapon\" no found!");
#endif

	//HookInfo::SelectItem.m_Address = g_pfnSelectItem;
	//HookInfo::SwitchWeapon.m_Address = g_pfnSwitchWeapon;

	//UTIL_PreparePatch(g_pfnSelectItem, UTIL_CreateTrampoline(true, 1, &::SelectItem), HookInfo::SelectItem.m_PatchedBytes, HookInfo::SelectItem.m_OriginalBytes);
	//UTIL_PreparePatch(g_pfnSwitchWeapon, UTIL_CreateTrampoline(true, 1, &::SwitchWeapon), HookInfo::SwitchWeapon.m_PatchedBytes, HookInfo::SwitchWeapon.m_OriginalBytes);

	//UTIL_DoPatch(g_pfnSelectItem, HookInfo::SelectItem.m_PatchedBytes);
	//UTIL_DoPatch(g_pfnSwitchWeapon, HookInfo::SwitchWeapon.m_PatchedBytes);

	bHooksPerformed = true;
}

void RetrieveMessageHandles(void) noexcept
{
	gmsgScreenFade::Retrieve();
	gmsgScreenShake::Retrieve();
	gmsgBarTime::Retrieve();
	gmsgWeaponList::Retrieve();
	gmsgWeapPickup::Retrieve();
	gmsgTextMsg::Retrieve();

	gmsgWeaponAnim::m_iMessageIndex = SVC_WEAPONANIM;
}

void RetrieveCVarHandles(void) noexcept
{
	gcvarFriendlyFire = g_engfuncs.pfnCVarGetPointer("mp_friendlyfire");
	gcvarMaxSpeed = g_engfuncs.pfnCVarGetPointer("sv_maxspeed");
	gcvarMaxVelocity = g_engfuncs.pfnCVarGetPointer("sv_maxvelocity");
}

// Meta API

int fw_Spawn(edict_t *pent) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[likely]]
	if (!g_bShouldPrecache)
		return 0;

	// plugin_precache

	Precache();

	g_bShouldPrecache = false;
	return 0;
}

extern META_RES OnClientCommand(CBasePlayer *pPlayer, const std::string &szCommand) noexcept;
void fw_ClientCommand(edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[unlikely]]
	if (pev_valid(pEdict) != 2)
		return;

	if (auto const pEntity = (CBaseEntity *)pEdict->pvPrivateData; !pEntity->IsPlayer())
		return;

	gpMetaGlobals->mres = OnClientCommand((CBasePlayer *)pEdict->pvPrivateData, g_engfuncs.pfnCmd_Argv(0));
	// pre
}

void fw_ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	// plugin_init

	DeployHooks();
	RetrieveMessageHandles();
	RetrieveCVarHandles();
	Waypoint_Read();

	// plugin_cfg

	g_engfuncs.pfnCvar_DirectSet(gcvarMaxSpeed, "9999.0");
	g_engfuncs.pfnCvar_DirectSet(gcvarMaxVelocity, "9999.0");

	TaskScheduler::Enroll(Task_UpdateTeams());

	// This hook is very special, since it is actually delete-newed in each new game.
	// Therefore we must hook it every time.
	if (!g_pGameRules)
	{
		auto addr = (std::uintptr_t)UTIL_SearchPattern("mp.dll", CWORLD_PRECACHE_FN_PATTERN, 1);

#ifdef _DEBUG
		assert(addr != 0);
#else
		[[unlikely]]
		if (!addr)
			LOG_ERROR("Function \"CWorld::Precache\" no found!");
#endif

		addr += (std::ptrdiff_t)(0xD29B4 - 0xD2940);
		g_pGameRules = *(CHalfLifeMultiplay **)(void **)(*(long *)addr);

		assert(g_pGameRules != nullptr);

		// However, the hook status remains even if the game reloaded.
		// Still need this method to make sure the hooks are happened only once.

		static bool bGameRuleHooked = false;

		[[unlikely]]
		if (!bGameRuleHooked)
		{
			auto const rgpfnCHalfLifeMultiplay = UTIL_RetrieveVirtualFunctionTable(g_pGameRules);

			UTIL_VirtualTableInjection(rgpfnCHalfLifeMultiplay, VFTIDX_CHalfLifeMultiplay_CleanUpMap, UTIL_CreateTrampoline(true, 0, OrpheuF_CleanUpMap), (void **)&g_pfnCleanUpMap);

			bGameRuleHooked = true;
		}
	}
}

void fw_PlayerPostThink(edict_t *pEntity) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// pre
}

void fw_TraceLine_Post(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr) noexcept
{
/*
	if (!ptr->pHit || !ent_cast<int>(ptr->pHit))
	{
		if (!pentToSkip || pev_valid(&pentToSkip->v) != 2)
			goto LIB_SKIP;

		if (CBaseEntity *pent = (CBaseEntity *)pentToSkip->pvPrivateData; !pent->IsPlayer() || !pent->IsAlive())
			goto LIB_SKIP;

		Vector &vecK = ptr->vecPlaneNormal;
		Quaternion Q = Quaternion::Rotate(Vector(0, 0, 1), vecK);
		//Vector vecI = CrossProduct(vecK, Vector(0, 0, 1)).Normalize();
		Vector vecI = Q * Vector(1, 0, 0);
		//Vector vecJ = CrossProduct(vecK, vecI).Normalize();
		Vector vecJ = Q * Vector(0, 1, 0);

		if ((CrossProduct(vecI, vecJ) - vecK).Length() > 0.001f)
			g_engfuncs.pfnClientPrintf(pentToSkip, print_center, std::format("{}", gpGlobals->time).c_str());

		MsgPVS(SVC_TEMPENTITY, ptr->vecEndPos);
		WriteData(TE_BEAMPOINTS);
		WriteData(ptr->vecEndPos);
		WriteData(ptr->vecEndPos + vecK * 32);
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_TRAIL]);
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)4);
		WriteData((byte)10);
		WriteData((byte)1);	//amp
		WriteData((byte)255);
		WriteData((byte)0);
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)0);
		MsgEnd();

		MsgPVS(SVC_TEMPENTITY, ptr->vecEndPos);
		WriteData(TE_BEAMPOINTS);
		WriteData(ptr->vecEndPos);
		WriteData(ptr->vecEndPos + vecJ * 32);
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_TRAIL]);
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)4);
		WriteData((byte)10);
		WriteData((byte)1);	//amp
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)0);
		MsgEnd();

		MsgPVS(SVC_TEMPENTITY, ptr->vecEndPos);
		WriteData(TE_BEAMPOINTS);
		WriteData(ptr->vecEndPos);
		WriteData(ptr->vecEndPos + vecI * 32);
		WriteData((short)Sprite::m_rgLibrary[Sprite::SMOKE_TRAIL]);
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)4);
		WriteData((byte)10);
		WriteData((byte)1);	//amp
		WriteData((byte)0);
		WriteData((byte)0);
		WriteData((byte)255);
		WriteData((byte)255);
		WriteData((byte)0);
		MsgEnd();
	}

LIB_SKIP:;
*/
	gpMetaGlobals->mres = MRES_IGNORED;
	// post
}

int fw_CheckVisibility(const edict_t *entity, unsigned char *pset) noexcept
{
	if (entity->v.classname == MAKE_STRING(Classname::JET))
	{
		gpMetaGlobals->mres = MRES_SUPERCEDE;
		return true;
	}

	gpMetaGlobals->mres = MRES_IGNORED;
	return 0;
	// pre
}

void fw_SetGroupMask_Post(int mask, int op) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post
}

qboolean fw_AddToFullPack(entity_state_t *pState, int iEntIndex, edict_t *pEdict, edict_t *pClientSendTo, qboolean cl_lw, qboolean bIsPlayer, unsigned char *pSet)
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[unlikely]]
	if (pEdict->v.classname == MAKE_STRING(CDynamicTarget::CLASSNAME) || pEdict->v.classname == MAKE_STRING(CFixedTarget::CLASSNAME))
	{
		auto const pClient = (CBasePlayer *)pClientSendTo->pvPrivateData;

		if (pEdict->v.team != pClient->m_iTeam)
		{
			gpMetaGlobals->mres = MRES_SUPERCEDE;
			return false;
		}
	}

	return true;
}

void fw_OnFreeEntPrivateData(edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[likely]]
	if (auto const pEntity = (CBaseEntity *)pEdict->pvPrivateData; pEntity != nullptr)
	{
		[[unlikely]]
		if (auto const pPrefab = dynamic_cast<Prefab_t *>(pEntity); pPrefab != nullptr)
		{
			std::destroy_at(pPrefab);	// Thanks to C++17 we can finally patch up this old game.

			gpMetaGlobals->mres = MRES_SUPERCEDE;
		}
	}
}

// Register Meta Hooks

inline constexpr DLL_FUNCTIONS gFunctionTable =
{
	.pfnGameInit	= nullptr,
	.pfnSpawn		= &fw_Spawn,
	.pfnThink		= nullptr,
	.pfnUse			= nullptr,
	.pfnTouch		= nullptr,
	.pfnBlocked		= nullptr,
	.pfnKeyValue	= nullptr,
	.pfnSave		= nullptr,
	.pfnRestore		= nullptr,
	.pfnSetAbsBox	= nullptr,

	.pfnSaveWriteFields	= nullptr,
	.pfnSaveReadFields	= nullptr,

	.pfnSaveGlobalState		= nullptr,
	.pfnRestoreGlobalState	= nullptr,
	.pfnResetGlobalState	= nullptr,

	.pfnClientConnect		= nullptr,
	.pfnClientDisconnect	= nullptr,
	.pfnClientKill			= nullptr,
	.pfnClientPutInServer	= nullptr,
	.pfnClientCommand		= &fw_ClientCommand,
	.pfnClientUserInfoChanged= nullptr,
	.pfnServerActivate		= nullptr,
	.pfnServerDeactivate	= nullptr,

	.pfnPlayerPreThink	= nullptr,
	.pfnPlayerPostThink	= &fw_PlayerPostThink,

	.pfnStartFrame		= nullptr,
	.pfnParmsNewLevel	= nullptr,
	.pfnParmsChangeLevel= nullptr,

	.pfnGetGameDescription	= nullptr,
	.pfnPlayerCustomization	= nullptr,

	.pfnSpectatorConnect	= nullptr,
	.pfnSpectatorDisconnect	= nullptr,
	.pfnSpectatorThink		= nullptr,

	.pfnSys_Error	= nullptr,

	.pfnPM_Move				= nullptr,
	.pfnPM_Init				= nullptr,
	.pfnPM_FindTextureType	= nullptr,

	.pfnSetupVisibility	= nullptr,
	.pfnUpdateClientData= nullptr,
	.pfnAddToFullPack	= &fw_AddToFullPack,
	.pfnCreateBaseline	= nullptr,
	.pfnRegisterEncoders= nullptr,
	.pfnGetWeaponData	= nullptr,
	.pfnCmdStart		= nullptr,
	.pfnCmdEnd			= nullptr,
	.pfnConnectionlessPacket	= nullptr,
	.pfnGetHullBounds			= nullptr,
	.pfnCreateInstancedBaselines= nullptr,
	.pfnInconsistentFile		= nullptr,
	.pfnAllowLagCompensation	= nullptr,
};

int HookGameDLLExportedFn(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLExportedFn' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLExportedFn' called with version mismatch. Expected: %d, receiving: %d.", INTERFACE_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return true;
}

inline constexpr DLL_FUNCTIONS gFunctionTable_Post =
{
	.pfnGameInit	= nullptr,
	.pfnSpawn		= nullptr,
	.pfnThink		= nullptr,
	.pfnUse			= nullptr,
	.pfnTouch		= nullptr,
	.pfnBlocked		= nullptr,
	.pfnKeyValue	= nullptr,
	.pfnSave		= nullptr,
	.pfnRestore		= nullptr,
	.pfnSetAbsBox	= nullptr,

	.pfnSaveWriteFields	= nullptr,
	.pfnSaveReadFields	= nullptr,

	.pfnSaveGlobalState		= nullptr,
	.pfnRestoreGlobalState	= nullptr,
	.pfnResetGlobalState	= nullptr,

	.pfnClientConnect		= nullptr,
	.pfnClientDisconnect	= nullptr,
	.pfnClientKill			= nullptr,
	.pfnClientPutInServer	= nullptr,
	.pfnClientCommand		= nullptr,
	.pfnClientUserInfoChanged= nullptr,
	.pfnServerActivate		= &fw_ServerActivate_Post,
	.pfnServerDeactivate	= []() noexcept { g_bShouldPrecache = true; g_pGameRules = nullptr; TaskScheduler::Clear(); },

	.pfnPlayerPreThink	= nullptr,
	.pfnPlayerPostThink	= nullptr,

	.pfnStartFrame		= &TaskScheduler::Think,
	.pfnParmsNewLevel	= nullptr,
	.pfnParmsChangeLevel= nullptr,

	.pfnGetGameDescription	= nullptr,
	.pfnPlayerCustomization	= nullptr,

	.pfnSpectatorConnect	= nullptr,
	.pfnSpectatorDisconnect	= nullptr,
	.pfnSpectatorThink		= nullptr,

	.pfnSys_Error	= nullptr,

	.pfnPM_Move				= nullptr,
	.pfnPM_Init				= nullptr,
	.pfnPM_FindTextureType	= nullptr,

	.pfnSetupVisibility	= nullptr,
	.pfnUpdateClientData= nullptr,
	.pfnAddToFullPack	= nullptr,
	.pfnCreateBaseline	= nullptr,
	.pfnRegisterEncoders= nullptr,
	.pfnGetWeaponData	= nullptr,
	.pfnCmdStart		= nullptr,
	.pfnCmdEnd			= nullptr,
	.pfnConnectionlessPacket	= nullptr,
	.pfnGetHullBounds			= nullptr,
	.pfnCreateInstancedBaselines= nullptr,
	.pfnInconsistentFile		= nullptr,
	.pfnAllowLagCompensation	= nullptr,
};

int HookGameDLLExportedFn_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLExportedFn_Post' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLExportedFn_Post' called with version mismatch. Expected: %d, receiving: %d.", INTERFACE_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gFunctionTable_Post, sizeof(DLL_FUNCTIONS));
	return true;
}

inline constexpr NEW_DLL_FUNCTIONS gNewFunctionTable =
{
	.pfnOnFreeEntPrivateData	= &fw_OnFreeEntPrivateData,
	.pfnGameShutdown			= nullptr,
	.pfnShouldCollide			= nullptr,
	.pfnCvarValue				= nullptr,
	.pfnCvarValue2				= nullptr,
};

int HookGameDLLNewFn(NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLNewFn' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != NEW_DLL_FUNCTIONS_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLNewFn' called with version mismatch. Expected: %d, receiving: %d.", NEW_DLL_FUNCTIONS_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gNewFunctionTable, sizeof(NEW_DLL_FUNCTIONS));
	return true;
}

inline constexpr enginefuncs_t gHookEngFns = 
{
	.pfnPrecacheModel	= nullptr,
	.pfnPrecacheSound	= nullptr,
	.pfnSetModel		= nullptr,
	.pfnModelIndex		= nullptr,
	.pfnModelFrames		= nullptr,

	.pfnSetSize			= nullptr,
	.pfnChangeLevel		= nullptr,
	.pfnGetSpawnParms	= nullptr,
	.pfnSaveSpawnParms	= nullptr,

	.pfnVecToYaw		= nullptr,
	.pfnVecToAngles		= nullptr,
	.pfnMoveToOrigin	= nullptr,
	.pfnChangeYaw		= nullptr,
	.pfnChangePitch		= nullptr,

	.pfnFindEntityByString	= nullptr,
	.pfnGetEntityIllum		= nullptr,
	.pfnFindEntityInSphere	= nullptr,
	.pfnFindClientInPVS		= nullptr,
	.pfnEntitiesInPVS		= nullptr,

	.pfnMakeVectors		= nullptr,
	.pfnAngleVectors	= nullptr,

	.pfnCreateEntity		= nullptr,
	.pfnRemoveEntity		= nullptr,
	.pfnCreateNamedEntity	= nullptr,

	.pfnMakeStatic		= nullptr,
	.pfnEntIsOnFloor	= nullptr,
	.pfnDropToFloor		= nullptr,

	.pfnWalkMove		= nullptr,
	.pfnSetOrigin		= nullptr,

	.pfnEmitSound		= nullptr,
	.pfnEmitAmbientSound= nullptr,

	.pfnTraceLine		= nullptr,
	.pfnTraceToss		= nullptr,
	.pfnTraceMonsterHull= nullptr,
	.pfnTraceHull		= nullptr,
	.pfnTraceModel		= nullptr,
	.pfnTraceTexture	= nullptr,
	.pfnTraceSphere		= nullptr,
	.pfnGetAimVector	= nullptr,

	.pfnServerCommand	= nullptr,
	.pfnServerExecute	= nullptr,
	.pfnClientCommand	= nullptr,

	.pfnParticleEffect	= nullptr,
	.pfnLightStyle		= nullptr,
	.pfnDecalIndex		= nullptr,
	.pfnPointContents	= nullptr,

	.pfnMessageBegin	= nullptr,
	.pfnMessageEnd		= nullptr,

	.pfnWriteByte	= nullptr,
	.pfnWriteChar	= nullptr,
	.pfnWriteShort	= nullptr,
	.pfnWriteLong	= nullptr,
	.pfnWriteAngle	= nullptr,
	.pfnWriteCoord	= nullptr,
	.pfnWriteString	= nullptr,
	.pfnWriteEntity	= nullptr,

	.pfnCVarRegister	= nullptr,
	.pfnCVarGetFloat	= nullptr,
	.pfnCVarGetString	= nullptr,
	.pfnCVarSetFloat	= nullptr,
	.pfnCVarSetString	= nullptr,

	.pfnAlertMessage	= nullptr,
	.pfnEngineFprintf	= nullptr,

	.pfnPvAllocEntPrivateData	= nullptr,
	.pfnPvEntPrivateData		= nullptr,
	.pfnFreeEntPrivateData		= nullptr,

	.pfnSzFromIndex		= nullptr,
	.pfnAllocString		= nullptr,

	.pfnGetVarsOfEnt		= nullptr,
	.pfnPEntityOfEntOffset	= nullptr,
	.pfnEntOffsetOfPEntity	= nullptr,
	.pfnIndexOfEdict		= nullptr,
	.pfnPEntityOfEntIndex	= nullptr,
	.pfnFindEntityByVars	= nullptr,
	.pfnGetModelPtr			= nullptr,

	.pfnRegUserMsg		= nullptr,

	.pfnAnimationAutomove	= nullptr,
	.pfnGetBonePosition		= nullptr,

	.pfnFunctionFromName	= nullptr,
	.pfnNameForFunction		= nullptr,

	.pfnClientPrintf	= nullptr,
	.pfnServerPrint		= nullptr,

	.pfnCmd_Args	= nullptr,
	.pfnCmd_Argv	= nullptr,
	.pfnCmd_Argc	= nullptr,

	.pfnGetAttachment	= nullptr,

	.pfnCRC32_Init			= nullptr,
	.pfnCRC32_ProcessBuffer	= nullptr,
	.pfnCRC32_ProcessByte	= nullptr,
	.pfnCRC32_Final			= nullptr,

	.pfnRandomLong	= nullptr,
	.pfnRandomFloat	= nullptr,

	.pfnSetView			= nullptr,
	.pfnTime			= nullptr,
	.pfnCrosshairAngle	= nullptr,

	.pfnLoadFileForMe	= nullptr,
	.pfnFreeFile		= nullptr,

	.pfnEndSection		= nullptr,
	.pfnCompareFileTime	= nullptr,
	.pfnGetGameDir		= nullptr,
	.pfnCvar_RegisterVariable	= nullptr,
	.pfnFadeClientVolume	= nullptr,
	.pfnSetClientMaxspeed	= nullptr,
	.pfnCreateFakeClient	= nullptr,
	.pfnRunPlayerMove		= nullptr,
	.pfnNumberOfEntities	= nullptr,

	.pfnGetInfoKeyBuffer	= nullptr,
	.pfnInfoKeyValue		= nullptr,
	.pfnSetKeyValue			= nullptr,
	.pfnSetClientKeyValue	= nullptr,

	.pfnIsMapValid		= nullptr,
	.pfnStaticDecal		= nullptr,
	.pfnPrecacheGeneric	= nullptr,
	.pfnGetPlayerUserId	= nullptr,
	.pfnBuildSoundMsg	= nullptr,
	.pfnIsDedicatedServer	= nullptr,
	.pfnCVarGetPointer	= nullptr,
	.pfnGetPlayerWONId	= nullptr,

	.pfnInfo_RemoveKey		= nullptr,
	.pfnGetPhysicsKeyValue	= nullptr,
	.pfnSetPhysicsKeyValue	= nullptr,
	.pfnGetPhysicsInfoString= nullptr,
	.pfnPrecacheEvent		= nullptr,
	.pfnPlaybackEvent		= nullptr,

	.pfnSetFatPVS		= nullptr,
	.pfnSetFatPAS		= nullptr,

	.pfnCheckVisibility	= &fw_CheckVisibility,

	.pfnDeltaSetField			= nullptr,
	.pfnDeltaUnsetField			= nullptr,
	.pfnDeltaAddEncoder			= nullptr,
	.pfnGetCurrentPlayer		= nullptr,
	.pfnCanSkipPlayer			= nullptr,
	.pfnDeltaFindField			= nullptr,
	.pfnDeltaSetFieldByIndex	= nullptr,
	.pfnDeltaUnsetFieldByIndex	= nullptr,

	.pfnSetGroupMask			= nullptr,

	.pfnCreateInstancedBaseline	= nullptr,
	.pfnCvar_DirectSet			= nullptr,

	.pfnForceUnmodified			= nullptr,

	.pfnGetPlayerStats			= nullptr,

	.pfnAddServerCommand		= nullptr,

	// Added in SDK 2.2:
	.pfnVoice_GetClientListening	= nullptr,
	.pfnVoice_SetClientListening	= nullptr,

	// Added for HL 1109 (no SDK update):
	.pfnGetPlayerAuthId	= nullptr,

	// Added 2003/11/10 (no SDK update):
	.pfnSequenceGet							= nullptr,
	.pfnSequencePickSentence				= nullptr,
	.pfnGetFileSize							= nullptr,
	.pfnGetApproxWavePlayLen				= nullptr,
	.pfnIsCareerMatch						= nullptr,
	.pfnGetLocalizedStringLength			= nullptr,
	.pfnRegisterTutorMessageShown			= nullptr,
	.pfnGetTimesTutorMessageShown			= nullptr,
	.pfnProcessTutorMessageDecayBuffer		= nullptr,
	.pfnConstructTutorMessageDecayBuffer	= nullptr,
	.pfnResetTutorMessageDecayData			= nullptr,

	// Added Added 2005-08-11 (no SDK update)
	.pfnQueryClientCvarValue	= nullptr,
	// Added Added 2005-11-22 (no SDK update)
	.pfnQueryClientCvarValue2	= nullptr,
	// Added 2009-06-17 (no SDK update)
	.pfnEngCheckParm			= nullptr,
};

int HookEngineAPI(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion) noexcept
{
	if (!pengfuncsFromEngine) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookEngineAPI' called with null 'pengfuncsFromEngine' parameter.");
		return false;
	}
	else if (*interfaceVersion != ENGINE_INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookEngineAPI' called with version mismatch. Expected: %d, receiving: %d.", ENGINE_INTERFACE_VERSION, *interfaceVersion);

		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return false;
	}

	memcpy(pengfuncsFromEngine, &gHookEngFns, sizeof(enginefuncs_t));
	return true;
}

inline constexpr enginefuncs_t gHookEngFns_Post = 
{
	.pfnPrecacheModel	= nullptr,
	.pfnPrecacheSound	= nullptr,
	.pfnSetModel		= nullptr,
	.pfnModelIndex		= nullptr,
	.pfnModelFrames		= nullptr,

	.pfnSetSize			= nullptr,
	.pfnChangeLevel		= nullptr,
	.pfnGetSpawnParms	= nullptr,
	.pfnSaveSpawnParms	= nullptr,

	.pfnVecToYaw		= nullptr,
	.pfnVecToAngles		= nullptr,
	.pfnMoveToOrigin	= nullptr,
	.pfnChangeYaw		= nullptr,
	.pfnChangePitch		= nullptr,

	.pfnFindEntityByString	= nullptr,
	.pfnGetEntityIllum		= nullptr,
	.pfnFindEntityInSphere	= nullptr,
	.pfnFindClientInPVS		= nullptr,
	.pfnEntitiesInPVS		= nullptr,

	.pfnMakeVectors		= nullptr,
	.pfnAngleVectors	= nullptr,

	.pfnCreateEntity		= nullptr,
	.pfnRemoveEntity		= nullptr,
	.pfnCreateNamedEntity	= nullptr,

	.pfnMakeStatic		= nullptr,
	.pfnEntIsOnFloor	= nullptr,
	.pfnDropToFloor		= nullptr,

	.pfnWalkMove		= nullptr,
	.pfnSetOrigin		= nullptr,

	.pfnEmitSound		= nullptr,
	.pfnEmitAmbientSound= nullptr,

	.pfnTraceLine		= &fw_TraceLine_Post,
	.pfnTraceToss		= nullptr,
	.pfnTraceMonsterHull= nullptr,
	.pfnTraceHull		= nullptr,
	.pfnTraceModel		= nullptr,
	.pfnTraceTexture	= nullptr,
	.pfnTraceSphere		= nullptr,
	.pfnGetAimVector	= nullptr,

	.pfnServerCommand	= nullptr,
	.pfnServerExecute	= nullptr,
	.pfnClientCommand	= nullptr,

	.pfnParticleEffect	= nullptr,
	.pfnLightStyle		= nullptr,
	.pfnDecalIndex		= nullptr,
	.pfnPointContents	= nullptr,

	.pfnMessageBegin	= nullptr,
	.pfnMessageEnd		= nullptr,

	.pfnWriteByte	= nullptr,
	.pfnWriteChar	= nullptr,
	.pfnWriteShort	= nullptr,
	.pfnWriteLong	= nullptr,
	.pfnWriteAngle	= nullptr,
	.pfnWriteCoord	= nullptr,
	.pfnWriteString	= nullptr,
	.pfnWriteEntity	= nullptr,

	.pfnCVarRegister	= nullptr,
	.pfnCVarGetFloat	= nullptr,
	.pfnCVarGetString	= nullptr,
	.pfnCVarSetFloat	= nullptr,
	.pfnCVarSetString	= nullptr,

	.pfnAlertMessage	= nullptr,
	.pfnEngineFprintf	= nullptr,

	.pfnPvAllocEntPrivateData	= nullptr,
	.pfnPvEntPrivateData		= nullptr,
	.pfnFreeEntPrivateData		= nullptr,

	.pfnSzFromIndex		= nullptr,
	.pfnAllocString		= nullptr,

	.pfnGetVarsOfEnt		= nullptr,
	.pfnPEntityOfEntOffset	= nullptr,
	.pfnEntOffsetOfPEntity	= nullptr,
	.pfnIndexOfEdict		= nullptr,
	.pfnPEntityOfEntIndex	= nullptr,
	.pfnFindEntityByVars	= nullptr,
	.pfnGetModelPtr			= nullptr,

	.pfnRegUserMsg		= nullptr,

	.pfnAnimationAutomove	= nullptr,
	.pfnGetBonePosition		= nullptr,

	.pfnFunctionFromName	= nullptr,
	.pfnNameForFunction		= nullptr,

	.pfnClientPrintf	= nullptr,
	.pfnServerPrint		= nullptr,

	.pfnCmd_Args	= nullptr,
	.pfnCmd_Argv	= nullptr,
	.pfnCmd_Argc	= nullptr,

	.pfnGetAttachment	= nullptr,

	.pfnCRC32_Init			= nullptr,
	.pfnCRC32_ProcessBuffer	= nullptr,
	.pfnCRC32_ProcessByte	= nullptr,
	.pfnCRC32_Final			= nullptr,

	.pfnRandomLong	= nullptr,
	.pfnRandomFloat	= nullptr,

	.pfnSetView			= nullptr,
	.pfnTime			= nullptr,
	.pfnCrosshairAngle	= nullptr,

	.pfnLoadFileForMe	= nullptr,
	.pfnFreeFile		= nullptr,

	.pfnEndSection		= nullptr,
	.pfnCompareFileTime	= nullptr,
	.pfnGetGameDir		= nullptr,
	.pfnCvar_RegisterVariable	= nullptr,
	.pfnFadeClientVolume	= nullptr,
	.pfnSetClientMaxspeed	= nullptr,
	.pfnCreateFakeClient	= nullptr,
	.pfnRunPlayerMove		= nullptr,
	.pfnNumberOfEntities	= nullptr,

	.pfnGetInfoKeyBuffer	= nullptr,
	.pfnInfoKeyValue		= nullptr,
	.pfnSetKeyValue			= nullptr,
	.pfnSetClientKeyValue	= nullptr,

	.pfnIsMapValid		= nullptr,
	.pfnStaticDecal		= nullptr,
	.pfnPrecacheGeneric	= nullptr,
	.pfnGetPlayerUserId	= nullptr,
	.pfnBuildSoundMsg	= nullptr,
	.pfnIsDedicatedServer	= nullptr,
	.pfnCVarGetPointer	= nullptr,
	.pfnGetPlayerWONId	= nullptr,

	.pfnInfo_RemoveKey		= nullptr,
	.pfnGetPhysicsKeyValue	= nullptr,
	.pfnSetPhysicsKeyValue	= nullptr,
	.pfnGetPhysicsInfoString= nullptr,
	.pfnPrecacheEvent		= nullptr,
	.pfnPlaybackEvent		= nullptr,

	.pfnSetFatPVS		= nullptr,
	.pfnSetFatPAS		= nullptr,

	.pfnCheckVisibility	= nullptr,

	.pfnDeltaSetField			= nullptr,
	.pfnDeltaUnsetField			= nullptr,
	.pfnDeltaAddEncoder			= nullptr,
	.pfnGetCurrentPlayer		= nullptr,
	.pfnCanSkipPlayer			= nullptr,
	.pfnDeltaFindField			= nullptr,
	.pfnDeltaSetFieldByIndex	= nullptr,
	.pfnDeltaUnsetFieldByIndex	= nullptr,

	.pfnSetGroupMask			= &fw_SetGroupMask_Post,

	.pfnCreateInstancedBaseline	= nullptr,
	.pfnCvar_DirectSet			= nullptr,

	.pfnForceUnmodified			= nullptr,

	.pfnGetPlayerStats			= nullptr,

	.pfnAddServerCommand		= nullptr,

	// Added in SDK 2.2:
	.pfnVoice_GetClientListening	= nullptr,
	.pfnVoice_SetClientListening	= nullptr,

	// Added for HL 1109 (no SDK update):
	.pfnGetPlayerAuthId	= nullptr,

	// Added 2003/11/10 (no SDK update):
	.pfnSequenceGet							= nullptr,
	.pfnSequencePickSentence				= nullptr,
	.pfnGetFileSize							= nullptr,
	.pfnGetApproxWavePlayLen				= nullptr,
	.pfnIsCareerMatch						= nullptr,
	.pfnGetLocalizedStringLength			= nullptr,
	.pfnRegisterTutorMessageShown			= nullptr,
	.pfnGetTimesTutorMessageShown			= nullptr,
	.pfnProcessTutorMessageDecayBuffer		= nullptr,
	.pfnConstructTutorMessageDecayBuffer	= nullptr,
	.pfnResetTutorMessageDecayData			= nullptr,

	// Added Added 2005-08-11 (no SDK update)
	.pfnQueryClientCvarValue	= nullptr,
	// Added Added 2005-11-22 (no SDK update)
	.pfnQueryClientCvarValue2	= nullptr,
	// Added 2009-06-17 (no SDK update)
	.pfnEngCheckParm			= nullptr,
};

int HookEngineAPI_Post(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion) noexcept
{
	if (!pengfuncsFromEngine) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookEngineAPI_Post' called with null 'pengfuncsFromEngine' parameter.");
		return false;
	}
	else if (*interfaceVersion != ENGINE_INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookEngineAPI_Post' called with version mismatch. Expected: %d, receiving: %d.", ENGINE_INTERFACE_VERSION, *interfaceVersion);

		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return false;
	}

	memcpy(pengfuncsFromEngine, &gHookEngFns_Post, sizeof(enginefuncs_t));
	return true;
}
