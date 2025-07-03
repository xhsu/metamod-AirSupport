module;

#include <cassert>
#include <cstddef>	// offsetof

export module CBase;

// LUNA: This module consists of multiple portions.
//		 The other one contains the runtime extension, like the hash table functions.
//		 Find more in ConditionZero.ixx
//		 For virtual functions, they are located in VTFH.ixx

import std;
import hlsdk;

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
export inline constexpr auto FCAP_CUSTOMSAVE = 0x00000001;
export inline constexpr auto FCAP_ACROSS_TRANSITION = 0x00000002; // Should transfer between transitions
export inline constexpr auto FCAP_MUST_SPAWN = 0x00000004; // Spawn after restore
export inline constexpr auto FCAP_DONT_SAVE = 0x80000000; // Don't save this
export inline constexpr auto FCAP_IMPULSE_USE = 0x00000008; // Can be used by the player
export inline constexpr auto FCAP_CONTINUOUS_USE = 0x00000010; // Can be used by the player
export inline constexpr auto FCAP_ONOFF_USE = 0x00000020; // Can be used by the player
export inline constexpr auto FCAP_DIRECTIONAL_USE = 0x00000040; // Player sends +/- 1 when using (currently only tracktrains)
export inline constexpr auto FCAP_MASTER = 0x00000080; // Can be used to "master" other entities (like multisource)
export inline constexpr auto FCAP_MUST_RESET = 0x00000100; // Should reset on the new round
export inline constexpr auto FCAP_MUST_RELEASE = 0x00000200; // Should release on the new round

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
export inline constexpr auto FCAP_FORCE_TRANSITION = 0x00000080; // ALWAYS goes across transitions

// for Classify
export inline constexpr auto CLASS_NONE = 0;
export inline constexpr auto CLASS_MACHINE = 1;
export inline constexpr auto CLASS_PLAYER = 2;
export inline constexpr auto CLASS_HUMAN_PASSIVE = 3;
export inline constexpr auto CLASS_HUMAN_MILITARY = 4;
export inline constexpr auto CLASS_ALIEN_MILITARY = 5;
export inline constexpr auto CLASS_ALIEN_PASSIVE = 6;
export inline constexpr auto CLASS_ALIEN_MONSTER = 7;
export inline constexpr auto CLASS_ALIEN_PREY = 8;
export inline constexpr auto CLASS_ALIEN_PREDATOR = 9;
export inline constexpr auto CLASS_INSECT = 10;
export inline constexpr auto CLASS_PLAYER_ALLY = 11;
export inline constexpr auto CLASS_PLAYER_BIOWEAPON = 12; // hornets and snarks.launched by players
export inline constexpr auto CLASS_ALIEN_BIOWEAPON = 13; // hornets and snarks.launched by the alien menace
export inline constexpr auto CLASS_VEHICLE = 14;
export inline constexpr auto CLASS_BARNACLE = 99; // special because no one pays attention to it, and it eats a wide cross-section of creatures.

export inline constexpr auto SF_NORESPAWN = (1<<30); // set this bit on guns and stuff that should never respawn.

export enum USE_TYPE
{
	USE_OFF,
	USE_ON,
	USE_SET,
	USE_TOGGLE
};

class CBasePlayerItem;
class CBaseMonster;
class CBasePlayer;

export class CBaseEntity
{
public:
	virtual void Spawn(void) = 0;
	virtual void Precache(void) = 0;
	virtual void Restart(void) = 0;
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual int Save(void *save) = 0;	// CSave &
	virtual int Restore(void *restore) = 0;	// CRestore &
	virtual int ObjectCaps(void) = 0;
	virtual void Activate(void) = 0;
	virtual void SetObjectCollisionBox(void) = 0;
	virtual int Classify(void) = 0;
	virtual void DeathNotice(entvars_t *pevChild) = 0;
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) = 0;
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual int TakeHealth(float flHealth, int bitsDamageType) = 0;
	virtual void Killed(entvars_t *pevAttacker, int iGib) = 0;
	virtual int BloodColor(void) = 0;
	virtual void TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) = 0;
	virtual qboolean IsTriggered(CBaseEntity *pActivator) = 0;
	virtual CBaseMonster *MyMonsterPointer(void) = 0;
	virtual void *MySquadMonsterPointer(void) = 0;	// CSquadMonster *
	virtual int GetToggleState(void) = 0;
	virtual void AddPoints(int score, qboolean bAllowNegativeScore) = 0;
	virtual void AddPointsToTeam(int score, qboolean bAllowNegativeScore) = 0;
	virtual qboolean AddPlayerItem(CBasePlayerItem *pItem) = 0;
	virtual qboolean RemovePlayerItem(CBasePlayerItem *pItem) = 0;
	virtual int GiveAmmo(int iAmount, char *szName, int iMax) = 0;	// LUNA: the 'char*' here is cursed. If I swap it to 'const char*', the entire [pure] virtual function table will be doomed. #INVESTIGATE
	virtual float GetDelay(void) = 0;
	virtual int IsMoving(void) = 0;
	virtual void OverrideReset(void) = 0;
	virtual int DamageDecal(int bitsDamageType) = 0;
	virtual void SetToggleState(int state) = 0;
	virtual void StartSneaking(void) = 0;
	virtual void StopSneaking(void) = 0;
	virtual qboolean OnControls(entvars_t *onpev) = 0;
	virtual qboolean IsSneaking(void) = 0;
	virtual qboolean IsAlive(void) = 0;
	virtual qboolean IsBSPModel(void) = 0;
	virtual qboolean ReflectGauss(void) = 0;
	virtual qboolean HasTarget(string_t targetname) = 0;
	virtual qboolean IsInWorld(void) = 0;
	virtual qboolean IsPlayer(void) = 0;
	virtual qboolean IsNetClient(void) = 0;
	virtual const char *TeamID(void) = 0;
	virtual CBaseEntity *GetNextTarget(void) = 0;
	virtual void Think(void) = 0;
	virtual void Touch(CBaseEntity *pOther) = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) = 0;
	virtual void Blocked(CBaseEntity *pOther) = 0;
	virtual CBaseEntity *Respawn(void) = 0;
	virtual void UpdateOwner(void) = 0;
	virtual qboolean FBecomeProne(void) = 0;
	virtual Vector Center(void) = 0;
	virtual Vector EyePosition(void) = 0;
	virtual Vector EarPosition(void) = 0;
	virtual Vector BodyTarget(const Vector &posSrc) = 0;
	virtual int Illumination(void) = 0;
	virtual qboolean FVisible(CBaseEntity *pEntity) = 0;
	virtual qboolean FVisible(const Vector &vecOrigin) = 0;

public:
	void __declspec(dllexport) SUB_Remove(void) noexcept
	{
		if (pev->health > 0)
		{
			// this situation can screw up monsters who can't tell their entity pointers are invalid.
			pev->health = 0;
			g_engfuncs.pfnAlertMessage(at_aiconsole, "SUB_Remove called on entity with health > 0\n");
		}

		g_engfuncs.pfnRemoveEntity(edict());
	}
	void __declspec(dllexport) SUB_DoNothing(void) noexcept {};
//	void EXPORT SUB_StartFadeOut(void);
//	void EXPORT SUB_FadeOut(void);
//	void EXPORT SUB_CallUseToggle(void) { Use(this, this, USE_TOGGLE, 0); }
//	void SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value);

	// LUNA: Templated type-safe version, from ReGameDLL

	__forceinline void SetThink(std::nullptr_t) noexcept { m_pfnThink = nullptr; }
	template <std::derived_from<CBaseEntity> T>
	__forceinline void SetThink(void (T::* pfn)()) noexcept
	{
		m_pfnThink = static_cast<decltype(m_pfnThink)>(pfn);
	}

	__forceinline void SetTouch(std::nullptr_t) noexcept { m_pfnTouch = nullptr; }
	template <std::derived_from<CBaseEntity> T>
	__forceinline void SetTouch(void (T::* pfn)(CBaseEntity* pOther)) noexcept
	{
		m_pfnTouch = static_cast<decltype(m_pfnTouch)>(pfn);
	}

	__forceinline void SetUse(std::nullptr_t) noexcept { m_pfnTouch = nullptr; }
	template <std::derived_from<CBaseEntity> T>
	__forceinline void SetUse(void (T::* pfn)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)) noexcept
	{
		m_pfnUse = static_cast<decltype(m_pfnUse)>(pfn);
	}

	__forceinline void SetBlocked(std::nullptr_t) noexcept { m_pfnTouch = nullptr; }
	template <std::derived_from<CBaseEntity> T>
	__forceinline void SetBlocked(void (T::* pfn)(CBaseEntity* pOther)) noexcept
	{
		m_pfnBlocked = static_cast<decltype(m_pfnBlocked)>(pfn);
	}

//public:
//	void UpdateOnRemove(void);
//	int ShouldToggle(USE_TYPE useType, qboolean currentState);
//	void FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL);
	Vector FireBullets3(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, qboolean bPistol, int shared_rand = 0);
	bool Intersects(CBaseEntity *pOther) noexcept { return (pOther->pev->absmin.x > pev->absmax.x || pOther->pev->absmin.y > pev->absmax.y || pOther->pev->absmin.z > pev->absmax.z || pOther->pev->absmax.x < pev->absmin.x || pOther->pev->absmax.y < pev->absmin.y || pOther->pev->absmax.z < pev->absmin.z); }
//	void MakeDormant(void);
	inline bool IsDormant() const noexcept { return (pev->flags & FL_DORMANT) == FL_DORMANT; }
//	qboolean IsLockedByMaster(void) { return FALSE; }

