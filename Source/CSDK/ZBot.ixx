module;

#include <cassert>

export module ZBot;

import std;

import UtlHook;

//import Engine;
import CBase;
import Platform;

export enum GameEventType
{
	EVENT_INVALID = 0,
	EVENT_WEAPON_FIRED,					// tell bots the player is attack (argumens: 1 = attacker, 2 = NULL)
	EVENT_WEAPON_FIRED_ON_EMPTY,		// tell bots the player is attack without clip ammo (argumens: 1 = attacker, 2 = NULL)
	EVENT_WEAPON_RELOADED,				// tell bots the player is reloading his weapon (argumens: 1 = reloader, 2 = NULL)

	EVENT_HE_GRENADE_EXPLODED,			// tell bots the HE grenade is exploded (argumens: 1 = grenade thrower, 2 = NULL)
	EVENT_FLASHBANG_GRENADE_EXPLODED,	// tell bots the flashbang grenade is exploded (argumens: 1 = grenade thrower, 2 = explosion origin)
	EVENT_SMOKE_GRENADE_EXPLODED,		// tell bots the smoke grenade is exploded (argumens: 1 = grenade thrower, 2 = NULL)
	EVENT_GRENADE_BOUNCED,

	EVENT_BEING_SHOT_AT,
	EVENT_PLAYER_BLINDED_BY_FLASHBANG,	// tell bots the player is flashed (argumens: 1 = flashed player, 2 = NULL)
	EVENT_PLAYER_FOOTSTEP,				// tell bots the player is running (argumens: 1 = runner, 2 = NULL)
	EVENT_PLAYER_JUMPED,				// tell bots the player is jumped (argumens: 1 = jumper, 2 = NULL)
	EVENT_PLAYER_DIED,					// tell bots the player is killed (argumens: 1 = victim, 2 = killer)
	EVENT_PLAYER_LANDED_FROM_HEIGHT,	// tell bots the player is fell with some damage (argumens: 1 = felled player, 2 = NULL)
	EVENT_PLAYER_TOOK_DAMAGE,			// tell bots the player is take damage (argumens: 1 = victim, 2 = attacker)
	EVENT_HOSTAGE_DAMAGED,				// tell bots the player has injured a hostage (argumens: 1 = hostage, 2 = injurer)
	EVENT_HOSTAGE_KILLED,				// tell bots the player has killed a hostage (argumens: 1 = hostage, 2 = killer)

	EVENT_DOOR,							// tell bots the door is moving (argumens: 1 = door, 2 = NULL)
	EVENT_BREAK_GLASS,					// tell bots the glass has break (argumens: 1 = glass, 2 = NULL)
	EVENT_BREAK_WOOD,					// tell bots the wood has break (argumens: 1 = wood, 2 = NULL)
	EVENT_BREAK_METAL,					// tell bots the metal/computer has break (argumens: 1 = metal/computer, 2 = NULL)
	EVENT_BREAK_FLESH,					// tell bots the flesh has break (argumens: 1 = flesh, 2 = NULL)
	EVENT_BREAK_CONCRETE,				// tell bots the concrete has break (argumens: 1 = concrete, 2 = NULL)

	EVENT_BOMB_PLANTED,					// tell bots the bomb has been planted (argumens: 1 = planter, 2 = NULL)
	EVENT_BOMB_DROPPED,					// tell bots the bomb has been dropped (argumens: 1 = NULL, 2 = NULL)
	EVENT_BOMB_PICKED_UP,				// let the bots hear the bomb pickup (argumens: 1 = player that pickup c4, 2 = NULL)
	EVENT_BOMB_BEEP,					// let the bots hear the bomb beeping (argumens: 1 = c4, 2 = NULL)
	EVENT_BOMB_DEFUSING,				// tell the bots someone has started defusing (argumens: 1 = defuser, 2 = NULL)
	EVENT_BOMB_DEFUSE_ABORTED,			// tell the bots someone has aborted defusing (argumens: 1 = NULL, 2 = NULL)
	EVENT_BOMB_DEFUSED,					// tell the bots the bomb is defused (argumens: 1 = defuser, 2 = NULL)
	EVENT_BOMB_EXPLODED,				// let the bots hear the bomb exploding (argumens: 1 = NULL, 2 = NULL)

