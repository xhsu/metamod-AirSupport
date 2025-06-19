export module Uranus:Functions;

import std;
import hlsdk;

import CBase;

using std::uint32_t;

export using PFN_ENTITYINIT = void (*)(entvars_t* pev) noexcept;

export struct uranus_func_collection_t final
{
	std::uintptr_t m_iVersion = 2024'10'10;

	CBaseEntity*	(__cdecl*		pfnCreate)					(const char* pszName, Vector const& vecOrigin, Angles const& vecAngles, edict_t* pentOwner) noexcept = nullptr;
	void			(__fastcall*	pfnFireBullets)				(CBaseEntity* pThis, void* edx, unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, EBulletTypes iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker) noexcept = nullptr;
	Vector*			(__fastcall*	pfnFireBullets3)			(CBaseEntity* pThis, void* edx, Vector* pret, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) noexcept = nullptr;
	void			(__thiscall*	pfnSUB_UseTargets)			(CBaseDelay* pObject, CBaseEntity* pActivator, USE_TYPE useType, float value) noexcept = nullptr;
	qboolean		(__fastcall*	pfnDefaultDeploy)			(CBasePlayerWeapon* pWeapon, std::uintptr_t, const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, qboolean skiplocal) noexcept = nullptr;
	bool			(__thiscall*	pfnHintMessage)				(CBasePlayer* pPlayer, const char* pMessage, qboolean bDisplayIfDead, qboolean bOverrideClientSettings) noexcept = nullptr;
	void			(__fastcall*	pfnSetAnimation)			(CBasePlayer* pPlayer, std::intptr_t, PLAYER_ANIM playerAnim) noexcept = nullptr;
	void			(__thiscall*	pfnDropShield)				(CBasePlayer* pPlayer, bool bCallDeploy) noexcept = nullptr;
	bool			(__thiscall*	pfnCanPlayerBuy)			(CBasePlayer* pPlayer, bool bShowMessage) noexcept = nullptr;
	void			(__thiscall*	pfnAddAccount)				(CBasePlayer* pPlayer, int32_t iAmount, bool bTrackChange) noexcept = nullptr;
	void			(__thiscall*	pfnSelectItem)				(CBasePlayer* pPlayer, const char* pszItemName) noexcept = nullptr;
	qboolean		(__fastcall*	pfnSwitchWeapon)			(CBasePlayer* pPlayer, std::uintptr_t, CBasePlayerItem* pWeapon) noexcept = nullptr;
	void			(__fastcall*	pfnDropPlayerItem)			(CBasePlayer* pPlayer, std::uintptr_t, char const* pszItemName) noexcept = nullptr;

	void			(__cdecl*		pfnEmptyEntityHashTable)	(void) noexcept = nullptr;
	void			(__cdecl*		pfnAddEntityHashValue)		(entvars_t* pev, const char* pszClassname, int32_t) noexcept = nullptr;
	void			(__cdecl*		pfnRemoveEntityHashValue)	(entvars_t* pev, const char* pszClassname, int32_t) noexcept = nullptr;

	void			(__cdecl*		pfnClearMultiDamage)		(void) noexcept = nullptr;
	void			(__cdecl*		pfnApplyMultiDamage)		(entvars_t* pevInflictor, entvars_t* pevAttacker) noexcept = nullptr;
	void			(__cdecl*		pfnAddMultiDamage)			(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType) noexcept = nullptr;

	float			(__cdecl*		pfnTEXTURETYPE_PlaySound)	(TraceResult* ptr, Vector vecSrc, Vector vecEnd, int iBulletType) noexcept = nullptr;
	char			(__cdecl*		pfnUTIL_TextureHit)			(TraceResult* ptr, Vector vecSrc, Vector vecEnd) noexcept = nullptr;

	float			(__cdecl*		pfnUTIL_SharedRandomFloat)	(uint32_t seed, float low, float high) noexcept = nullptr;

	void			(__cdecl*		pfnW_Precache)				(void) noexcept = nullptr;
	void			(__cdecl*		pfnUTIL_PrecacheOther)		(const char* szClassname) noexcept = nullptr;
	void			(__cdecl*		pfnUTIL_PrecacheOtherWeapon)(const char* szClassname) noexcept = nullptr;
	void			(__cdecl*		pfnWriteSigonMessages)		(void) noexcept = nullptr;
	void			(__cdecl*		pfnCheckStartMoney)			(void) noexcept = nullptr;
	void			(__cdecl*		pfnRadiusFlash)				(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage) noexcept = nullptr;