//public:
//	static CBaseEntity *Instance(edict_t *pent) { return (CBaseEntity *)GET_PRIVATE(pent ? pent : ENT(0)); }
//	static CBaseEntity *Instance(entvars_t *instpev) { return Instance(ENT(instpev)); }
//	static CBaseEntity *Instance(int inst_eoffset) { return Instance(ENT(inst_eoffset)); }
//
//	CBaseMonster *GetMonsterPointer(entvars_t *pevMonster)
//	{
//		CBaseEntity *pEntity = Instance(pevMonster);
//
//		if (pEntity)
//			return pEntity->MyMonsterPointer();
//
//		return NULL;
//	}
//
//	CBaseMonster *GetMonsterPointer(edict_t *pentMonster)
//	{
//		CBaseEntity *pEntity = Instance(pentMonster);
//
//		if (pEntity)
//			return pEntity->MyMonsterPointer();
//
//		return NULL;
//	}
//


	static CBaseEntity *Create(char const *szName, const Vector &vecOrigin, const Angles &vecAngles, edict_t *pentOwner = nullptr) noexcept;

	__forceinline edict_t *edict(void) const noexcept { return pev->pContainingEntity; }
	__forceinline int eoffset(void) const noexcept { return g_engfuncs.pfnEntOffsetOfPEntity(edict()); }
	__forceinline short entindex(void) const noexcept { return g_engfuncs.pfnIndexOfEdict(edict()); }

	// LUNA: NEVER enable ANY of these override of 'new' 'delete' in modern C++.
	// the original valve devs are quite toxic.
	// 1. The constructor of your CBaseXXX will NEVER be called. They misunderstood what 'placement new' actually means, hence these sick'o operator new overrides coming from.
	// 2. The destructor of your CBaseXXX will NEVER be called. These is due to the nature of C/C++ comp issue.
	// These two a lethal enough for modern C++.
	// Checkout my Prefab.ixx and use the UTIL_CreateNamedPrefab() function provided.
	// Also, DO hook pfnOnFreeEntPrivateData() fn from NEW_FUNCTION_TABLE

	//void *operator new(size_t iBlockSize, entvars_t *pevnew) noexcept { return g_engfuncs.pfnPvAllocEntPrivateData(ent_cast<edict_t *>(pevnew), iBlockSize); static_assert(false, "Read the comment above."); }
	//void *operator new(size_t iBlockSize, edict_t *pent) noexcept { return g_engfuncs.pfnPvAllocEntPrivateData(pent, iBlockSize); static_assert(false, "Read the comment above."); }

	//void operator delete(void *pMem, entvars_t *pevnew) noexcept { pevnew->flags |= FL_KILLME; static_assert(false, "Read the comment above."); }

//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	entvars_t *pev{};
	CBaseEntity *m_pGoalEnt{};
	CBaseEntity *m_pLink{};
	void (CBaseEntity:: *m_pfnThink)(void) {};
	void (CBaseEntity:: *m_pfnTouch)(CBaseEntity *pOther) {};
	void (CBaseEntity:: *m_pfnUse)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) {};
	void (CBaseEntity:: *m_pfnBlocked)(CBaseEntity *pOther) {};
	int current_ammo{};
	int currentammo{};
	int maxammo_buckshot{};
	int ammo_buckshot{};
	int maxammo_9mm{};
	int ammo_9mm{};
	int maxammo_556nato{};
	int ammo_556nato{};
	int maxammo_556natobox{};
	int ammo_556natobox{};
	int maxammo_762nato{};
	int ammo_762nato{};
	int maxammo_45acp{};
	int ammo_45acp{};
	int maxammo_50ae{};
	int ammo_50ae{};
	int maxammo_338mag{};
	int ammo_338mag{};
	int maxammo_57mm{};
	int ammo_57mm{};
	int maxammo_357sig{};
	int ammo_357sig{};
	float m_flStartThrow{};
	float m_flReleaseThrow{};
	int m_iSwing{};
	qboolean has_disconnected{};
};

export template <typename T>
struct EHANDLE final
{
	EHANDLE(void) noexcept {}
	EHANDLE(edict_t *pEdict) noexcept { Set(pEdict); }
	EHANDLE(entvars_t *pev) noexcept { if (pev) Set(pev->pContainingEntity); }
	EHANDLE(T* pEntity) noexcept { if (pEntity) Set(pEntity->pev->pContainingEntity); }
	EHANDLE(std::nullptr_t) noexcept : EHANDLE() {}
	explicit EHANDLE(short iEntIndex) noexcept { if (iEntIndex > 0) Set(ent_cast<edict_t *>(iEntIndex)); }

	EHANDLE(EHANDLE<T> const &rhs) noexcept : m_pent(rhs.m_pent), m_serialnumber(rhs.m_serialnumber) {}
	EHANDLE<T> &operator=(EHANDLE<T> const &rhs) noexcept { m_pent = rhs.m_pent; m_serialnumber = rhs.m_serialnumber; return *this; }

	inline edict_t *Get(void) const noexcept
	{
		if (!m_pent || !m_pent->pvPrivateData || ent_cast<short>(m_pent) < 0)	// index 0, CWorld, is a valid entity.
			return nullptr;

		if (m_pent->serialnumber != m_serialnumber)
			return nullptr;

		return m_pent;
	}
	inline edict_t *Set(edict_t *pent) noexcept
	{
		m_pent = pent;

		if (m_pent && m_pent->pvPrivateData && ent_cast<short>(m_pent) > 0)
			m_serialnumber = m_pent->serialnumber;

		return pent;
	}
	inline void Reset(void) noexcept
	{
		m_pent = nullptr;
		m_serialnumber = 0;
	}

	template <typename U>
	inline bool Is(void) const noexcept
	{
		if constexpr (std::is_base_of_v<U, T> || std::is_same_v<U, T>)
		{
			return Get() != nullptr;
		}
		else
		{
			if constexpr (requires { { m_pent->v.classname == MAKE_STRING(U::CLASSNAME) } -> std::same_as<bool>; })
			{
				// Even the classname won't match, one can still not rule out the case of base class.
				if (auto const ent = Get(); ent && ent->v.classname == MAKE_STRING(U::CLASSNAME))
					return true;
			}

			if (Get())
			{
				auto const p = static_cast<T *>(m_pent->pvPrivateData);
				auto const p2 = dynamic_cast<U *>(p);

				return p2 != nullptr;
			}
			else
			{
				return false;
			}
		}
	}
	template <typename U>
	__forceinline bool IsNot(void) noexcept { return !Is<U>(); }

	template <typename U>
	inline U *As(void) noexcept
	{
		if constexpr (std::is_same_v<U, T>)
		{
			if (Get())
				return static_cast<T *>(m_pent->pvPrivateData);

			return nullptr;
		}
		if constexpr (std::is_base_of_v<U, T>)
		{
			// case from derived class to base class.

			if (Get())
			{
				auto const p = static_cast<T *>(m_pent->pvPrivateData);
				return static_cast<U *>(p);
			}

			return nullptr;
		}
		else
		{
			if (Get())
			{
				auto const p = static_cast<T *>(m_pent->pvPrivateData);
				return dynamic_cast<U *>(p);
			}

			return nullptr;
		}
	}

	inline operator bool() const noexcept { return Get() != nullptr; }
	inline operator T *() const noexcept { auto const pent = Get(); return pent ? (T *)pent->pvPrivateData : nullptr; }

	inline T *operator= (T *pEntity) noexcept
	{
		if (pEntity && ent_cast<short>(pEntity->pev) > 0 && ent_cast<T*>(pEntity->pev) == pEntity)
		{
			m_pent = ent_cast<edict_t *>(pEntity->pev);
			m_serialnumber = m_pent->serialnumber;
		}
		else
		{
			m_pent = nullptr;
			m_serialnumber = 0;
		}

		return pEntity;
	}

	// Using a faster version under release mode.
#ifdef _DEBUG
	inline T* operator-> () const noexcept { auto const pent = Get(); assert(pent != nullptr); return pent ? (T*)pent->pvPrivateData : nullptr; }
	inline T& operator* () const noexcept { auto const pent = Get(); assert(pent != nullptr); return *(T*)pent->pvPrivateData; }
#else
	inline T* operator-> () const noexcept { return m_pent ? (T*)m_pent->pvPrivateData : nullptr; }
	inline T& operator* () const noexcept { return *(T*)m_pent->pvPrivateData; }
#endif

	// ent comperasion
	//inline bool operator== (short iIndex) noexcept { auto const pent = Get(); return pent && ent_cast<short>(pent) == iIndex; }
	//inline bool operator== (edict_t *pEdict) noexcept { auto const pent = Get(); return pent && pent == pEdict; }
	//inline bool operator== (entvars_t *pev) noexcept { auto const pent = Get(); return pent && pent == pev->pContainingEntity; }
	//inline bool operator== (T *pEntity) noexcept { auto const pent = Get(); return pent && pEntity == pent->pvPrivateData; }
	//inline bool operator== (EHANDLE<T> const &rhs) const noexcept { return m_pent == rhs.m_pent && m_serialnumber == rhs.m_serialnumber; }

	edict_t *m_pent = nullptr;
	int m_serialnumber = 0;
};

// ent comperasion
export template <typename T, typename U> inline
bool operator== (EHANDLE<T> const& lhs, EHANDLE<U> const& rhs) noexcept
{
	return lhs.m_pent == rhs.m_pent && lhs.m_serialnumber == rhs.m_serialnumber;
}

export class CBaseDelay : public CBaseEntity
{
public:
	void KeyValue(KeyValueData *pkvd) = 0;
	int Save(void *save) = 0;	// CSave &
	int Restore(void *restore) = 0;	// CRestore &

public:
	void SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value) noexcept;

//public:
//	void EXPORT DelayThink(void);

//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	float m_flDelay{};
	int m_iszKillTarget{};
};

export class CBaseAnimating : public CBaseDelay
{
public:
	virtual int Save(void *save) = 0;	// CSave &
	virtual int Restore(void *restore) = 0;	// CRestore &
	virtual void HandleAnimEvent(MonsterEvent_t* pEvent) = 0;

//public:
//	float StudioFrameAdvance(float flInterval = 0);
//	int GetSequenceFlags(void);
//	int LookupActivity(int activity);
//	int LookupActivityHeaviest(int activity);
//	int LookupSequence(const char *label);
//	void ResetSequenceInfo(void);
//	void DispatchAnimEvents(float flFutureInterval = 0.1);
//	float SetBoneController(int iController, float flValue);
//	void InitBoneControllers(void);
//	float SetBlending(int iBlender, float flValue);
//	void GetBonePosition(int iBone, Vector &origin, Vector &angles);
//	void GetAutomovement(Vector &origin, Vector &angles, float flInterval = 0.1);
//	int FindTransition(int iEndingSequence, int iGoalSequence, int *piDir);
//	void GetAttachment(int iAttachment, Vector &origin, Vector &angles);
//	void SetBodygroup(int iGroup, int iValue);
//	int GetBodygroup(int iGroup);
//	int ExtractBbox(int sequence, float *mins, float *maxs);
//	void SetSequenceBox(void);

//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	float m_flFrameRate{};
	float m_flGroundSpeed{};
	float m_flLastEventCheck{};
	qboolean m_fSequenceFinished{};
	qboolean m_fSequenceLoops{};
};

