export module PlayerItem;

export import ConditionZero;
export import GameRules;
export import Message;
#ifndef __INTELLISENSE__
export import Task;
#endif
export import VTFH;

export inline constexpr auto WEAPON_IS_ONTARGET = 0x40;

export constexpr float UTIL_WeaponTimeBase() noexcept { return 0.f; }	// or return gpGlobals->time; if not CLIENT_WEAPONS
export constexpr bool CanAttack(float attack_time, float curtime, bool isPredicted) noexcept
{
	if (!isPredicted)
		return attack_time <= curtime;

	return attack_time <= 0;
}
export constexpr bool IsSecondaryWeapon(int id) noexcept
{
	switch (id)
	{
	case WEAPON_P228:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
	case WEAPON_USP:
	case WEAPON_GLOCK18:
	case WEAPON_DEAGLE:
		return true;
	default:
		break;
	}

	return false;
}


// LUNA:	the diamond inheritance will actually affect the memory layout.
//			therefore unfortunately it is need to make a full copy of the prefab above.

export struct CPrefabWeapon : CBasePlayerWeapon
{
	// Patch the loose end.
	CPrefabWeapon(void) noexcept = default;
	CPrefabWeapon(const CPrefabWeapon&) noexcept = delete;
	CPrefabWeapon(CPrefabWeapon&&) noexcept = delete;
	CPrefabWeapon& operator=(const CPrefabWeapon&) noexcept = delete;
	CPrefabWeapon& operator=(CPrefabWeapon&&) noexcept = delete;
	virtual ~CPrefabWeapon() noexcept = default;

	// Define all missing function from our pure virtual class.
	void Spawn() noexcept override {}
	void Precache() noexcept override {}
	void Restart() noexcept override { m_Scheduler.Clear(); }
	void KeyValue(KeyValueData* pkvd) noexcept override { pkvd->fHandled = false; }
	int Save(void* save) noexcept override { return 0; }
	int Restore(void* restore) noexcept override { return 0; }
	int ObjectCaps() noexcept override { return FCAP_ACROSS_TRANSITION; }
	void Activate() noexcept override {}

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	// void SetObjectCollisionBox() noexcept override; - overridden by CBasePlayerItem class.

	// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
	// still realize that they are teammates. (overridden for monsters that form groups)
	int Classify() noexcept override { return CLASS_NONE; }

