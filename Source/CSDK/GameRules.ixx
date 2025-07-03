module;

#ifdef _DEBUG
#include <cassert>
#endif

export module GameRules;

import std;
import hlsdk;

import UtlHook;

import CBase;
import Engine;
import Platform;

export class IVoiceGameMgrHelper
{
public:
	virtual				~IVoiceGameMgrHelper() = default;

	// Called each frame to determine which players are allowed to hear each other.	This overrides
	// whatever squelch settings players have.
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker) = 0;
};

// CVoiceGameMgr manages which clients can hear which other clients.
export class CVoiceGameMgr
{
public:
	CVoiceGameMgr()							= default;
	virtual				~CVoiceGameMgr()	= default;

	bool				Init(
		IVoiceGameMgrHelper *m_pHelper,
		int maxClients
	);

	void				SetHelper(IVoiceGameMgrHelper *pHelper);

	// Updates which players can hear which other players.
	// If gameplay mode is DM, then only players within the PVS can hear each other.
	// If gameplay mode is teamplay, then only players on the same team can hear each other.
	// Player masks are always applied.
	void				Update(double frametime);

	// Called when a new client connects (unsquelches its entity for everyone).
	void				ClientConnected(struct edict_s *pEdict);

	// Called on ClientCommand. Checks for the squelch and unsquelch commands.
	// Returns true if it handled the command.
	bool				ClientCommand(CBasePlayer *pPlayer, const char *cmd);

	// Called to determine if the Receiver has muted (blocked) the Sender
	// Returns true if the receiver has blocked the sender
	bool				PlayerHasBlockedPlayer(CBasePlayer *pReceiver, CBasePlayer *pSender);


private:

	// Force it to update the client masks.
	void				UpdateMasks();


private:
	int					m_msgPlayerVoiceMask;
	int					m_msgRequestState;

	IVoiceGameMgrHelper *m_pHelper;
	int					m_nMaxPlayers;
	double				m_UpdateInterval;						// How long since the last update.
};

export class CGameRules
{
public:
	virtual void RefreshSkillData(void) = 0;
	virtual void Think(void) = 0;
	virtual qboolean IsAllowedToSpawn(CBaseEntity *pEntity) = 0;
	virtual qboolean FAllowFlashlight(void) = 0;
	virtual qboolean FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual qboolean GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon) = 0;
	virtual qboolean IsMultiplayer(void) = 0;
	virtual qboolean IsDeathmatch(void) = 0;
	virtual qboolean IsTeamplay(void) = 0;
	virtual qboolean IsCoOp(void) = 0;
	virtual const char *GetGameDescription(void) = 0;
	virtual qboolean ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]) = 0;
	virtual void InitHUD(CBasePlayer *pl) = 0;
	virtual void ClientDisconnected(edict_t *pClient) = 0;
	virtual void UpdateGameMode(CBasePlayer *pPlayer) = 0;
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer) = 0;
	virtual qboolean FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) = 0;
	virtual qboolean ShouldAutoAim(CBasePlayer *pPlayer, edict_t *target) = 0;
	virtual void PlayerSpawn(CBasePlayer *pPlayer) = 0;
	virtual void PlayerThink(CBasePlayer *pPlayer) = 0;
	virtual qboolean FPlayerCanRespawn(CBasePlayer *pPlayer) = 0;
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer) = 0;
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer) = 0;
	virtual qboolean AllowAutoTargetCrosshair(void) = 0;
	virtual qboolean ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd) = 0;
	virtual qboolean ClientCommand(CBasePlayer *pPlayer, const char *pcmd) = 0;
	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer) = 0;
	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled) = 0;
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual qboolean CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon) = 0;
	virtual qboolean CanHaveItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual int ItemShouldRespawn(CItem *pItem) = 0;
	virtual float FlItemRespawnTime(CItem *pItem) = 0;
	virtual Vector VecItemRespawnSpot(CItem *pItem) = 0;
	virtual qboolean CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry) = 0;
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount) = 0;
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo) = 0;
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlHealthChargerRechargeTime(void) = 0;
	virtual float FlHEVChargerRechargeTime(void) = 0;
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer) = 0;
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer) = 0;
	virtual const char *GetTeamID(CBaseEntity *pEntity) = 0;
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget) = 0;
	virtual int GetTeamIndex(const char *pTeamName) = 0;
	virtual const char *GetIndexedTeamName(int teamIndex) = 0;
	virtual qboolean IsValidTeam(const char *pTeamName) = 0;
	virtual void ChangePlayerTeam(CBasePlayer *pPlayer, const char *pTeamName, qboolean bKill, qboolean bGib) = 0;
	virtual const char *SetDefaultPlayerTeam(CBasePlayer *pPlayer) = 0;
	virtual qboolean PlayTextureSounds(void) = 0;
	virtual qboolean FAllowMonsters(void) = 0;
	virtual void EndMultiplayerGame(void) = 0;
	virtual qboolean IsFreezePeriod(void) = 0;
	virtual void ServerDeactivate(void) = 0;
	virtual void CheckMapConditions(void) = 0;

