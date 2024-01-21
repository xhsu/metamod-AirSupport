/*

Created Date: Aug 29 2021

Modern Warfare Dev Team
Programmer - Luna the Reborn

*/
module;

#define USING_METAMOD

#ifdef __INTELLISENSE__
#include <cassert>
#include <cstdint>

#include <bit>
#endif

export module Message;

#ifndef __INTELLISENSE__
export import <cassert>;
export import <cstdint>;
#endif

export import util;
export import vector;

#ifdef USING_METAMOD
export import Plugin;
#endif

export import UtlConcepts;

using std::bit_cast;

// A wrapper that force pfnWriteAngle
export struct msg_angle_t final
{
	constexpr msg_angle_t(float val) noexcept : m_angle_data{ val } {}
	float m_angle_data{};
};

export inline void MsgSend(entvars_t *pev, int iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_ONE, iMessageIndex, nullptr, ent_cast<edict_t *>(pev));
}

export inline void MsgAll(int iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_ALL, iMessageIndex, nullptr, nullptr);
}

export inline void MsgBroadcast(int iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_BROADCAST, iMessageIndex, nullptr, nullptr);
}

export inline void MsgPVS(int iMessageIndex, const Vector &vecOrigin) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_PVS, iMessageIndex, vecOrigin, nullptr);
}

export inline void WriteData(auto&& arg) noexcept
{
	using T = std::remove_cvref_t<std::decay_t<decltype(arg)>>;

	// some special objects.

	if constexpr (std::is_convertible_v<T, std::string_view>)
		g_engfuncs.pfnWriteString(arg);
	else if constexpr (std::is_same_v<T, Vector> || std::is_same_v<T, Angles>)
	{
		g_engfuncs.pfnWriteCoord(arg[0]);
		g_engfuncs.pfnWriteCoord(arg[1]);
		g_engfuncs.pfnWriteCoord(arg[2]);
	}
	else if constexpr (std::is_same_v<T, msg_angle_t>)
		g_engfuncs.pfnWriteAngle(arg.m_angle_data);

	// general cases

	else if constexpr (sizeof(T) == 4)	// long, int, float
		g_engfuncs.pfnWriteLong(bit_cast<int>(arg));

	else if constexpr (sizeof(T) == 2)	// short, char16_t
		g_engfuncs.pfnWriteShort(bit_cast<short>(arg));

	else if constexpr (std::is_same_v<T, bool> || (std::is_unsigned_v<T> && sizeof(T) == 1))	// bool, uchar, byte
		g_engfuncs.pfnWriteByte(bit_cast<unsigned char>(arg));
	else if constexpr (sizeof(T) == 1)	// signed char
		g_engfuncs.pfnWriteChar(arg);

	else
		static_assert(AlwaysFalse<T>, "Invalid argument!");
};

export __forceinline void MsgEnd(void) noexcept
{
	g_engfuncs.pfnMessageEnd();
}

export template <unsigned int iScale>
inline unsigned short ScaledFloat(double fl) noexcept
{
	return (unsigned short)std::round(std::clamp<double>(fl * iScale, 0, 0xFFFF));
}

export template <unsigned int iScale>
inline void WriteScaledFloat(double fl) noexcept
{
	auto const val = (unsigned short)std::round(std::clamp<double>(fl * iScale, 0, 0xFFFF));
	WriteData(val);
}

struct StringLiteral
{
	// Constructor
	template <size_t N> constexpr StringLiteral(const char(&str)[N]) noexcept : m_psz(str), m_length(N) {}
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
	static inline constexpr bool HAS_VECTOR = AnySame<Vector, Tys...> || AnySame<Angles, Tys...>;	// The vector uses WRITE_COORD, hence it has total size of 3*2=6 instead of 3*4=12.
	static inline constexpr auto SIZE = (sizeof(Tys) + ... + 0);
	static inline constexpr auto IDX_SEQ = std::index_sequence_for<Tys...>{};

	// Constrains
	static_assert(_name.m_length <= 12U, "Name of message must less than 11 characters.");	// one extra for '\0'
	static_assert(SIZE < 192U, "The size of entire message must less than 192 bytes.");

	// Members
	static inline int m_iMessageIndex = 0;