	void DeathNotice(entvars_t* pevChild) noexcept override {}
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) noexcept override { return g_pfnEntityTraceAttack(this, pevAttacker, flDamage, vecDir, ptr, bitsDamageType); }
	qboolean TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) noexcept override { return g_pfnEntityTakeDamage(this, pevInflictor, pevAttacker, flDamage, bitsDamageType); }
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
	void Killed(entvars_t* pevAttacker, int iGib) noexcept override { return g_pfnEntityKilled(this, pevAttacker, iGib); }
	int BloodColor() noexcept override { return DONT_BLEED; }
	void TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) noexcept override { return g_pfnEntityTraceBleed(this, flDamage, vecDir, ptr, bitsDamageType); }
	qboolean IsTriggered(CBaseEntity* pActivator) noexcept override { return true; }
	CBaseMonster* MyMonsterPointer() noexcept override { return nullptr; }
	void* MySquadMonsterPointer() noexcept override { return nullptr; }
	int GetToggleState() noexcept override { return TS_AT_TOP; }
	void AddPoints(int score, qboolean bAllowNegativeScore) noexcept override {}
	void AddPointsToTeam(int score, qboolean bAllowNegativeScore) noexcept override {}
	qboolean AddPlayerItem(CBasePlayerItem* pItem) noexcept override { return false; }
	qboolean RemovePlayerItem(CBasePlayerItem* pItem) noexcept override { return false; }
	int GiveAmmo(int iAmount, char* szName, int iMax = -1) noexcept override { return -1; }
	float GetDelay() noexcept override { return 0.0f; }
	int IsMoving() noexcept override { return (pev->velocity != Vector::Zero()); }
	void OverrideReset() noexcept override {}
	int DamageDecal(int bitsDamageType) noexcept override { return g_pfnEntityDamageDecal(this, bitsDamageType); }

	// This is ONLY used by the node graph to test movement through a door
	void SetToggleState(int state) noexcept override {}

	// LUNA: Totally unused. Swap to others?
	void StartSneaking() noexcept override {}
	void StopSneaking() noexcept override {}

	qboolean OnControls(entvars_t* onpev) noexcept override { return false; }
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
	const char* TeamID() noexcept override { return ""; }
	CBaseEntity* GetNextTarget() noexcept override { return g_pfnEntityGetNextTarget(this); }
	void Think() noexcept final { m_Scheduler.Think(); pev->nextthink = 0.1f; }	// ensure the think can never be block by child classes.
	void Touch(CBaseEntity* pOther) noexcept override { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); }
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType = USE_OFF, float value = 0.0f) noexcept override { if (m_pfnUse) (this->*m_pfnUse)(pActivator, pCaller, useType, value); }
	void Blocked(CBaseEntity* pOther) noexcept override { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); }
	//CBaseEntity* Respawn() noexcept override { return nullptr; }	- overridden by CBasePlayerItem

	// used by monsters that are created by the MonsterMaker
	void UpdateOwner() noexcept override {}
	qboolean FBecomeProne() noexcept override { return false; }

	Vector Center() noexcept override { return (pev->absmax + pev->absmin) * 0.5f; }		// center point of entity
	Vector EyePosition() noexcept override { return (pev->origin + pev->view_ofs); }		// position of eyes
	Vector EarPosition() noexcept override { return (pev->origin + pev->view_ofs); }		// position of ears
	Vector BodyTarget(const Vector& posSrc) noexcept override { return Center(); }		// position to shoot at

	int Illumination() noexcept override { return g_engfuncs.pfnGetEntityIllum(edict()); }

	qboolean FVisible(CBaseEntity* pEntity) noexcept override
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
		g_engfuncs.pfnTraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters | ignore_glass, edict(), &tr);

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
	qboolean FVisible(const Vector& vecOrigin) noexcept override
	{
		//look through the caller's 'eyes'
		auto const vecLookerOrigin = EyePosition();

		TraceResult tr{};
		g_engfuncs.pfnTraceLine(vecLookerOrigin, vecOrigin, ignore_monsters | ignore_glass, edict(), &tr);

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


public:	// CBaseDelay
	// Nothing here. No new virtual functions.

public:	// CBaseAnimating
	void HandleAnimEvent(MonsterEvent_t* pEvent) noexcept override {}


public:	// CBasePlayerItem
	void SetObjectCollisionBox(void) noexcept override
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 16);
	}
	CBaseEntity* Respawn() noexcept override
	{
		// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
		// will decide when to make the weapon visible and touchable.
		auto pNewWeapon = CBaseEntity::Create(
			STRING(pev->classname),
			g_pGameRules->VecWeaponRespawnSpot(this),
			pev->angles,
			pev->owner
		);

		if (pNewWeapon)
		{
			// invisible for now
			pNewWeapon->pev->effects |= EF_NODRAW;

			// no touch
			pNewWeapon->SetTouch(nullptr);
			pNewWeapon->SetThink(&CPrefabWeapon::AttemptToMaterialize);	// should be: CBasePlayerItem

			g_engfuncs.pfnDropToFloor(edict());

			// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
			// but when it should respawn is based on conditions belonging to the weapon that was taken.
			pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime(this);
		}
		else
		{
			g_engfuncs.pfnAlertMessage(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
		}

		return pNewWeapon;
	}
	//qboolean AddToPlayer(CBasePlayer* pPlayer) noexcept override;	- overridden by CBasePlayerWeapon
	//qboolean AddDuplicate(CBasePlayerItem* pItem) noexcept override;	- overridden by CBasePlayerWeapon
	int GetItemInfo(ItemInfo* p) noexcept override { return 0; }
	//qboolean CanDeploy(void) noexcept override { return true; }	- overridden by CBasePlayerWeapon
	qboolean CanDrop(void) noexcept override { return true; }
	qboolean Deploy(void) noexcept override { return true; }
	//qboolean IsWeapon(void) noexcept override { return false; }	- overridden by CBasePlayerWeapon
	qboolean CanHolster(void) noexcept override { return true; }
	//void Holster(int skiplocal = 0) noexcept override;	- overridden by CBasePlayerWeapon
	//void UpdateItemInfo(void) noexcept override {}	- overridden by CBasePlayerWeapon
	void ItemPreFrame(void) noexcept override {}
	//void ItemPostFrame(void) noexcept override {}	- overridden by CBasePlayerWeapon
	void Drop(void) noexcept override
	{
		SetTouch(nullptr);
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1f;
	}
	void Kill(void) noexcept override
	{
		SetTouch(nullptr);
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1f;
	}
	void AttachToPlayer(CBasePlayer* pPlayer) noexcept override
	{
		pev->movetype = MOVETYPE_FOLLOW;
		pev->solid = SOLID_NOT;
		pev->aiment = pPlayer->edict();
		pev->effects = EF_NODRAW;

		// server won't send down to clients if modelindex == 0
		pev->modelindex = 0;
		pev->model = 0;
		pev->owner = pPlayer->edict();

		// Remove think - prevents futher attempts to materialize
		pev->nextthink = 0;
		SetThink(nullptr);

		SetTouch(nullptr);
	}
	int PrimaryAmmoIndex(void) noexcept override { return -1; }
	int SecondaryAmmoIndex(void) noexcept override { return -1; }
	//int UpdateClientData(CBasePlayer* pPlayer) noexcept override { return 0; }	- overridden by CBasePlayerWeapon
	//CBasePlayerItem* GetWeaponPtr(void) noexcept override { return nullptr; }	- overridden by CBasePlayerWeapon
	float GetMaxSpeed(void) noexcept override { return 260; }
	int iItemSlot(void) noexcept override { return 0; }


public:	// CBasePlayerWeapon
	qboolean AddToPlayer(CBasePlayer* pPlayer) noexcept override
	{
		m_pPlayer = pPlayer;

		gmsgWeapPickup::Send(pPlayer->edict(), m_iId);

		return true;
	}
	qboolean AddDuplicate(CBasePlayerItem* pItem) noexcept override
	{
		if (m_iDefaultAmmo)
			return ExtractAmmo((CBasePlayerWeapon*)pItem);

		return ExtractClipAmmo((CBasePlayerWeapon*)pItem);
	}
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) noexcept override
	{
		int res = 0;
		if (pszAmmo1())
		{
			// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
			// we only get the ammo in the weapon's clip, which is what we want.
			res = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, (char*)pszAmmo1(), iMaxClip(), iMaxAmmo1());
			m_iDefaultAmmo = 0;
		}

		if (pszAmmo2())
		{
			res = AddSecondaryAmmo(0, (char*)pszAmmo2(), iMaxAmmo2());
		}

		return res;
	}
	int ExtractClipAmmo(CBasePlayerWeapon* pWeapon) noexcept override
	{
		int iAmmo;
		if (m_iClip == -1)
		{
			// guns with no clips always come empty if they are second-hand
			iAmmo = 0;
		}
		else
		{
			iAmmo = m_iClip;
		}

		return pWeapon->m_pPlayer->GiveAmmo(iAmmo, (char*)pszAmmo1(), iMaxAmmo1());
	}
	int AddWeapon(void) noexcept override { ExtractAmmo(this); return true; }
	void UpdateItemInfo(void) noexcept override {};
	qboolean PlayEmptySound(void) noexcept override
	{
		if (m_iPlayEmptySound)
		{
			switch (m_iId)
			{
			case WEAPON_USP:
			case WEAPON_GLOCK18:
			case WEAPON_P228:
			case WEAPON_DEAGLE:
			case WEAPON_ELITE:
			case WEAPON_FIVESEVEN:
				g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/dryfire_pistol.wav", 0.8f, ATTN_NORM, 0, PITCH_NORM);
				break;
			default:
				g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_WEAPON, "weapons/dryfire_rifle.wav", 0.8f, ATTN_NORM, 0, PITCH_NORM);
				break;
			}
		}

		return false;	// LUNA: why??? all path leads to false??
	}
	void ResetEmptySound(void) noexcept override { m_iPlayEmptySound = 1; }
	void SendWeaponAnim(int iAnim, qboolean skiplocal = 0) noexcept override
	{
		m_pPlayer->pev->weaponanim = iAnim;

		if (skiplocal && g_engfuncs.pfnCanSkipPlayer(m_pPlayer->edict()))
			return;

		gmsgWeaponAnim::Send(m_pPlayer->edict(), iAnim, pev->body);
	}
	qboolean CanDeploy(void) noexcept override { return true; }
	qboolean IsWeapon(void) noexcept override { return true; }
	qboolean IsUseable(void) noexcept override
	{
		if (m_iClip <= 0)
		{
			if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] <= 0 && iMaxAmmo1() != -1)
			{
				// clip is empty (or nonexistant) and the player has no more ammo of this type.
				return false;
			}
		}

		return true;
	}
	void ItemPostFrame(void) noexcept override
	{
		int usableButtons = m_pPlayer->pev->button;

		if (!HasSecondaryAttack())
		{
			usableButtons &= ~IN_ATTACK2;
		}

		if (m_flGlock18Shoot != 0)
		{
			FireRemaining(m_iGlock18ShotsFired, m_flGlock18Shoot, true);
		}
		else if (gpGlobals->time > m_flFamasShoot && m_flFamasShoot != 0)
		{
			FireRemaining(m_iFamasShotsFired, m_flFamasShoot, false);
		}

		// Return zoom level back to previous zoom level before we fired a shot.
		// This is used only for the AWP and Scout
		if (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
		{
			if (m_pPlayer->m_bResumeZoom)
			{
				m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;
				m_pPlayer->pev->fov = (float)m_pPlayer->m_iFOV;

				if (m_pPlayer->m_iFOV == m_pPlayer->m_iLastZoom)
				{
					// return the fade level in zoom.
					m_pPlayer->m_bResumeZoom = false;
				}
			}
		}

		if (m_pPlayer->m_flEjectBrass != 0 && m_pPlayer->m_flEjectBrass <= gpGlobals->time)
		{
			m_pPlayer->m_flEjectBrass = 0;
			EjectBrassLate();
		}

		if (!(m_pPlayer->pev->button & IN_ATTACK))
		{
			m_flLastFireTime = 0;
		}

		if (m_pPlayer->HasShield())
		{
			if (m_fInReload && (m_pPlayer->pev->button & IN_ATTACK2))
			{
				SecondaryAttack();
				m_pPlayer->pev->button &= ~IN_ATTACK2;
				m_fInReload = false;
				m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase();
			}
		}

		if (m_fInReload && m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase())
		{
			// complete the reload.
			auto const j = std::min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

			// Add them to the clip
			m_iClip += j;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

			m_pPlayer->TabulateAmmo();
			m_fInReload = false;
		}

		if ((usableButtons & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, UTIL_WeaponTimeBase(), UseDecrement())
			&& !m_pPlayer->m_bIsDefusing // In-line: I think it's fine to block secondary attack, when defusing. It's better then blocking speed resets in weapons.
			)
		{
			if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
			{
				m_fFireOnEmpty = true;
			}

			SecondaryAttack();
			m_pPlayer->pev->button &= ~IN_ATTACK2;
		}
		else if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, UTIL_WeaponTimeBase(), UseDecrement()))
		{
			if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == WEAPON_NOCLIP && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
			{
				m_fFireOnEmpty = true;
			}

			m_pPlayer->TabulateAmmo();

			// Can't shoot during the freeze period
			// Always allow firing in single player
			if ((m_pPlayer->m_bCanShoot && g_pGameRules->IsMultiplayer() && !g_pGameRules->IsFreezePeriod() && !m_pPlayer->m_bIsDefusing) || !g_pGameRules->IsMultiplayer())
			{
				PrimaryAttack();
			}
		}
		else if ((m_pPlayer->pev->button & IN_RELOAD) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		{
			// reload when reload is pressed, or if no buttons are down and weapon is empty.
			if (m_flFamasShoot == 0 && m_flGlock18Shoot == 0)
			{
				if (!(m_iWeaponState & WPNSTATE_SHIELD_DRAWN))
				{
					// reload when reload is pressed, or if no buttons are down and weapon is empty.
					Reload();
				}
			}
		}
		else if (!(usableButtons & (IN_ATTACK | IN_ATTACK2)))
		{
			// no fire buttons down

			// The following code prevents the player from tapping the firebutton repeatedly
			// to simulate full auto and retaining the single shot accuracy of single fire
			if (m_bDelayFire)
			{
				m_bDelayFire = false;

				if (m_iShotsFired > 15)
				{
					m_iShotsFired = 15;
				}

				m_flDecreaseShotsFired = gpGlobals->time + 0.4f;
			}

			m_fFireOnEmpty = false;

			// if it's a pistol then set the shots fired to 0 after the player releases a button
			if (IsSecondaryWeapon(m_iId))
			{
				m_iShotsFired = 0;
			}
			else
			{
				if (m_iShotsFired > 0 && m_flDecreaseShotsFired < gpGlobals->time)
				{
					m_flDecreaseShotsFired = gpGlobals->time + 0.0225f;
					m_iShotsFired--;
				}
			}

			if (!IsUseable() && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
			{
#if 0
				// weapon isn't useable, switch.
				if (!(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(m_pPlayer, this))
				{
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3f;
					return;
				}
#endif
			}
			else
			{
				if (!(m_iWeaponState & WPNSTATE_SHIELD_DRAWN))
				{
					// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
					if (!m_iClip && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
					{
						if (m_flFamasShoot == 0 && m_flGlock18Shoot == 0)
						{
							Reload();
							return;
						}
					}
				}
			}

			WeaponIdle();
			return;
		}

		// catch all
		if (ShouldWeaponIdle())
		{
			WeaponIdle();
		}
	}
	void PrimaryAttack(void) noexcept override {}
	void SecondaryAttack(void) noexcept override {}
	void Reload(void) noexcept override {}
	void WeaponIdle(void) noexcept override {}
	int UpdateClientData(CBasePlayer* pPlayer) noexcept override
	{
		bool bSend = false;
		int state = 0;

		if (pPlayer->m_pActiveItem == this)
		{
			if (pPlayer->m_fOnTarget)
				state = WEAPON_IS_ONTARGET;
			else
				state = 1;
		}

		if (!pPlayer->m_fWeapon)
			bSend = true;

		if (this == pPlayer->m_pActiveItem || this == pPlayer->m_pClientActiveItem)
		{
			if (pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem)
				bSend = true;
		}

		if (m_iClip != m_iClientClip || state != m_iClientWeaponState || pPlayer->m_iFOV != pPlayer->m_iClientFOV)
			bSend = true;

		if (bSend)
		{
			gmsgCurWeapon::Send(pPlayer->edict(), state, m_iId, m_iClip);

			m_iClientClip = m_iClip;
			m_iClientWeaponState = state;
			pPlayer->m_fWeapon = true;
		}

		if (m_pNext)
		{
			m_pNext->UpdateClientData(pPlayer);
		}

		return 1;
	}
	void RetireWeapon(void) noexcept override
	{
		// first, no viewmodel at all.
		m_pPlayer->pev->viewmodel = 0;
		m_pPlayer->pev->weaponmodel = 0;

		g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
	}
	qboolean ShouldWeaponIdle(void) noexcept override { return false; }
	void Holster(int skiplocal = 0) noexcept override
	{
		// cancel any reload in progress.
		m_fInReload = false;
		m_pPlayer->pev->viewmodel = 0;
		m_pPlayer->pev->weaponmodel = 0;
	}
	qboolean UseDecrement(void) noexcept override { return false; }
	CBasePlayerItem* GetWeaponPtr(void) noexcept override { return (CBasePlayerItem*)this; }


public:	// ReGameDLL addition member.
	// hle time creep vars
	float m_flPrevPrimaryAttack{};
	float m_flLastFireTime{};

public:
	// LUNA: Extended Virtual Funcs: Be adviced that original CBaseEntity does not containing these!
	virtual bool ShouldCollide(EHANDLE<CBaseEntity> pOther) noexcept { return true; }

	// Forwarding all arguments to the Create() of that class.
	template <typename T, typename... Tys>
	static __forceinline T* Create(Tys&&... args)
		noexcept (noexcept(T::Create(std::forward<Tys>(args)...)))
		requires (requires{ { T::Create(std::forward<Tys>(args)...) } -> std::same_as<T*>; })
	{
		return T::Create(std::forward<Tys>(args)...);
	}

	TaskScheduler_t m_Scheduler{};
};
