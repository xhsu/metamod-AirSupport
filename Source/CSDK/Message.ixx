/*

Created Date: Aug 29 2021

Modern Warfare Dev Team
Programmer - Luna the Reborn

*/
module;

#include <cassert>

#define USING_METAMOD
#define USING_BUILTIN_WRITING
#define GOLDSRC_MESSAGE_MODULE_VERSION 20250710L

export module Message;

import std;
import hlsdk;

#ifdef USING_METAMOD
import Plugin;
#endif
#ifdef USING_BUILTIN_WRITING
import Uranus;
#endif

import UtlConcepts;
#ifdef USING_BUILTIN_WRITING
import UtlHook;
#endif

using std::bit_cast;
using std::int16_t;
using std::int32_t;
using std::int8_t;
using std::uint16_t;
using std::uint8_t;

// The user message buffer
export inline sizebuf_t* gpMsgBuffer = nullptr;

// A wrapper that force pfnWriteAngle
export struct msg_angle_t final
{
	constexpr msg_angle_t(float val) noexcept : m_angle_data{ val } {}
	float m_angle_data{};
};

export inline void MsgSend(entvars_t *pev, ESvcCommands iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_ONE, iMessageIndex, nullptr, ent_cast<edict_t *>(pev));
}

export inline void MsgAll(ESvcCommands iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_ALL, iMessageIndex, nullptr, nullptr);
}

export inline void MsgBroadcast(ESvcCommands iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_BROADCAST, iMessageIndex, nullptr, nullptr);
}

export inline void MsgPVS(ESvcCommands iMessageIndex, const Vector &vecOrigin) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_PVS, iMessageIndex, vecOrigin, nullptr);
}

export inline void WriteData(auto&& arg) noexcept
{
	using T = std::remove_cvref_t<std::decay_t<decltype(arg)>>;

#ifndef USING_BUILTIN_WRITING

	// some special objects.

	if constexpr (std::is_convertible_v<T, std::string_view>)
		g_engfuncs.pfnWriteString(arg);
	else if constexpr (std::is_same_v<T, Vector> || std::is_same_v<T, Angles>)
	{
		g_engfuncs.pfnWriteCoord(arg[0]);
		g_engfuncs.pfnWriteCoord(arg[1]);
		g_engfuncs.pfnWriteCoord(arg[2]);
	}
	else if constexpr (std::is_same_v<T, Vector2D>)
	{
		g_engfuncs.pfnWriteCoord(arg[0]);
		g_engfuncs.pfnWriteCoord(arg[1]);
	}
	else if constexpr (std::is_same_v<T, msg_angle_t>)
		g_engfuncs.pfnWriteAngle(arg.m_angle_data);

	// general cases

	else if constexpr (sizeof(T) == 4)	// long, int, float
		g_engfuncs.pfnWriteLong(bit_cast<int32_t>(arg));

	else if constexpr (sizeof(T) == 2)	// short, char16_t
		g_engfuncs.pfnWriteShort(bit_cast<int16_t>(arg));

	else if constexpr (std::is_same_v<T, bool> || ((std::is_unsigned_v<T> || std::is_enum_v<T>) && sizeof(T) == 1))	// bool, uchar, byte
		g_engfuncs.pfnWriteByte(bit_cast<uint8_t>(arg));
	else if constexpr (sizeof(T) == 1)	// signed char
		g_engfuncs.pfnWriteChar(bit_cast<char>(arg));
#else

	// some special objects.

	if constexpr (std::is_convertible_v<T, std::string_view>)
	{
		std::string_view const sz{ arg };
		auto const p = reinterpret_cast<char*>(gUranusCollection.pfnSZ_GetSpace(gpMsgBuffer, sz.length() + 1));
		std::memcpy(p, sz.data(), sz.length());
		p[sz.length()] = '\0';
	}
	else if constexpr (std::is_same_v<T, Vector> || std::is_same_v<T, Angles>)
	{
		std::array const arr{
			std::bit_cast<uint16_t>((int16_t)std::lround(arg[0] * 8.0)),
			std::bit_cast<uint16_t>((int16_t)std::lround(arg[1] * 8.0)),
			std::bit_cast<uint16_t>((int16_t)std::lround(arg[2] * 8.0)),
		};
		static_assert(sizeof(arr) == 6);
		auto const p = gUranusCollection.pfnSZ_GetSpace(gpMsgBuffer, sizeof(arr));

		std::memcpy(p, &arr[0], sizeof(arr));
	}
	else if constexpr (std::is_same_v<T, Vector2D>)
	{
		std::array const arr{
			std::bit_cast<uint16_t>((int16_t)std::lround(arg[0] * 8.0)),
			std::bit_cast<uint16_t>((int16_t)std::lround(arg[1] * 8.0)),
		};
		static_assert(sizeof(arr) == 4);
		auto const p = gUranusCollection.pfnSZ_GetSpace(gpMsgBuffer, sizeof(arr));

		std::memcpy(p, &arr[0], sizeof(arr));
	}
	else if constexpr (std::is_same_v<T, msg_angle_t>)
	{
		auto const p = reinterpret_cast<uint8_t*>(gUranusCollection.pfnSZ_GetSpace(gpMsgBuffer, 1));
		auto const val = std::fmodf(arg.m_angle_data, 360.f) * 255.f / 360.f;
		*p = static_cast<uint8_t>(std::lroundf(val));
	}

	// general cases

	else if constexpr (sizeof(T) <= 4 && !std::is_pointer_v<T>)
	{
		auto const p = gUranusCollection.pfnSZ_GetSpace(gpMsgBuffer, sizeof(T));
		std::memcpy(p, std::addressof(arg), sizeof(T));
	}

#endif
	else
		static_assert(false, "Invalid argument!");
};

