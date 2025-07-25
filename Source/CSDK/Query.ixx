module;

#ifdef __INTELLISENSE__
#include <algorithm>
#include <ranges>
#endif

export module Query;

import std;
import hlsdk;

import CBase;

import <experimental/generator>;	// #MSVC_BUG_GENERATOR

namespace Query
{
	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_players(void) noexcept
	{
		return
			std::span(g_engfuncs.pfnPEntityOfEntIndex(1), gpGlobals->maxClients) // from 1 to 32 actually, iota parsed as [1, 33)
			| std::views::transform([](edict_t &ent) static noexcept { return ent.free ? nullptr : (CBasePlayer *)ent.pvPrivateData; })
			| std::views::filter([](void *p) static noexcept { return p != nullptr; })

			// Connected but not necessary alive.
			| std::views::filter([](CBasePlayer *pPlayer) static noexcept { return !pPlayer->has_disconnected && !(pPlayer->pev->flags & FL_DORMANT); })
			;
	}

	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_living_players(void) noexcept
	{
		return
			std::span(g_engfuncs.pfnPEntityOfEntIndex(1), gpGlobals->maxClients) |
			std::views::transform([](edict_t &ent) static noexcept { return ent.free ? nullptr : (CBasePlayer *)ent.pvPrivateData; }) |
			std::views::filter([](void *p) static noexcept { return p != nullptr; }) |

			// Only player who is alive, and connected. Disconnected player will be marked as DEAD_DEAD therefore filtered.
			std::views::filter([](CBasePlayer *pPlayer) static noexcept { return pPlayer->pev->deadflag == DEAD_NO; })
			;
	}

	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_observers(void) noexcept
	{
		return
			all_players()

			// Observer is depending on pev->iuser1
			| std::views::filter([](CBasePlayer* pPlayer) static noexcept { return pPlayer->pev->iuser1 != OBS_NONE; })
			;
	}

	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_who_is_observing(edict_t* pTarget) noexcept
	{
		return
			all_observers()

			// LUNA: using directly m_pent is because the player entity never actually deallocated during a game.
			| std::views::filter([=](CBasePlayer* pPlayer) noexcept { return pPlayer->m_hObserverTarget && pPlayer->m_hObserverTarget.m_pent == pTarget; })
			;
	}

	// Iterating type: CBaseEntity*
	export inline decltype(auto) all_entities(void) noexcept
	{
		return
			std::span(g_engfuncs.pfnPEntityOfEntIndex(0), gpGlobals->maxEntities) |
			std::views::transform([](edict_t& ent) static noexcept { return ent.free ? nullptr : (CBaseEntity *)ent.pvPrivateData; }) |
			std::views::filter([](void *p) static noexcept { return p != nullptr; })
			;
	}

	// Iterating type: CBaseEntity*
	export inline decltype(auto) all_nonplayer_entities(void) noexcept
	{
		return
			std::span(g_engfuncs.pfnPEntityOfEntIndex(gpGlobals->maxClients + 1), gpGlobals->maxEntities - (gpGlobals->maxClients + 1)) |
			std::views::transform([](edict_t &ent) static noexcept { return ent.free ? nullptr : (CBaseEntity *)ent.pvPrivateData; }) |
			std::views::filter([](void *p) static noexcept { return p != nullptr; })
			;
	}

	// Iterating type: CBaseEntity*
	export inline decltype(auto) all_entities_in_radius(const Vector &vecOrigin, double const flDistance) noexcept
	{
		return
			all_entities() |
			std::views::filter([&, flLenSq = flDistance * flDistance](CBaseEntity *p) noexcept { return (p->Center() - vecOrigin).LengthSquared() < flLenSq; })
			;
	}

	// Iterating type: T*
	export template <typename T> inline decltype(auto) all_instances_of(void) noexcept
	{
		static_assert(requires { { MAKE_STRING(T::CLASSNAME) == string_t{} } -> std::same_as<bool>; }, "Must be local class!");

		return
			std::span(g_engfuncs.pfnPEntityOfEntIndex(gpGlobals->maxClients + 1), gpGlobals->maxEntities - (gpGlobals->maxClients + 1))
			| std::views::filter([istr = MAKE_STRING(T::CLASSNAME)](edict_t& ent) noexcept { return istr == ent.v.classname; })
			| std::views::transform([](edict_t& ent) static noexcept { return ent.free ? nullptr : (T*)ent.pvPrivateData; })
			| std::views::filter([](T* p) static noexcept { return p != nullptr; })
			;
	}

	// Iterating type: CBasePlayerWeapon*
	export std::experimental::generator<CBasePlayerWeapon*> all_weapons_belongs_to(CBasePlayer const* pPlayer) noexcept
	{
		for (auto&& p : pPlayer->m_rgpPlayerItems)
		{
			if (!p || !p->IsWeapon())
				continue;

			co_yield static_cast<CBasePlayerWeapon*>(p);

			for (auto pNext = p->m_pNext; pNext != nullptr; pNext = pNext->m_pNext)
			{
				if (p->IsWeapon())
					co_yield static_cast<CBasePlayerWeapon*>(pNext);
			}
		}
	}

	export template <typename... Tys> inline decltype(auto) is(void) noexcept
	{
		static_assert(sizeof...(Tys) > 0, "Must be at least one type to test!");

		return
			std::views::filter([](auto &&ent) static noexcept -> bool { auto const hdl = EHANDLE<CBaseEntity>(ent); return (... || hdl.Is<Tys>()); });
	}

	export template <typename... Tys> inline decltype(auto) exactly(void) noexcept
	{
		static_assert(sizeof...(Tys) > 0, "Must be at least one type to test!");
		static_assert(requires { { (... || (MAKE_STRING(Tys::CLASSNAME) == std::ptrdiff_t{})) } -> std::same_as<bool>; }, "Must be local class!");

		return
			std::views::filter([](auto&& ent) static noexcept -> bool { auto const pEdict = ent_cast<edict_t*>(ent); return (... || (MAKE_STRING(Tys::CLASSNAME) == pEdict->v.classname)); });
	}

	export template <typename T> inline decltype(auto) as(void) noexcept
	{
		return
			std::views::transform([](auto &&ent) static noexcept -> T* { return EHANDLE<CBaseEntity>(ent).As<T>(); });
	}

	export template <size_t N> inline decltype(auto) with_classname_of(const char (&szClassname)[N]) noexcept
	{
		return
			std::views::filter([&](auto&& ent) noexcept -> bool { auto const pEdict = ent_cast<edict_t*>(ent); return MAKE_STRING(szClassname) == pEdict->v.classname; });
	}
}