	EVENT_HOSTAGE_USED,					// tell bots the hostage is used (argumens: 1 = user, 2 = NULL)
	EVENT_HOSTAGE_RESCUED,				// tell bots the hostage is rescued (argumens: 1 = rescuer (CBasePlayer *), 2 = hostage (CHostage *))
	EVENT_ALL_HOSTAGES_RESCUED,			// tell bots the all hostages are rescued (argumens: 1 = NULL, 2 = NULL)

	EVENT_VIP_ESCAPED,					// tell bots the VIP is escaped (argumens: 1 = NULL, 2 = NULL)
	EVENT_VIP_ASSASSINATED,				// tell bots the VIP is assassinated (argumens: 1 = NULL, 2 = NULL)
	EVENT_TERRORISTS_WIN,				// tell bots the terrorists won the round (argumens: 1 = NULL, 2 = NULL)
	EVENT_CTS_WIN,						// tell bots the CTs won the round (argumens: 1 = NULL, 2 = NULL)
	EVENT_ROUND_DRAW,					// tell bots the round was a draw (argumens: 1 = NULL, 2 = NULL)
	EVENT_ROUND_WIN,					// tell carreer the round was a win (argumens: 1 = NULL, 2 = NULL)
	EVENT_ROUND_LOSS,					// tell carreer the round was a loss (argumens: 1 = NULL, 2 = NULL)
	EVENT_ROUND_START,					// tell bots the round was started (when freeze period is expired) (argumens: 1 = NULL, 2 = NULL)
	EVENT_PLAYER_SPAWNED,				// tell bots the player is spawned (argumens: 1 = spawned player, 2 = NULL)
	EVENT_CLIENT_CORPSE_SPAWNED,
	EVENT_BUY_TIME_START,
	EVENT_PLAYER_LEFT_BUY_ZONE,
	EVENT_DEATH_CAMERA_START,
	EVENT_KILL_ALL,
	EVENT_ROUND_TIME,
	EVENT_DIE,
	EVENT_KILL,
	EVENT_HEADSHOT,
	EVENT_KILL_FLASHBANGED,
	EVENT_TUTOR_BUY_MENU_OPENNED,
	EVENT_TUTOR_AUTOBUY,
	EVENT_PLAYER_BOUGHT_SOMETHING,
	EVENT_TUTOR_NOT_BUYING_ANYTHING,
	EVENT_TUTOR_NEED_TO_BUY_PRIMARY_WEAPON,
	EVENT_TUTOR_NEED_TO_BUY_PRIMARY_AMMO,
	EVENT_TUTOR_NEED_TO_BUY_SECONDARY_AMMO,
	EVENT_TUTOR_NEED_TO_BUY_ARMOR,
	EVENT_TUTOR_NEED_TO_BUY_DEFUSE_KIT,
	EVENT_TUTOR_NEED_TO_BUY_GRENADE,
	EVENT_CAREER_TASK_DONE,

	EVENT_START_RADIO_1,
	EVENT_RADIO_COVER_ME,
	EVENT_RADIO_YOU_TAKE_THE_POINT,
	EVENT_RADIO_HOLD_THIS_POSITION,
	EVENT_RADIO_REGROUP_TEAM,
	EVENT_RADIO_FOLLOW_ME,
	EVENT_RADIO_TAKING_FIRE,
	EVENT_START_RADIO_2,
	EVENT_RADIO_GO_GO_GO,
	EVENT_RADIO_TEAM_FALL_BACK,
	EVENT_RADIO_STICK_TOGETHER_TEAM,
	EVENT_RADIO_GET_IN_POSITION_AND_WAIT,
	EVENT_RADIO_STORM_THE_FRONT,
	EVENT_RADIO_REPORT_IN_TEAM,
	EVENT_START_RADIO_3,
	EVENT_RADIO_AFFIRMATIVE,
	EVENT_RADIO_ENEMY_SPOTTED,
	EVENT_RADIO_NEED_BACKUP,
	EVENT_RADIO_SECTOR_CLEAR,
	EVENT_RADIO_IN_POSITION,
	EVENT_RADIO_REPORTING_IN,
	EVENT_RADIO_GET_OUT_OF_THERE,
	EVENT_RADIO_NEGATIVE,
	EVENT_RADIO_ENEMY_DOWN,
	EVENT_END_RADIO,