export __forceinline void MsgEnd(void) noexcept
{
	g_engfuncs.pfnMessageEnd();
}

export template <uint32_t iScale>
inline uint16_t ScaledFloat(double fl) noexcept
{
	return (unsigned short)std::lround(std::clamp<double>(fl * iScale, 0, 0xFFFF));
}

export template <uint32_t iScale>
inline void WriteScaledFloat(double fl) noexcept
{
	auto const val = (uint16_t)std::lround(std::clamp<double>(fl * iScale, 0, 0xFFFF));
	WriteData(val);
}

struct StringLiteral
{
	// Constructor
#ifdef __INTELLISENSE__
	template <size_t N> constexpr StringLiteral(const char(&str)[N]) noexcept {}	// Fucking buggy intellisense. #MSVC_BUG_INTELLISENSE
#else
	template <size_t N> constexpr StringLiteral(const char(&str)[N]) noexcept : m_psz(str), m_length(N) {}
#endif
	constexpr StringLiteral(StringLiteral const&) noexcept = default;
	constexpr StringLiteral& operator= (StringLiteral const&) noexcept = default;

	// Operators
	constexpr operator const char *() const noexcept { return m_psz; }	// automatically convert to char* if necessary.
	constexpr char operator[] (size_t index) const noexcept { assert(index < m_length); return m_psz[index]; }

	char const *m_psz{};
	size_t m_length{};
};

export template <StringLiteral _name, typename... Tys>
struct Message_t final
{
	// Reflection
	using MsgArgs_t = std::tuple<Tys...>;
	using This_t = Message_t<_name, Tys...>;

	// Constants
	static inline constexpr auto NAME = _name;	// Convertible to char* at anytime.
	static inline constexpr auto COUNT = sizeof...(Tys);
	static inline constexpr bool HAS_STRING = AnySame<const char*, Tys...>;	// Once a string is placed, there will be no chance for a constant length message.
	static inline constexpr auto SIZE = ((std::same_as<Tys, Vector> || std::same_as<Tys, Angles> ? 6u : std::same_as<Tys, Vector2D> ? 4u : sizeof(Tys)) + ... + 0);	// The 3d vector uses WRITE_COORD, hence it has total size of 3*2=6 instead of 3*4=12.

	// Constrains
	static_assert(_name.m_length <= 12U, "Name of message must less than 11 characters.");	// one extra for '\0'
	static_assert(SIZE < 192U, "The size of entire message must less than 192 bytes.");

	// Members
	static inline ESvcCommands m_iMessageIndex = (ESvcCommands)0;	// initialize with SVC_BAD

