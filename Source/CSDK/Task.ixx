export module Task;

export import <cstdint>;

export import <array>;
import <coroutine>;
import <functional>;
import <list>;

import progdefs;

using namespace std;

export struct Task final
{
	struct promise_type final // FIXED NAME
	{
		float m_flNextThink{ gpGlobals->time + 3600 };

		static constexpr void unhandled_exception(void) noexcept {}	// Fixed name. What to do in a case of an exception.
		Task get_return_object(void) noexcept { return Task{ this }; }	// Fixed name. Coroutine creation.
		static constexpr suspend_never initial_suspend(void) noexcept { return {}; }	// Fixed name. Startup.
		suspend_always yield_value(float flTimeFrame) noexcept { m_flNextThink = gpGlobals->time + flTimeFrame; return {}; }	// Fixed name. Value from co_yield
		suspend_always await_transform(float flTimeFrame) noexcept { m_flNextThink = gpGlobals->time + flTimeFrame; return {}; }	// Fixed name. Value from co_await
		//static constexpr void return_value(void) noexcept {}	// Fixed name. Value from co_return. LUNA: Mutually exclusive with return_void?
		static constexpr void return_void(void) noexcept {}	// Fixed name. Value from co_return. LUNA: Mutually exclusive with return_value?
		static constexpr suspend_always final_suspend(void) noexcept { return {}; }	// Ending. LUNA: must be suspend_always otherwise it will cause memory access violation: accessing freed mem.
	};

	using Handle_t = std::coroutine_handle<promise_type>;
	Handle_t m_handle;
	uint64_t m_iCoroutineMarker{};	// LUNA: use for marking the removal of coroutine

	explicit Task(promise_type *ppt) noexcept : m_handle{ Handle_t::from_promise(*ppt) } {}	// Get handle and form the promise
	Task(Task &&rhs) noexcept : m_handle{ std::exchange(rhs.m_handle, nullptr) }, m_iCoroutineMarker{ rhs.m_iCoroutineMarker } {}	// Move only
	~Task() noexcept { if (m_handle) m_handle.destroy(); }

	__forceinline bool Done(void) const noexcept { return m_handle.done(); }
	__forceinline bool ShouldResume(void) const noexcept { return m_handle.promise().m_flNextThink < gpGlobals->time; }
	__forceinline void Resume(void) const noexcept { return m_handle.resume(); }
	__forceinline auto operator<=> (Task const &rhs) const noexcept { return this->m_handle.promise().m_flNextThink <=> rhs.m_handle.promise().m_flNextThink; }
};

export struct TaskScheduler_t final
{
	list<Task> m_List{};

	inline void Think(void) noexcept
	{
		while (!m_List.empty())
		{
			m_List.sort();

			while (!m_List.empty() && m_List.front().Done())
			{
				m_List.pop_front();
			}

			if (m_List.empty())
				break;

			[[likely]]
			if (m_List.front().ShouldResume())
				m_List.front().Resume();
			else
				break;	// if the first one in queue is not going to resume, then nor should anyone else.
		}
	}

	inline void Enroll(Task &&obj, uint64_t iCoroutineMarker = 0ull) noexcept
	{
		[[likely]]
		if (!obj.Done())
		{
			obj.m_iCoroutineMarker = iCoroutineMarker;

			m_List.emplace_back(std::forward<Task>(obj));
			m_List.sort();
			m_List.remove_if(std::bind_front(&Task::Done));
		}
	}

	inline size_t Delist(uint64_t iCoroutineMarker) noexcept
	{
		if (!m_List.empty())
			return m_List.remove_if([=](Task const &obj) noexcept { return (obj.m_iCoroutineMarker & iCoroutineMarker) != 0; });

		return 0;
	}

	inline size_t Count(uint64_t iCoroutineMarker) noexcept
	{
		size_t ret{};

		for (auto const &obj : m_List)
		{
			if ((obj.m_iCoroutineMarker & iCoroutineMarker) != 0)
				++ret;
		}

		return ret;
	}

	inline bool Exist(uint64_t iCoroutineMarker) noexcept
	{
		for (auto const &obj : m_List)
		{
			[[unlikely]]
			if ((obj.m_iCoroutineMarker & iCoroutineMarker) != 0)
				return true;
		}

		return false;
	}

	inline void Clear(void) noexcept
	{
		m_List.clear();
	}
};

export namespace TaskScheduler
{
	inline TaskScheduler_t m_GlobalScheduler{};

	inline decltype(auto) Think(void) noexcept { return m_GlobalScheduler.Think(); }

	inline decltype(auto) Enroll(Task &&obj, uint64_t iCoroutineMarker = 0ull) noexcept { return m_GlobalScheduler.Enroll(std::forward<Task>(obj), iCoroutineMarker); }

	inline decltype(auto) Delist(uint64_t iCoroutineMarker) noexcept { return m_GlobalScheduler.Delist(iCoroutineMarker); }

	inline decltype(auto) Count(uint64_t iCoroutineMarker) noexcept { return m_GlobalScheduler.Count(iCoroutineMarker); }

	inline decltype(auto) Exist(uint64_t iCoroutineMarker) noexcept { return m_GlobalScheduler.Exist(iCoroutineMarker); }

	inline decltype(auto) Clear(void) noexcept { return m_GlobalScheduler.Clear(); }
};

export namespace TaskScheduler::NextFrame
{
	inline constexpr array Rank{
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon(),
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 2.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 3.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 4.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 5.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 6.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 7.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 8.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 9.f,
		std::numeric_limits<decltype(gpGlobals->time)>::epsilon() * 10.f,
	};
};
