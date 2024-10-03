export module Weapons:Pistol;

export import PlayerItem;

class CG18C : public CPrefabWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo* p);
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return m_fMaxSpeed; }
	int iItemSlot(void) { return WPNSLOT_SECONDARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void GLOCK18Fire(float flSpread, float flCycleTime, BOOL fUseBurstMode);

private:
	int m_iShell;
	bool m_bBurstFire;
};