	// Methods
	static void Register(void) noexcept
	{
		if (m_iMessageIndex)
			return;

		if constexpr (HAS_STRING)
			m_iMessageIndex = g_engfuncs.pfnRegUserMsg(NAME, -1);	// Any length.	(Written bytes unchecked by engine)
		else
			m_iMessageIndex = g_engfuncs.pfnRegUserMsg(NAME, SIZE);	// No message arg case is included.
	}

#ifdef USING_METAMOD
	static inline void Retrieve(void) noexcept
	{
		if (m_iMessageIndex)
			return;

#ifdef _DEBUG
		int iSize = 0;
		m_iMessageIndex = gpMetaUtilFuncs->pfnGetUserMsgID(PLID, NAME, &iSize);

		assert(iSize == -1 || iSize == SIZE);
#else
		m_iMessageIndex = gpMetaUtilFuncs->pfnGetUserMsgID(PLID, NAME, nullptr);
#endif
	}
#endif

	template <MSG_DEST iDest>
	static void Marshalled(const Vector &vecOrigin, edict_t *pClient, Tys const&... args) noexcept
	{
		assert(m_iMessageIndex > 0);

		if constexpr (iDest == MSG_ONE || iDest == MSG_ONE_UNRELIABLE || iDest == MSG_INIT)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, pClient);
		else if constexpr (iDest == MSG_ALL || iDest == MSG_BROADCAST || iDest == MSG_SPEC)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, nullptr);
		else if constexpr (iDest == MSG_PAS || iDest == MSG_PAS_R || iDest == MSG_PVS || iDest == MSG_PVS_R)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, vecOrigin, nullptr);
		else
			static_assert(false, "Invalid message casting method!");

		(WriteData(args), ...);

		g_engfuncs.pfnMessageEnd();
	}

	template <MSG_DEST iDest>
	static void Unmanaged(const Vector &vecOrigin, edict_t *pClient, auto&&... args) noexcept
	{
		assert(m_iMessageIndex > 0);

		if constexpr (iDest == MSG_ONE || iDest == MSG_ONE_UNRELIABLE || iDest == MSG_INIT)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, pClient);
		else if constexpr (iDest == MSG_ALL || iDest == MSG_BROADCAST || iDest == MSG_SPEC)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex);
		else if constexpr (iDest == MSG_PAS || iDest == MSG_PAS_R || iDest == MSG_PVS || iDest == MSG_PVS_R)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, vecOrigin);
		else
			static_assert(false, "Invalid message casting method!");

		(WriteData(std::forward<decltype(args)>(args)), ...);

		g_engfuncs.pfnMessageEnd();
	}

	static inline void Send(edict_t *pClient, const Tys&... args) noexcept { return Marshalled<MSG_ONE>(Vector::Zero(), pClient, args...); }
	template <MSG_DEST _dest> static inline void Broadcast(const Tys&... args) noexcept { return Marshalled<_dest>(Vector::Zero(), nullptr, args...); }
	template <MSG_DEST _dest> static inline void Region(const Vector &vecOrigin, const Tys&... args) noexcept { return Marshalled<_dest>(vecOrigin, nullptr, args...); }

	// Client side stuff.

	using pfnReceiver_t = void(*)(std::conditional_t<sizeof(Tys) <= sizeof(std::uintptr_t), Tys, Tys const &>...) noexcept;

	static void Receiver(std::conditional_t<sizeof(Tys) <= sizeof(std::uintptr_t), Tys, Tys const &>...) noexcept;	// #UPDATE_AT_MSVC_FIX cannot specialize on 19.35

	/* #NO_URGENT
	static void Parse(void)
	{
		auto fnRead = [](auto &val)
		{
			using T = std::decay_t<decltype(val)>;

			if constexpr (std::is_same_v<T, int>)
				val = std::numeric_limits<int>::max();
			else if constexpr (std::is_same_v<T, float>)
				val = std::numeric_limits<float>::max();
			else if constexpr (std::is_same_v<T, const char *>)
			{
				constexpr auto str = "3.1415926535";	// strdup.
				val = str;
			}
			else if constexpr (std::is_same_v<T, Vector>)
				val = Vector(3, 4, 5);
			else if constexpr (std::is_same_v<T, short>)
				val = std::numeric_limits<short>::max();
			else if constexpr (std::is_same_v<T, BYTE>)
				val = std::numeric_limits<BYTE>::max();
			else
				static_assert(always_false_v<T>, "non-exhaustive visitor!");
		};

		MsgArgs_t tplArgs;
		auto fnFill = [&]<size_t... I>(std::index_sequence<I...>)
		{
			(fnRead(std::get<I>(tplArgs)), ...);
		};

		auto fnApply = [&tplArgs]<size_t... I>(std::index_sequence<I...>)	// Fuck the std::apply. It totally does not work.
		{
			MsgReceived<This_t, ArgTys...>(std::get<I>(tplArgs)...);
		};

		fnFill(IDX_SEQ);
		fnApply(IDX_SEQ);
	}
	*/
};