export enum WeaponIdType
{
	WEAPON_NONE,
	WEAPON_P228,
	WEAPON_NIL,
	WEAPON_SCOUT,
	WEAPON_HEGRENADE,
	WEAPON_XM1014,
	WEAPON_C4,
	WEAPON_MAC10,
	WEAPON_AUG,
	WEAPON_SMOKEGRENADE,
	WEAPON_ELITE,
	WEAPON_FIVESEVEN,
	WEAPON_UMP45,
	WEAPON_SG550,
	WEAPON_GALIL,
	WEAPON_FAMAS,
	WEAPON_USP,
	WEAPON_GLOCK18,
	WEAPON_AWP,
	WEAPON_MP5N,
	WEAPON_M249,
	WEAPON_M3,
	WEAPON_M4A1,
	WEAPON_TMP,
	WEAPON_G3SG1,
	WEAPON_FLASHBANG,
	WEAPON_DEAGLE,
	WEAPON_SG552,
	WEAPON_AK47,
	WEAPON_KNIFE,
	WEAPON_P90,
	WEAPON_SHIELDGUN = 99
};

export struct ItemInfo
{
	int iSlot{};

	// No reference outside SignOn Message.
	int iPosition{};

	/*
	* Internal usage:
	ItemPostFrame - just to set m_fFireOnEmpty, useless.
	AddToPlayer - setup m_iPrimaryAmmoType
	ExtractAmmo - giving default ammo?
	ExtractClipAmmo - giving clip ammo; prob used in HL1

	* External usage:
	CGameRules::CanHavePlayerItem - can have ammo used by this gun
	CBasePlayer::DropPlayerItem, ::packPlayerItem - packing ammo when dropping
	::UTIL_PrecacheOtherWeapon - register ammo
	::MaxAmmoCarry
	::WriteSigonMessages
	*/
	const char* pszAmmo1{};

	/*
	* Internal usage:
	IsUseable - called in ItemPostFrame to prevent auto-reload, when weapon is not using ammo.
	ExtractAmmo, ExtractClipAmmo - see pszAmmo1, same usage.

	* External usage:
	CGameRules::CanHavePlayerItem - see pszAmmo1, same usage.
	::MaxAmmoCarry
	::BuyGunAmmo
	::WriteSigonMessages
	*/
	int iMaxAmmo1{};

	const char* pszAmmo2{};
	int iMaxAmmo2{};

	/*
	* Internal usage:
	(NONE)

	* External usage:
	CHalfLifeMultiplay::DeathNotice
	::GetWeaponName
	::WriteSigonMessages
	*/
	const char* pszName{};

	/*
	* Internal usage:
	ItemPostFrame - for reloading a gun.
	ExtractAmmo - for calling AddPrimaryAmmo

	* External usage:
	(NONE)
	*/
	int iMaxClip{};

	/*
	* Internal usage:
	ItemPostFrame - for reloading a gun.
	ExtractAmmo - for calling AddPrimaryAmmo

	* External usage:
	dllexport ::GetWeaponData
	dllexport ::UpdateClientData
	*/
	int iId{};

	/*
	* Interal usage:
	ItemPostFrame - ITEM_FLAG_NOAUTORELOAD

	* External usage:
	CBasePlayer::UpdateClientData - ITEM_FLAG_EXHAUSTIBLE
		just to print "Hint_out_of_ammo"
	CBasePlayer::DropPlayerItem - ITEM_FLAG_EXHAUSTIBLE
	CHalfLifeMultiplay::FlWeaponTryRespawn - ITEM_FLAG_LIMITINWORLD
	::WriteSigonMessages
	*/
	int iFlags{};

	/*
	* Interal usage:
	(NONE)

	* External usage:
	CHalfLifeMultiplay::FShouldSwitchWeapon - comparing weight between cur and other.
	CHalfLifeMultiplay::GetNextBestWeapon
	CBasePlayer::PackDeadPlayerItems - finding the most important weapon to drop.
	*/
	int iWeight{};
};

export struct AmmoInfo
{
	const char* pszName{};
	int iId{};
};

export inline constexpr auto WEAPON_NOCLIP = -1;

export inline constexpr auto ITEM_FLAG_SELECTONEMPTY = (1 << 0);
export inline constexpr auto ITEM_FLAG_NOAUTORELOAD = (1 << 1);
export inline constexpr auto ITEM_FLAG_NOAUTOSWITCHEMPTY = (1 << 2);
export inline constexpr auto ITEM_FLAG_LIMITINWORLD = (1 << 3);
export inline constexpr auto ITEM_FLAG_EXHAUSTIBLE = (1 << 4); // A player can totally exhaust their ammo supply and lose this weapon
export inline constexpr auto ITEM_FLAG_NOFIREUNDERWATER = (1 << 5);	// ReGameDLL ext

export class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual int Save(void *save) = 0;	// CSave &
	virtual int Restore(void *restore) = 0;	// CRestore &
	virtual void SetObjectCollisionBox(void) = 0;
	virtual qboolean AddToPlayer(CBasePlayer *pPlayer) = 0;
	virtual qboolean AddDuplicate(CBasePlayerItem *pItem) = 0;
	virtual qboolean GetItemInfo(ItemInfo* p) = 0;
	virtual qboolean CanDeploy(void) = 0;
	virtual qboolean CanDrop(void) = 0;
	virtual qboolean Deploy(void) = 0;
	virtual qboolean IsWeapon(void) = 0;
	virtual qboolean CanHolster(void) = 0;
	virtual void Holster(int skiplocal = 0) = 0;
	virtual void UpdateItemInfo(void) = 0;
	virtual void ItemPreFrame(void) = 0;
	virtual void ItemPostFrame(void) = 0;
	virtual void Drop(void) = 0;
	virtual void Kill(void) = 0;
	virtual void AttachToPlayer(CBasePlayer *pPlayer) = 0;
	virtual int PrimaryAmmoIndex(void) = 0;
	virtual int SecondaryAmmoIndex(void) = 0;
	virtual int UpdateClientData(CBasePlayer *pPlayer) = 0;
	virtual CBasePlayerItem *GetWeaponPtr(void) = 0;
	virtual float GetMaxSpeed(void) = 0;
	virtual int iItemSlot(void) = 0;

public:
	void __declspec(dllexport) DestroyItem(void) noexcept;
	void __declspec(dllexport) DefaultTouch(CBaseEntity *pOther) noexcept;
	void __declspec(dllexport) FallThink(void) noexcept;
	void __declspec(dllexport) Materialize(void) noexcept;
	void __declspec(dllexport) AttemptToMaterialize(void) noexcept;
//	CBaseEntity *Respawn(void);
	void FallInit(void) noexcept;
//	void CheckRespawn(void);

public:
//	static TYPEDESCRIPTION m_SaveData[];
	static inline std::span<ItemInfo/*, MAX_WEAPONS*/> ItemInfoArray;	// LUNA: default constructor disallowed in fixed extent
	static inline std::span<AmmoInfo/*, MAX_AMMO_SLOTS*/> AmmoInfoArray;

public:
	CBasePlayer* m_pPlayer{};
	CBasePlayerItem* m_pNext{};
	int m_iId{};

