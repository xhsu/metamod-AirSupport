module;

#ifdef __INTELLISENSE__
#include <tuple>
#endif

export module Uranus:Functions;

#ifndef __INTELLISENSE__
export import <tuple>;
#endif

export import sizebuf;

export import CBase;

export struct uranus_func_collection_t final
{
	std::uintptr_t m_iVersion = 5;

	CBaseEntity*	(__cdecl*		pfnCreate)					(const char* pszName, Vector const& vecOrigin, Angles const& vecAngles, edict_t* pentOwner) noexcept = nullptr;
	Vector*			(__fastcall*	pfnFireBullets3)			(CBaseEntity* pThis, void* edx, Vector* pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) noexcept = nullptr;
	void			(__thiscall*	pfnSUB_UseTargets)			(CBaseDelay* pObject, CBaseEntity* pActivator, USE_TYPE useType, float value) noexcept = nullptr;
	qboolean		(__thiscall*	pfnDefaultDeploy)			(CBasePlayerWeapon* pWeapon, const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, qboolean skiplocal) noexcept = nullptr;
	void			(__fastcall*	pfnSetAnimation)			(CBasePlayer* pPlayer, std::intptr_t, PLAYER_ANIM playerAnim) noexcept = nullptr;

	void			(__cdecl*		pfnEmptyEntityHashTable)	(void) noexcept = nullptr;
	void			(__cdecl*		pfnAddEntityHashValue)		(entvars_t* pev, const char* pszClassname, int32_t) noexcept = nullptr;
	void			(__cdecl*		pfnRemoveEntityHashValue)	(entvars_t* pev, const char* pszClassname, int32_t) noexcept = nullptr;

	void			(__cdecl*		pfnClearMultiDamage)		(void) noexcept = nullptr;
	void			(__cdecl*		pfnApplyMultiDamage)		(entvars_t* pevInflictor, entvars_t* pevAttacker) noexcept = nullptr;
	void			(__cdecl*		pfnAddMultiDamage)			(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType) noexcept = nullptr;

	float			(__cdecl*		pfnTEXTURETYPE_PlaySound)	(TraceResult* ptr, Vector vecSrc, Vector vecEnd, int iBulletType) noexcept = nullptr;

	void			(__cdecl*		pfnW_Precache)				(void) noexcept = nullptr;
	void			(__cdecl*		pfnUTIL_PrecacheOther)		(const char* szClassname) noexcept = nullptr;
	void			(__cdecl*		pfnUTIL_PrecacheOtherWeapon)(const char* szClassname) noexcept = nullptr;

	// hw.dll

	void			(__cdecl*		pfnSys_Error)				(const char* Format, ...) noexcept = nullptr;	// [[noreturn]]
	void*			(__cdecl*		pfnSZ_GetSpace)				(sizebuf_t* buf, uint32_t length) noexcept = nullptr;
};

export inline uranus_func_collection_t gUranusCollection;

// #UPDATE_AT_CPP23 static operator()
export namespace Uranus
{
	struct EmptyEntityHashTable final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::EmptyEntityHashTable";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x8E\x2A\x2A\x2A\x2A\x53\x57\x8B\x3D\x2A\x2A\x2A\x2A\x33\xDB\x66\x0F\x1F\x44\x00"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnEmptyEntityHashTable;

