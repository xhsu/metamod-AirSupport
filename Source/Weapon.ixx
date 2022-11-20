export module Weapon;

export import CBase;
export import Task;

export inline constexpr auto RADIO_KEY = 16486345;

export extern "C++" namespace Weapon
{
	extern Task Task_RadioDeploy(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioRejected(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern Task Task_RadioAccepted(EHANDLE<CBasePlayerWeapon> pThis) noexcept;
	extern void OnRadioHolster(CBasePlayerWeapon *pThis) noexcept;
};
