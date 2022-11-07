import Resources;

void Precache(void) noexcept
{
	for (auto &&psz : Models::PLANE)
		g_engfuncs.pfnPrecacheModel(psz);

	for (auto &&psz : Models::PROJECTILE)
		g_engfuncs.pfnPrecacheModel(psz);

	for (auto &&psz : Sounds::RADIO)
		g_engfuncs.pfnPrecacheSound(psz);

	g_engfuncs.pfnPrecacheSound(Sounds::REQUESTING);
	g_engfuncs.pfnPrecacheSound(Sounds::REJECTING);
}
