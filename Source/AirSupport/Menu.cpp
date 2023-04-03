import Menu;
import Projectile;
import Target;
import Weapon;

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
		if (iSlot >= 1 && iSlot <= 6)
		{
			g_rgiAirSupportSelected[pPlayer->entindex()] = EAirSupportTypes(iSlot - 1);
			if (auto const &pWeapon = pPlayer->m_pActiveItem; pWeapon && pWeapon->m_iId == WEAPON_KNIFE && pWeapon->pev->weapons == RADIO_KEY)
			{
				if (EHANDLE<CDynamicTarget> pTarget = DYN_TARGET(pWeapon); pTarget)
					pTarget->UpdateEvalMethod();
			}
		}
		break;

	default:
		return;
	}
}
