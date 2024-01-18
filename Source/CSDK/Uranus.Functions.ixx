module;

#ifdef __INTELLISENSE__
#include <tuple>
#endif

export module Uranus:Functions;

#ifndef __INTELLISENSE__
export import <tuple>;
#endif

export import CBase;

export struct uranus_func_collection_t final
{
	std::uintptr_t m_iVersion = 1;

	Vector*	(__fastcall* pfnFireBullets3)	(CBaseEntity* pThis, void* edx, Vector* pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) noexcept = nullptr;
	void	(__fastcall* pfnSetAnimation)	(CBasePlayer* pPlayer, std::intptr_t, PLAYER_ANIM playerAnim) noexcept = nullptr;
};

export inline uranus_func_collection_t gUranusCollection;

// #UPDATE_AT_CPP23 static operator()
export namespace Uranus
{
	namespace BaseEntity
	{
		struct FireBullets3 final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"CBaseEntity::FireBullets3";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x00\x01\x00\x00\x53\x55\x89\x44\x24\x0C\xA1\x2A\x2A\x2A\x2A\x56\x57\x8B\xF9\x8B\x48\x40\x8B\x50\x44"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\xEC\x74\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnFireBullets3;

			inline Vector operator() (CBaseEntity* pThis, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) const noexcept
			{
				Vector ret{};
				pfn(
					pThis, nullptr, &ret,
					vecSrc, vecDirShooting, flSpread, flDistance,
					iPenetration, iBulletType, iDamage, flRangeModifier,
					pevAttacker,
					bPistol,
					shared_rand
				);

				return ret;
			}
		};
	};

	namespace BasePlayer
	{
		struct SetAnimation final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"CBasePlayer::SetAnimation";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x83\xEC\x4C\x53\x55\x8B\x2A\x56\x57\x8B\x4D\x04\x8B\x2A\x2A\x2A\x2A\x2A\x85\xC0"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x54\xA1\x2A\x2A\x2A\x2A\x33\xC4\x89\x44\x24\x50"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnSetAnimation;

			inline void operator() (CBasePlayer* pPlayer, PLAYER_ANIM playerAnim) const noexcept
			{
				pfn(pPlayer, 0, playerAnim);
			}
		};
	};
}