		inline void operator() (void) const noexcept
		{
			return pfn();
		}
	};

	struct AddEntityHashValue final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::AddEntityHashValue";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x51\x83\x7D\x10\x00\x0F\x85\x2A\x2A\x2A\x2A\x53\x8B\x5D\x08\x83\x3B\x00\x0F\x84\x2A\x2A\x2A\x2A\xA1"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnAddEntityHashValue;

		inline void operator() (entvars_t* pev, const char* pszClassname) const noexcept
		{
			return pfn(pev, pszClassname, 0);
		}
	};

	struct RemoveEntityHashValue final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::RemoveEntityHashValue";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x8B\x4D\x0C\x33\xD2\x56\x57\x8A\x01\x84\xC0\x74\x18\x0F\xBE\xF0\x03\xD2\x2C\x41\x3C\x19\x77\x03\x83\xC2\x20\x8A\x41\x01"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnRemoveEntityHashValue;

		inline void operator() (entvars_t* pev, const char* pszClassname) const noexcept
		{
			return pfn(pev, pszClassname, 0);
		}
	};

	struct ClearMultiDamage final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::ClearMultiDamage";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x33\xC0\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xC3\x90"),	// NEW
			std::cref("\xC3\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC7\x05\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\xC3\xCC"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnClearMultiDamage;

		inline void operator() (void) const noexcept
		{
			return pfn();
		}
	};

	struct ApplyMultiDamage final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::ApplyMultiDamage";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x8B\x0D\x2A\x2A\x2A\x2A\x85\xC9\x74\x2A\x8B\x15\x2A\x2A\x2A\x2A\x8B\x01\x52\x8B\x15\x2A\x2A\x2A\x2A\x52\x8B\x54\x24\x10\x52\x8B\x54\x24\x10"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x8B\x0D\x2A\x2A\x2A\x2A\x85\xC9\x74\x1F\xFF\x35\x2A\x2A\x2A\x2A\xF3\x0F\x10\x05"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnApplyMultiDamage;

		inline void operator() (entvars_t* pevInflictor, entvars_t* pevAttacker) const noexcept
		{
			return pfn(pevInflictor, pevAttacker);
		}
	};

	struct AddMultiDamage final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::AddMultiDamage";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x56\x8B\x74\x24\x0C\x85\xF6\x74\x2A\xA1\x2A\x2A\x2A\x2A\x8B\x4C\x24\x14"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x56\x8B\x75\x0C\x85\xF6\x74\x6A\xA1\x2A\x2A\x2A\x2A\x0B\x45"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnAddMultiDamage;

		inline void operator() (entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType) const noexcept
		{
			return pfn(pevInflictor, pEntity, flDamage, bitsDamageType);
		}
	};

	struct TEXTURETYPE_PlaySound final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::TEXTURETYPE_PlaySound";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x33\xC5\x89\x45\xFC\x8B\x0D"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnTEXTURETYPE_PlaySound;

		inline auto operator() (TraceResult* ptr, Vector vecSrc, Vector vecEnd, int iBulletType) const noexcept
		{
			return pfn(ptr, vecSrc, vecEnd, iBulletType);
		}
	};

	struct W_Precache final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::W_Precache";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x68\x2A\x2A\x2A\x2A\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xC7\x05"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnW_Precache;

		inline void operator() (void) const noexcept
		{
			return pfn();
		}
	};

	struct UTIL_PrecacheOther final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::UTIL_PrecacheOther";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x8B\x4D\x08\x56\x2B\x88\x2A\x2A\x2A\x2A\x51\xE8\x2A\x2A\x2A\x2A\x8B"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnUTIL_PrecacheOther;

		inline void operator() (const char* szClassname) const noexcept
		{
			return pfn(szClassname);
		}
	};

	struct UTIL_PrecacheOtherWeapon final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::UTIL_PrecacheOtherWeapon";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x83\xEC\x2C\x8B\x4D\x08\x2B\x88\x2A\x2A\x2A\x2A\x56"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnUTIL_PrecacheOtherWeapon;

		inline void operator() (const char* szClassname) const noexcept
		{
			return pfn(szClassname);
		}
	};

	namespace BaseEntity
	{
		struct Create final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"CBaseEntity::Create";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x83\xEC\x0C\x56\x8B\x75\x08\x2B\xB0\x2A\x2A\x2A\x2A\x57\x56\xFF\x15\x2A\x2A\x2A\x2A\x8B\xF8\x83\xC4\x04"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnCreate;

			inline CBaseEntity* operator() (const char* pszName, Vector const& vecOrigin, Angles const& vecAngles, edict_t* pentOwner) const noexcept
			{
				return pfn(pszName, vecOrigin, vecAngles, pentOwner);
			}
		};

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

	namespace BaseDelay
	{
		struct SUB_UseTargets final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"CBaseDelay::SUB_UseTargets";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\xCC\x55\x8B\xEC\x57\x8B\xF9\x8B\x4F\x04\x83\xB9\x2A\x2A\x2A\x2A\x2A\x75\x0D\x83\xBF\x2A\x2A\x2A\x2A\x2A\x0F\x84\x2A\x2A\x2A\x2A\xF3\x0F\x10\x8F"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnSUB_UseTargets;

			inline void operator() (CBaseDelay* pObject, CBaseEntity* pActivator, USE_TYPE useType, float value) const noexcept
			{
				return pfn(pObject, pActivator, useType, value);
			}
		};
	};

	namespace BaseWeapon
	{
		struct DefaultDeploy final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"CBasePlayerWeapon::DefaultDeploy";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x56\x8B\xF1\x8B\x06\xFF\x90\xF8\x00\x00\x00\x85\xC0\x75\x2A\x5E"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x56\x8B\xF1\x8B\x06\xFF\x90\x2A\x2A\x2A\x2A\x85\xC0"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnDefaultDeploy;

			inline auto operator() (CBasePlayerWeapon* pWeapon, const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, bool skiplocal) const noexcept
			{
				return pfn(pWeapon, szViewModel, szWeaponModel, iAnim, szAnimExt, skiplocal);
			}
		};
	}

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

export namespace HW
{
	struct Sys_Error final
	{
		static inline constexpr char MODULE[] = "hw.dll";
		static inline constexpr char NAME[] = u8"::Sys_Error";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x33\xC5\x89\x45\xFC\x56\x8D\x45\x0C\x50\x6A\x00\xFF\x75\x08\x8D\x85"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnSys_Error;
	};

	struct SZ_GetSpace final
	{
		static inline constexpr char MODULE[] = "hw.dll";
		static inline constexpr char NAME[] = u8"::SZ_GetSpace";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x53\x8B\x5D\x0C\x56\x8B\x75\x08\x8B\x4E\x10\x8B\x56\x0C"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnSZ_GetSpace;
	};
}