#ifdef USING_METAMOD

export using gmsgBarTime = Message_t<"BarTime", int16_t>;
export using gmsgBlinkAcct = Message_t<"BlinkAcct", uint8_t/*numBlinks*/>;
export using gmsgBrass = Message_t<"Brass", Vector/*origin*/, Vector/*velocity*/, msg_angle_t/*rotation*/, int16_t/*model*/, uint8_t/*soundtype*/, uint8_t/*entityIndex*/>;
export using gmsgCurWeapon = Message_t<"CurWeapon", uint8_t/*state*/, uint8_t/*iId*/, uint8_t/*iClip*/>;
export using gmsgHudText = Message_t<"HudTextPro", const char*/*message*/, uint8_t/*is_hint*/>;	// Arkshine: not usable except build-in texts.
export using gmsgReloadSound = Message_t<"ReloadSound", uint8_t/*volume*/, uint8_t/*bIsShotgun*/>;
export using gmsgRoundTime = Message_t<"RoundTime", uint16_t/*countdown*/>;
export using gmsgScreenFade = Message_t<"ScreenFade", uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t>;
export using gmsgScreenShake = Message_t<"ScreenShake", uint16_t, uint16_t, uint16_t>;
export using gmsgShowMenu = Message_t<"ShowMenu", uint16_t, int8_t, uint8_t, const char*>;
export using gmsgTextMsg = Message_t<"TextMsg", uint8_t, const char*>;	// 4 args more actually, but whatever.
export using gmsgWeapPickup = Message_t<"WeapPickup", uint8_t>;
export using gmsgWeaponAnim = Message_t<"WeapAnim", uint8_t, uint8_t>;	// actually no such message exist. pure wrapper.
export using gmsgWeaponList = Message_t<"WeaponList", const char*/*name*/, uint8_t/*prim ammo*/, uint8_t/*prim ammo max*/, uint8_t/*sec ammo*/, uint8_t/*sec ammo max*/, uint8_t/*slot id*/, uint8_t/*order in slot*/, uint8_t/*iId*/, uint8_t/*flags*/>;

// Goal: Retrieve commonly used messages.
// Call once in ServerActivate_Post.
export void RetrieveMessageHandles(void) noexcept
{
	gmsgBarTime::Retrieve();
	gmsgBlinkAcct::Retrieve();
	gmsgBrass::Retrieve();
	gmsgCurWeapon::Retrieve();
	gmsgHudText::Retrieve();
	gmsgReloadSound::Retrieve();
	gmsgRoundTime::Retrieve();
	gmsgScreenFade::Retrieve();
	gmsgScreenShake::Retrieve();
	gmsgShowMenu::Retrieve();
	gmsgTextMsg::Retrieve();
	gmsgWeapPickup::Retrieve();
	gmsgWeaponList::Retrieve();

	gmsgWeaponAnim::m_iMessageIndex = SVC_WEAPONANIM;

#ifdef USING_BUILTIN_WRITING
	if (g_engfuncs.pfnWriteByte != nullptr) [[likely]]
	{
		// It's 0x1C for 3266.
		static constexpr auto GLB_MSG_BUF_OFS = 0x101E596D - 0x101E5950;	// 9980, HL25
		gpMsgBuffer = UTIL_RetrieveGlobalVariable<sizebuf_t>(g_engfuncs.pfnWriteByte, GLB_MSG_BUF_OFS);

		assert(gpMsgBuffer && std::strcmp(gpMsgBuffer->buffername, "MessageBegin/End") == 0);
	}
#endif
}

#endif
