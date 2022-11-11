/*

Created Date: Aug 29 2021

Modern Warfare Dev Team
Programmer - Luna the Reborn

*/
module;

#define USING_METAMOD

#include <cassert>

export module Message;

import <cstdint>;

import <algorithm>;
import <array>;
import <bit>;
import <functional>;
import <numeric>;
import <string>;
import <tuple>;
import <type_traits>;
import <utility>;

import util;
import vector;

#ifdef USING_METAMOD
import Plugin;
#endif

import UtlConcepts;

using std::bit_cast;

export inline void MsgSend(entvars_t *pev, int iMessageIndex) noexcept
{
	g_engfuncs.pfnMessageBegin(MSG_ONE, iMessageIndex, nullptr, ent_cast<edict_t *>(pev));
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
	else if constexpr (std::is_same_v<T, Vector>)
	{
		g_engfuncs.pfnWriteCoord(arg.x);
		g_engfuncs.pfnWriteCoord(arg.y);
		g_engfuncs.pfnWriteCoord(arg.z);
	}

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
		static_assert(std::_Always_false<T>, "Invalid argument!");
};

export __forceinline void MsgEnd(void) noexcept
{
	g_engfuncs.pfnMessageEnd();
}

export template <std::size_t N>
struct StringLiteral
{
	// Constructor
	consteval StringLiteral(const char(&str)[N]) noexcept { std::copy_n(str, N, m_rgsz); }

	// Operators
	constexpr operator const char *() const noexcept { return &m_rgsz[0]; }	// automatically convert to char* if necessary.
	constexpr operator char *() noexcept { return &m_rgsz[0]; }
	constexpr decltype(auto) operator[] (std::size_t index) const noexcept { assert(index < N); return m_rgsz[index]; }

	static constexpr size_t length = N - 1U;	// Remove the '\0' at the end.
	char m_rgsz[N];
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
	static inline constexpr bool HAS_VECTOR = AnySame<Vector, Tys...>;	// The vector uses WRITE_COORD, hence it has total size of 3*2=6 instead of 3*4=12.
	static inline constexpr auto SIZE = (sizeof(Tys) + ... + 0);
	static inline constexpr auto IDX_SEQ = std::index_sequence_for<Tys...>{};

	// Constrains
	static_assert(sizeof(_name.length) <= 11U, "Name of message must less than 11 characters.");
	static_assert(SIZE <= 192U, "The size of entire message must less than 192 bytes.");

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
		m_iMessageIndex = gpMetaUtilFuncs->pfnGetUserMsgID(&gPluginInfo, NAME, &iSize);

		assert(iSize == -1 || iSize == SIZE);
#else
		m_iMessageIndex = gpMetaUtilFuncs->pfnGetUserMsgID(&gPluginInfo, NAME, nullptr);
#endif
	}
#endif

	template <int iDest>
	static void ManagedBegin(const Vector &vecOrigin, edict_t *pClient, const Tys&... args) noexcept
	{
		assert(m_iMessageIndex > 0);

		if constexpr (iDest == MSG_ONE || iDest == MSG_ONE_UNRELIABLE || iDest == MSG_INIT)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, nullptr, pClient);
		else if constexpr (iDest == MSG_ALL || iDest == MSG_BROADCAST || iDest == MSG_SPEC)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex);
		else if constexpr (iDest == MSG_PAS || iDest == MSG_PAS_R || iDest == MSG_PVS || iDest == MSG_PVS_R)
			g_engfuncs.pfnMessageBegin(iDest, m_iMessageIndex, vecOrigin);
		//else
		//	static_assert(std::_Always_false<This_t>, "Invalid message casting method!");	// #INVESTIGATE

		MsgArgs_t tplArgs = std::make_tuple(args...);

		// No panic, this is a instant-called lambda function.
		// De facto static_for.
		[&]<size_t... I>(std::index_sequence<I...>) noexcept
		{
			(WriteData(std::get<I>(tplArgs)), ...);
		}
		(IDX_SEQ);

		g_engfuncs.pfnMessageEnd();
	}

	static inline void Send(edict_t *pClient, const Tys&... args) noexcept { return ManagedBegin<MSG_ONE>(Vector::Zero(), pClient, args...); }
	template <int _dest> static inline void Broadcast(const Tys&... args) noexcept { return ManagedBegin<_dest>(Vector::Zero(), nullptr, args...); }
	template <int _dest> static inline void Region(const Vector &vecOrigin, const Tys&... args) noexcept { return ManagedBegin<_dest>(vecOrigin, nullptr, args...); }

	/* LUNA: clinet side stuff. #UNTESTED #UNDONE
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