	EVENT_NEW_MATCH,				// tell bots the game is new (argumens: 1 = NULL, 2 = NULL)
	EVENT_PLAYER_CHANGED_TEAM,		// tell bots the player is switch his team (also called from ClientPutInServer()) (argumens: 1 = switcher, 2 = NULL)
	EVENT_BULLET_IMPACT,			// tell bots the player is shoot at wall (argumens: 1 = shooter, 2 = shoot trace end position)
	EVENT_GAME_COMMENCE,			// tell bots the game is commencing (argumens: 1 = NULL, 2 = NULL)
	EVENT_WEAPON_ZOOMED,			// tell bots the player is switch weapon zoom (argumens: 1 = zoom switcher, 2 = NULL)
	EVENT_HOSTAGE_CALLED_FOR_HELP,	// tell bots the hostage is talking (argumens: 1 = listener, 2 = NULL)
	NUM_GAME_EVENTS,
};


export class ActiveGrenade
{
public:
	//ActiveGrenade(int weaponID, CGrenade* grenadeEntity);

	//void OnEntityGone();
	//bool IsValid() const;

	//bool IsEntity(CGrenade* grenade) const { return (grenade == m_entity) ? true : false; }
	//int GetID() const { return m_id; }
	//const Vector* GetDetonationPosition() const { return &m_detonationPosition; }
	//const Vector* GetPosition() const;

private:
	int m_id;
	CGrenade* m_entity;
	Vector m_detonationPosition;
	float m_dieTimestamp;
};

using ActiveGrenadeList = std::list<ActiveGrenade*>;

export class CBotManager
{
public:
	CBotManager();
	virtual ~CBotManager() {}

	virtual void ClientDisconnect(CBasePlayer* pPlayer) = 0;
	virtual qboolean ClientCommand(CBasePlayer* pPlayer, const char* pcmd) = 0;

	virtual void ServerActivate() = 0;
	virtual void ServerDeactivate() = 0;

	virtual void ServerCommand(const char* pcmd) = 0;
	virtual void AddServerCommand(const char* cmd) = 0;
	virtual void AddServerCommands() = 0;

	virtual void RestartRound();
	virtual void StartFrame();

	// Events are propogated to all bots.
	virtual void OnEvent(GameEventType event, CBaseEntity* pEntity = nullptr, CBaseEntity* pOther = nullptr);	// Invoked when event occurs in the game (some events have NULL entity).
	virtual unsigned int GetPlayerPriority(CBasePlayer* pPlayer) const = 0;										// return priority of player (0 = max pri)

public:
	//const char* GetNavMapFilename() const;										// return the filename for this map's "nav" file

	//void AddGrenade(int type, CGrenade* grenade);								// add an active grenade to the bot's awareness
	//void RemoveGrenade(CGrenade* grenade);										// the grenade entity in the world is going away
	//void ValidateActiveGrenades();												// destroy any invalid active grenades
	//void DestroyAllGrenades();

	//bool IsLineBlockedBySmoke(const Vector* from, const Vector* to);			// return true if line intersects smoke volume
	//bool IsInsideSmokeCloud(const Vector* pos);									// return true if position is inside a smoke cloud

private:
	// the list of active grenades the bots are aware of
	ActiveGrenadeList m_activeGrenadeList;
};

// Simple class for counting down a short interval of time
export class CountdownTimer
{
public:
	CountdownTimer() { m_timestamp = -1.0f; m_duration = 0.0f; }
	void Reset() { m_timestamp = gpGlobals->time + m_duration; }