public:
	int			iItemPosition(void)	const noexcept { return ItemInfoArray[m_iId].iPosition; }
	const char*	pszAmmo1(void)		const noexcept { return ItemInfoArray[m_iId].pszAmmo1; }
	int			iMaxAmmo1(void)		const noexcept { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char*	pszAmmo2(void)		const noexcept { return ItemInfoArray[m_iId].pszAmmo2; }
	int			iMaxAmmo2(void)		const noexcept { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char*	pszName(void)		const noexcept { return ItemInfoArray[m_iId].pszName; }
	int			iMaxClip(void)		const noexcept { return ItemInfoArray[m_iId].iMaxClip; }
	int			iWeight(void)		const noexcept { return ItemInfoArray[m_iId].iWeight; }
	int			iFlags(void)		const noexcept { return ItemInfoArray[m_iId].iFlags; }
};

export enum EWeaponState
{
	WPNSTATE_USP_SILENCED = (1 << 0),
	WPNSTATE_GLOCK18_BURST_MODE = (1 << 1),
	WPNSTATE_M4A1_SILENCED = (1 << 2),
	WPNSTATE_ELITE_LEFT = (1 << 3),
	WPNSTATE_FAMAS_BURST_MODE = (1 << 4),
	WPNSTATE_SHIELD_DRAWN = (1 << 5),
};

export enum EBulletTypes
{
	BULLET_NONE,
	BULLET_PLAYER_9MM,
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_357,
	BULLET_PLAYER_BUCKSHOT,
	BULLET_PLAYER_CROWBAR,
	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
	BULLET_PLAYER_45ACP,
	BULLET_PLAYER_338MAG,
	BULLET_PLAYER_762MM,
	BULLET_PLAYER_556MM,
	BULLET_PLAYER_50AE,
	BULLET_PLAYER_57MM,
	BULLET_PLAYER_357SIG,
};

export inline constexpr auto LOUD_GUN_VOLUME = 1000;
export inline constexpr auto NORMAL_GUN_VOLUME = 600;
export inline constexpr auto QUIET_GUN_VOLUME = 200;

export inline constexpr auto BIG_EXPLOSION_VOLUME = 2048;
export inline constexpr auto NORMAL_EXPLOSION_VOLUME = 1024;
export inline constexpr auto SMALL_EXPLOSION_VOLUME = 512;

export inline constexpr auto BRIGHT_GUN_FLASH = 512;
export inline constexpr auto NORMAL_GUN_FLASH = 256;
export inline constexpr auto DIM_GUN_FLASH = 128;

export class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int Save(void *save) = 0;	// CSave &
	virtual int Restore(void *restore) = 0;	// CRestore &
	virtual qboolean AddToPlayer(CBasePlayer *pPlayer) = 0;
	virtual qboolean AddDuplicate(CBasePlayerItem *pItem) = 0;
	virtual qboolean ExtractAmmo(CBasePlayerWeapon *pWeapon) = 0;
	virtual qboolean ExtractClipAmmo(CBasePlayerWeapon *pWeapon) = 0;
	virtual qboolean AddWeapon(void) = 0;
	virtual void UpdateItemInfo(void) = 0;
	virtual qboolean PlayEmptySound(void) = 0;
	virtual void ResetEmptySound(void) = 0;
	virtual void SendWeaponAnim(int iAnim, int skiplocal = false) = 0;
	virtual qboolean CanDeploy(void) = 0;
	virtual qboolean IsWeapon(void) = 0;
	virtual qboolean IsUseable(void) = 0;
	virtual void ItemPostFrame(void) = 0;
	virtual void PrimaryAttack(void) = 0;
	virtual void SecondaryAttack(void) = 0;
	virtual void Reload(void) = 0;
	virtual void WeaponIdle(void) = 0;
	virtual int UpdateClientData(CBasePlayer *pPlayer) = 0;
	virtual void RetireWeapon(void) = 0;
	virtual qboolean ShouldWeaponIdle(void) = 0;
	virtual void Holster(int skiplocal = 0) = 0;
	virtual qboolean UseDecrement(void) = 0;
	virtual CBasePlayerItem *GetWeaponPtr(void) = 0;

//public:
	bool DefaultDeploy(char const *szViewModel, char const *szWeaponModel, int iAnim, char const *szAnimExt, int skiplocal = 0) noexcept;
	qboolean DefaultReload(int iClipSize, int iAnim, float fDelay, int body = 0) noexcept;
	void ReloadSound(void) noexcept;
	qboolean AddPrimaryAmmo(int iCount, const char* szName, int iMaxClip, int iMaxCarry) noexcept;
	qboolean AddSecondaryAmmo(int iCount, const char* szName, int iMaxCarry) noexcept;
//	int PrimaryAmmoIndex(void);
//	int SecondaryAmmoIndex(void);
	void EjectBrassLate(void) const noexcept;
//	void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change);
	void FireRemaining(int &shotsFired, float &shootTime, bool isGlock18) noexcept;
//	void SetPlayerShieldAnim(void);
//	void ResetPlayerShieldAnim(void);
//	bool ShieldSecondaryFire(int up_anim, int down_anim);
	bool HasSecondaryAttack(void) const noexcept;
	float GetNextAttackDelay(float delay) noexcept;	// ReGameDLL, build >= 6153

//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	qboolean m_iPlayEmptySound{};	// #PLANNED_PIW_cbase_rewrite only for M3 and M1014, they won't constantly play empty fire SFX when holding LMB.
	qboolean m_fFireOnEmpty{};	// #PLANNED_PIW_cbase_useless should be abolish. Always true when needed, always false when don't needed.
	float m_flNextPrimaryAttack{};
	float m_flNextSecondaryAttack{};
	float m_flTimeWeaponIdle{};
	int m_iPrimaryAmmoType{};
	int m_iSecondaryAmmoType{};	// #PLANNED_PIW_cbase_rewrite probably just drop this one in CS.
	int m_iClip{};
	int m_iClientClip{};
	int m_iClientWeaponState{};
	qboolean m_fInReload{};
	qboolean m_fInSpecialReload{};
	int m_iDefaultAmmo{};	// #PLANNED_PIW_cbase_rewrite should be a data sheet.
	int m_iShellId{};	// #PLANNED_PIW_cbase_rewrite used by M3 and AWP, should be templated.
	float m_fMaxSpeed{};	// #PLANNED_PIW_cbase_useless what the hell is the point? Just return the value!
	bool m_bDelayFire{};
	qboolean m_iDirection{};
	bool m_bSecondarySilencerOn{};	// #PLANNED_PIW_cbase_useless unreferenced.
	float m_flAccuracy{};
	float m_flLastFire{};
	int m_iShotsFired{};
	Vector m_vVecAiming{};	// #PLANNED_PIW_cbase_useless used without assigning value. ref by M1014.
	string_t model_name{};	// #PLANNED_PIW_cbase_useless assigned in DefaultDeploy() without any usage.
	float m_flGlock18Shoot{};
	int m_iGlock18ShotsFired{};
	float m_flFamasShoot{};
	int m_iFamasShotsFired{};
	float m_fBurstSpread{};	// #PLANNED_PIW_cbase_rewrite FAMAS only.
	int m_iWeaponState{};	// #PLANNED_PIW_cbase_useless Used in M3 and M1014, value assigned, sent to client, but never used in SV.
	float m_flNextReload{};
	float m_flDecreaseShotsFired{};
	unsigned short m_usFireGlock18{};
	unsigned short m_usFireFamas{};

	// LUNA: ReGameDLL added? Or build >= 6153??
	// hle time creep vars
	float m_flPrevPrimaryAttack{}; // #PLANNED_PIW_cbase_useless dead code in 6153
	float m_flLastFireTime{}; // #PLANNED_PIW_cbase_useless dead code in 6153
};

export class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn(void) = 0;
	virtual qboolean AddAmmo(CBaseEntity *pOther) = 0;

public:
	//void Materialize(void) = 0;
	//void DefaultTouch(CBaseEntity *pOther) = 0;
	CBaseEntity *Respawn(void) = 0;
};

export class CItem : public CBaseEntity
{
public:
	virtual void Spawn(void) = 0;
	virtual CBaseEntity *Respawn(void) = 0;
	virtual qboolean MyTouch(CBasePlayer *pPlayer) = 0;

//public:
//	void EXPORT ItemTouch(CBaseEntity *pOther) = 0;
//	void EXPORT Materialize(void) = 0;
};

export class CBaseToggle : public CBaseAnimating
{
public:
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual int Save(void *save) = 0;	// CSave &
	virtual int Restore(void *restore) = 0;	// CRestore &
	virtual int GetToggleState(void) = 0;
	virtual float GetDelay(void) = 0;

//public:
//	void LinearMove(Vector vecDest, float flSpeed);
//	void EXPORT LinearMoveDone(void);
//	void AngularMove(Vector vecDestAngle, float flSpeed);
//	void EXPORT AngularMoveDone(void);
//	qboolean IsLockedByMaster(void);

//public:
//	static float AxisValue(int flags, const Vector &angles);
//	static void AxisDir(entvars_t *pev);
//	static float AxisDelta(int flags, const Vector &angle1, const Vector &angle2);

//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	TOGGLE_STATE m_toggle_state;
	float m_flActivateFinished;
	float m_flMoveDistance;
	float m_flWait;
	float m_flLip;
	float m_flTWidth;
	float m_flTLength;
	Vector m_vecPosition1;
	Vector m_vecPosition2;
	Vector m_vecAngle1;
	Vector m_vecAngle2;
	int m_cTriggersLeft;
	float m_flHeight;
	EHANDLE<CBaseEntity> m_hActivator;
	void (CBaseToggle:: *m_pfnCallWhenMoveDone)(void);
	Vector m_vecFinalDest;
	Vector m_vecFinalAngle;
	int m_bitsDamageInflict;
	string_t m_sMaster;
};

export inline constexpr auto ROUTE_SIZE = 8;
export inline constexpr auto MAX_OLD_ENEMIES = 4;

export inline constexpr auto bits_CAP_DUCK = (1 << 0);
export inline constexpr auto bits_CAP_JUMP = (1 << 1);
export inline constexpr auto bits_CAP_STRAFE = (1 << 2);
export inline constexpr auto bits_CAP_SQUAD = (1 << 3);
export inline constexpr auto bits_CAP_SWIM = (1 << 4);
export inline constexpr auto bits_CAP_CLIMB = (1 << 5);
export inline constexpr auto bits_CAP_USE = (1 << 6);
export inline constexpr auto bits_CAP_HEAR = (1 << 7);
export inline constexpr auto bits_CAP_AUTO_DOORS = (1 << 8);
export inline constexpr auto bits_CAP_OPEN_DOORS = (1 << 9);
export inline constexpr auto bits_CAP_TURN_HEAD = (1 << 10);
export inline constexpr auto bits_CAP_RANGE_ATTACK1 = (1 << 11);
export inline constexpr auto bits_CAP_RANGE_ATTACK2 = (1 << 12);
export inline constexpr auto bits_CAP_MELEE_ATTACK1 = (1 << 13);
export inline constexpr auto bits_CAP_MELEE_ATTACK2 = (1 << 14);
export inline constexpr auto bits_CAP_FLY = (1 << 15);
export inline constexpr auto bits_CAP_DOORS_GROUP = (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS);

export inline constexpr auto DMG_GENERIC = 0;		// generic damage was done
export inline constexpr auto DMG_CRUSH = (1 << 0);	// crushed by falling or moving object
export inline constexpr auto DMG_BULLET = (1 << 1);	// shot
export inline constexpr auto DMG_SLASH = (1 << 2);	// cut, clawed, stabbed
export inline constexpr auto DMG_BURN = (1 << 3);	// heat burned
export inline constexpr auto DMG_FREEZE = (1 << 4);	// frozen
export inline constexpr auto DMG_FALL = (1 << 5);	// fell too far
export inline constexpr auto DMG_BLAST = (1 << 6);	// explosive blast damage
export inline constexpr auto DMG_CLUB = (1 << 7);	// crowbar, punch, headbutt
export inline constexpr auto DMG_SHOCK = (1 << 8);	// electric shock
export inline constexpr auto DMG_SONIC = (1 << 9);	// sound pulse shockwave
export inline constexpr auto DMG_ENERGYBEAM = (1 << 10);	// laser or other high energy beam
export inline constexpr auto DMG_NEVERGIB = (1 << 12);		// with this bit OR'd in, no damage type will be able to gib victims upon death
export inline constexpr auto DMG_ALWAYSGIB = (1 << 13);		// with this bit OR'd in, any damage type can be made to gib victims upon death
export inline constexpr auto DMG_DROWN = (1 << 14);			// Drowning

// time-based damage
export inline constexpr auto DMG_TIMEBASED = (~(0x3FFF));	// mask for time-based damage

