module;

#include <cassert>

export module Prefab;

import <cmath>;

import <concepts>;
import <functional>;
import <list>;

import util;

export import CBase;
export import Hook;
export import Task;

using std::list;

export struct Prefab_t : public CBaseEntity
{
	// Patch the loose end.
	virtual ~Prefab_t() noexcept = default;

	// Define all missing function from our pure virtual class.
	void Spawn() noexcept override {}
	void Precache() noexcept override {}
	void Restart() noexcept override { m_Scheduler.Clear(); }
	void KeyValue(KeyValueData *pkvd) noexcept override { pkvd->fHandled = false; }
	int Save(void *save) noexcept override { return 0; }
	int Restore(void *restore) noexcept override { return 0; }
	int ObjectCaps() noexcept override { return FCAP_ACROSS_TRANSITION; }
	void Activate() noexcept override {}

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	void SetObjectCollisionBox() noexcept override
	{
		if (pev->solid == SOLID_BSP && (pev->angles.x || pev->angles.y || pev->angles.z))
		{
			// expand for rotation
			double max, v;
			int i;

			max = 0;
			for (i = 0; i < 3; i++)
			{
				v = abs(double(((float *)pev->mins)[i]));
				if (v > max)
				{
					max = v;
				}

				v = abs(double(((float *)pev->maxs)[i]));
				if (v > max)
				{
					max = v;
				}
			}
			for (i = 0; i < 3; i++)
			{
				((float *)pev->absmin)[i] = ((float *)pev->origin)[i] - (float)max;
				((float *)pev->absmax)[i] = ((float *)pev->origin)[i] + (float)max;
			}
		}
		else
		{
			pev->absmin = pev->origin + pev->mins;
			pev->absmax = pev->origin + pev->maxs;
		}

		pev->absmin.x -= 1;
		pev->absmin.y -= 1;
		pev->absmin.z -= 1;

		pev->absmax.x += 1;
		pev->absmax.y += 1;
		pev->absmax.z += 1;
	}

	// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
	// still realize that they are teammates. (overridden for monsters that form groups)
	int Classify() noexcept override { return CLASS_NONE; }

