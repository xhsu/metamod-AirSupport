export module Query;

export import <ranges>;

export import eiface;
export import progdefs;
export import util;

export import CBase;

namespace Query
{
	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_players(void) noexcept
	{
		return
			std::views::iota(1, gpGlobals->maxClients + 1) |	// from 1 to 32 actually, iota parsed as [1, 33)
			std::views::transform([](int idx) noexcept { auto const pent = g_engfuncs.pfnPEntityOfEntIndex(idx); return pent ? (CBasePlayer *)pent->pvPrivateData : nullptr; }) |
			std::views::filter([](void *p) noexcept { return p != nullptr; }) |

			// Connected but not necessary alive.
			std::views::filter([](CBasePlayer *pPlayer) noexcept { return !pPlayer->has_disconnected && !(pPlayer->pev->flags & FL_DORMANT); })
			;
	}

	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_living_players(void) noexcept
	{
		return
			std::views::iota(1, gpGlobals->maxClients + 1) |
			std::views::transform([](int idx) noexcept { auto const pent = g_engfuncs.pfnPEntityOfEntIndex(idx); return pent ? (CBasePlayer *)pent->pvPrivateData : nullptr; }) |
			std::views::filter([](void *p) noexcept { return p != nullptr; }) |

			// Only player who is alive, and connected. Disconnected player will be marked as DEAD_DEAD therefore filtered.
			std::views::filter([](CBasePlayer *pPlayer) noexcept { return pPlayer->pev->deadflag == DEAD_NO && pPlayer->pev->takedamage != DAMAGE_NO; })
			;
	}

	// Iterating type: CBaseEntity*
	export inline decltype(auto) all_entities(void) noexcept
	{
		return
			std::views::iota(0, gpGlobals->maxEntities + 1) |
			std::views::transform([](int idx) noexcept { auto const pent = g_engfuncs.pfnPEntityOfEntIndex(idx); return pent ? (CBaseEntity *)pent->pvPrivateData : nullptr; }) |
			std::views::filter([](void *p) noexcept { return p != nullptr; })
			;
	}

	// Iterating type: CBaseEntity*
	export inline decltype(auto) all_nonplayer_entities(void) noexcept
	{
		return
			std::views::iota(33, gpGlobals->maxEntities + 1) |
			std::views::transform([](int idx) noexcept { auto const pent = g_engfuncs.pfnPEntityOfEntIndex(idx); return pent ? (CBaseEntity *)pent->pvPrivateData : nullptr; }) |
			std::views::filter([](void *p) noexcept { return p != nullptr; })
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

	export template <typename T> inline decltype(auto) is(void) noexcept
	{
		return
			std::views::filter([](auto &&ent) noexcept -> bool { return EHANDLE<CBaseEntity>(ent).Is<T>(); });
	}

	export template <typename T> inline decltype(auto) as(void) noexcept
	{
		return
			std::views::transform([](auto &&ent) noexcept -> T* { return EHANDLE<CBaseEntity>(ent).As<T>(); });
	}
}
