export module Query;

export import <ranges>;

export import eiface;
export import progdefs;
export import util;

export import CBase;

import UtlFunction;

namespace GoldSrc::Impl
{
#pragma region as
	template <typename T>
	inline constexpr auto _impl_as = LambdaSet
	{
		[](int iEntIndex) noexcept { return (T *)g_engfuncs.pfnPEntityOfEntIndex(iEntIndex)->pvPrivateData; },
		[](edict_t *pEdict) noexcept { return (T *)pEdict->pvPrivateData; },
		[](entvars_t *pev) noexcept { return (T *)pev->pContainingEntity->pvPrivateData; },
		[](CBaseEntity *pEntity) noexcept { return dynamic_cast<T *>(pEntity); },
	};

	template <>
	inline constexpr auto _impl_as<edict_t *> = LambdaSet
	{
		[](int iEntIndex) noexcept { return g_engfuncs.pfnPEntityOfEntIndex(iEntIndex); },
		[](edict_t *pEdict) noexcept { return pEdict; },
		[](entvars_t *pev) noexcept { return pev->pContainingEntity; },
		[](CBaseEntity *pEntity) noexcept { return pEntity->edict(); },
	};

	template <>
	inline constexpr auto _impl_as<entvars_t *> = LambdaSet
	{
		[](int iEntIndex) noexcept { return &g_engfuncs.pfnPEntityOfEntIndex(iEntIndex)->v; },
		[](edict_t *pEdict) noexcept { return &pEdict->v; },
		[](entvars_t *pev) noexcept { return pev; },
		[](CBaseEntity *pEntity) noexcept { return pEntity->pev; },
	};

	template <std::integral T>
	inline constexpr auto _impl_as<T> = LambdaSet
	{
		[](std::integral auto iEntIndex) noexcept { return static_cast<T>(iEntIndex); },
		[](edict_t *pEdict) noexcept { return static_cast<T>(g_engfuncs.pfnIndexOfEdict(pEdict)); },
		[](entvars_t *pev) noexcept { return static_cast<T>(g_engfuncs.pfnIndexOfEdict(pev->pContainingEntity)); },
		[](CBaseEntity *pEntity) noexcept { return static_cast<T>(pEntity->entindex()); },
	};
#pragma endregion

#pragma region skip_disconnected
	inline constexpr auto _impl_is_user_connected = LambdaSet
	{
		[](int iEntIndex) noexcept { auto const pEntity = (CBaseEntity *)ent_cast<edict_t *>(iEntIndex)->pvPrivateData; return !pEntity->has_disconnected && !(pEntity->pev->flags & FL_DORMANT); },
		[](edict_t *pEdict) noexcept { auto const pEntity = (CBaseEntity *)pEdict->pvPrivateData; return !pEntity->has_disconnected && !(pEntity->pev->flags & FL_DORMANT); },
		[](entvars_t *pev) noexcept { auto const pEntity = (CBaseEntity *)ent_cast<edict_t *>(pev)->pvPrivateData; return !pEntity->has_disconnected && !(pEntity->pev->flags & FL_DORMANT); },
		[](CBaseEntity *pEntity) noexcept { return !pEntity->has_disconnected && !(pEntity->pev->flags & FL_DORMANT); },
	};
#pragma endregion
};

namespace GoldSrc
{
	export inline auto all_players(void) noexcept
	{
		return std::views::iota(1, gpGlobals->maxClients + 1);	// from 1 to 32 actually, iota parsed as [1, 33)
	}

	export inline auto all_entities(void) noexcept
	{
		return std::views::iota(1, gpGlobals->maxEntities + 1);
	}

	export inline constexpr auto skip_disconnected = std::views::filter(Impl::_impl_is_user_connected);
	
	export inline constexpr auto skip_dead = std::views::filter([](CBaseEntity *pEntity) noexcept { return pEntity->IsAlive(); });

	export inline constexpr auto skip_invincible = std::views::filter([](CBaseEntity *pEntity) noexcept { return pEntity->pev->takedamage != DAMAGE_NO; });

	export inline constexpr auto skip_invalid = std::views::filter([](auto &&e) noexcept { return pev_valid(e) == 2; });

	export template <typename T> inline constexpr auto as = std::views::transform(Impl::_impl_as<T>);
};

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

	// Iterating type: CBaseEntity*
	export inline decltype(auto) all_entities(void) noexcept
	{
		return
			std::views::iota(0, gpGlobals->maxEntities + 1) |
			std::views::transform([](int idx) noexcept { auto const pent = g_engfuncs.pfnPEntityOfEntIndex(idx); return pent ? (CBaseEntity *)pent->pvPrivateData : nullptr; }) |
			std::views::filter([](void *p) noexcept { return p != nullptr; })
			;
	}

	// Iterating type: CBasePlayer*
	export inline decltype(auto) all_alive_player(void) noexcept
	{
		return
			std::views::iota(1, gpGlobals->maxClients + 1) |
			std::views::transform([](int idx) noexcept { auto const pent = g_engfuncs.pfnPEntityOfEntIndex(idx); return pent ? (CBasePlayer *)pent->pvPrivateData : nullptr; }) |
			std::views::filter([](void *p) noexcept { return p != nullptr; }) |

			// Only player who is alive. The pev->deadflag can filter disconnected player as well.
			std::views::filter([](CBasePlayer *pPlayer) noexcept { return pPlayer->pev->deadflag == DEAD_NO && pPlayer->pev->takedamage != DAMAGE_NO; })
			;
	}
}