export inline constexpr auto DMG_PARALYZE = (1 << 15);		// slows affected creature down
export inline constexpr auto DMG_NERVEGAS = (1 << 16);		// nerve toxins, very bad
export inline constexpr auto DMG_POISON = (1 << 17);		// blood poisioning
export inline constexpr auto DMG_RADIATION = (1 << 18);		// radiation exposure
export inline constexpr auto DMG_DROWNRECOVER = (1 << 19);	// drowning recovery
export inline constexpr auto DMG_ACID = (1 << 20);			// toxic chemicals or acid burns
export inline constexpr auto DMG_SLOWBURN = (1 << 21);		// in an oven
export inline constexpr auto DMG_SLOWFREEZE = (1 << 22);	// in a subzero freezer
export inline constexpr auto DMG_MORTAR = (1 << 23);		// Hit by air raid (done to distinguish grenade from mortar)
export inline constexpr auto DMG_EXPLOSION = (1 << 24);

// These are the damage types that are allowed to gib corpses
export inline constexpr auto DMG_GIB_CORPSE = (DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB);

// These are the damage types that have client hud art
export inline constexpr auto DMG_SHOWNHUD = (DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK);

export inline constexpr auto PARALYZE_DURATION = 2;
export inline constexpr auto PARALYZE_DAMAGE = 1.0;

export inline constexpr auto NERVEGAS_DURATION = 2;
export inline constexpr auto NERVEGAS_DAMAGE = 5.0;

export inline constexpr auto POISON_DURATION = 5;
export inline constexpr auto POISON_DAMAGE = 2.0;

export inline constexpr auto RADIATION_DURATION = 2;
export inline constexpr auto RADIATION_DAMAGE = 1.0;

export inline constexpr auto ACID_DURATION = 2;
export inline constexpr auto ACID_DAMAGE = 5.0;

export inline constexpr auto SLOWBURN_DURATION = 2;
export inline constexpr auto SLOWBURN_DAMAGE = 1.0;

export inline constexpr auto SLOWFREEZE_DURATION = 2;
export inline constexpr auto SLOWFREEZE_DAMAGE = 1.0;

export inline constexpr auto itbd_Paralyze = 0;
export inline constexpr auto itbd_NerveGas = 1;
export inline constexpr auto itbd_Poison = 2;
export inline constexpr auto itbd_Radiation = 3;
export inline constexpr auto itbd_DrownRecover = 4;
export inline constexpr auto itbd_Acid = 5;
export inline constexpr auto itbd_SlowBurn = 6;
export inline constexpr auto itbd_SlowFreeze = 7;
export inline constexpr auto CDMG_TIMEBASED = 8;

// When calling KILLED(), a value that governs gib behavior is expected to be
// one of these three values
export inline constexpr auto GIB_NORMAL = 0; // Gib if entity was overkilled
export inline constexpr auto GIB_NEVER = 1; // Never gib, no matter how much death damage is done ( freezing, etc )
export inline constexpr auto GIB_ALWAYS = 2; // Always gib ( Houndeye Shock, Barnacle Bite )
export inline constexpr auto GIB_HEALTH_VALUE = -30;

export enum EHitBoxGroup : std::uint32_t
{
	HITGROUP_GENERIC,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFTARM,
	HITGROUP_RIGHTARM,
	HITGROUP_LEFTLEG,
	HITGROUP_RIGHTLEG,
	HITGROUP_SHIELD,

	NUM_HITGROUPS,
};

export class CBaseMonster : public CBaseToggle
{
public:
	virtual void KeyValue(KeyValueData *pkvd) = 0;
	virtual float ChangeYaw(int speed) = 0;
	virtual qboolean HasHumanGibs(void) = 0;
	virtual qboolean HasAlienGibs(void) = 0;
	virtual void FadeMonster(void) = 0;
	virtual void GibMonster(void) = 0;
	virtual Activity GetDeathActivity(void) = 0;
	virtual void BecomeDead(void) = 0;
	virtual qboolean ShouldFadeOnDeath(void) = 0;
	virtual int IRelationship(CBaseEntity *pTarget) = 0;
	virtual int TakeHealth(float flHealth, int bitsDamageType) = 0;
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual void Killed(entvars_t *pevAttacker, int iGib) = 0;
	virtual void PainSound(void) = 0;
	virtual void ResetMaxSpeed(void) = 0;
	virtual void ReportAIState(void) = 0;
	virtual void MonsterInitDead(void) = 0;
	virtual void Look(int iDistance) = 0;
	virtual CBaseEntity *BestVisibleEnemy(void) = 0;
	virtual qboolean FInViewCone(CBaseEntity *pEntity) = 0;
	virtual qboolean FInViewCone(Vector *pOrigin) = 0;
	virtual int BloodColor(void) = 0;
	virtual qboolean IsAlive(void) = 0;

//public:
//	void MakeIdealYaw(Vector vecTarget)override;
//	Activity GetSmallFlinchActivity(void)override;
//	qboolean ShouldGibMonster(int iGib)override;
//	void CallGibMonster(void)override;
//	qboolean FCheckAITrigger(void)override;
//	int DeadTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)override;
//	float DamageForce(float damage)override;
//	void RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)override;
//	void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)override;
//	void EXPORT CorpseFallThink(void)override;
//	CBaseEntity *CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)override;
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) = 0;
//	void MakeDamageBloodDecal(int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir)override;
//	void BloodSplat(const Vector &vecPos, const Vector &vecDir, int hitgroup, int iDamage)override;

//public:
//	inline void SetConditions(int iConditions) { m_afConditions |= iConditions; }
//	inline void ClearConditions(int iConditions) { m_afConditions &= ~iConditions; }
//	inline BOOL HasConditions(int iConditions) { if (m_afConditions & iConditions) return TRUE; return FALSE; }
//	inline BOOL HasAllConditions(int iConditions) { if ((m_afConditions & iConditions) == iConditions) return TRUE; return FALSE; }
//	inline void Remember(int iMemory) { m_afMemory |= iMemory; }
//	inline void Forget(int iMemory) { m_afMemory &= ~iMemory; }
//	inline BOOL HasMemory(int iMemory) { if (m_afMemory & iMemory) return TRUE; return FALSE; }
//	inline BOOL HasAllMemories(int iMemory) { if ((m_afMemory & iMemory) == iMemory) return TRUE; return FALSE; }
//	inline void StopAnimation(void) { pev->framerate = 0; }

public:
	Activity m_Activity;
	Activity m_IdealActivity;
	EHitBoxGroup m_LastHitGroup;
	int m_bitsDamageType;
	uint8_t m_rgbTimeBasedDamage[CDMG_TIMEBASED];
	MONSTERSTATE m_MonsterState;
	MONSTERSTATE m_IdealMonsterState;
	int m_afConditions;
	int m_afMemory;
	float m_flNextAttack;
	EHANDLE<CBaseEntity> m_hEnemy;
	EHANDLE<CBaseEntity> m_hTargetEnt;
	float m_flFieldOfView;
	int m_bloodColor;
	Vector m_HackedGunPos;
	Vector m_vecEnemyLKP;
};

export enum struct JoinState
{
	JOINED,
	SHOWLTEXT,
	READINGLTEXT,
	SHOWTEAMSELECT,
	PICKINGTEAM,
	GETINTOGAME
};

export inline constexpr auto MAX_PLAYER_NAME_LENGTH = 32;
export inline constexpr auto MAX_AUTOBUY_LENGTH = 256;
export inline constexpr auto MAX_REBUY_LENGTH = 256;
export inline constexpr auto MAX_LACTION_LENGTH = 32;

export inline constexpr auto PLAYER_FATAL_FALL_SPEED = (float)1100;
export inline constexpr auto PLAYER_MAX_SAFE_FALL_SPEED = (float)500;
export inline constexpr auto DAMAGE_FOR_FALL_SPEED = (float)100.0 / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED);
export inline constexpr auto PLAYER_MIN_BOUNCE_SPEED = (float)350;
export inline constexpr auto PLAYER_FALL_PUNCH_THRESHHOLD = (float)250.0;

export inline constexpr auto PFLAG_ONLADDER = (1 << 0);
export inline constexpr auto PFLAG_ONSWING = (1 << 0);
export inline constexpr auto PFLAG_ONTRAIN = (1 << 1);
export inline constexpr auto PFLAG_ONBARNACLE = (1 << 2);
export inline constexpr auto PFLAG_DUCKING = (1 << 3);
export inline constexpr auto PFLAG_USING = (1 << 4);
export inline constexpr auto PFLAG_OBSERVER = (1 << 5);

export inline constexpr auto TRAIN_ACTIVE = 0x80;
export inline constexpr auto TRAIN_NEW = 0xc0;
export inline constexpr auto TRAIN_OFF = 0x00;
export inline constexpr auto TRAIN_NEUTRAL = 0x01;
export inline constexpr auto TRAIN_SLOW = 0x02;
export inline constexpr auto TRAIN_MEDIUM = 0x03;
export inline constexpr auto TRAIN_FAST = 0x04;
export inline constexpr auto TRAIN_BACK = 0x05;

export inline constexpr auto DHF_ROUND_STARTED = (1 << 1);
export inline constexpr auto DHF_HOSTAGE_SEEN_FAR = (1 << 2);
export inline constexpr auto DHF_HOSTAGE_SEEN_NEAR = (1 << 3);
export inline constexpr auto DHF_HOSTAGE_USED = (1 << 4);
export inline constexpr auto DHF_HOSTAGE_INJURED = (1 << 5);
export inline constexpr auto DHF_HOSTAGE_KILLED = (1 << 6);
export inline constexpr auto DHF_FRIEND_SEEN = (1 << 7);
export inline constexpr auto DHF_ENEMY_SEEN = (1 << 8);
export inline constexpr auto DHF_FRIEND_INJURED = (1 << 9);
export inline constexpr auto DHF_FRIEND_KILLED = (1 << 10);
export inline constexpr auto DHF_ENEMY_KILLED = (1 << 11);
export inline constexpr auto DHF_BOMB_RETRIEVED = (1 << 12);
export inline constexpr auto DHF_AMMO_EXHAUSTED = (1 << 15);
export inline constexpr auto DHF_IN_TARGET_ZONE = (1 << 16);
export inline constexpr auto DHF_IN_RESCUE_ZONE = (1 << 17);
export inline constexpr auto DHF_IN_ESCAPE_ZONE = (1 << 18);
export inline constexpr auto DHF_IN_VIPSAFETY_ZONE = (1 << 19);
export inline constexpr auto DHF_NIGHTVISION = (1 << 20);
export inline constexpr auto DHF_HOSTAGE_CTMOVE = (1 << 21);
export inline constexpr auto DHF_SPEC_DUCK = (1 << 22);