	// hw.dll

	void			(__cdecl*		pfnSys_Error)				(const char* Format, ...) noexcept = nullptr;	// [[noreturn]]
	void*			(__cdecl*		pfnSZ_GetSpace)				(sizebuf_t* buf, uint32_t length) noexcept = nullptr;
	PFN_ENTITYINIT	(__cdecl*		pfnGetDispatch)				(char const* pszClassName) noexcept = nullptr;
};

export inline uranus_func_collection_t gUranusCollection;

export namespace Uranus
{
	struct EmptyEntityHashTable final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::EmptyEntityHashTable";
		static inline constexpr std::tuple PATTERNS
		{
			// 3266 __stdcall??? 90 A1 ? ? ? ? 53 55 33 DB 33 ED 3B C3 7E 54 56 57 33 FF A1
			std::cref("\x90\xA1\x2A\x2A\x2A\x2A\x53\x55\x33\xDB\x33\xED\x3B\xC3\x7E\x54\x56\x57\x33\xFF\xA1"),	// NEW
			std::cref("\xCC\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x8E\x2A\x2A\x2A\x2A\x53\x57\x8B\x3D\x2A\x2A\x2A\x2A\x33\xDB\x66\x0F\x1F\x44\x00"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnEmptyEntityHashTable;

		static inline void operator() (void) noexcept
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
			// 3266 90 8B 44 24 0C 83 EC 08 85 C0 53 55 56 57 0F 85 ? ? ? ? 8B
			std::cref("\x90\x8B\x44\x24\x0C\x83\xEC\x08\x85\xC0\x53\x55\x56\x57\x0F\x85\x2A\x2A\x2A\x2A\x8B"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x51\x83\x7D\x10\x00\x0F\x85\x2A\x2A\x2A\x2A\x53\x8B\x5D\x08\x83\x3B\x00\x0F\x84\x2A\x2A\x2A\x2A\xA1"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnAddEntityHashValue;

		static inline void operator() (entvars_t* pev, const char* pszClassname) noexcept
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
			// 3266 90 8B 4C 24 08 53 8B D1 55 8A 09 33 C0 56 8B 35 ? ? ? ? 84
			std::cref("\x90\x8B\x4C\x24\x08\x53\x8B\xD1\x55\x8A\x09\x33\xC0\x56\x8B\x35\x2A\x2A\x2A\x2A\x84"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x8B\x4D\x0C\x33\xD2\x56\x57\x8A\x01\x84\xC0\x74\x18\x0F\xBE\xF0\x03\xD2\x2C\x41\x3C\x19\x77\x03\x83\xC2\x20\x8A\x41\x01"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnRemoveEntityHashValue;

		static inline void operator() (entvars_t* pev, const char* pszClassname) noexcept
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

		static inline void operator() (void) noexcept
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

		static inline void operator() (entvars_t* pevInflictor, entvars_t* pevAttacker) noexcept
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

		static inline void operator() (entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType) noexcept
		{
			return pfn(pevInflictor, pEntity, flDamage, bitsDamageType);
		}
	};

	struct UTIL_TextureHit final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::UTIL_TextureHit";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x83\xEC\x5C\xA1****\x33\xC5\x89\x45\xFC\x8B\x45\x08\x56\x57\x8B\x40\x30\x85\xC0\x0F\x85"),	// 9980
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnUTIL_TextureHit;

		static inline auto operator() (TraceResult* ptr, Vector const& vecSrc, Vector const& vecEnd) noexcept
		{
			return pfn(ptr, vecSrc, vecEnd);
		}
	};

	struct TEXTURETYPE_PlaySound final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::TEXTURETYPE_PlaySound";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x83\xEC\x78\x8B\x0D\x2A\x2A\x2A\x2A\x53\x55\x56\x8B\x01\x57\xC7\x44\x24"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x33\xC5\x89\x45\xFC\x8B\x0D"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnTEXTURETYPE_PlaySound;

		static inline auto operator() (TraceResult* ptr, Vector const& vecSrc, Vector const& vecEnd, int iBulletType) noexcept
		{
			return pfn(ptr, vecSrc, vecEnd, iBulletType);
		}
	};

