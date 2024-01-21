export module Weapon;

export import CBase;
export import PlayerItem;
export import Target;
export import Task.Const;
export import Task;


//export inline constexpr auto RADIO_KEY = 16486345;

export __forceinline uint64_t UTIL_CombineTaskAndPlayerIndex(uint64_t iTask, uint64_t iPlayer) noexcept { return iTask | (1ull << (iPlayer + 32ull)); }

//export extern "C++" namespace Weapon
//{
//	extern Task Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
//	extern Task Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
//	extern Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
//	extern void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept;
//};
//
//export __forceinline edict_t *&DYN_TARGET(CBasePlayerItem *pThis) noexcept { return pThis->pev->euser1; }

export inline bool g_bIsSomeoneShooting = false;

export struct CRadio : CPrefabWeapon
{
public:
	static inline constexpr char CLASSNAME[] = "weapon_radio";

public:
	void		Spawn()					noexcept override;
//	void		Precache()				noexcept override;
	qboolean	GetItemInfo(ItemInfo*)	noexcept override;

	qboolean	Deploy()				noexcept override;
	void		ItemPostFrame()			noexcept override;
	void		Holster(int)			noexcept override;

	qboolean	CanDrop()				noexcept override { return false; }
	qboolean	CanHolster()			noexcept override { return m_bCanHolster; }
	float		GetMaxSpeed()			noexcept override { return 250.f; }
	int			iItemSlot()				noexcept override { return 5; }
	qboolean	UseDecrement()			noexcept override { return true; }

public:
	Task		Task_Deploy()											noexcept;
	Task		Task_RadioRejected()									noexcept;
	Task		Task_RadioAccepted()									noexcept;
	Task		Task_AcceptedAnim()										noexcept;
	Task		Task_AcceptedSound(EHANDLE<CFixedTarget> pFixedTarget)	noexcept;

public:
	EHANDLE<CDynamicTarget> m_pTarget{};
	bool m_bCanHolster{ true };
	bool m_bTargetActivated{};
};

export inline
CRadio* UTIL_CurUsingRadio(EHANDLE<CBasePlayer> player) noexcept
{
	if (!player)
		return nullptr;

	EHANDLE pWeapon{ player->m_pActiveItem };
	return pWeapon.As<CRadio>();
}