public:
	qboolean m_bFreezePeriod;
	qboolean m_bBombDropped;
};

export inline constexpr auto MAX_MAPS = 100;
export inline constexpr auto MAX_VIPQUEUES = 5;

export class CHalfLifeMultiplay : public CGameRules
{
//public:
//	CHalfLifeMultiplay(void);

public:
	virtual void Think(void) = 0;
	virtual void RefreshSkillData(void) = 0;
	virtual qboolean IsAllowedToSpawn(CBaseEntity *pEntity) = 0;
	virtual qboolean FAllowFlashlight(void) = 0;
	virtual qboolean FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual qboolean GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon) = 0;
	virtual qboolean IsMultiplayer(void) = 0;
	virtual qboolean IsDeathmatch(void) = 0;
	virtual qboolean IsCoOp(void) = 0;
	virtual qboolean ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]) = 0;
	virtual void InitHUD(CBasePlayer *pl) = 0;
	virtual void ClientDisconnected(edict_t *pClient) = 0;
	virtual void UpdateGameMode(CBasePlayer *pPlayer) = 0;
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer) = 0;
	virtual qboolean FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) = 0;
	virtual void PlayerSpawn(CBasePlayer *pPlayer) = 0;
	virtual void PlayerThink(CBasePlayer *pPlayer) = 0;
	virtual qboolean FPlayerCanRespawn(CBasePlayer *pPlayer) = 0;
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer) = 0;
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer) = 0;
	virtual qboolean AllowAutoTargetCrosshair(void) = 0;
	virtual qboolean ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd) = 0;
	virtual qboolean ClientCommand(CBasePlayer *pPlayer, const char *pcmd) = 0;
	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer) = 0;
	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled) = 0;
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual qboolean CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon) = 0;
	virtual qboolean CanHaveItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual int ItemShouldRespawn(CItem *pItem) = 0;
	virtual float FlItemRespawnTime(CItem *pItem) = 0;
	virtual Vector VecItemRespawnSpot(CItem *pItem) = 0;
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount) = 0;
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo) = 0;
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlHealthChargerRechargeTime(void) = 0;
	virtual float FlHEVChargerRechargeTime(void) = 0;
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer) = 0;
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer) = 0;
	virtual const char *GetTeamID(CBaseEntity *pEntity) = 0;
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget) = 0;
	virtual qboolean PlayTextureSounds(void) = 0;
	virtual qboolean FAllowMonsters(void) = 0;
	virtual void EndMultiplayerGame(void) = 0;
	virtual void CheckMapConditions(void) = 0;
	virtual void ServerDeactivate(void) = 0;
	virtual void CleanUpMap(void) = 0;
	virtual void RestartRound(void) = 0;
	virtual void CheckWinConditions(void) = 0;
	virtual void RemoveGuns(void) = 0;
	virtual void GiveC4(void) = 0;
	virtual void ChangeLevel(void) = 0;
	virtual void GoToIntermission(void) = 0;

