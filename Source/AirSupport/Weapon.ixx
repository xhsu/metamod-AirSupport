export module Weapon;

export import CBase;
export import Task;

export inline constexpr auto RADIO_KEY = 16486345;

export enum ERadioPlayerItemTasks : uint64_t
{
	TASK_RADIO_DEPLOY = (1 << 16),
	TASK_RADIO_REJECTED = (1 << 17),
	TASK_RADIO_ACCEPTED = (1 << 18),
};

export __forceinline uint64_t UTIL_CombineTaskAndPlayerIndex(uint64_t iTask, uint64_t iPlayer) noexcept { return iTask | (1ull << (iPlayer + 32ull)); }

export extern "C++" namespace Weapon
{
	extern Task Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept;
};

export __forceinline edict_t *&DYN_TARGET(CBasePlayerItem *pThis) noexcept { return pThis->pev->euser1; }

export inline bool g_bIsSomeoneShooting = false;
