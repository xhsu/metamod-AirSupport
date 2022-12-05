import Transpiler;

void __stdcall ReceiveCSharpFnPtr(int(__cdecl *pfnLoadModel)(const char *), int(__cdecl *pfnLoadSound)(const char *)) noexcept
{
	g_engfuncs.pfnPrecacheModel = pfnLoadModel;
	g_engfuncs.pfnPrecacheSound = pfnLoadSound;
	g_engfuncs.pfnDecalIndex = [](const char *) -> int { return 0; };
}
