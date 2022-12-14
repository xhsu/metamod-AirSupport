#include <cassert>

import <cstring>;

import <string>;

import progdefs;	// edict_t
import util;

import UtlHook;

import Effects;
import Engine;
import FileSystem;
import GameRules;
import Hook;
import Jet;
import Platform;
import Plugin;
import Round;
import Task;
import Weapon;

// Resources.cpp
extern void Precache(void) noexcept;
//

// Weapon.cpp
extern qboolean __fastcall HamF_Item_AddToPlayer(CBasePlayerItem *pThis, int, CBasePlayer *pPlayer) noexcept;
extern qboolean __fastcall HamF_Item_Deploy(CBasePlayerItem *pItem, int) noexcept;
extern void __fastcall HamF_Item_PostFrame(CBasePlayerItem *pItem, int) noexcept;
extern void __fastcall HamF_Item_Holster(CBasePlayerItem *pThis, int, int skiplocal) noexcept;
extern void __fastcall OrpheuF_FireBullets(CBaseEntity *pThis, int, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) noexcept;
extern Vector __fastcall OrpheuF_FireBullets3(long argument1, long argument2, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, bool bPistol, int shared_rand) noexcept;
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

		UTIL_Terminate("Failed to retrieve classtype for \"weapon_knife\".");
		return;
	}

	auto const rgpfnCKnife = UTIL_RetrieveVirtualFunctionTable(pEnt->pvPrivateData);

	g_engfuncs.pfnRemoveEntity(pEnt);
	pEnt = nullptr;

	if (rgpfnCKnife == nullptr) [[unlikely]]
	{
		UTIL_Terminate("Failed to retrieve vtable for \"weapon_knife\".");
		return;
	}

	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_ADDTOPLAYER, &HamF_Item_AddToPlayer, (void **)&g_pfnItemAddToPlayer);
	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_DEPLOY, &HamF_Item_Deploy, (void **)&g_pfnItemDeploy);
	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_POSTFRAME, &HamF_Item_PostFrame, (void **)&g_pfnItemPostFrame);
	UTIL_VirtualTableInjection(rgpfnCKnife, VFTIDX_ITEM_HOLSTER, &HamF_Item_Holster, (void **)&g_pfnItemHolster);

	g_pfnRadiusFlash = (fnRadiusFlash_t)UTIL_SearchPattern("mp.dll", RADIUS_FLASH_FN_PATTERN, 1);
	g_pfnSelectItem = (fnSelectItem_t)UTIL_SearchPattern("mp.dll", SELECT_ITEM_FN_PATTERN, 1);
	g_pfnApplyMultiDamage = (fnApplyMultiDamage_t)UTIL_SearchPattern("mp.dll", APPLY_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnClearMultiDamage = (fnClearMultiDamage_t)UTIL_SearchPattern("mp.dll", CLEAR_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnAddMultiDamage = (fnAddMultiDamage_t)UTIL_SearchPattern("mp.dll", ADD_MULTI_DAMAGE_FN_PATTERN, 1);
	g_pfnDefaultDeploy = (fnDefaultDeploy_t)UTIL_SearchPattern("mp.dll", DEFAULT_DEPLOY_FN_PATTERN, 1);
	g_pfnSwitchWeapon = (fnSwitchWeapon_t)UTIL_SearchPattern("mp.dll", SWITCH_WEAPON_FN_PATTERN, 1);
	g_pfnFireBullets = (fnFireBullets_t)UTIL_SearchPattern("mp.dll", FIRE_BULLETS_FN_PATTERN, 1);
	g_pfnFireBullets3 = (fnFireBullets3_t)UTIL_SearchPattern("mp.dll", FIRE_BULLETS_3_FN_PATTERN, 1);

	pEnt = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"));	// Technically this is not CBaseEntity, but it is the closest one. It overrides Spawn() and ObjectCaps(), so it is still pure enough.

	if (!pEnt || !pEnt->pvPrivateData) [[unlikely]]
	{
		if (pEnt)
			g_engfuncs.pfnRemoveEntity(pEnt);

		UTIL_Terminate("Failed to retrieve classtype for \"info_target\".");
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
	assert(g_pfnFireBullets != nullptr);
	assert(g_pfnFireBullets3 != nullptr);
#else
	[[unlikely]]
	if (!g_pfnRadiusFlash)
		UTIL_Terminate("Function \"::RadiusFlash\" no found!");
	[[unlikely]]
	if (!g_pfnSelectItem)
		UTIL_Terminate("Function \"CBasePlayer::SelectItem\" no found!");
	[[unlikely]]
	if (!g_pfnApplyMultiDamage)
		UTIL_Terminate("Function \"::ApplyMultiDamage\" no found!");
	[[unlikely]]
	if (!g_pfnClearMultiDamage)
		UTIL_Terminate("Function \"::ClearMultiDamage\" no found!");
	[[unlikely]]
	if (!g_pfnAddMultiDamage)
		UTIL_Terminate("Function \"::AddMultiDamage\" no found!");
	[[unlikely]]
	if (!g_pfnDefaultDeploy)
		UTIL_Terminate("Function \"CBasePlayerWeapon::DefaultDeploy\" no found!");
	[[unlikely]]
	if (!g_pfnSwitchWeapon)
		UTIL_Terminate("Function \"CBasePlayer::SwitchWeapon\" no found!");
	[[unlikely]]
	if (!g_pfnFireBullets)
		UTIL_Terminate("Function \"CBaseEntity::FireBullets\" no found!");
	[[unlikely]]
	if (!g_pfnFireBullets3)
		UTIL_Terminate("Function \"CBaseEntity::FireBullets3\" no found!");
#endif

	HookInfo::FireBullets.m_Address = g_pfnFireBullets;
	HookInfo::FireBullets3.m_Address = g_pfnFireBullets3;

	UTIL_PreparePatch(g_pfnFireBullets, &OrpheuF_FireBullets, HookInfo::FireBullets.m_PatchedBytes, HookInfo::FireBullets.m_OriginalBytes);
	UTIL_PreparePatch(g_pfnFireBullets3, &OrpheuF_FireBullets3, HookInfo::FireBullets3.m_PatchedBytes, HookInfo::FireBullets3.m_OriginalBytes);

	UTIL_DoPatch(g_pfnFireBullets, HookInfo::FireBullets.m_PatchedBytes);
	UTIL_DoPatch(g_pfnFireBullets3, HookInfo::FireBullets3.m_PatchedBytes);

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
	gmsgShowMenu::Retrieve();

	gmsgWeaponAnim::m_iMessageIndex = SVC_WEAPONANIM;
}

void RetrieveCVarHandles(void) noexcept
{
	gcvarFriendlyFire = g_engfuncs.pfnCVarGetPointer("mp_friendlyfire");
	gcvarMaxSpeed = g_engfuncs.pfnCVarGetPointer("sv_maxspeed");
	gcvarMaxVelocity = g_engfuncs.pfnCVarGetPointer("sv_maxvelocity");
}

// Meta API

void fw_GameInit_Post(void) noexcept
{
	FileSystem_Init();
	Engine::Init();

	if (Engine::BUILD_NUMBER < 8684)
		UTIL_Terminate("Engine build '%d' mismatch from expected value: 8684.\nPlease use this plugin on a legal STEAM copy.", Engine::BUILD_NUMBER);
}

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

void fw_Think(edict_t *pent) noexcept
{
	[[unlikely]]
	if (auto const pGrenade = EHANDLE<CBaseEntity>(pent).As<CGrenade>();
		pGrenade != nullptr && !pGrenade->m_bIsC4 && pGrenade->pev->dmgtime < gpGlobals->time)
	{
		// This should be our cloud, if everything goes with the plan.
		if (auto const pCloud = pGrenade->m_pBombDefuser.As<CFuelAirCloud>();
			pCloud != nullptr && !pCloud->m_bIgnited)
		{
			pCloud->Ignite();
		}
	}
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
	CDynamicTarget::RetrieveModelInfo();

	// plugin_cfg

	g_engfuncs.pfnCvar_DirectSet(gcvarMaxSpeed, "99999.0");
	g_engfuncs.pfnCvar_DirectSet(gcvarMaxVelocity, "99999.0");

	TaskScheduler::Enroll(Task_GlobalCoughThink());
	TaskScheduler::Enroll(Task_UpdateTeams());
	TaskScheduler::Enroll(CFuelAirCloud::Task_AirPressure());

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
			UTIL_Terminate("Function \"CWorld::Precache\" no found!");
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

void fw_ServerDeactivate_Post(void) noexcept
{
	// Precache should be done across on every map change.
	g_bShouldPrecache = true;

	// CGameRules class is re-install every map change. Hence we should re-hook it everytime.
	g_pGameRules = nullptr;

	// Remove ALL existing tasks.
	TaskScheduler::Clear();
	
	/************ Regular Re-zero Actions ************/

	g_rgiAirSupportSelected.fill(AIR_STRIKE);
	g_rgpPlayersOfCT.clear();
	g_rgpPlayersOfTerrorist.clear();
}

void fw_PlayerPostThink(edict_t *pEntity) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// pre
}

int fw_PrecacheSound(const char *s) noexcept
{
	static constexpr std::array rgszRemoveSoundsPrecache =
	{
		"items/suitcharge1.wav",
		"items/suitchargeno1.wav",
		"items/suitchargeok1.wav",
		"player/geiger6.wav",
		"player/geiger5.wav",
		"player/geiger4.wav",
		"player/geiger3.wav",
		"player/geiger2.wav",
		"player/geiger1.wav",
		"weapons/bullet_hit1.wav",
		"weapons/bullet_hit2.wav",
		"items/weapondrop1.wav",
		"weapons/generic_reload.wav",
		"buttons/bell1.wav",
		"buttons/blip1.wav",
		"buttons/blip2.wav",
		"buttons/button11.wav",
		"buttons/latchunlocked2.wav",
		"buttons/lightswitch2.wav",
		"ambience/quail1.wav",
		"events/tutor_msg.wav",
		"events/enemy_died.wav",
		"events/friend_died.wav",
		"events/task_complete.wav",
		"weapons/ak47_clipout.wav",
		"weapons/ak47_clipin.wav",
		"weapons/ak47_boltpull.wav",
		"weapons/aug_clipout.wav",
		"weapons/aug_clipin.wav",
		"weapons/aug_boltpull.wav",
		"weapons/aug_boltslap.wav",
		"weapons/aug_forearm.wav",
		"weapons/c4_click.wav",
		"weapons/c4_beep1.wav",
		"weapons/c4_beep2.wav",
		"weapons/c4_beep3.wav",
		"weapons/c4_beep4.wav",
		"weapons/c4_beep5.wav",
		"weapons/c4_explode1.wav",
		"weapons/c4_plant.wav",
		"weapons/c4_disarm.wav",
		"weapons/c4_disarmed.wav",
		"weapons/elite_reloadstart.wav",
		"weapons/elite_leftclipin.wav",
		"weapons/elite_clipout.wav",
		"weapons/elite_sliderelease.wav",
		"weapons/elite_rightclipin.wav",
		"weapons/elite_deploy.wav",
		"weapons/famas_clipout.wav",
		"weapons/famas_clipin.wav",
		"weapons/famas_boltpull.wav",
		"weapons/famas_boltslap.wav",
		"weapons/famas_forearm.wav",
		"weapons/g3sg1_slide.wav",
		"weapons/g3sg1_clipin.wav",
		"weapons/g3sg1_clipout.wav",
		"weapons/galil_clipout.wav",
		"weapons/galil_clipin.wav",
		"weapons/galil_boltpull.wav",
		"weapons/m4a1_clipin.wav",
		"weapons/m4a1_clipout.wav",
		"weapons/m4a1_boltpull.wav",
		"weapons/m4a1_deploy.wav",
		"weapons/m4a1_silencer_on.wav",
		"weapons/m4a1_silencer_off.wav",
		"weapons/m249_boxout.wav",
		"weapons/m249_boxin.wav",
		"weapons/m249_chain.wav",
		"weapons/m249_coverup.wav",
		"weapons/m249_coverdown.wav",
		"weapons/mac10_clipout.wav",
		"weapons/mac10_clipin.wav",
		"weapons/mac10_boltpull.wav",
		"weapons/mp5_clipout.wav",
		"weapons/mp5_clipin.wav",
		"weapons/mp5_slideback.wav",
		"weapons/p90_clipout.wav",
		"weapons/p90_clipin.wav",
		"weapons/p90_boltpull.wav",
		"weapons/p90_cliprelease.wav",
		"weapons/p228_clipout.wav",
		"weapons/p228_clipin.wav",
		"weapons/p228_sliderelease.wav",
		"weapons/p228_slidepull.wav",
		"weapons/scout_bolt.wav",
		"weapons/scout_clipin.wav",
		"weapons/scout_clipout.wav",
		"weapons/sg550_boltpull.wav",
		"weapons/sg550_clipin.wav",
		"weapons/sg550_clipout.wav",
		"weapons/sg552_clipout.wav",
		"weapons/sg552_clipin.wav",
		"weapons/sg552_boltpull.wav",
		"weapons/ump45_clipout.wav",
		"weapons/ump45_clipin.wav",
		"weapons/ump45_boltslap.wav",
		"weapons/usp_clipout.wav",
		"weapons/usp_clipin.wav",
		"weapons/usp_silencer_on.wav",
		"weapons/usp_silencer_off.wav",
		"weapons/usp_sliderelease.wav",
		"weapons/usp_slideback.wav",
	};

	for (auto &&psz : rgszRemoveSoundsPrecache)
	{
		if (!strcmp(s, psz))
		{
			gpMetaGlobals->mres = MRES_SUPERCEDE;
			return 0;
		}
	}

	gpMetaGlobals->mres = MRES_IGNORED;
	return 0;
}

void fw_TraceLine_Post(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post

	if (g_bIsSomeoneShooting)
		CFuelAirCloud::OnTraceAttack(*ptr, pentToSkip);
}

int fw_CheckVisibility(const edict_t *entity, unsigned char *pset) noexcept
{
	if (entity->v.classname == MAKE_STRING(CJet::CLASSNAME))
	{
		gpMetaGlobals->mres = MRES_SUPERCEDE;
		return true;
	}

	gpMetaGlobals->mres = MRES_IGNORED;
	return false;
	// pre
}

void fw_SetGroupMask_Post(int mask, int op) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post
}

void fw_UpdateClientData_Post(const edict_t *ent, int sendweapons, clientdata_t *cd) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;
	// post

	if (EHANDLE<CBasePlayer> pPlayer(ent->v.pContainingEntity);	// fuck the constness
		cd->deadflag == DEAD_NO &&
		cd->m_iId == WEAPON_KNIFE &&
		pPlayer->m_pActiveItem &&
		pPlayer->m_pActiveItem->pev->weapons == RADIO_KEY &&
		pPlayer->m_flNextAttack <= 0)
	{
		cd->m_iId = WEAPON_NONE;	// remove client prediction.
	}
}

