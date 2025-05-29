/*
Created Date: Jan 27 2024
*/

export module ConsoleVar;

export import std;
export import hlsdk;

using std::uint32_t;	// #MSVC_BUG_STDCOMPAT std.compat crashing INTELLISENSE
using std::strlen;

struct console_variable_t;	// forward declearation

// Luna: The initialization order was unspecified due to CVARs being defined in other translation units.
// Only singleton can resolve this problem.
// Hence we are here. Fuck C++export struct CVarManager
export struct CVarManager final
{
	std::vector<std::move_only_function<void() const noexcept>> m_InitFuncs{};
	std::unordered_map<std::string_view, std::tuple<std::string_view, std::string_view, std::string_view>> m_Desc{};
	std::set<console_variable_t*, std::less<>> m_all{};	// keep a record. sorted by address because the m_handle is nullptr at this point.

	static CVarManager* Instance() noexcept
	{
		static CVarManager singleton{};
		return &singleton;
	}

	static void Init() noexcept
	{
		for (auto&& fn : Instance()->m_InitFuncs)
			std::invoke(fn);
	}
};

export struct console_variable_t final
{
	// MUST be used with static string literal
	template <size_t N1, size_t N2>
	console_variable_t(const char(&szCVarName)[N1], const char(&szValue)[N2]) noexcept
	{
		CVarManager::Instance()->m_InitFuncs.emplace_back(
			// Capture this by ref, the others with copy.
			[this, szCVarName, szValue]() noexcept { Init(szCVarName, szValue); }
		);

		CVarManager::Instance()->m_all.insert(this);
	}

	// MUST be used with static string literal
	template <size_t N1, size_t N2, size_t N3, size_t N4>
	console_variable_t(const char(&szCVarName)[N1], const char(&szDefValue)[N2], const char(&szDomain)[N3], const char(&szDescription)[N4]) noexcept
		: console_variable_t(szCVarName, szDefValue)
	{
		CVarManager::Instance()->m_Desc.try_emplace(szCVarName, szDefValue, szDomain, szDescription);
	}

	bool Init(const char* pszCVarName, const char* pszValue, uint32_t bitsFlags = FCVAR_SERVER | FCVAR_EXTDLL) noexcept
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

	bool Register(const char* pszCVarName, const char* pszValue, uint32_t bitsFlags = FCVAR_SERVER | FCVAR_EXTDLL) noexcept
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
			static_assert(false, "Unsupported type to retrieve from CVar.");
			return m_handle->value;
		}
	}

	constexpr cvar_t* Handle() const noexcept { return m_handle; }

	constexpr cvar_t& operator*() const noexcept { return *m_handle; }
	constexpr cvar_t* operator->() const noexcept { return m_handle; }

	// A bit dangerous, but necessary.
	template <typename T> __forceinline
		explicit operator T() const noexcept
	{
		return Get<T>();
	}

private:
	cvar_t* m_handle{};
};

// msvc++ bug?
// decltype(lambda) gets error..
export struct cvar_wrapper_sorter final
{
	static constexpr bool operator() (console_variable_t* lhs, console_variable_t* rhs) noexcept
	{
		// remember to sort the string, not the address of that string.
		return
			std::string_view{ lhs->Handle()->name }
			<
			std::string_view{ rhs->Handle()->name };
	}
};
