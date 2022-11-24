import Resources;

void Precache(void) noexcept
{
	// Models

	for (auto &&psz : Models::PLANE)
		Models::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	for (auto &&psz : Models::PROJECTILE)
		Models::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	Models::m_rgLibrary[Models::V_RADIO] = g_engfuncs.pfnPrecacheModel(Models::V_RADIO);
	Models::m_rgLibrary[Models::P_RADIO] = g_engfuncs.pfnPrecacheModel(Models::P_RADIO);

	Models::m_rgLibrary[Models::GIBS_BRICK] = g_engfuncs.pfnPrecacheModel(Models::GIBS_BRICK);
	Models::m_rgLibrary[Models::GIBS_WALL_BROWN] = g_engfuncs.pfnPrecacheModel(Models::GIBS_WALL_BROWN);
	Models::m_rgLibrary[Models::GIBS_WOOD] = g_engfuncs.pfnPrecacheModel(Models::GIBS_WOOD);

	Models::m_rgLibrary[Models::TARGET] = g_engfuncs.pfnPrecacheModel(Models::TARGET);

	// Sounds

	for (auto &&psz : Sounds::RADIO)
		g_engfuncs.pfnPrecacheSound(psz);

	g_engfuncs.pfnPrecacheSound(Sounds::REQUESTING);
	g_engfuncs.pfnPrecacheSound(Sounds::REJECTING);
	g_engfuncs.pfnPrecacheSound(Sounds::NOISE);

	g_engfuncs.pfnPrecacheSound(Sounds::TRAVEL);

	g_engfuncs.pfnPrecacheSound(Sounds::AMBIENT_FIRE);

	g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_BREATHE);
	g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_EAR_RINGING);
	g_engfuncs.pfnPrecacheSound(Sounds::PLAYER_HEARTBEAT);

	for (auto &&psz : Sounds::EXPLOSION)
		g_engfuncs.pfnPrecacheSound(psz);

	for (auto &&psz : Sounds::JET)
		g_engfuncs.pfnPrecacheSound(psz);

	// Sprite

	Sprites::m_rgLibrary[Sprites::SMOKE] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE);
	Sprites::m_rgLibrary[Sprites::SMOKE2] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE2);
	Sprites::m_rgLibrary[Sprites::ROCKET_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::ROCKET_EXPLO);
	Sprites::m_rgLibrary[Sprites::ROCKET_EXPLO2] = g_engfuncs.pfnPrecacheModel(Sprites::ROCKET_EXPLO2);
	Sprites::m_rgLibrary[Sprites::SMOKE_TRAIL] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE_TRAIL);
	Sprites::m_rgLibrary[Sprites::FIRE] = g_engfuncs.pfnPrecacheModel(Sprites::FIRE);
	Sprites::m_rgLibrary[Sprites::FIRE2] = g_engfuncs.pfnPrecacheModel(Sprites::FIRE2);
	Sprites::m_rgLibrary[Sprites::SMOKE_1] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE_1);
	Sprites::m_rgLibrary[Sprites::SMOKE_2] = g_engfuncs.pfnPrecacheModel(Sprites::SMOKE_2);
	Sprites::m_rgLibrary[Sprites::PERSISTENT_SMOKE] = g_engfuncs.pfnPrecacheModel(Sprites::PERSISTENT_SMOKE);
	Sprites::m_rgLibrary[Sprites::MINOR_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::MINOR_EXPLO);
	Sprites::m_rgLibrary[Sprites::AIRBURST] = g_engfuncs.pfnPrecacheModel(Sprites::AIRBURST);
	Sprites::m_rgLibrary[Sprites::CARPET_FRAGMENT_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprites::CARPET_FRAGMENT_EXPLO);

	Sprites::m_rgLibrary[Sprites::BEAM] = g_engfuncs.pfnPrecacheModel(Sprites::BEAM);
	//Sprite::m_rgLibrary[Sprite::AIM] = g_engfuncs.pfnPrecacheModel(Sprite::AIM);

	Sprites::m_rgLibrary[Sprites::TRAIL] = g_engfuncs.pfnPrecacheModel(Sprites::TRAIL);

	for (auto &&psz : Sprites::FLAME)
		Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	for (auto &&psz : Sprites::BLACK_SMOKE)
		Sprites::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	// Decal

	for (auto &&decal : Decal::GUNSHOT)
		decal.Initialize();

	for (auto &&decal : Decal::SCORCH)
		decal.Initialize();
}
