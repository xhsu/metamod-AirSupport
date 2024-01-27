/*
Created Data: Jan 27 2024
*/

export module ConsoleVar;

export import <concepts>;
export import <format>;

export import eiface;
export import cvardef;

// #UNTESTED
export struct console_variable_t final
{
	bool Init(const char* pszCVarName, const char* pszValue, int bitsFlags = FCVAR_SERVER | FCVAR_EXTDLL) noexcept
	{
		if ((m_handle = g_engfuncs.pfnCVarGetPointer(pszCVarName)) == nullptr)
		{
			cvar_t data{ pszCVarName, pszValue, bitsFlags };
			g_engfuncs.pfnCVarRegister(&data);
		}
		else
			return true;

		return (m_handle = g_engfuncs.pfnCVarGetPointer(pszCVarName)) != nullptr;
	}

	bool Retrieve(const char* pszCVarName) noexcept
	{
		m_handle = g_engfuncs.pfnCVarGetPointer(pszCVarName);
		return m_handle != nullptr;
	}

	bool Register(const char* pszCVarName, const char* pszValue, int bitsFlags = FCVAR_SERVER | FCVAR_EXTDLL) noexcept
	{
		cvar_t data{ pszCVarName, pszValue, bitsFlags };
		g_engfuncs.pfnCVarRegister(&data);

		m_handle = g_engfuncs.pfnCVarGetPointer(pszCVarName);
		return m_handle != nullptr;
	}

	void Set(auto&& value) const noexcept
	{
		auto const sz = std::format("{}", value);
		g_engfuncs.pfnCvar_DirectSet(m_handle, sz.c_str());
	}

	template <typename T>
	T Get(std::type_identity_t<T> def_val = {}) const noexcept
	{
		if constexpr (std::is_convertible_v<T, std::string_view>)
		{
			return T{ m_handle->string };
		}
		else if constexpr (std::is_arithmetic_v<T>)
		{
			std::from_chars(m_handle->string, m_handle->string + strlen(m_handle->string), def_val);
			return def_val;	// the error code doesn't matter. it's default value anyway.
		}
		else
		{
			// #UPDATE_AT_CPP23 static_assert(false);
			return m_handle->value;
		}
	}

	constexpr decltype(auto) operator*() const noexcept { return *m_handle; }
	constexpr decltype(auto) operator->() const noexcept { return m_handle; }

private:
	cvar_t* m_handle{};
};
