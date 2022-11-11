#include <cassert>

import <cstring>;

import <string>;

import progdefs;	// edict_t
import util;

import UtlHook;

import CBase;
import Entity;
import Hook;
import Plugin;
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
extern void HamF_Item_Holster(CBasePlayerItem *pThis, int skiplocal) noexcept;
//

void DeployHooks(void) noexcept
{
	edict_t *pEnt = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("weapon_knife"));

	if (!pEnt || !pEnt->pvPrivateData) [[unlikely]]
	{
		if (pEnt)
			g_engfuncs.pfnRemoveEntity(pEnt);

		LOG_ERROR("Failed to retrieve classtype for \"weapon_knife\".");
		return;
	}

	auto const vft = UTIL_RetrieveVirtualFunctionTable(pEnt->pvPrivateData);

	g_engfuncs.pfnRemoveEntity(pEnt);
	pEnt = nullptr;

	if (vft == nullptr) [[unlikely]]
	{
		LOG_ERROR("Failed to retrieve vtable for \"weapon_knife\".");
		return;
	}

	UTIL_VirtualTableInjection(vft, VFTIDX_ITEM_ADDTOPLAYER, UTIL_CreateTrampoline(true, 1, &HamF_Item_AddToPlayer), (void **)&g_pfnItemAddToPlayer);
	UTIL_VirtualTableInjection(vft, VFTIDX_ITEM_DEPLOY, UTIL_CreateTrampoline(true, 0, &HamF_Item_Deploy), (void **)&g_pfnItemDeploy);
	UTIL_VirtualTableInjection(vft, VFTIDX_ITEM_POSTFRAME, UTIL_CreateTrampoline(true, 0, &HamF_Item_PostFrame), (void **)&g_pfnItemPostFrame);
	UTIL_VirtualTableInjection(vft, VFTIDX_WEAPON_PRIMARYATTACK, UTIL_CreateTrampoline(true, 0, &HamF_Weapon_PrimaryAttack), (void **)&g_pfnWeaponPrimaryAttack);
	UTIL_VirtualTableInjection(vft, VFTIDX_WEAPON_SECONDARYATTACK, UTIL_CreateTrampoline(true, 0, &HamF_Weapon_SecondaryAttack), (void **)&g_pfnWeaponSecondaryAttack);
	UTIL_VirtualTableInjection(vft, VFTIDX_ITEM_HOLSTER, UTIL_CreateTrampoline(true, 1, &HamF_Item_Holster), (void **)&g_pfnItemHolster);

	g_pfnRadiusFlash = (fnRadiusFlash_t)UTIL_SearchPattern("mp.dll", RADIUS_FLASH_FN_PATTERN, 1);
	g_pfnSelectItem = (fnSelectItem_t)UTIL_SearchPattern("mp.dll", SELECT_ITEM_FN_PATTERN, 1);
	g_pfnApplyMultiDamage = (fnApplyMultiDamage_t)UTIL_SearchPattern("mp.dll", APPLY_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnClearMultiDamage = (fnClearMultiDamage_t)UTIL_SearchPattern("mp.dll", CLEAR_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnDefaultDeploy = (fnDefaultDeploy_t)UTIL_SearchPattern("mp.dll", DEFAULT_DEPLOY_FN_PATTERN, 1);

	assert(g_pfnRadiusFlash != nullptr);
	assert(g_pfnSelectItem != nullptr);
	assert(g_pfnApplyMultiDamage != nullptr);
	assert(g_pfnClearMultiDamage != nullptr);
	assert(g_pfnDefaultDeploy != nullptr);
}

void RetrieveMessageHandles(void) noexcept
{
	gmsgScreenFade::Retrieve();
	gmsgScreenShake::Retrieve();
	gmsgBarTime::Retrieve();
	gmsgWeaponList::Retrieve();
	gmsgWeapPickup::Retrieve();

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

//	static bool bInitialized = false;

//	[[likely]]
//	if (bInitialized)
//		return 0;

	// plugin_precache

	Precache();

//	bInitialized = true;
	return 0;
}

extern META_RES OnThink(CBaseEntity *pEntity) noexcept;
void fw_Think_Post(edict_t *pent) noexcept
{
	gpMetaGlobals->mres = pev_valid(pent) == 2 ? OnThink((CBaseEntity *)pent->pvPrivateData) : MRES_IGNORED;
	// post
}

extern META_RES OnTouch(CBaseEntity *pEntity, CBaseEntity *pOther) noexcept;
void fw_Touch_Post(edict_t *pentTouched, edict_t *pentOther) noexcept
{
	gpMetaGlobals->mres = pev_valid(pentTouched) == 2 ? OnTouch((CBaseEntity *)pentTouched->pvPrivateData, (CBaseEntity *)pentOther->pvPrivateData) : MRES_IGNORED;
	// post
}

extern META_RES OnClientCommand(CBasePlayer *pPlayer, const std::string &szCommand) noexcept;
void fw_ClientCommand(edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[unlikely]]
	if (!pEdict->pvPrivateData)
		return;

	if (auto const pEntity = (CBaseEntity *)pEdict->pvPrivateData; !pEntity->IsPlayer())
		return;

	gpMetaGlobals->mres = OnClientCommand((CBasePlayer *)pEdict->pvPrivateData, g_engfuncs.pfnCmd_Argv(0));
	// pre
}

void fw_ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	static bool bInitialized = false;

	[[likely]]
	if (bInitialized)
		return;

	// plugin_init

	DeployHooks();
	RetrieveMessageHandles();
	RetrieveCVarHandles();

	// plugin_cfg
	g_engfuncs.pfnCvar_DirectSet(gcvarMaxSpeed, "9999.0");
	g_engfuncs.pfnCvar_DirectSet(gcvarMaxVelocity, "9999.0");

	bInitialized = true;
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
	.pfnThink		= &fw_Think_Post,
	.pfnUse			= nullptr,
	.pfnTouch		= &fw_Touch_Post,
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
	.pfnServerDeactivate	= nullptr,

	.pfnPlayerPreThink	= nullptr,
	.pfnPlayerPostThink	= nullptr,

	.pfnStartFrame		= &TimedFnMgr::Think,
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