	// Methods
	static void Register(void) noexcept
	{
		if (m_iMessageIndex)
			return;

		if constexpr (HAS_STRING || HAS_VECTOR)
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

	template <int iDest>
	static void Marshalled(const Vector &vecOrigin, edict_t *pClient, const Tys&... args) noexcept
	{
		assert(m_iMessageIndex > 0);

		if constexpr (iDest == MSG_ONE || iDest == MSG_ONE_UNRELIABLE || iDest == MSG_INIT)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, pClient);
		else if constexpr (iDest == MSG_ALL || iDest == MSG_BROADCAST || iDest == MSG_SPEC)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, nullptr);
		else if constexpr (iDest == MSG_PAS || iDest == MSG_PAS_R || iDest == MSG_PVS || iDest == MSG_PVS_R)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, vecOrigin, nullptr);
		//else
		//	[]() noexcept { static_assert(AlwaysFalse<This_t>, "Invalid message casting method!"); }();	// #UPDATE_AT_CPP23 CWG2518 allowing static_assert(false)

		MsgArgs_t tplArgs = std::make_tuple(args...);

		// No panic, this is a instant-called lambda function.
		// De facto static_for. #UPDATE_AT_CPP26 pack indexing?
		[&]<size_t... I>(std::index_sequence<I...>) noexcept
		{
			(WriteData(std::get<I>(tplArgs)), ...);
		}
		(IDX_SEQ);

		g_engfuncs.pfnMessageEnd();
	}

	template <int iDest, typename... Ts>
	static void Unmanaged(const Vector &vecOrigin, edict_t *pClient, const Ts&... args) noexcept
	{
		assert(m_iMessageIndex > 0);

		if constexpr (iDest == MSG_ONE || iDest == MSG_ONE_UNRELIABLE || iDest == MSG_INIT)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, pClient);
		else if constexpr (iDest == MSG_ALL || iDest == MSG_BROADCAST || iDest == MSG_SPEC)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex);
		else if constexpr (iDest == MSG_PAS || iDest == MSG_PAS_R || iDest == MSG_PVS || iDest == MSG_PVS_R)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, vecOrigin);
		//else
		//	[]() noexcept { static_assert(AlwaysFalse<This_t>, "Invalid message casting method!"); }();	// #UPDATE_AT_CPP23 CWG2518 allowing static_assert(false)

		auto const tplArgs = std::make_tuple(args...);

		// No panic, this is a instant-called lambda function.
		// De facto static_for.
		[&]<size_t... I>(std::index_sequence<I...>) noexcept
		{
			(WriteData(std::get<I>(tplArgs)), ...);
		}
		(std::index_sequence_for<Ts...>{});

		g_engfuncs.pfnMessageEnd();
	}

	static inline void Send(edict_t *pClient, const Tys&... args) noexcept { return Marshalled<MSG_ONE>(Vector::Zero(), pClient, args...); }
	template <int _dest> static inline void Broadcast(const Tys&... args) noexcept { return Marshalled<_dest>(Vector::Zero(), nullptr, args...); }
	template <int _dest> static inline void Region(const Vector &vecOrigin, const Tys&... args) noexcept { return Marshalled<_dest>(vecOrigin, nullptr, args...); }

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
export using gmsgBrass = Message_t<"Brass", Vector/*origin*/, Vector/*velocity*/, msg_angle_t/*rotation*/, int16_t/*model*/, uint8_t/*soundtype*/, uint8_t/*entityIndex*/>;
export using gmsgCurWeapon = Message_t<"CurWeapon", uint8_t/*state*/, uint8_t/*iId*/, uint8_t/*iClip*/>;
export using gmsgScreenFade = Message_t<"ScreenFade", uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t>;
export using gmsgScreenShake = Message_t<"ScreenShake", uint16_t, uint16_t, uint16_t>;
export using gmsgShowMenu = Message_t<"ShowMenu", uint16_t, int8_t, uint8_t, const char*>;
export using gmsgTextMsg = Message_t<"TextMsg", uint8_t, const char*>;	// 4 args more actually, but whatever.
export using gmsgWeapPickup = Message_t<"WeapPickup", uint8_t>;
export using gmsgWeaponAnim = Message_t<"WeapAnim", uint8_t, uint8_t>;	// actually no such message exist. pure wrapper.
export using gmsgWeaponList = Message_t<"WeaponList", const char*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>;


// Goal: Retrieve commonly used messages.
// Call once in ServerActivate_Post.
export void RetrieveMessageHandles(void) noexcept
{
	gmsgBarTime::Retrieve();
	gmsgBrass::Retrieve();
	gmsgCurWeapon::Retrieve();
	gmsgScreenFade::Retrieve();
	gmsgScreenShake::Retrieve();
	gmsgShowMenu::Retrieve();
	gmsgTextMsg::Retrieve();
	gmsgWeapPickup::Retrieve();
	gmsgWeaponList::Retrieve();

	gmsgWeaponAnim::m_iMessageIndex = SVC_WEAPONANIM;
}

#endif
