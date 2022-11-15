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

	Sprite::m_rgLibrary[Sprite::SMOKE] = g_engfuncs.pfnPrecacheModel(Sprite::SMOKE);
	Sprite::m_rgLibrary[Sprite::SMOKE2] = g_engfuncs.pfnPrecacheModel(Sprite::SMOKE2);
	Sprite::m_rgLibrary[Sprite::ROCKET_EXPLO] = g_engfuncs.pfnPrecacheModel(Sprite::ROCKET_EXPLO);
	Sprite::m_rgLibrary[Sprite::ROCKET_EXPLO2] = g_engfuncs.pfnPrecacheModel(Sprite::ROCKET_EXPLO2);
	Sprite::m_rgLibrary[Sprite::SMOKE_TRAIL] = g_engfuncs.pfnPrecacheModel(Sprite::SMOKE_TRAIL);
	Sprite::m_rgLibrary[Sprite::FIRE] = g_engfuncs.pfnPrecacheModel(Sprite::FIRE);
	Sprite::m_rgLibrary[Sprite::FIRE2] = g_engfuncs.pfnPrecacheModel(Sprite::FIRE2);
	Sprite::m_rgLibrary[Sprite::SMOKE_1] = g_engfuncs.pfnPrecacheModel(Sprite::SMOKE_1);
	Sprite::m_rgLibrary[Sprite::SMOKE_2] = g_engfuncs.pfnPrecacheModel(Sprite::SMOKE_2);
	Sprite::m_rgLibrary[Sprite::BLACK_SMOKE] = g_engfuncs.pfnPrecacheModel(Sprite::BLACK_SMOKE);

	Sprite::m_rgLibrary[Sprite::BEAM] = g_engfuncs.pfnPrecacheModel(Sprite::BEAM);
	//Sprite::m_rgLibrary[Sprite::AIM] = g_engfuncs.pfnPrecacheModel(Sprite::AIM);

	Sprite::m_rgLibrary[Sprite::TRAIL] = g_engfuncs.pfnPrecacheModel(Sprite::TRAIL);

	for (auto &&psz : Sprite::FLAME)
		Sprite::m_rgLibrary[psz] = g_engfuncs.pfnPrecacheModel(psz);

	// Decal

	for (auto &&decal : Decal::GUNSHOT)
		decal.Initialize();

	for (auto &&decal : Decal::SCORCH)
		decal.Initialize();
}