//public:
//	void SendMOTDToClient(edict_t *client);
//	void InitializePlayerCounts(int &NumAliveTerrorist, int &NumAliveCT, int &NumDeadTerrorist, int &NumDeadCT);
//	qboolean NeededPlayersCheck(qboolean &bNeededPlayers);
//	qboolean VIPRoundEndCheck(qboolean bNeededPlayers);
//	qboolean PrisonRoundEndCheck(int NumAliveTerrorist, int NumAliveCT, int NumDeadTerrorist, int NumDeadCT, qboolean bNeededPlayers);
//	qboolean BombRoundEndCheck(qboolean bNeededPlayers);
//	qboolean TeamExterminationCheck(int NumAliveTerrorist, int NumAliveCT, int NumDeadTerrorist, int NumDeadCT, qboolean bNeededPlayers);
//	qboolean HostageRescueRoundEndCheck(qboolean bNeededPlayers);
//	void BalanceTeams(void);
//	qboolean IsThereABomber(void);
//	qboolean IsThereABomb(void);
//	qboolean TeamFull(int team_id);
//	qboolean TeamStacked(int newTeam_id, int curTeam_id);
//	void StackVIPQueue(void);
//	void CheckVIPQueue(void);
//	qboolean IsVIPQueueEmpty(void);
//	qboolean AddToVIPQueue(CBasePlayer *pPlayer);
//	void ResetCurrentVIP(void);
//	void PickNextVIP(void);
//	void CheckFreezePeriodExpired(void);
//	void CheckRoundTimeExpired(void);
//	void CheckLevelInitialized(void);
//	void CheckRestartRound(void);
//	qboolean CheckTimeLimit(void);
//	qboolean CheckMaxRounds(void);
//	qboolean CheckGameOver(void);
//	qboolean CheckWinLimit(void);
//	void CheckAllowSpecator(void);
//	void CheckGameCvar(void);
//	void DisplayMaps(CBasePlayer *pPlayer, int mapId);
//	void ResetAllMapVotes(void);
//	void ProcessMapVote(CBasePlayer *pPlayer, int mapId);
//	void UpdateTeamScores(void);
//	void SwapAllPlayers(void);
//	void TerminateRound(float tmDelay, int iWinStatus);
//	void QueueCareerRoundEndMenu(float tmDelay, int iWinStatus);
//	float TimeRemaining(void) { return m_iRoundTimeSecs - gpGlobals->time + m_fRoundCount; }
//	qboolean HasRoundTimeExpired(void);
//	qboolean IsBombPlanted(void);
//	void MarkLivingPlayersOnTeamAsNotReceivingMoneyNextRound(int team);
//	void CareerRestart(void);
//	qboolean IsCareer(void) { return FALSE; }

