import <algorithm>;

import Resources;

void PrecacheModel(const char *psz) noexcept
{
	Models::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);
}

void PrecacheSprite(const char *psz) noexcept
{
	Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);
}

void Precache(void) noexcept
{
	// Models

	std::ranges::for_each(Models::PLANE, PrecacheModel);
	Models::m_rgLibrary[Models::PROJECTILE] = g_engfuncs.pfnPrecacheModel(Models::PROJECTILE);

	Models::m_rgLibrary[Models::V_RADIO] = g_engfuncs.pfnPrecacheModel(Models::V_RADIO);
	Models::m_rgLibrary[Models::P_RADIO] = g_engfuncs.pfnPrecacheModel(Models::P_RADIO);

	Models::m_rgLibrary[Models::GIBS_CONCRETE] = g_engfuncs.pfnPrecacheModel(Models::GIBS_CONCRETE);
	Models::m_rgLibrary[Models::GIBS_METAL] = g_engfuncs.pfnPrecacheModel(Models::GIBS_METAL);
	//Models::m_rgLibrary[Models::GIBS_RUBBLE] = g_engfuncs.pfnPrecacheModel(Models::GIBS_RUBBLE);
	//Models::m_rgLibrary[Models::GIBS_WOOD] = g_engfuncs.pfnPrecacheModel(Models::GIBS_WOOD);

	Models::m_rgLibrary[Models::TARGET] = g_engfuncs.pfnPrecacheModel(Models::TARGET);

	Models::m_rgLibrary[Models::SPARK] = g_engfuncs.pfnPrecacheModel(Models::SPARK);

	// Sounds

	std::ranges::for_each(Sounds::ACCEPTING, g_engfuncs.pfnPrecacheSound);

	g_engfuncs.pfnPrecacheSound(Sounds::REQUESTING);
	std::ranges::for_each(Sounds::REJECTING, g_engfuncs.pfnPrecacheSound);
	g_engfuncs.pfnPrecacheSound(Sounds::NOISE);

	g_engfuncs.pfnPrecacheSound(Sounds::TRAVEL);

	//g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_BREATHE);
	g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_EAR_RINGING);
	//g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_HEARTBEAT);
	g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_HB_AND_ER);

	std::ranges::for_each(Sounds::PLAYER_COUGH, g_engfuncs.pfnPrecacheSound);

	std::ranges::for_each(Sounds::EXPLOSION, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::EXPLOSION_SHORT, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::JET, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::WHIZZ, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::BOMBER, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::HIT_METAL, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::EXPLOSION_BIG, g_engfuncs.pfnPrecacheSound);

	g_engfuncs.pfnPrecacheSound(Sounds::CLUSTER_BOMB_DROP);

	std::ranges::for_each(Sounds::GRENADE_BOUNCE, g_engfuncs.pfnPrecacheSound);

	// namespace Gunship
	{
		std::ranges::for_each(Sounds::Gunship::AC130_AMBIENT, g_engfuncs.pfnPrecacheSound);
		std::ranges::for_each(Sounds::Gunship::AC130_DEPARTURE, g_engfuncs.pfnPrecacheSound);

		g_engfuncs.pfnPrecacheSound(Sounds::Gunship::AC130_IS_IN_AIR);
		//g_engfuncs.pfnPrecacheSound(Sounds::Gunship::UAV_IS_ONLINE);

		std::ranges::for_each(Sounds::Gunship::KILL_CONFIRMED, g_engfuncs.pfnPrecacheSound);

		g_engfuncs.pfnPrecacheSound(Sounds::Gunship::NOISE_PILOT);

		std::ranges::for_each(Sounds::Gunship::AC130_FIRE_25MM, g_engfuncs.pfnPrecacheSound);
		std::ranges::for_each(Sounds::Gunship::AC130_RELOAD, g_engfuncs.pfnPrecacheSound);

		g_engfuncs.pfnPrecacheSound(Sounds::Gunship::RESELECT_TARGET);

		g_engfuncs.pfnPrecacheSound(Sounds::Gunship::TARGET_RAN_TO_COVER);
	}

	// namespace FuelAirBomb
	{
		g_engfuncs.pfnPrecacheSound(Sounds::FuelAirBomb::GAS_LEAK_FADEOUT);
		g_engfuncs.pfnPrecacheSound(Sounds::FuelAirBomb::GAS_LEAK_LOOP);

		std::ranges::for_each(Sounds::FuelAirBomb::GAS_EXPLO, g_engfuncs.pfnPrecacheSound);
	}

	// namespace Flame
	{
		//std::ranges::for_each(Sounds::Flame::FLAME, g_engfuncs.pfnPrecacheSound);
		//g_engfuncs.pfnPrecacheSound(Sounds::Flame::FLAME_FADEOUT);
	}

#ifdef PACKING_RESOURCES
	g_engfuncs.pfnPrecacheSound(Sounds::ALERT_AC130);
	g_engfuncs.pfnPrecacheSound(Sounds::ALERT_AIRSTRIKE);
	g_engfuncs.pfnPrecacheSound(Sounds::ALERT_APACHE);
#endif

	// Sprite

	Sprites::m_rgLibrary[Sprites::SMOKE] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE);
	Sprites::m_rgLibrary[Sprites::ROCKET_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::ROCKET_EXPLO);
	Sprites::m_rgLibrary[Sprites::ROCKET_EXPLO2] = g_engfuncs.pfnPrecacheModel(Sprites::ROCKET_EXPLO2);
	Sprites::m_rgLibrary[Sprites::FIRE] = g_engfuncs.pfnPrecacheModel(Sprites::FIRE);
	Sprites::m_rgLibrary[Sprites::FIRE2] = g_engfuncs.pfnPrecacheModel(Sprites::FIRE2);
	Sprites::m_rgLibrary[Sprites::SMOKE_1] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE_1);
	Sprites::m_rgLibrary[Sprites::SMOKE_2] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE_2);
	Sprites::m_rgLibrary[Sprites::MINOR_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::MINOR_EXPLO);
	Sprites::m_rgLibrary[Sprites::AIRBURST] = g_engfuncs.pfnPrecacheModel(Sprites::AIRBURST);
	Sprites::m_rgLibrary[Sprites::CARPET_FRAGMENT_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::CARPET_FRAGMENT_EXPLO);
	Sprites::m_rgLibrary[Sprites::SHOCKWAVE] = g_engfuncs.pfnPrecacheModel(Sprites::SHOCKWAVE);
	Sprites::m_rgLibrary[Sprites::SPARK] = g_engfuncs.pfnPrecacheModel(Sprites::SPARK);
	Sprites::m_rgLibrary[Sprites::LIFTED_DUST] = g_engfuncs.pfnPrecacheModel(Sprites::LIFTED_DUST);
	Sprites::m_rgLibrary[Sprites::GROUNDED_DUST] = g_engfuncs.pfnPrecacheModel(Sprites::GROUNDED_DUST);
	Sprites::m_rgLibrary[Sprites::GIGANTIC_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::GIGANTIC_EXPLO);

	Sprites::m_rgLibrary[Sprites::BEAM] = g_engfuncs.pfnPrecacheModel(Sprites::BEAM);

	Sprites::m_rgLibrary[Sprites::TRAIL] = g_engfuncs.pfnPrecacheModel(Sprites::TRAIL);

	for (auto &&psz : Sprites::FLAME)
		Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	for (auto &&psz : Sprites::BLACK_SMOKE)
		Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	for (auto &&psz : Sprites::GAS_EXPLO)
		Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

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
