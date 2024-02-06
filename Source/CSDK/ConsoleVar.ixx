/*
Created Data: Jan 27 2024
*/

export module ConsoleVar;

export import <concepts>;
export import <format>;
export import <functional>;
export import <unordered_set>;
export import <vector>;

export import eiface;
export import cvardef;

export inline std::vector<std::move_only_function<void()>> grgCVarInitFN;


export struct console_variable_t final
{
	template <size_t N1, size_t N2>
	console_variable_t(const char (&szCVarName)[N1], const char (&szValue)[N2]) noexcept
	{
		grgCVarInitFN.emplace_back(
			// Capture this by ref, the others with copy.
			[this, szCVarName, szValue]() noexcept { Init(szCVarName, szValue); }
		);

		all.insert(this);
	}

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
		else if constexpr (std::same_as<T, bool>)
		{
			return (int)std::roundf(m_handle->value) >= 1;
		}
		else if constexpr (std::is_arithmetic_v<T>)
		{
			if constexpr (std::same_as<T, float>)
			{
				return m_handle->value;
			}
			else
			{
				std::from_chars(m_handle->string, m_handle->string + strlen(m_handle->string), def_val);
				return def_val;	// the error code doesn't matter. it's default value anyway.
			}
		}
		else
		{
			// #UPDATE_AT_CPP23 static_assert(false);
			return m_handle->value;
		}
	}

	constexpr cvar_t* Handle() const noexcept { return m_handle; }

	constexpr decltype(auto) operator*() const noexcept { return *m_handle; }
	constexpr decltype(auto) operator->() const noexcept { return m_handle; }

	// A bit dangerous, but necessary.
	template <typename T> __forceinline
	explicit operator T() const noexcept
	{
		return Get<T>();
	}

	// keep a record.
	static inline std::unordered_set<console_variable_t*> all;

private:
	cvar_t* m_handle{};
};
