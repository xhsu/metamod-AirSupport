import <algorithm>;

import Resources;

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
import CRC64;
import FileSystem;

#include "../../Enforcer/ResourceCRC64.hpp"

bool CheckResource(const char* psz) noexcept
{
	if (!g_rgiCRC64.contains(psz))
		return true;

	if (auto f = g_pFileSystem->Open(psz, "rb"); f)
	{
		g_pFileSystem->Seek(f, 0, FILESYSTEM_SEEK_TAIL);

		auto const iFileSize = g_pFileSystem->Tell(f);
		auto p = (std::byte*)malloc(iFileSize);

		g_pFileSystem->Seek(f, 0, FILESYSTEM_SEEK_HEAD);
		g_pFileSystem->Read(p, iFileSize, f);

		if (auto const crc = CRC64::CheckStream(p, iFileSize); crc != g_rgiCRC64.at(psz))
			return false;

		g_pFileSystem->Close(f);
		free(p);
	}

	return true;
}

#endif

__forceinline void PrecacheModel(const char *psz) noexcept
{
	Models::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (!CheckResource(psz))
		UTIL_Terminate("File '%s' has been altered.\nYou are not allowed to modify any file came with resource pack.", psz);
#endif
}

__forceinline void PrecacheSprite(const char *psz) noexcept
{
	Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (!CheckResource(psz))
		UTIL_Terminate("File '%s' has been altered.\nYou are not allowed to modify any file came with resource pack.", psz);
#endif
}

__forceinline void PrecacheSound(const char* psz) noexcept
{
	g_engfuncs.pfnPrecacheSound(psz);

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (!CheckResource(psz))
		UTIL_Terminate("File '%s' has been altered.\nYou are not allowed to modify any file came with resource pack.", psz);
#endif
}

void Precache(void) noexcept
{
	// Models

	std::ranges::for_each(Models::PLANE, PrecacheModel);
	PrecacheModel(Models::PROJECTILE);

	PrecacheModel(Models::V_RADIO);
	PrecacheModel(Models::P_RADIO);

	PrecacheModel(Models::GIBS_CONCRETE);
	PrecacheModel(Models::GIBS_METAL);
	//PrecacheModel(Models::GIBS_RUBBLE);
	//PrecacheModel(Models::GIBS_WOOD);

	PrecacheModel(Models::TARGET);

	PrecacheModel(Models::SPARK);

	// Sounds

	std::ranges::for_each(Sounds::ACCEPTING, PrecacheSound);

	PrecacheSound(Sounds::REQUESTING);
	std::ranges::for_each(Sounds::REJECTING, PrecacheSound);
	PrecacheSound(Sounds::NOISE);

	PrecacheSound(Sounds::TRAVEL);

	//PrecacheSound(Sounds::PLAYER_BREATHE);
	PrecacheSound(Sounds::PLAYER_EAR_RINGING);
	//PrecacheSound(Sounds::PLAYER_HEARTBEAT);
	PrecacheSound(Sounds::PLAYER_HB_AND_ER);

	std::ranges::for_each(Sounds::PLAYER_COUGH, PrecacheSound);

	std::ranges::for_each(Sounds::EXPLOSION, PrecacheSound);
	std::ranges::for_each(Sounds::EXPLOSION_SHORT, PrecacheSound);
	std::ranges::for_each(Sounds::JET, PrecacheSound);
	std::ranges::for_each(Sounds::WHIZZ, PrecacheSound);
	std::ranges::for_each(Sounds::BOMBER, PrecacheSound);
	std::ranges::for_each(Sounds::HIT_METAL, PrecacheSound);
	std::ranges::for_each(Sounds::EXPLOSION_BIG, PrecacheSound);

	PrecacheSound(Sounds::CLUSTER_BOMB_DROP);

	std::ranges::for_each(Sounds::GRENADE_BOUNCE, PrecacheSound);

	// namespace Gunship
	{
		std::ranges::for_each(Sounds::Gunship::AC130_AMBIENT, PrecacheSound);
		std::ranges::for_each(Sounds::Gunship::AC130_DEPARTURE, PrecacheSound);

		PrecacheSound(Sounds::Gunship::AC130_IS_IN_AIR);
		//PrecacheSound(Sounds::Gunship::UAV_IS_ONLINE);

		std::ranges::for_each(Sounds::Gunship::KILL_CONFIRMED, PrecacheSound);

		PrecacheSound(Sounds::Gunship::NOISE_PILOT);

		std::ranges::for_each(Sounds::Gunship::AC130_FIRE_25MM, PrecacheSound);
		std::ranges::for_each(Sounds::Gunship::AC130_RELOAD, PrecacheSound);

		PrecacheSound(Sounds::Gunship::RESELECT_TARGET);

		PrecacheSound(Sounds::Gunship::TARGET_RAN_TO_COVER);
	}

	// namespace FuelAirBomb
	{
		PrecacheSound(Sounds::FuelAirBomb::GAS_LEAK_FADEOUT);
		PrecacheSound(Sounds::FuelAirBomb::GAS_LEAK_LOOP);

		std::ranges::for_each(Sounds::FuelAirBomb::GAS_EXPLO, PrecacheSound);
	}

	// namespace Flame
	{
		//std::ranges::for_each(Sounds::Flame::FLAME, PrecacheSound);
		//PrecacheSound(Sounds::Flame::FLAME_FADEOUT);
	}

#ifdef PACKING_RESOURCES
	PrecacheSound(Sounds::ALERT_AC130);
	PrecacheSound(Sounds::ALERT_AIRSTRIKE);
	PrecacheSound(Sounds::ALERT_APACHE);
#endif

	// Sprite

	PrecacheSprite(Sprites::ROCKET_EXPLO);
	PrecacheSprite(Sprites::ROCKET_EXPLO2);
	PrecacheSprite(Sprites::FIRE);
	PrecacheSprite(Sprites::FIRE2);
	PrecacheSprite(Sprites::MINOR_EXPLO);
	PrecacheSprite(Sprites::AIRBURST);
	PrecacheSprite(Sprites::CARPET_FRAGMENT_EXPLO);
	PrecacheSprite(Sprites::SHOCKWAVE);
	PrecacheSprite(Sprites::SPARK);
	PrecacheSprite(Sprites::LIFTED_DUST);
	PrecacheSprite(Sprites::GROUNDED_DUST);
	PrecacheSprite(Sprites::GIGANTIC_EXPLO);
	PrecacheSprite(Sprites::STATIC_SMOKE_THIN);
	PrecacheSprite(Sprites::STATIC_SMOKE_THICK);

	PrecacheSprite(Sprites::BEAM);

	PrecacheSprite(Sprites::TRAIL);

	std::ranges::for_each(Sprites::FLAME, PrecacheSprite);
	std::ranges::for_each(Sprites::BLACK_SMOKE, PrecacheSprite);
	std::ranges::for_each(Sprites::ROCKET_TRAIL_SMOKE, PrecacheSprite);
	std::ranges::for_each(Sprites::GAS_EXPLO, PrecacheSprite);

	// Decal

	for (auto &&decal : Decal::GUNSHOT)
		decal.Initialize();

	for (auto &&decal : Decal::BIGSHOT)
		decal.Initialize();

	for (auto &&decal : Decal::SCORCH)
		decal.Initialize();

	for (auto &&decal : Decal::SMALL_SCORCH)
		decal.Initialize();
}