	void Start(float duration) { m_timestamp = gpGlobals->time + duration; m_duration = duration; }
	bool HasStarted() const { return (m_timestamp > 0.0f); }

	void Invalidate() { m_timestamp = -1.0f; }
	bool IsElapsed() const { return (gpGlobals->time > m_timestamp); }

private:
	float m_duration;
	float m_timestamp;
};

export class CCSBotManager : public CBotManager
{
public:
	CCSBotManager();

	virtual void ClientDisconnect(CBasePlayer* pPlayer) = 0;
	virtual qboolean ClientCommand(CBasePlayer* pPlayer, const char* pcmd) = 0;

	virtual void ServerActivate() = 0;
	virtual void ServerDeactivate() = 0;

	virtual void ServerCommand(const char* pcmd) = 0;
	virtual void AddServerCommand(const char* cmd) = 0;
	virtual void AddServerCommands() = 0;

	virtual void RestartRound() = 0;										// (EXTEND) invoked when a new round begins
	virtual void StartFrame() = 0;											// (EXTEND) called each frame

	virtual void OnEvent(GameEventType event, CBaseEntity* pEntity = nullptr, CBaseEntity* pOther = nullptr) = 0;
	virtual unsigned int GetPlayerPriority(CBasePlayer* pPlayer) const = 0;	// return priority of pPlayer (0 = max pri)
	virtual bool IsImportantPlayer(CBasePlayer* pPlayer) const = 0;			// return true if pPlayer is important to scenario (VIP, bomb carrier, etc)

public:
	//void ValidateMapData();
	//void OnFreeEntPrivateData(CBaseEntity* pEntity);
	//bool IsLearningMap() const { return m_isLearningMap; }
	//void SetLearningMapFlag() { m_isLearningMap = true; }
	//bool IsAnalysisRequested() const { return m_isAnalysisRequested; }
	//void RequestAnalysis() { m_isAnalysisRequested = true; }
	//void AckAnalysisRequest() { m_isAnalysisRequested = false; }

	//// difficulty levels
	//static BotDifficultyType GetDifficultyLevel()
	//{
	//	if (cv_bot_difficulty.value < 0.9f)
	//		return BOT_EASY;

	//	if (cv_bot_difficulty.value < 1.9f)
	//		return BOT_NORMAL;

	//	if (cv_bot_difficulty.value < 2.9f)
	//		return BOT_HARD;

	//	return BOT_EXPERT;
	//}

	// the supported game scenarios
	enum GameScenarioType
	{
		SCENARIO_DEATHMATCH,
		SCENARIO_DEFUSE_BOMB,
		SCENARIO_RESCUE_HOSTAGES,
		SCENARIO_ESCORT_VIP
	};

	//GameScenarioType GetScenario() const
	//{
	//	// if we have included deathmatch mode, so set the game type like SCENARIO_DEATHMATCH
	//	if (cv_bot_deathmatch.value > 0)
	//		return SCENARIO_DEATHMATCH;

	//	return m_gameScenario;
	//}

	// "zones"
	// depending on the game mode, these are bomb zones, rescue zones, etc.
	//enum { MAX_ZONES = 4 };						// max # of zones in a map
	//enum { MAX_ZONE_NAV_AREAS = 16 };			// max # of nav areas in a zone
	//struct Zone
	//{
	//	CBaseEntity* m_entity;					// the map entity
	//	CNavArea* m_area[MAX_ZONE_NAV_AREAS];	// nav areas that overlap this zone
	//	int m_areaCount;
	//	Vector m_center;
	//	bool m_isLegacy;						// if true, use pev->origin and 256 unit radius as zone
	//	int m_index;
	//	Extent m_extent;
	//};

	//const Zone* GetZone(int i) const { return &m_zone[i]; }
	//const Zone* GetZone(const Vector* pos) const;										// return the zone that contains the given position
	//const Zone* GetClosestZone(const Vector* pos) const;								// return the closest zone to the given position
	//const Zone* GetClosestZone(const CBaseEntity* pEntity) const { return GetClosestZone(&pEntity->pev->origin); }		// return the closest zone to the given entity
	//int GetZoneCount() const { return m_zoneCount; }

