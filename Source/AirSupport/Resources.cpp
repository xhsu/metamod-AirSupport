import <algorithm>;

import Resources;

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
import <span>;

import CRC64;
import FileSystem;
import Sprite;
import Wave;

static bool g_bShouldEnforceModels = true;

static bool VerifyByCRC64(const char* psz) noexcept
{
	if (!g_rgiCRC64.contains(psz) || !g_bShouldEnforceModels)
		return true;

	if (auto f = FileSystem::StandardOpen(psz, "rb"); f != nullptr)
	{
		auto const crc = CRC64::CheckFile(f);
		fclose(f);

		return crc == g_rgiCRC64.at(psz);
	}

	return true;
}

static bool VerifyBySoundLength(const char* psz) noexcept
{
	if (!g_rgflSoundTime.contains(psz) || !g_bShouldEnforceModels)
		return true;

	std::string const sz = std::string("sound/") + psz;

	if (auto f = FileSystem::StandardOpen(sz.c_str(), "rb"); f != nullptr)
	{
		auto const flCurFileLength = Wave::Length(f);
		fclose(f);

		return std::abs(flCurFileLength - g_rgflSoundTime.at(psz)) < 0.1;
	}

	return true;
}

static bool VerifyByModelAnimations(const char *psz) noexcept
{
	if (!g_rgrgflAnimTime.contains(psz) || !g_bShouldEnforceModels)
		return true;

	if (auto f = FileSystem::StandardOpen(psz, "rb"); f != nullptr)
	{
		fseek(f, 0, SEEK_END);
		auto const iSize = ftell(f);

		fseek(f, 0, SEEK_SET);
		auto pBuffer = calloc(1, iSize);
		fread(pBuffer, iSize, 1, f);

		auto const phdr = (studiohdr_t *)pBuffer;
		auto const pseq = (mstudioseqdesc_t *)((uint8_t *)pBuffer + phdr->seqindex);
		auto const rgSeq = std::span(pseq, phdr->numseq);
		auto const fnCmp = [](mstudioseqdesc_t const& SeqInfo, double const& flTime) /*static*/ noexcept -> bool
		{
			// unlike other cmp fn, this one acts as a predicate and therefore return true means equal.

			return std::abs(
				(double)SeqInfo.numframes / (double)SeqInfo.fps - flTime
			) < 0.01;
		};

		bool ret = false;

		// One may add more animation later on, but no deletion allowed.
		if (rgSeq.size() >= g_rgrgflAnimTime.at(psz).size())
			ret = std::ranges::equal(rgSeq, g_rgrgflAnimTime.at(psz), fnCmp);

		free(pBuffer);
		fclose(f);

		return ret;
	}

	return false;
}

static bool VerifyBySpriteFrame(const char *psz) noexcept
{
	if (!g_rgiSpriteFrameCount.contains(psz) || !g_bShouldEnforceModels)
		return true;

	if (auto f = FileSystem::StandardOpen(psz, "rb"); f != nullptr)
	{
		GoldSrc::Sprite_t const hSprite{ f };
		fclose(f);

		return hSprite.m_iNumOfFrames == g_rgiSpriteFrameCount.at(psz);
	}

	return false;
}

#endif

static void PrecacheModel(const char *psz) noexcept
{
	Models::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (!VerifyByModelAnimations(psz))
		UTIL_Terminate("Critical model '%s' has been altered.\nYou are only swap it to a model with same animation length.", psz);
#endif
}

static void PrecacheSprite(const char *psz) noexcept
{
	Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (!VerifyBySpriteFrame(psz))
		UTIL_Terminate("File '%s' has been altered.\nThe altering sprite file must have a same frame count.", psz);
#endif
}

static void PrecacheSound(const char* psz) noexcept
{
	g_engfuncs.pfnPrecacheSound(psz);

#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (!VerifyBySoundLength(psz))
		UTIL_Terminate("File '%s' has been altered.\nThe altering wave file must have a same lasting time.", psz);
#endif
}

void Precache(void) noexcept
{
#if !defined PACKING_RESOURCES && !defined CREATING_ENFORCING_TABLE
	if (FileSystem::m_pObject->FileExists("addons/metamod/AirSupport/Config/DONT_ENFORCE_RES"))
		g_bShouldEnforceModels = false;
#endif

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

	// namespace Thermite
	{
		PrecacheSound(Sounds::Thermite::BURNING_LOOP);
		PrecacheSound(Sounds::Thermite::BURNING_END);
	}

#ifdef PACKING_RESOURCES
	PrecacheSound(Sounds::ALERT_AC130);
	PrecacheSound(Sounds::ALERT_AIRSTRIKE);
	PrecacheSound(Sounds::ALERT_APACHE);
#endif

	// Sprite

	PrecacheSprite(Sprites::ROCKET_EXPLO);
	PrecacheSprite(Sprites::ROCKET_EXPLO2);
	PrecacheSprite(Sprites::ROCKET_EXHAUST_FLAME);
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
	PrecacheSprite(Sprites::PHOSPHORUS_TRACE_HEAD);
	PrecacheSprite(Sprites::PHOSPHORUS_FLAME);
	PrecacheSprite(Sprites::PHOSPHORUS_SMOKE);
	PrecacheSprite(Sprites::PHOSPHORUS_MINOR_SPARK);
	PrecacheSprite(Sprites::PHOSPHORUS_MAJOR_SPARK);
	PrecacheSprite(Sprites::FLAME_ON_PLAYER);

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