	struct UTIL_SharedRandomFloat final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::UTIL_SharedRandomFloat";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\xCC\x55\x8B\xEC\x8B\x45\x10\x03\x45\x0C\x03\x45\x08\x0F\xB6\xC0\xF3\x0F\x10\x45"),	// 9980
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnUTIL_SharedRandomFloat;

		static inline auto operator() (uint32_t seed, float low, float high) noexcept
		{
			return pfn(seed, low, high);
		}
	};

	struct W_Precache final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::W_Precache";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x57\xB9\x2A\x2A\x2A\x2A\x33\xC0\xBF\x2A\x2A\x2A\x2A\xF3\xAB\xB9\x2A\x2A\x2A\x2A\xBF\x2A\x2A\x2A\x2A\xF3\xAB\x68"),	// NEW
			std::cref("\xCC\x68\x2A\x2A\x2A\x2A\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x6A\x00\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xC7\x05"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnW_Precache;

		static inline void operator() (void) noexcept
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
			std::cref("\x90\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x44\x24\x04\x56\x2B\x81\x2A\x2A\x2A\x2A\x50\xE8\x2A\x2A\x2A\x2A\x8B"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x8B\x4D\x08\x56\x2B\x88\x2A\x2A\x2A\x2A\x51\xE8\x2A\x2A\x2A\x2A\x8B"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnUTIL_PrecacheOther;

		static inline void operator() (const char* szClassname) noexcept
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
			std::cref("\x90\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x44\x24\x04\x83\xEC\x2C\x53\x56\x2B\x81\x2A\x2A\x2A\x2A\x50"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x83\xEC\x2C\x8B\x4D\x08\x2B\x88\x2A\x2A\x2A\x2A\x56"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnUTIL_PrecacheOtherWeapon;

		static inline void operator() (const char* szClassname) noexcept
		{
			return pfn(szClassname);
		}
	};

	struct WriteSigonMessages final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::WriteSigonMessages";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x53\x56\x57\xBF****\x8B\x47\x08\x85\xC0\x0F\x84****\x8B\x37\x85\xF6\x75\x05\xBE"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x51\x53\x8B\x1D\x2A\x2A\x2A\x2A\x56\x57\xBF\x2A\x2A\x2A\x2A\x83\x7F\x10\x00"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnWriteSigonMessages;

		static inline void operator() (void) noexcept
		{
			return pfn();
		}
	};

	struct CheckStartMoney final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::CheckStartMoney";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\xD9\x05****\xE8****\x3D\x80\x3E\x00\x00\x7E\x14\x68****\x68****\xFF\x15"),	// NEW
			std::cref("\xCC\xF3\x0F\x2C\x05\x2A\x2A\x2A\x2A\x3D\x2A\x2A\x2A\x2A\x7E\x17\x51"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnCheckStartMoney;

		static inline void operator() (void) noexcept
		{
			return pfn();
		}
	};

	struct RadiusFlash final
	{
		static inline constexpr char MODULE[] = "mp.dll";
		static inline constexpr char NAME[] = u8"::RadiusFlash";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\x81\xEC\x2A\x2A\x2A\x2A\xD9\x84\x24\xE4\x00\x00\x00\xD8\x0D\x2A\x2A\x2A\x2A\x8D"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x83\xE4\xF8\x81\xEC\x2A\x2A\x2A\x2A\xF3\x0F\x10\x45\x2A\x8D\x45\x08"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnRadiusFlash;

		static inline auto operator()(Vector const& vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage) noexcept
		{
			return pfn(vecSrc, pevInflictor, pevAttacker, flDamage);
		}
	};

	namespace BaseEntity
	{
		struct Create final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBaseEntity::Create";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x00\xA1\x2A\x2A\x2A\x2A\x56\x8B\x74\x24\x08\x57\x2B\xB0\x2A\x2A\x2A\x2A\x56\xFF\x15"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x83\xEC\x0C\x56\x8B\x75\x08\x2B\xB0\x2A\x2A\x2A\x2A\x57\x56\xFF\x15\x2A\x2A\x2A\x2A\x8B\xF8\x83\xC4\x04"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnCreate;

			static inline auto operator() (const char* pszName, Vector const& vecOrigin, Angles const& vecAngles, edict_t* pentOwner) noexcept
			{
				return pfn(pszName, vecOrigin, vecAngles, pentOwner);
			}
		};

		struct FireBullets final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBaseEntity::FireBullets";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x53\x55\x8B\xE9\x8B\x48\x40\x8B\x50\x44\x89\x8C\x24\x8C\x00\x00\x00\x8B\x48\x48\x89\x94\x24\x90\x00\x00\x00"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\xEC\x78\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnFireBullets;

			static inline auto operator() (CBaseEntity* pThis, unsigned long cShots, Vector const& vecSrc, Vector const& vecDirShooting, Vector const& vecSpread, float flDistance, EBulletTypes iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker) noexcept
			{
				return pfn(
					pThis, nullptr,
					cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage,
					pevAttacker
				);
			}
		};

		struct FireBullets3 final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBaseEntity::FireBullets3";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x00\x01\x00\x00\x53\x55\x89\x44\x24\x0C\xA1\x2A\x2A\x2A\x2A\x56\x57\x8B\xF9\x8B\x48\x40\x8B\x50\x44"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\xEC\x74\xA1\x2A\x2A\x2A\x2A\x53\x56\x57\xF3\x0F\x10\x40\x2A\x8B"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnFireBullets3;

			static inline Vector operator() (CBaseEntity* pThis, Vector const& vecSrc, Vector const& vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, qboolean bPistol, int shared_rand) noexcept
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
			static inline constexpr char NAME[] = u8"::CBaseDelay::SUB_UseTargets";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x53\x55\x56\x57\x8B\xF9\x33\xED\x8B\x47\x04\x39\xA8\x2A\x2A\x2A\x2A\x75\x0C\x39\xAF"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x57\x8B\xF9\x8B\x4F\x04\x83\xB9\x2A\x2A\x2A\x2A\x2A\x75\x0D\x83\xBF\x2A\x2A\x2A\x2A\x2A\x0F\x84\x2A\x2A\x2A\x2A\xF3\x0F\x10\x8F"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnSUB_UseTargets;

			static inline void operator() (CBaseDelay* pObject, CBaseEntity* pActivator, USE_TYPE useType, float value) noexcept
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
			static inline constexpr char NAME[] = u8"::CBasePlayerWeapon::DefaultDeploy";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x56\x8B\xF1\x8B\x06\xFF\x90\xF8\x00\x00\x00\x85\xC0\x75\x2A\x5E"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x56\x8B\xF1\x8B\x06\xFF\x90\x2A\x2A\x2A\x2A\x85\xC0"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnDefaultDeploy;

			static inline auto operator() (CBasePlayerWeapon* pWeapon, const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, bool skiplocal) noexcept
			{
				return pfn(pWeapon, 0, szViewModel, szWeaponModel, iAnim, szAnimExt, skiplocal);
			}
		};
	}

	namespace BasePlayer
	{
		struct HintMessage final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::HintMessage";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x8B\x44\x24\x08\x56\x85\xC0\x8B\xF1\x75\x12\x8B\x06\xFF\x90\x2A\x2A\x2A\x2A\x85"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\x7D\x0C\x00\x56\x8B\xF1\x75\x13\x8B\x06\xFF\x90"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnHintMessage;

			static inline bool operator() (CBasePlayer* pPlayer, const char* pMessage, qboolean bDisplayIfDead, qboolean bOverrideClientSettings) noexcept
			{
				return pfn(pPlayer, pMessage, bDisplayIfDead, bOverrideClientSettings);
			}
		};

		struct SetAnimation final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::SetAnimation";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x83\xEC\x4C\x53\x55\x8B\x2A\x56\x57\x8B\x4D\x04\x8B\x2A\x2A\x2A\x2A\x2A\x85\xC0"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x54\xA1\x2A\x2A\x2A\x2A\x33\xC4\x89\x44\x24\x50"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnSetAnimation;

			static inline void operator() (CBasePlayer* pPlayer, PLAYER_ANIM playerAnim) noexcept
			{
				pfn(pPlayer, 0, playerAnim);
			}
		};

		struct DropShield final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::DropShield";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x83\xEC\x18\x53\x56\x8B\xF1\x33\xDB\x38\x9E\x2A\x2A\x2A\x2A\x0F\x84"),	// LEGACY
				std::cref("\xCC\x55\x8B\xEC\x83\xEC\x18\x57\x8B\xF9\x80\xBF\x2A\x2A\x2A\x2A\x2A\x0F\x84"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnDropShield;

			static inline auto operator() (CBasePlayer* pPlayer, bool bCallDeploy = true) noexcept
			{
				return pfn(pPlayer, bCallDeploy);
			}
		};

		struct CanPlayerBuy final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::CanPlayerBuy";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x51\x53\x55\x56\x57\x8B\xF9\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x01\xFF\x50\x18"),	// LEGACY
				std::cref("\xCC\x55\x8B\xEC\x51\x56\x8B\xF1\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x01\xFF\x50\x18"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnCanPlayerBuy;

			static inline auto operator() (CBasePlayer* pPlayer, bool bShowMessage = true) noexcept
			{
				return pfn(pPlayer, bShowMessage);
			}
		};

		struct AddAccount final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::AddAccount";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x8B\x44\x24\x04\x56\x8B\xF1\x8B\x8E****\x03\xC8\x89\x8E****\x8B\xC1"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x8B\x45\x08\x56\x8B\xF1\x01\x86\x2A\x2A\x2A\x2A\x79\x0C"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnAddAccount;

			static inline auto operator() (CBasePlayer* pPlayer, int32_t iAmount, bool bTrackChange = true) noexcept
			{
				return pfn(pPlayer, iAmount, bTrackChange);
			}
		};

		struct SelectItem final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::SelectItem";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x8B\x44\x24\x04\x83\xEC\x14\x85\xC0\x53\x55\x56\x57\x8B\xE9\x0F\x84\x2A\x2A\x2A\x2A\x33"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x83\xEC\x08\x53\x8B\x5D\x08\x57\x8B\xF9\x89\x7D\xF8\x85\xDB"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnSelectItem;

			static inline auto operator() (CBasePlayer* pPlayer, const char* pszItemName) noexcept
			{
				return pfn(pPlayer, pszItemName);
			}
		};

		struct SwitchWeapon final	// ESP corruption? #INVESTIGATE
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::SwitchWeapon";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x83\xEC\x0C\x56\x57\x8B\x7C\x24\x18\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90\xF8\x00\x00\x00"),	// NEW
				std::cref("\xCC\x55\x8B\xEC\x56\x57\x8B\x7D\x08\x8B\xF1\x8B\xCF\x8B\x07\xFF\x90"),	// ANNIV
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnSwitchWeapon;

			static inline auto operator() (CBasePlayer* pPlayer, CBasePlayerItem* pWeapon) noexcept
			{
				return pfn(pPlayer, 0, pWeapon);
			}
		};

		struct DropPlayerItem final
		{
			static inline constexpr char MODULE[] = "mp.dll";
			static inline constexpr char NAME[] = u8"::CBasePlayer::DropPlayerItem";
			static inline constexpr std::tuple PATTERNS
			{
				std::cref("\x90\x83\xEC\x24\x33\xC0\x53\x55\x56\x8B\x74\x24\x34\x57\x8B\xD9\x8B\xFE"),	// 8684
				std::cref("\xCC\x55\x8B\xEC\x8B\x55\x08\x83\xEC\x1C\x53\x8B\xD9\x8B\xCA\x56\x8D\x71\x01"),	// 9920
			};
			static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
			static inline auto& pfn = gUranusCollection.pfnDropPlayerItem;

			static inline auto operator() (CBasePlayer* pPlayer, char const* pszItemName) noexcept
			{
				return pfn(pPlayer, 0, pszItemName);
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
			std::cref("\x90\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\x8B\x4D\x08\x8D\x45\x0C\x50\x51\x8D\x95"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x33\xC5\x89\x45\xFC"),	// ANNIV
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
			std::cref("\x90\x55\x8B\xEC\x56\x8B\x75\x08\x57\x8B\x7D\x0C\x8B\x4E\x10\x8B\x46\x0C\x03\xCF\x3B\xC8\x0F\x8E"),	// NEW
			std::cref("\xCC\x55\x8B\xEC\x53\x8B\x5D\x0C\x56\x8B\x75\x08\x8B\x4E\x10\x8B\x56\x0C"),	// ANNIV
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnSZ_GetSpace;
	};

	struct GetDispatch final
	{
		static inline constexpr char MODULE[] = "hw.dll";
		static inline constexpr char NAME[] = u8"::GetDispatch";
		static inline constexpr std::tuple PATTERNS
		{
			std::cref("\x90\xA1****\x53\x55\x56\x57\x33\xFF\x85\xC0\x7E\x26"),	// 3266
			std::cref("\x90\x55\x8B\xEC\xA1****\x53\x56\x57\x33\xFF\x85\xC0\x7E\x23"),	// 8684
			std::cref("\xCC\x55\x8B\xEC\x53\x56\x33\xF6\x57\x39\x35****\x7E\x29\x8B\x5D\x08\xBF"),	// 9920
		};
		static inline constexpr std::ptrdiff_t DISPLACEMENT = 1;
		static inline auto& pfn = gUranusCollection.pfnGetDispatch;

		static inline auto operator() (const char* pszClassName) noexcept
		{
			return pfn(pszClassName);
		}
	};
}