export inline constexpr auto DHM_ROUND_CLEAR = (DHF_ROUND_STARTED | DHF_HOSTAGE_KILLED | DHF_FRIEND_KILLED | DHF_BOMB_RETRIEVED);
export inline constexpr auto DHM_CONNECT_CLEAR = (DHF_HOSTAGE_SEEN_FAR | DHF_HOSTAGE_SEEN_NEAR | DHF_HOSTAGE_USED | DHF_HOSTAGE_INJURED | DHF_FRIEND_SEEN | DHF_ENEMY_SEEN | DHF_FRIEND_INJURED | DHF_ENEMY_KILLED | DHF_AMMO_EXHAUSTED | DHF_IN_TARGET_ZONE | DHF_IN_RESCUE_ZONE | DHF_IN_ESCAPE_ZONE | DHF_IN_VIPSAFETY_ZONE | DHF_HOSTAGE_CTMOVE | DHF_SPEC_DUCK);

export inline constexpr auto SIGNAL_BUY = (1 << 0);
export inline constexpr auto SIGNAL_BOMB = (1 << 1);
export inline constexpr auto SIGNAL_RESCUE = (1 << 2);
export inline constexpr auto SIGNAL_ESCAPE = (1 << 3);
export inline constexpr auto SIGNAL_VIPSAFETY = (1 << 4);

export class CUnifiedSignals
{
public:
	void Update(void) noexcept
	{
		m_flState = m_flSignal;
		m_flSignal = 0;
	}

	void Signal(int flags) noexcept { m_flSignal |= flags; }
	int GetSignal(void) const noexcept { return m_flSignal; }
	int GetState(void) const noexcept { return m_flState; }

private:
	int m_flSignal = 0;
	int m_flState = 0;
};

export inline constexpr auto IGNOREMSG_NONE = 0;
export inline constexpr auto IGNOREMSG_ENEMY = 1;
export inline constexpr auto IGNOREMSG_TEAM = 2;

export inline constexpr auto CSUITPLAYLIST = 4;

export inline constexpr auto SUIT_GROUP = true;
export inline constexpr auto SUIT_SENTENCE = false;

export inline constexpr auto SUIT_REPEAT_OK = 0;
export inline constexpr auto SUIT_NEXT_IN_30SEC = 30;
export inline constexpr auto SUIT_NEXT_IN_1MIN = 60;
export inline constexpr auto SUIT_NEXT_IN_5MIN = 300;
export inline constexpr auto SUIT_NEXT_IN_10MIN = 600;
export inline constexpr auto SUIT_NEXT_IN_30MIN = 1800;
export inline constexpr auto SUIT_NEXT_IN_1HOUR = 3600;

export inline constexpr auto AUTOAIM_2DEGREES = 0.0348994967025f;
export inline constexpr auto AUTOAIM_5DEGREES = 0.08715574274766f;
export inline constexpr auto AUTOAIM_8DEGREES = 0.1391731009601f;
export inline constexpr auto AUTOAIM_10DEGREES = 0.1736481776669f;

export inline constexpr auto CSUITNOREPEAT = 32;

export inline constexpr char SOUND_FLASHLIGHT_ON[] = "items/flashlight1.wav";
export inline constexpr char SOUND_FLASHLIGHT_OFF[] = "items/flashlight1.wav";

export inline constexpr auto TEAM_NAME_LENGTH = 16;

export enum PLAYER_ANIM
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	PLAYER_ATTACK2,
	PLAYER_FLINCH,
	PLAYER_LARGE_FLINCH,
	PLAYER_RELOAD,
	PLAYER_HOLDBOMB
};

export enum sbar_data
{
	SBAR_ID_TARGETTYPE = 1,
	SBAR_ID_TARGETNAME,
	SBAR_ID_TARGETHEALTH,
	SBAR_END
};

export inline constexpr auto MAX_ID_RANGE = 2048;
export inline constexpr auto MAX_SPECTATOR_ID_RANGE = 8192;
export inline constexpr auto SBAR_STRING_SIZE = 128;

export inline constexpr auto SBAR_TARGETTYPE_TEAMMATE = 1;
export inline constexpr auto SBAR_TARGETTYPE_ENEMY = 2;
export inline constexpr auto SBAR_TARGETTYPE_HOSTAGE = 3;

export enum EObserverMode : decltype(entvars_t::iuser1)
{
	OBS_NONE = 0,
	OBS_CHASE_LOCKED,
	OBS_CHASE_FREE,
	OBS_ROAMING,
	OBS_IN_EYE,
	OBS_MAP_FREE,
	OBS_MAP_CHASE,
};

export struct RebuyStruct
{
	int m_primaryWeapon;
	int m_primaryAmmo;
	int m_secondaryWeapon;
	int m_secondaryAmmo;
	int m_heGrenade;
	int m_flashbang;
	int m_smokeGrenade;
	qboolean m_defuser;
	qboolean m_nightVision;
	int m_armor;
};

export enum struct MusicState
{
	SILENT,
	CALM,
	INTENSE
};

export enum ECsTeams : std::int32_t
{
	TEAM_UNASSIGNED,
	TEAM_TERRORIST,
	TEAM_CT,
	TEAM_SPECTATOR,
};

export class CBasePlayer : public CBaseMonster
{
public:
	virtual void Spawn(void) = 0;
	virtual void Precache(void) = 0;
	virtual void Restart(void) = 0;
	virtual int Save(void *save) = 0;	// CSave &
	virtual int Restore(void *restore) = 0;	// CRestore &
	virtual int ObjectCaps(void) = 0;
	virtual int Classify(void) = 0;
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) = 0;
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual int TakeHealth(float flHealth, int bitsDamageType) = 0;
	virtual void Killed(entvars_t *pevAttacker, int iGib) = 0;
	virtual void AddPoints(int score, qboolean bAllowNegativeScore) = 0;
	virtual void AddPointsToTeam(int score, qboolean bAllowNegativeScore) = 0;
	virtual qboolean AddPlayerItem(CBasePlayerItem *pItem) = 0;
	virtual qboolean RemovePlayerItem(CBasePlayerItem *pItem) = 0;
	virtual int GiveAmmo(int iAmount, char *szName, int iMax) = 0;
	virtual void StartSneaking(void) = 0;
	virtual void StopSneaking(void) = 0;
	virtual qboolean IsSneaking(void) = 0;
	virtual qboolean IsAlive(void) = 0;
	virtual qboolean IsPlayer(void) = 0;
	virtual qboolean IsNetClient(void) = 0;
	virtual const char *TeamID(void) = 0;
	virtual qboolean FBecomeProne(void) = 0;
	virtual Vector BodyTarget(const Vector &posSrc) = 0;
	virtual int Illumination(void) = 0;
	virtual qboolean ShouldFadeOnDeath(void) = 0;
	virtual void ResetMaxSpeed(void) = 0;
	virtual void Jump(void) = 0;
	virtual void Duck(void) = 0;
	virtual void PreThink(void) = 0;
	virtual void PostThink(void) = 0;
	virtual Vector GetGunPosition(void) = 0;
	virtual qboolean IsBot(void) = 0;
	virtual void UpdateClientData(void) = 0;
	virtual void ImpulseCommands(void) = 0;
	virtual void RoundRespawn(void) = 0;
	virtual Vector GetAutoaimVector(float flDelta) = 0;
	virtual void Blind(float flUntilTime, float flHoldTime, float flFadeTime, int iAlpha) = 0;
	virtual void OnTouchingWeapon(CBasePlayerWeapon *pWeapon) = 0;	// Could be CBasePlayerWeapon or CWeaponBox. Weird.

//public:
//	void Pain(int hitgroup, bool hitkevlar) override;
//	void RenewItems(void) override;
//	void PackDeadPlayerItems(void) override;
//	void RemoveAllItems(qboolean removeSuit) override;
//	void SwitchTeam(void) override;
//	qboolean SwitchWeapon(CBasePlayerItem *pWeapon) override;
//	qboolean IsOnLadder(void) override;
//	qboolean FlashlightIsOn(void) override;
//	void FlashlightTurnOn(void) override;
//	void FlashlightTurnOff(void) override;
//	void UpdatePlayerSound(void) override;
//	void DeathSound(void) override;
	void SetAnimation(PLAYER_ANIM playerAnim) noexcept;
//	void SetWeaponAnimType(const char *szExtention) override;
//	void CheatImpulseCommands(int iImpulse) override;
//	void StartDeathCam(void) override;
//	void StartObserver(Vector vecPosition, Vector vecViewAngle) override;
//	CBaseEntity *Observer_IsValidTarget(int iTarget, bool bOnlyTeam) override;
//	void Observer_FindNextPlayer(bool bReverse, char *name = NULL) override;
//	void Observer_HandleButtons(void) override;
//	void Observer_SetMode(int iMode) override;
//	void Observer_CheckTarget(void) override;
//	void Observer_CheckProperties(void) override;
//	int IsObserver(void) override { return pev->iuser1; }
//	bool IsObservingPlayer(CBasePlayer *pTarget) override;
//	void SetObserverAutoDirector(bool bState) override;
//	qboolean CanSwitchObserverModes(void) override;
//	void DropPlayerItem(const char *pszItemName) override;
//	void ThrowPrimary(void) override;
//	void ThrowWeapon(char *pszWeaponName) override;
	qboolean HasPlayerItem(CBasePlayerItem *pCheckItem) const noexcept;
//	qboolean HasNamedPlayerItem(const char *pszItemName) override;
//	qboolean HasWeapons(void) override;
//	void SelectPrevItem(int iItem) override;
//	void SelectNextItem(int iItem) override;
//	void SelectLastItem(void) override;
//	void SelectItem(const char *pstr) override;
//	void ItemPreFrame(void) override;
//	void ItemPostFrame(void) override;
//	void GiveNamedItem(const char *szName) override;
//	void EnableControl(qboolean fControl) override;
//	void SendAmmoUpdate(void) override;
//	void SendFOV(int iFOV) override;
//	void SendHostagePos(void) override;
//	void SendHostageIcons(void) override;
//	void SendWeatherInfo(void) override;
//	void WaterMove(void) override;
//	void EXPORT PlayerDeathThink(void) override;
//	void PlayerUse(void) override;
//	void CheckSuitUpdate(void) override;
//	void SetSuitUpdate(char *name, int fgroup, int iNoRepeat) override;
//	void UpdateGeigerCounter(void) override;
//	void CheckTimeBasedDamage(void) override;
//	void BarnacleVictimBitten(entvars_t *pevBarnacle) override;
//	void BarnacleVictimReleased(void) override;
	static int GetAmmoIndex(const char *psz) noexcept;
	int AmmoInventory(int iAmmoIndex) const noexcept;
