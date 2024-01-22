export module Weapon;

export import CBase;
export import PlayerItem;
export import Target;
export import Task.Const;
export import Task;


//export inline constexpr auto RADIO_KEY = 16486345;

export __forceinline uint64_t UTIL_CombineTaskAndPlayerIndex(uint64_t iTask, uint64_t iPlayer) noexcept { return iTask | (1ull << (iPlayer + 32ull)); }

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
	Task		Task_CallAnimation(bool bGotoHolster)					noexcept;
	Task		Task_CallSoundFx(bool bAccepted)						noexcept;
	Task		Task_FixedTargetCalling()								noexcept;

public:
	EHANDLE<CDynamicTarget> m_pTarget{};
	bool m_bCanHolster{ true };
	bool m_bSoundSeqFinished{ true };
	bool m_bHintPressAndHold{};
};

export inline
CRadio* UTIL_CurUsingRadio(EHANDLE<CBasePlayer> player) noexcept
{
	if (!player)
		return nullptr;

	EHANDLE pWeapon{ player->m_pActiveItem };
	return pWeapon.As<CRadio>();
}