	//const Vector* GetRandomPositionInZone(const Zone* zone) const;
	//CNavArea* GetRandomAreaInZone(const Zone* zone) const;

	// Return the zone closest to the given position, using the given cost heuristic
	//template<typename CostFunctor>
	//const Zone* GetClosestZone(CNavArea* startArea, CostFunctor costFunc, float* travelDistance = nullptr) const
	//{
	//	const Zone* closeZone = nullptr;
	//	float closeDist = 99999999.9f;

	//	if (startArea == nullptr)
	//		return nullptr;

	//	for (int i = 0; i < m_zoneCount; i++)
	//	{
	//		if (m_zone[i].m_areaCount == 0)
	//			continue;

	//		// just use the first overlapping nav area as a reasonable approximation
	//		real_t dist = NavAreaTravelDistance(startArea, m_zone[i].m_area[0], costFunc);

	//		if (dist >= 0.0f && dist < closeDist)
	//		{
	//			closeZone = &m_zone[i];
	//			closeDist = dist;
	//		}
	//	}

	//	if (travelDistance)
	//		*travelDistance = closeDist;

	//	return closeZone;
	//}

	// pick a zone at random and return it
	//const Zone* GetRandomZone() const
	//{
	//	if (!m_zoneCount)
	//		return nullptr;

	//	return &m_zone[RANDOM_LONG(0, m_zoneCount - 1)];
	//}

	bool IsBombPlanted() const { return m_isBombPlanted; }											// returns true if bomb has been planted
	float GetBombPlantTimestamp() const { return m_bombPlantTimestamp; }							// return time bomb was planted
	bool IsTimeToPlantBomb() const { return (gpGlobals->time >= m_earliestBombPlantTimestamp); }	// return true if it's ok to try to plant bomb
	CBasePlayer* GetBombDefuser() const { return m_bombDefuser; }									// return the player currently defusing the bomb, or NULL
	//float GetBombTimeLeft() const;																	// get the time remaining before the planted bomb explodes
	CBaseEntity* GetLooseBomb() { return m_looseBomb; }												// return the bomb if it is loose on the ground
	//CNavArea* GetLooseBombArea() const { return m_looseBombArea; }									// return area that bomb is in/near
	//void SetLooseBomb(CBaseEntity* bomb);

	//float GetRadioMessageTimestamp(GameEventType event, int teamID) const;							// return the last time the given radio message was sent for given team
	//float GetRadioMessageInterval(GameEventType event, int teamID) const;							// return the interval since the last time this message was sent
	//void SetRadioMessageTimestamp(GameEventType event, int teamID);
	//void ResetRadioMessageTimestamps();

	float GetLastSeenEnemyTimestamp() const { return m_lastSeenEnemyTimestamp; }					// return the last time anyone has seen an enemy
	void SetLastSeenEnemyTimestamp() { m_lastSeenEnemyTimestamp = gpGlobals->time; }

	float GetRoundStartTime()      const { return m_roundStartTimestamp; }
	float GetElapsedRoundTime()    const { return gpGlobals->time - m_roundStartTimestamp; }			// return the elapsed time since the current round began

	//bool AllowRogues()             const { return cv_bot_allow_rogues.value != 0.0f; }
	//bool AllowPistols()            const { return cv_bot_allow_pistols.value != 0.0f; }
	//bool AllowShotguns()           const { return cv_bot_allow_shotguns.value != 0.0f; }
	//bool AllowSubMachineGuns()     const { return cv_bot_allow_sub_machine_guns.value != 0.0f; }
	//bool AllowRifles()             const { return cv_bot_allow_rifles.value != 0.0f; }
	//bool AllowMachineGuns()        const { return cv_bot_allow_machine_guns.value != 0.0f; }
	//bool AllowGrenades()           const { return cv_bot_allow_grenades.value != 0.0f; }
	//bool AllowSnipers()            const { return cv_bot_allow_snipers.value != 0.0f; }
	//bool AllowTacticalShield()     const { return cv_bot_allow_shield.value != 0.0f; }
	//bool AllowFriendlyFireDamage() const { return friendlyfire.value != 0.0f; }