//	void ResetAutoaim(void) override;
//	Vector AutoaimDeflection(Vector &vecSrc, float flDist, float flDelta) override;
//	void ForceClientDllUpdate(void) override;
//	void SetCustomDecalFrames(int nFrames) override;
//	int GetCustomDecalFrames(void) override;
	void TabulateAmmo(void) noexcept;
//	void SetProgressBarTime(int iTime) override;
//	void SetProgressBarTime2(int iTime, float flLastTime) override;
//	void SetPlayerModel(qboolean HasC4) override;
//	void SetNewPlayerModel(const char *model) override;
//	void CheckPowerups(entvars_t *pev) override;
//	void SmartRadio(void) override;
//	void Radio(const char *msg_id, const char *msg_verbose, int pitch = 100, bool showIcon = true) override;
//	void GiveDefaultItems(void) override;
//	void SetBombIcon(qboolean bFlash) override;
//	void SetScoreAttrib(CBasePlayer *dest) override;
//	void SetScoreboardAttributes(CBasePlayer *pPlayer = NULL) override;
//	qboolean IsBombGuy(void) override;
//	qboolean ShouldDoLargeFlinch(int nHitGroup, int nGunType) override;
//	qboolean IsArmored(int nHitGroup) override;
//	bool HintMessage(const char *pMessage, qboolean bDisplayIfDead = FALSE, qboolean bOverrideClientSettings = FALSE) override;
//	void AddAccount(int amount, bool bTrackChange = true) override;
//	void SyncRoundTimer(void) override;
//	void MenuPrint(const char *text) override;
//	void ResetMenu(void) override;
//	void MakeVIP(void) override;
//	void JoiningThink(void) override;
//	void ResetStamina(void) override;
//	void Disappear(void) override;
//	void RemoveLevelText(void) override;
//	void MoveToNextIntroCamera(void) override;
//	void SpawnClientSideCorpse(void) override;
//	void SetPrefsFromUserinfo(char *infobuffer) override;
//	void HostageUsed(void) override;
	bool CanPlayerBuy(bool display) noexcept;
//	void StudioEstimateGait(void) override;
//	void CalculatePitchBlend(void) override;
//	void CalculateYawBlend(void) override;
//	void StudioProcessGait(void) override;
//	void HandleSignals(void) override;
//	void EnterEscapeZone(void) override;
//	void LeaveEscapeZone(void) override;
//	void EnterVIPSafetyZone(void) override;
//	void LeaveVIPSafetyZone(void) override;
//	void InitStatusBar(void) override;
//	void UpdateStatusBar(void) override;
//	bool IsHittingShield(const Vector &vecDirection, TraceResult *ptr) override;
//	bool IsReloading(void) override;
//	bool IsThrowingGrenade(void) override;
//	void StopReload(void) override;
//	void DrawnShiled(void) override;
	bool HasShield(void) const noexcept { return m_bOwnsShield; }
	void UpdateShieldCrosshair(bool bShieldDrawn) noexcept;
	void DropShield(bool bDeploy = true) noexcept;
//	void GiveShield(bool bRetire) override;
//	bool IsProtectedByShield(void) override;
//	void RemoveShield(void) override;
//	void UpdateLocation(bool bForceUpdate) override;
//	void ClientCommand(const char *arg0, const char *arg1 = NULL, const char *arg2 = NULL, const char *arg3 = NULL) override;
//	void ClearAutoBuyData(void) override;
//	void AddAutoBuyData(const char *string) override;
//	void InitRebuyData(const char *string) override;
//	void AutoBuy(void) override;
//	bool ShouldExecuteAutoBuyCommand(const AutoBuyInfoStruct *commandInfo, bool boughtPrimary, bool boughtSecondary) override;
//	AutoBuyInfoStruct *GetAutoBuyCommandInfo(const char *command) override;
//	void PrioritizeAutoBuyString(char *autobuyString, const char *priorityString) override;
//	void ParseAutoBuyString(const char *string, bool &boughtPrimary, bool &boughtSecondary) override;
//	void PostAutoBuyCommandProcessing(const AutoBuyInfoStruct *commandInfo, bool &boughtPrimary, bool &boughtSecondary) override;
//	void BuildRebuyStruct(void) override;
//	void Rebuy(void) override;
//	void RebuyPrimaryWeapon(void) override;
//	void RebuyPrimaryAmmo(void) override;
//	void RebuySecondaryWeapon(void) override;
//	void RebuySecondaryAmmo(void) override;
//	void RebuyHEGrenade(void) override;
//	void RebuyFlashbang(void) override;
//	void RebuySmokeGrenade(void) override;
//	void RebuyDefuser(void) override;
//	void RebuyNightVision(void) override;
//	void RebuyArmor(void) override;

//public:
//	static TYPEDESCRIPTION m_playerSaveData[];

public:
	int random_seed;
	unsigned short m_usPlayerBleed;
	EHANDLE<CBasePlayer> m_hObserverTarget;
	float m_flNextObserverInput;
	int m_iObserverWeapon;
	int m_iObserverC4State;
	bool m_bObserverHasDefuser;
	int m_iObserverLastMode;
	float m_flFlinchTime;
	float m_flAnimTime;
	bool m_bHighDamage;
	float m_flVelocityModifier;
	int m_iLastZoom;
	bool m_bResumeZoom;
	float m_flEjectBrass;
	int m_iKevlar;
	bool m_bNotKilled;
	ECsTeams m_iTeam;
	int m_iAccount;
	bool m_bHasPrimary;
	float m_flDeathThrowTime;
	int m_iThrowDirection;
	float m_flLastTalk;
	bool m_bJustConnected;
	bool m_bContextHelp;
	JoinState m_iJoiningState;
	CBaseEntity *m_pIntroCamera;
	float m_fIntroCamTime;
	float m_fLastMovement;
	bool m_bMissionBriefing;
	bool m_bTeamChanged;
	int m_iModelName;
	int m_iTeamKills;
	int m_iIgnoreGlobalChat;
	bool m_bHasNightVision;
	bool m_bNightVisionOn;
	Vector m_vRecentPath[20];
	float m_flIdleCheckTime;
	float m_flRadioTime;
	int m_iRadioMessages;
	bool m_bIgnoreRadio;
	bool m_bHasC4;
	bool m_bHasDefuser;
	bool m_bKilledByBomb;
	Vector m_vBlastVector;
	bool m_bKilledByGrenade;

//	CHintMessageQueue m_hintMessageQueue;
	float _PADDING_FLOAT_1{};
	void *_PADDING_POINTER_1{};
	int _PADDING_INT_1{};
	int _PADDING_INT_2{};
	int _PADDING_INT_3{};
	void *_PADDING_POINTER_2{};

	int m_flDisplayHistory;
	int m_iMenu;
	int m_iChaseTarget;
	CBaseEntity *m_pChaseTarget;
	qboolean m_fCamSwitch;
	bool m_bEscaped;
	bool m_bIsVIP;
	float m_tmNextRadarUpdate;
	Vector m_vLastOrigin;
	int m_iCurrentKickVote;
	float m_flNextVoteTime;
	bool m_bJustKilledTeammate;
	int m_iHostagesKilled;
	int m_iMapVote;
	bool m_bCanShoot;
	float m_flLastFired;
	float m_flLastAttackedTeammate;
	bool m_bHeadshotKilled;
	bool m_bPunishedForTK;
	bool m_bReceivesNoMoneyNextRound;
	int m_iTimeCheckAllowed;
	bool m_bHasChangedName;
	char m_szNewName[MAX_PLAYER_NAME_LENGTH];
	bool m_bIsDefusing;
	float m_tmHandleSignals;
	CUnifiedSignals m_signals;
	edict_t *m_pentCurBombTarget;
	int m_iPlayerSound;
	int m_iTargetVolume;
	int m_iWeaponVolume;
	int m_iExtraSoundTypes;
	int m_iWeaponFlash;
	float m_flStopExtraSoundTime;
	float m_flFlashLightTime;
	int m_iFlashBattery;
	int m_afButtonLast;
	int m_afButtonPressed;
	int m_afButtonReleased;
	edict_t *m_pentSndLast;
	float m_flSndRoomtype;
	float m_flSndRange;
	float m_flFallVelocity;
	int m_rgItems[4];	// MAX_ITEMS HLSDK: 5, Counter-Strike: 4
	int m_fNewAmmo;
	unsigned int m_afPhysicsFlags;
	float m_fNextSuicideTime;
	float m_flTimeStepSound;
	float m_flTimeWeaponIdle;
	float m_flSwimTime;
	float m_flDuckTime;
	float m_flWallJumpTime;
	float m_flSuitUpdate;
	int m_rgSuitPlayList[CSUITPLAYLIST];
	int m_iSuitPlayNext;
	int m_rgiSuitNoRepeat[CSUITNOREPEAT];
	float m_rgflSuitNoRepeatTime[CSUITNOREPEAT];
	int m_lastDamageAmount;
	float m_tbdPrev;
	float m_flgeigerRange;
	float m_flgeigerDelay;
	int m_igeigerRangePrev;
	int m_iStepLeft;
	char m_szTextureName[CBTEXTURENAMEMAX];
	char m_chTextureType;
	int m_idrowndmg;
	int m_idrownrestored;
	int m_bitsHUDDamage;
	qboolean m_fInitHUD;
	qboolean m_fGameHUDInitialized;
	int m_iTrain;
	qboolean m_fWeapon;
	EHANDLE<CBaseEntity> m_pTank;
	float m_fDeadTime;
	qboolean m_fNoPlayerSound;
	qboolean m_fLongJump;
	float m_tSneaking;
	int m_iUpdateTime;
	int m_iClientHealth;
	int m_iClientBattery;
	int m_iHideHUD;
	int m_iClientHideHUD;
	int m_iFOV;
	int m_iClientFOV;
	int m_iNumSpawns;
	CBaseEntity *m_pObserver;
	CBasePlayerItem *m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;
	CBasePlayerItem *m_pLastItem;
	int m_rgAmmo[MAX_AMMO_SLOTS];
	int m_rgAmmoLast[MAX_AMMO_SLOTS];
	Vector m_vecAutoAim;
	qboolean m_fOnTarget;
	int m_iDeaths;
	int m_izSBarState[SBAR_END];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[SBAR_STRING_SIZE];
	int m_lastx, m_lasty;
	int m_nCustomSprayFrames;
	float m_flNextDecalTime;
	char m_szTeamName[TEAM_NAME_LENGTH];
	int m_modelIndexPlayer;
	char m_szAnimExtention[32];
	int m_iGaitsequence;
	float m_flGaitframe;
	float m_flGaityaw;
	vec3_t m_prevgaitorigin;
	float m_flPitch;
	float m_flYaw;
	float m_flGaitMovement;
	int m_iAutoWepSwitch;
	bool m_bVGUIMenus;
	bool m_bShowHints;
	bool m_bShieldDrawn;
	bool m_bOwnsShield;
	bool m_bWasFollowing;
	float m_flNextFollowTime;
	float m_flYawModifier;
	float m_blindUntilTime;
	float m_blindStartTime;
	float m_blindHoldTime;
	float m_blindFadeTime;
	float m_blindAlpha;
	float m_allowAutoFollowTime;
	char m_autoBuyString[MAX_AUTOBUY_LENGTH];
	char *m_rebuyString;
	RebuyStruct m_rebuyStruct;
	bool m_bIsInRebuy;
	float m_flLastUpdateTime;
	char m_lastLocation[MAX_LACTION_LENGTH];
	int m_progressStart;
	int m_progressEnd;
	bool m_bObserverAutoDirector;
	bool m_canSwitchObserverModes;
	float m_heartBeatTime;
	int m_intenseTimestamp;
	int m_silentTimestamp;
	MusicState m_musicState;
	int m_flLastCommandTime[8];
};