public:
	CVoiceGameMgr m_VoiceGameMgr;
	float m_flRestartRoundTime;			// ReGameDLL: The global time when the round is supposed to end, if this is not 0 (deprecated name m_fTeamCount)
	float m_flCheckWinConditions;
	float m_fRoundStartTime;			// ReGameDLL: Time round has started (deprecated name m_fRoundCount)
	int m_iRoundTime;
	int m_iRoundTimeSecs;
	int m_iIntroRoundTime;
	float m_fIntroRoundCount;
	int m_iAccountTerrorist;
	int m_iAccountCT;
	int m_iNumTerrorist;
	int m_iNumCT;
	int m_iNumSpawnableTerrorist;
	int m_iNumSpawnableCT;
	int m_iSpawnPointCount_Terrorist;
	int m_iSpawnPointCount_CT;
	int m_iHostagesRescued;
	int m_iHostagesTouched;
	int m_iRoundWinStatus;
	short m_iNumCTWins;
	short m_iNumTerroristWins;
	bool m_bTargetBombed;
	bool m_bBombDefused;
	bool m_bMapHasBombTarget;
	bool m_bMapHasBombZone;
	bool m_bMapHasBuyZone;
	bool m_bMapHasRescueZone;
	bool m_bMapHasEscapeZone;
	int m_iMapHasVIPSafetyZone;
	bool m_bMapHasCameras;
	int m_iC4Timer;
	int m_iC4Guy;
	int m_iLoserBonus;
	int m_iNumConsecutiveCTLoses;
	int m_iNumConsecutiveTerroristLoses;
	float m_fMaxIdlePeriod;
	int m_iLimitTeams;
	bool m_bLevelInitialized;
	bool m_bRoundTerminating;
	bool m_bCompleteReset;
	float m_flRequiredEscapeRatio;
	int m_iNumEscapers;
	int m_iHaveEscaped;
	bool m_bCTCantBuy;
	bool m_bTCantBuy;
	float m_flBombRadius;
	int m_iConsecutiveVIP;
	int m_iTotalGunCount;
	int m_iTotalGrenadeCount;
	int m_iTotalArmourCount;
	int m_iUnBalancedRounds;
	int m_iNumEscapeRounds;
	int m_iMapVotes[MAX_MAPS];
	int m_iLastPick;
	int m_iMaxMapTime;
	int m_iMaxRounds;
	int m_iTotalRoundsPlayed;
	int m_iMaxRoundsWon;
	int m_iStoredSpectValue;
	float m_flForceCameraValue;
	float m_flForceChaseCamValue;
	float m_flFadeToBlackValue;
	CBasePlayer *m_pVIP;
	CBasePlayer *VIPQueue[MAX_VIPQUEUES];
	float m_flIntermissionEndTime;
	float m_flIntermissionStartTime;
	int m_iEndIntermissionButtonHit;
	float m_tmNextPeriodicThink;
	bool m_bFirstConnected;
	bool m_bInCareerGame;
	float m_fCareerRoundMenuTime;
	int m_iCareerMatchWins;
	int m_iRoundWinDifference;
	float m_fCareerMatchMenuTime;
	bool m_bSkipSpawn;
};

// Reset this value to nullptr during ServerDeactivate_Post
export inline CHalfLifeMultiplay *g_pGameRules = nullptr;

export inline constexpr unsigned char CWORLD_PRECACHE_FN_NEW_PATTERN[] = "\x90\x55\x57\x33\xFF\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x8B\xE9\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D";
export inline constexpr unsigned char CWORLD_PRECACHE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x51\x57\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x8B\xF9\xC7\x05";

// This hook is very special, since it is actually delete-newed in each new game.
// Therefore we must hook it every time.
export void RetrieveGameRules() noexcept
{
	auto addr = (std::uintptr_t)UTIL_SearchPattern("mp.dll", 1, CWORLD_PRECACHE_FN_NEW_PATTERN, CWORLD_PRECACHE_FN_ANNIV_PATTERN);

#ifdef _DEBUG
	assert(addr != 0);
#else
	[[unlikely]]
	if (!addr)
		UTIL_Terminate("Function \"CWorld::Precache\" no found!");
#endif

	static constexpr std::ptrdiff_t ofs_anniv = 0xC24E3 - 0xC2440;
	static constexpr std::ptrdiff_t ofs_new = 0xD29B4 - 0xD2940;

	addr += Engine::BUILD_NUMBER >= Engine::ANNIVERSARY ? ofs_anniv : ofs_new;
	g_pGameRules = *(CHalfLifeMultiplay**)(void**)(*(long*)addr);

#ifdef _DEBUG
	assert(g_pGameRules != nullptr);
#else
	[[unlikely]]
	if (!g_pGameRules)
		UTIL_Terminate("Pointer \"g_pGameRules\" remains nullptr after pattern searching!");
#endif

	// However, the hook status remains even if the game reloaded.
	// Still need this method to make sure the hooks are happened only once.
}