	//bool IsWeaponUseable(CBasePlayerItem* item) const;						// return true if the bot can use this weapon
	//bool IsWeaponUseable(ArmouryItemPack item) const;

	bool IsDefenseRushing() const { return m_isDefenseRushing; }			// returns true if defense team has "decided" to rush this round
	//bool IsOnDefense(CBasePlayer* pPlayer) const;							// return true if this player is on "defense"
	//bool IsOnOffense(CBasePlayer* pPlayer) const;							// return true if this player is on "offense"

	bool IsRoundOver() const { return m_isRoundOver; }						// return true if the round has ended

	unsigned int GetNavPlace() const { return m_navPlace; }
	void SetNavPlace(unsigned int place) { m_navPlace = place; }

	enum SkillType { LOW, AVERAGE, HIGH, RANDOM };
	//const char* GetRandomBotName(SkillType skill);

//	void MonitorBotCVars();
//	void MaintainBotQuota();
//	bool AddBot(const BotProfile* profile, BotProfileTeamType team);
//
//#define FROM_CONSOLE true
//	bool BotAddCommand(BotProfileTeamType team, bool isFromConsole = false);	// process the "bot_add" console command

private:
	//static float m_flNextCVarCheck;
	//static bool m_isMapDataLoaded;		// true if we've attempted to load map data
	//static bool m_isLearningMap;
	//static bool m_isAnalysisRequested;

	GameScenarioType m_gameScenario;			// what kind of game are we playing

	//Zone m_zone[MAX_ZONES];
	std::uint8_t m_zone[116 * 4];
	int m_zoneCount;

	bool m_isBombPlanted;						// true if bomb has been planted
	float m_bombPlantTimestamp;					// time bomb was planted
	float m_earliestBombPlantTimestamp;			// don't allow planting until after this time has elapsed
	CBasePlayer* m_bombDefuser;					// the player currently defusing a bomb
	EHANDLE<CBaseEntity> m_looseBomb;			// will be non-NULL if bomb is loose on the ground
	//CNavArea* m_looseBombArea;					// area that bomb is is/near
	void* m_looseBombArea;

	bool m_isRoundOver;							// true if the round has ended

	float m_radioMsgTimestamp[24][2];

	float m_lastSeenEnemyTimestamp;
	float m_roundStartTimestamp;				// the time when the current round began

	bool m_isDefenseRushing;					// whether defensive team is rushing this round or not

	//static NavEditCmdType m_editCmd;
	unsigned int m_navPlace;
	CountdownTimer m_respawnTimer;
	bool m_isRespawnStarted;
	bool m_canRespawn;
	bool m_bServerActive;
};


export namespace ZBot
{
	inline constexpr unsigned char INSTALL_BOT_CONTROL_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x2A\x2A\x2A\x2A\x50\x51\x56\x57\xA1\x2A\x2A\x2A\x2A\x33\xC5";

	inline CBotManager** ppTheBots{ nullptr };

	// Need to be retrieve every new game. #NO_URGENT
	void RetrieveManager(void) noexcept
	{
		auto addr = (std::uintptr_t)UTIL_SearchPattern("mp.dll", INSTALL_BOT_CONTROL_FN_ANNIV_PATTERN, 1);

		[[unlikely]]
		if (!addr)
		{
			UTIL_Terminate("Function \"::InstallBotControl\" no found!");
		}

		static constexpr std::ptrdiff_t ofs_anniv = 0x1F357 - 0x1F330;

		addr += ofs_anniv;
		//auto v = *(CBotManager**)(void**)(*(long*)addr);

		//assert(v != nullptr);
		ppTheBots = (CBotManager**)(void**)(*(long*)addr);
	}
}