static_assert(offsetof(CBasePlayer, m_allowAutoFollowTime) == 519 * 4);
static_assert(offsetof(CBasePlayerWeapon, m_flDecreaseShotsFired) == 76 * 4);

export class CGrenade : public CBaseMonster
{
public:
	virtual void Spawn(void) override = 0;
	virtual int Save(void *save) override = 0;	// CSave &
	virtual int Restore(void *restore) override = 0;	// CRestore &
	virtual void BounceSound(void) = 0;
	virtual int ObjectCaps(void) override = 0;
	virtual int BloodColor(void) override = 0;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) override = 0;
	virtual void Killed(entvars_t *pevAttacker, int iGib) override = 0;

//public:
//	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;
//
//public:
//	static CGrenade *ShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time);
//	static CGrenade *ShootTimed2(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, int iTeam, unsigned short usEvent);
//	static CGrenade *ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
//	static CGrenade *ShootSmokeGrenade(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, unsigned short usEvent);
//	static CGrenade *ShootSatchelCharge(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
//	static void UseSatchelCharges(entvars_t *pevOwner, SATCHELCODE code);
//
//public:
//	void Explode(Vector vecSrc, Vector vecAim);
//	void Explode(TraceResult *pTrace, int bitsDamageType);
//	void Explode2(TraceResult *pTrace, int bitsDamageType);
//	void Explode3(TraceResult *pTrace, int bitsDamageType);
//	void EXPORT Smoke(void);
//	void EXPORT SG_Smoke(void);
//	void EXPORT Smoke2(void);
//	void EXPORT Smoke3_A(void);
//	void EXPORT Smoke3_B(void);
//	void EXPORT Smoke3_C(void);
//	void EXPORT BounceTouch(CBaseEntity *pOther);
//	void EXPORT SlideTouch(CBaseEntity *pOther);
//	void EXPORT ExplodeTouch(CBaseEntity *pOther);
//	void EXPORT DangerSoundThink(void);
//	void EXPORT PreDetonate(void);
//	void EXPORT Detonate(void);
//	void EXPORT SG_Detonate(void);
//	void EXPORT Detonate2(void);
//	void EXPORT Detonate3(void);
//	void EXPORT DetonateUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
//	void EXPORT TumbleThink(void);
//	void EXPORT SG_TumbleThink(void);
//	void EXPORT C4Think(void);
//	void EXPORT C4Touch(CBaseEntity *pOther) {}
//
//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	bool m_bStartDefuse;
	bool m_bIsC4;
	EHANDLE<CBaseEntity> m_pBombDefuser;
	float m_flDefuseCountDown;
	float m_flC4Blow;
	float m_flNextFreqInterval;
	float m_flNextBeep;
	float m_flNextFreq;
	char *m_sBeepName;
	float m_fAttenu;
	float m_flNextBlink;
	float m_fNextDefuse;
	bool m_bJustBlew;
	int m_iTeam;
	int m_iCurWave;
	edict_t *m_pentCurBombTarget;
	int m_SGSmoke;
	int m_angle;
	unsigned short m_usEvent;
	bool m_bLightSmoke;
	bool m_bDetonated;
	Vector m_vSmokeDetonate;
	int m_iBounceCount;
	qboolean m_fRegisteredSound;
};

export class CWeaponBox : public CBaseEntity
{
public:
	void Spawn(void) noexcept override = 0;
	void Precache(void) noexcept override = 0;
	void KeyValue(KeyValueData* pkvd) noexcept override = 0;
	int Save(void* save) noexcept override = 0;
	int Restore(void* restore) noexcept override = 0;
	void Touch(CBaseEntity* pOther) noexcept override = 0;
	void SetObjectCollisionBox(void) noexcept override = 0;

//public:
//	BOOL IsEmpty(void);
//	int GiveAmmo(int iCount, char* szName, int iMax, int* pIndex = NULL);
//	void EXPORT Kill(void);
//	BOOL HasWeapon(CBasePlayerItem* pCheckItem);
//	BOOL PackWeapon(CBasePlayerItem* pWeapon);
//	BOOL PackAmmo(int iszName, int iCount);

//public:
//	static TYPEDESCRIPTION m_SaveData[];

public:
	CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES];
	int m_rgiszAmmo[MAX_AMMO_SLOTS];
	int m_rgAmmo[MAX_AMMO_SLOTS];
	int m_cAmmoTypes;
};

export class CHostage : public CBaseMonster
{
public:
	void Spawn() override = 0;
	void Precache() override = 0;
	int ObjectCaps() override = 0;		// make hostage "useable"
	int Classify()  override = 0;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override = 0;
	qboolean TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override = 0;
	int BloodColor() override = 0;

#ifndef REGAMEDLL_FIXES
	qboolean IsAlive() override = 0;
#endif

	void Touch(CBaseEntity* pOther) override = 0;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override = 0;

public:
	//void EXPORT IdleThink();
	//void EXPORT Remove();
	//void RePosition();
	//void SetActivity(Activity act);
	//Activity GetActivity() { return m_Activity; }
	//float GetModifiedDamage(float flDamage, int nHitGroup);
	//void SetFlinchActivity();
	//void SetDeathActivity();
	//void PlayPainSound();
	//void PlayFollowRescueSound();
	//void AnnounceDeath(CBasePlayer* pAttacker);
	//void ApplyHostagePenalty(CBasePlayer* pAttacker);
	//void GiveCTTouchBonus(CBasePlayer* pPlayer);
	//void SendHostagePositionMsg();
	//void SendHostageEventMsg();
	//void DoFollow();
	//BOOL IsOnLadder();
	//void PointAt(const Vector& vecLoc);
	//void MoveToward(const Vector& vecLoc);
	//void NavReady();
	//void Wiggle();
	//void PreThink();
	//bool CanTakeDamage(entvars_t* pevAttacker);

	// queries
	//bool IsFollowingSomeone() noexcept { return IsFollowing(); }
	//CBaseEntity* GetLeader()				// return our leader, or NULL
	//{
	//	if (m_improv) {
	//		return m_improv->GetFollowLeader();
	//	}

	//	return m_hTargetEnt;
	//}
	//bool IsFollowing(const CBaseEntity* pEntity = nullptr)
	//{
	//	if (m_improv) {
	//		return m_improv->IsFollowing(pEntity);
	//	}

	//	if ((!pEntity && !m_hTargetEnt) || (pEntity && m_hTargetEnt != pEntity))
	//		return false;

	//	if (m_State != FOLLOW)
	//		return false;

	//	return true;
	//}

	bool IsValid()  const noexcept { return (pev->takedamage == DAMAGE_YES); }
	bool IsDead()   const noexcept { return (pev->deadflag == DEAD_DEAD); }
	bool IsAtHome() const noexcept { return (pev->origin - m_vStart).LengthSquared() < (20.0 * 20.0); }
	auto GetHomePosition() const noexcept -> Vector const& { return m_vStart; }

public:
	Activity m_Activity;	// Missing from ReGameDLL
	qboolean m_bTouched;
	qboolean m_bRescueMe;
	float m_flFlinchTime;
	float m_flNextChange;
	float m_flMarkPosition;
	int m_iModel;
	int m_iSkin;
	float m_flNextRadarTime;
	enum state { FOLLOW, STAND, DUCK, SCARED, IDLE, FOLLOWPATH };
	state m_State;
	Vector m_vStart;
	Vector m_vStartAngles;
	Vector m_vPathToFollow[20];	// MAX_HOSTAGES_NAV
	int m_iWaypoint;
	CBasePlayer* m_target;
	void* m_LocalNav;	// CLocalNav*
	int m_nTargetNode;
	Vector vecNodes[100];	// MAX_NODES
	EHANDLE<CBasePlayer> m_hStoppedTargetEnt;
	float m_flNextFullThink;
	float m_flPathCheckInterval;
	float m_flLastPathCheck;
	int m_nPathNodes;
	qboolean m_fHasPath;
	float m_flPathAcquired;
	Vector m_vOldPos;
	int m_iHostageIndex;
	qboolean m_bStuck;
	float m_flStuckTime;
	void* m_improv;	// CHostageImprov*

	enum ModelType { REGULAR_GUY, OLD_GUY, BLACK_GUY, GOOFY_GUY };
	ModelType m_whichModel;
};
