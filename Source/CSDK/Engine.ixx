export module Engine;

export import eiface;

export import Platform;

export import UtlHook;

export namespace Engine
{
	inline constexpr unsigned char BUILD_NUMBER_PATTERN[] = "\xA1\x2A\x2A\x2A\x2A\x83\xEC\x08\x2A\x33\x2A\x85\xC0";
	inline constexpr unsigned char BUILD_NUMBER_NEW_PATTERN[] = "\x55\x8B\xEC\x83\xEC\x08\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\x53\x33\xDB\x8B\x04\x9D";

	inline bool m_bIsNewEngine = false;
	inline int(*m_pfnBuildNumber)(void) = nullptr;
	inline int BUILD_NUMBER = 0;

	inline const char* GetDLLName(void) noexcept
	{
		[[unlikely]]
		if (!g_engfuncs.pfnEngCheckParm)
			UTIL_Terminate("Cannot retrieve engine function 'g_engfuncs.pfnEngCheckParm'.\nEither you are using a illegal copy or you disconnected from the internet since 2009.");

		// #UNDONE
		//auto const ret1 = g_engfuncs.pfnEngCheckParm("-soft", nullptr);
		//auto const ret2 = g_engfuncs.pfnEngCheckParm("-software", nullptr);
		//auto const ret3 = g_engfuncs.pfnEngCheckParm("-gl", nullptr);
		//auto const ret4 = g_engfuncs.pfnEngCheckParm("-d3d", nullptr);

		return "hw.dll";
	}

	void Init(void) noexcept
	{
		m_bIsNewEngine = false;
		m_pfnBuildNumber = (decltype(m_pfnBuildNumber))UTIL_SearchPattern(GetDLLName(), BUILD_NUMBER_PATTERN);

		if (!m_pfnBuildNumber)
		{
			m_pfnBuildNumber = (decltype(m_pfnBuildNumber))UTIL_SearchPattern(GetDLLName(), BUILD_NUMBER_NEW_PATTERN);
			m_bIsNewEngine = true;
		}

		[[unlikely]]
		if (!m_pfnBuildNumber)
		{
			UTIL_Terminate("Engine is unsupported.");
		}

		BUILD_NUMBER = m_pfnBuildNumber();
	}
};
