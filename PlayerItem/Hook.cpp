import <cstdio>;
import <print>;

import eiface;

import meta_api;

import CBase;
import ConditionZero;
import Engine;
import FileSystem;
import GameRules;
import Message;
import Plugin;
import Prefab;
import Task;
import Uranus;
import VTFH;
import ZBot;

import UtlHook;

import Weapons;


template <typename T>
struct FunctionHook
{
	constexpr FunctionHook(T const& local_fn) noexcept
		: m_LocalFN{ local_fn }
	{
		;
	}

	auto PreparePatch(T const& addr) noexcept
	{
		m_Address = addr;
		return UTIL_PreparePatch(m_Address, m_LocalFN, m_PatchedBytes, m_OriginalBytes);
	}

	auto DoPatch() const noexcept
	{
		return UTIL_DoPatch(m_Address, m_PatchedBytes);
	}

	auto UndoPatch() const noexcept
	{
		return UTIL_UndoPatch(m_Address, m_OriginalBytes);
	}

	__forceinline void ApplyOn(T const& addr) noexcept
	{
		PreparePatch(addr);
		DoPatch();
	}

	auto CallOriginal(auto&&... args) const noexcept
	{
		if constexpr (requires{ { m_Address(std::forward<decltype(args)>(args)...) } -> std::same_as<void>; })
		{
			UndoPatch();
			m_Address(std::forward<decltype(args)>(args)...);
			DoPatch();
		}
		else
		{
			UndoPatch();
			auto const ret = m_Address(std::forward<decltype(args)>(args)...);
			DoPatch();

			return ret;
		}
	}

	__forceinline auto operator() (auto&&... args) const noexcept { return CallOriginal(std::forward<decltype(args)>(args)...); }

	unsigned char m_OriginalBytes[5]{};
	unsigned char m_PatchedBytes[5]{};
	T m_Address{};
	T m_LocalFN{};

	static_assert(std::is_pointer_v<T>, "Must be a local function pointer!");
};

extern edict_t* OrpheuF_CreateNamedEntity(string_t className) noexcept;
extern void* OrpheuF_SZ_GetSpace(sizebuf_t* buf, uint32_t length) noexcept;

namespace HookInfo
{
	FunctionHook CreateNamedEntity{ &OrpheuF_CreateNamedEntity };
	FunctionHook SZ_GetSpace{ &OrpheuF_SZ_GetSpace };
};

edict_t* OrpheuF_CreateNamedEntity(string_t className) noexcept
{
	std::string_view szClass{ STRING(className) };
	//g_engfuncs.pfnServerPrint(std::format("OrpheuF_CreateNamedEntity: <{}>\n", szClass).c_str());

	if (szClass == "weapon_knife")
	{
		auto const [pEdict, pKnife] = UTIL_CreateNamedPrefab<CKnife2>();
		return pEdict;
	}

	return HookInfo::CreateNamedEntity(className);
}

void* OrpheuF_SZ_GetSpace(sizebuf_t* buf, uint32_t length) noexcept
{
	if (buf->cursize + length > buf->maxsize)
	{
		if (auto f = std::fopen("SZ_GetSpace.log", "wb"); f)
		{
			std::print(f,
				"buf->buffername: {}\n"
				"buf->cursize: {}\n"
				"buf->flags: 0x{:X}\n"
				"buf->maxsize: {}\n"
				"\n"
				,
				buf->buffername ? buf->buffername : "nullptr",
				buf->cursize,
				buf->flags,
				buf->maxsize
			);

			fwrite(buf->data, 1, buf->cursize, f);
			fclose(f);
		}

		//UTIL_Terminate("SZ_GetSpace was not happy.");
	}

	return HookInfo::SZ_GetSpace(buf, length);
}

static void DeployHooks() noexcept
{
	static bool bHooked = false;

	[[likely]]
	if (bHooked)
		return;

	HookInfo::CreateNamedEntity.ApplyOn(g_engfuncs.pfnCreateNamedEntity);
	HookInfo::SZ_GetSpace.ApplyOn(gUranusCollection.pfnSZ_GetSpace);

	bHooked = true;
}

// Meta Forwards

META_RES fw_ClientCommand(EHANDLE<CBasePlayer> pPlayer) noexcept
{
	// pre event

	return MRES_IGNORED;
}

void fw_GameInit_Post() noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post event

	FileSystem::Init();
	Engine::Init();
	Uranus::RetrieveUranusLocal();

	// This one got to be hook early.
	DeployHooks();

	if (Engine::BUILD_NUMBER < Engine::NEW)
		UTIL_Terminate("Engine build '%d' mismatch from expected value: %d.\nPlease use this plugin on a legal STEAM copy.", Engine::BUILD_NUMBER, Engine::NEW);
}

void fw_ServerActivate_Post(edict_t* pEdictList, int edictCount, int clientMax) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post event

	RetrieveCBaseVirtualFn();
	RetrieveConditionZeroVar();
	RetrieveGameRules();
	RetrieveMessageHandles();
	ZBot::RetrieveManager();
}

void fw_ServerDeactivate_Post() noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post event

	// CGameRules class is re-install every map change. Hence we should re-hook it everytime.
	g_pGameRules = nullptr;

	// Remove ALL existing tasks.
	TaskScheduler::Clear();
}

void fw_OnFreeEntPrivateData(edict_t* pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[likely]]
	if (auto const pEntity = (CBaseEntity*)pEdict->pvPrivateData; pEntity != nullptr)
	{
		if (gpMetaGlobals->prev_mres == MRES_SUPERCEDE)	// It had been handled by other similar plugins.
			return;

		[[unlikely]]
		if (auto const pPrefab = dynamic_cast<CPrefabWeapon*>(pEntity); pPrefab != nullptr)
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
	.pfnClientCommand		= +[](edict_t* pPlayer) noexcept { gpMetaGlobals->mres = fw_ClientCommand(pPlayer); },
	.pfnClientUserInfoChanged= nullptr,
	.pfnServerActivate		= nullptr,
	.pfnServerDeactivate	= nullptr,

	.pfnPlayerPreThink	= nullptr,
	.pfnPlayerPostThink	= nullptr,

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
	.pfnGameInit	= &fw_GameInit_Post,
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
	.pfnServerDeactivate	= &fw_ServerDeactivate_Post,

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

	.pfnCheckVisibility	= nullptr,

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

	.pfnCheckVisibility	= nullptr,

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