qboolean fw_AddToFullPack(entity_state_t *pState, int iEntIndex, edict_t *pEdict, edict_t *pClientSendTo, qboolean cl_lw, qboolean bIsPlayer, unsigned char *pSet) noexcept
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
			if (gpMetaGlobals->prev_mres == MRES_SUPERCEDE)	// It had been handled by other similar plugins.
				return;

			std::destroy_at(pPrefab);	// Thanks to C++17 we can finally patch up this old game.
			gpMetaGlobals->mres = MRES_SUPERCEDE;
		}
	}
}

qboolean fw_ShouldCollide(edict_t *pentTouched, edict_t *pentOther) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	if (gpMetaGlobals->prev_mres == MRES_SUPERCEDE)
		return *(int *)gpMetaGlobals->orig_ret;

	EHANDLE<CBaseEntity> pEntity(pentTouched), pOther(pentOther);

	if (auto const pPrefab = pEntity.As<Prefab_t>(); pPrefab && pOther)
	{
		gpMetaGlobals->mres = MRES_SUPERCEDE;
		return pPrefab->ShouldCollide(pOther);
	}

	return *(int *)gpMetaGlobals->orig_ret;
}

// Register Meta Hooks

inline constexpr DLL_FUNCTIONS gFunctionTable =
{
	.pfnGameInit	= nullptr,
	.pfnSpawn		= &fw_Spawn,
	.pfnThink		= &fw_Think,
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
	.pfnUpdateClientData= &fw_UpdateClientData_Post,
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
	.pfnGameShutdown			= []() noexcept { FileSystem_Shutdown(); },
	.pfnShouldCollide			= &fw_ShouldCollide,
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
	.pfnPrecacheSound	= &fw_PrecacheSound,
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
