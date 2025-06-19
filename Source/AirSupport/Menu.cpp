import std;
import hlsdk;

import Menu;
import Message;
import Target;
import Weapon;


using std::uint16_t;


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

			if (auto const pRadio = UTIL_CurUsingRadio(pPlayer); pRadio != nullptr)
			{
				if (pRadio->m_pTarget)
					pRadio->m_pTarget->UpdateEvalMethod();
			}
		}
		break;

	default:
		return;
	}
}
