import Menu;
import Missile;

void UTIL_ShowMenu(edict_t *pPlayer, uint16_t bitsValidSlots, std::string szText) noexcept
{
LAB_RESTART:;
	gmsgShowMenu::Send(pPlayer,
		bitsValidSlots,
		-1,
		szText.size() >= 175,
		szText.substr(0, 175).c_str()
	);

	if (szText.size() >= 175)
	{
		szText.erase(0, 175);
		goto LAB_RESTART;
	}
}

void OnMenuSelection(CBasePlayer *pPlayer, int iSlot) noexcept
{
	switch (pPlayer->m_iMenu)
	{
	case Menu_AirSupport:
		if (iSlot >= 1 && iSlot <= 5)
			g_rgiAirSupportSelected[pPlayer->entindex()] = EAirSupportTypes(iSlot - 1);

	default:
		return;
	}
}