	void DeathNotice(entvars_t *pevChild) noexcept override {}
	void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) noexcept override { return g_pfnEntityTraceAttack(this, pevAttacker, flDamage, vecDir, ptr, bitsDamageType); }
	qboolean TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) noexcept override { return g_pfnEntityTakeDamage(this, pevInflictor, pevAttacker, flDamage, bitsDamageType); }
	qboolean TakeHealth(float flHealth, int bitsDamageType) noexcept override
	{
		if (pev->takedamage == DAMAGE_NO)
			return false;

		if (pev->health >= pev->max_health)
			return false;

		pev->health += flHealth;

		if (pev->health > pev->max_health)
		{
			pev->health = pev->max_health;
		}

		return true;
	}
	void Killed(entvars_t *pevAttacker, int iGib) noexcept override { return g_pfnEntityKilled(this, pevAttacker, iGib); }
	int BloodColor() noexcept override { return DONT_BLEED; }
	void TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) noexcept override { return g_pfnEntityTraceBleed(this, flDamage, vecDir, ptr, bitsDamageType); }
	qboolean IsTriggered(CBaseEntity *pActivator) noexcept override { return true; }
	CBaseMonster *MyMonsterPointer() noexcept override { return nullptr; }
	void *MySquadMonsterPointer() noexcept override { return nullptr; }
	int GetToggleState() noexcept override { return TS_AT_TOP; }
	void AddPoints(int score, qboolean bAllowNegativeScore) noexcept override {}
	void AddPointsToTeam(int score, qboolean bAllowNegativeScore) noexcept override {}
	qboolean AddPlayerItem(CBasePlayerItem *pItem) noexcept override { return false; }
	qboolean RemovePlayerItem(CBasePlayerItem *pItem) noexcept override { return false; }
	int GiveAmmo(int iAmount, char *szName, int iMax = -1) noexcept override { return -1; }
	float GetDelay() noexcept override { return 0.0f; }
	int IsMoving() noexcept override { return (pev->velocity != Vector::Zero()); }
	void OverrideReset() noexcept override {}
	int DamageDecal(int bitsDamageType) noexcept override { return g_pfnEntityDamageDecal(this, bitsDamageType); }

	// This is ONLY used by the node graph to test movement through a door
	void SetToggleState(int state) noexcept override {}

	// LUNA: Totally unused. Swap to others?
	void StartSneaking() noexcept override {}
	void StopSneaking() noexcept override {}

	qboolean OnControls(entvars_t *onpev) noexcept override { return false; }
	qboolean IsSneaking() noexcept override { return false; }
	qboolean IsAlive() noexcept override { return (pev->deadflag == DEAD_NO && pev->health > 0.0f); }
	qboolean IsBSPModel() noexcept override { return (pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP); }
	qboolean ReflectGauss() noexcept override { return !!(IsBSPModel() && pev->takedamage == DAMAGE_NO); }
	qboolean HasTarget(string_t targetname) noexcept override { return FStrEq(STRING(targetname), STRING(pev->targetname)); }
	qboolean IsInWorld() noexcept override
	{
		// position
		if (pev->origin.x >= 4096.0 || pev->origin.y >= 4096.0 || pev->origin.z >= 4096.0)
		{
			return false;
		}
		if (pev->origin.x <= -4096.0 || pev->origin.y <= -4096.0 || pev->origin.z <= -4096.0)
		{
			return false;
		}

		// speed
		if (pev->velocity.x >= 2000.0 || pev->velocity.y >= 2000.0 || pev->velocity.z >= 2000.0)
		{
			return false;
		}
		if (pev->velocity.x <= -2000.0 || pev->velocity.y <= -2000.0 || pev->velocity.z <= -2000.0)
		{
			return false;
		}

		return true;
	}
	qboolean IsPlayer() noexcept override { return false; }
	qboolean IsNetClient() noexcept override { return false; }
	const char *TeamID() noexcept override { return ""; }
	CBaseEntity *GetNextTarget() noexcept override { return g_pfnEntityGetNextTarget(this); }
	void Think() noexcept final { m_Scheduler.Think(); pev->nextthink = 0.1f; }	// ensure the think can never be block by child classes.
	void Touch(CBaseEntity *pOther) noexcept override { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); }
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType = USE_OFF, float value = 0.0f) noexcept override { if (m_pfnUse) (this->*m_pfnUse)(pActivator, pCaller, useType, value); }
	void Blocked(CBaseEntity *pOther) noexcept override { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); }
	CBaseEntity *Respawn() noexcept override { return nullptr; }

	// used by monsters that are created by the MonsterMaker
	void UpdateOwner() noexcept override {}
	qboolean FBecomeProne() noexcept override { return false; }

	Vector Center() noexcept override { return (pev->absmax + pev->absmin) * 0.5f; }		// center point of entity
	Vector EyePosition() noexcept override { return (pev->origin + pev->view_ofs); }		// position of eyes
	Vector EarPosition() noexcept override { return (pev->origin + pev->view_ofs); }		// position of ears
	Vector BodyTarget(const Vector &posSrc) noexcept override { return Center(); }		// position to shoot at

	int Illumination() noexcept override { return g_engfuncs.pfnGetEntityIllum(edict()); }

	qboolean FVisible(CBaseEntity *pEntity) noexcept override
	{
		if (pEntity->pev->flags & FL_NOTARGET)
			return false;

		// don't look through water
		if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) || (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
			return false;

		//look through the caller's 'eyes'
		auto const vecLookerOrigin = pev->origin + pev->view_ofs;
		auto const vecTargetOrigin = pEntity->EyePosition();

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters|ignore_glass, edict(), &tr);

		if (tr.flFraction != 1.0f)
		{
			// Line of sight is not established
			return false;
		}
		else
		{
			// line of sight is valid.
			return true;
		}
	}
	qboolean FVisible(const Vector &vecOrigin) noexcept override
	{
		//look through the caller's 'eyes'
		auto const vecLookerOrigin = EyePosition();

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecLookerOrigin, vecOrigin, ignore_monsters|ignore_glass, edict(), &tr);

		if (tr.flFraction != 1.0f)
		{
			// Line of sight is not established
			return false;
		}
		else
		{
			// line of sight is valid.
			return true;
		}
	}

	// Have to entity have to be created like this.
	template <typename T, size_t N>
	static T *Create(const char (&szClassName)[N], const Vector& vecOrigin, const Vector& vecAngles = Vector::Zero()) noexcept
	{
		auto const pEdict = g_engfuncs.pfnCreateEntity();

		assert(pEdict != nullptr);
		assert(pEdict->pvPrivateData == nullptr);

		auto const pPrefab = new(pEdict) T;

		pPrefab->pev = &pEdict->v;

		assert(pPrefab->pev != nullptr);
		assert(pEdict->pvPrivateData != nullptr);
		assert(pEdict->v.pContainingEntity == pEdict);

		pEdict->v.classname = MAKE_STRING(szClassName);
		pEdict->v.angles = vecAngles;
		pEdict->v.origin = vecOrigin;

		pPrefab->Spawn();
		pPrefab->pev->nextthink = 0.1f;

		return pPrefab;
	}

	struct PrefabScheduler_t
	{
		list<Task> m_List{};

		inline void Think(void) noexcept
		{
			if (m_List.empty())
				return;

			while (!m_List.empty())
			{
				m_List.sort();

				while (!m_List.empty() && m_List.front().Done())
				{
					m_List.pop_front();
				}

				if (m_List.empty())
					break;

				[[likely]]
				if (m_List.front().ShouldResume())
					m_List.front().Resume();
				else
					break;	// if the first one in queue is not going to resume, then nor should anyone else.
			}
		}

		inline void Enroll(Task &&obj, unsigned long iCoroutineMarker = 0ul) noexcept
		{
			[[likely]]
			if (!obj.Done())
			{
				obj.m_iCoroutineMarker = iCoroutineMarker;

				m_List.emplace_back(std::forward<Task>(obj));
				m_List.sort();
				m_List.remove_if(std::bind_front(&Task::Done));
			}
		}

		inline unsigned int Delist(unsigned long iCoroutineMarker) noexcept
		{
			if (!m_List.empty())
				return m_List.remove_if([=](Task const &obj) noexcept { return obj.m_iCoroutineMarker == iCoroutineMarker; });

			return 0;
		}

		inline unsigned int Count(unsigned long iCoroutineMarker) noexcept
		{
			unsigned int ret{};

			for (auto const &obj : m_List)
			{
				if (obj.m_iCoroutineMarker == iCoroutineMarker)
					++ret;
			}

			return ret;
		}

		inline bool Exist(unsigned long iCoroutineMarker) noexcept
		{
			for (auto const &obj : m_List)
			{
				[[unlikely]]
				if (obj.m_iCoroutineMarker == iCoroutineMarker)
					return true;
			}

			return false;
		}

		inline void Clear(void) noexcept
		{
			m_List.clear();
		}
	}
	m_Scheduler{};

	// Move these to original CBaseEntity?

	__forceinline void SetTouch(std::nullptr_t) noexcept { m_pfnTouch = nullptr; }
	template <typename T>
	__forceinline void SetTouch(void (T:: *pfn)(CBaseEntity *pOther)) noexcept
	{
		m_pfnTouch = reinterpret_cast<decltype(m_pfnTouch)>(pfn);
	}

	__forceinline void SetUse(std::nullptr_t) noexcept { m_pfnTouch = nullptr; }
	template <typename T>
	__forceinline void SetUse(void (T:: *pfn)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)) noexcept
	{
		m_pfnUse = reinterpret_cast<decltype(m_pfnUse)>(pfn);
	}

	__forceinline void SetBlocked(std::nullptr_t) noexcept { m_pfnTouch = nullptr; }
	template <typename T>
	__forceinline void SetBlocked(void (T:: *pfn)(CBaseEntity *pOther)) noexcept
	{
		m_pfnBlocked = reinterpret_cast<decltype(m_pfnBlocked)>(pfn);
	}
};
