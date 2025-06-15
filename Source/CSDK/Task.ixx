export module Task;

import std;
import hlsdk;

import Platform;

using namespace std;

export struct Task final
{
	struct promise_type final // FIXED NAME
	{
		float m_flNextThink{ gpGlobals->time + 3600 };

		// Fixed name. What to do in a case of an exception.
		static void unhandled_exception(void) noexcept
		{
			try
			{
				if (auto pEx = std::current_exception(); pEx)
					std::rethrow_exception(pEx);
			}
			catch (const std::exception &e)
			{
				UTIL_Terminate("Unhandled exception on Task: %s", e.what());
			}
			catch (...)
			{
				UTIL_Terminate("Unhandled exception on Task with unknown type.");
			}
		}

		// Fixed name. Coroutine creation.
		Task get_return_object(void) noexcept { return Task{ this }; }

		// Fixed name. Startup.
		static constexpr suspend_never initial_suspend(void) noexcept { return {}; }

		// Fixed name. Value from co_yield
		suspend_always yield_value(float flTimeFrame) noexcept { m_flNextThink = gpGlobals->time + flTimeFrame; return {}; }

		// Fixed name. Value from co_await
		suspend_always await_transform(float flTimeFrame) noexcept { m_flNextThink = gpGlobals->time + flTimeFrame; return {}; }

		//static constexpr void return_value(void) noexcept {}	// Fixed name. Value from co_return. LUNA: Mutually exclusive with return_void?
		static constexpr void return_void(void) noexcept {}	// Fixed name. Value from co_return. LUNA: Mutually exclusive with return_value?

		// Ending. LUNA: must be suspend_always otherwise it will cause memory access violation: accessing freed mem.
		static constexpr suspend_always final_suspend(void) noexcept { return {}; }
	};

	using Handle_t = std::coroutine_handle<promise_type>;
	Handle_t m_handle;
	uint64_t m_iCoroutineMarker{};	// LUNA: use for marking the removal of coroutine

	explicit Task(promise_type *ppt) noexcept
		: m_handle{ Handle_t::from_promise(*ppt) } {}	// Get handle and form the promise
	Task(Task const&) noexcept = delete;	// One cannot copy a task.
	Task(Task &&rhs) noexcept
		: m_handle{ std::exchange(rhs.m_handle, nullptr) }, m_iCoroutineMarker{ rhs.m_iCoroutineMarker } {}	// Move only
	~Task() noexcept { if (m_handle) m_handle.destroy(); }

	inline bool Done(void) const noexcept { return m_handle.done(); }
	inline bool ShouldResume(void) const noexcept { return m_handle.promise().m_flNextThink < gpGlobals->time; }
	inline void Resume(void) const noexcept { return m_handle.resume(); }
	inline auto operator<=> (Task const& rhs) const noexcept -> decltype(0.f <=> 0.f) { return this->m_handle.promise().m_flNextThink <=> rhs.m_handle.promise().m_flNextThink; }
};

export enum struct ESchedulerPolicy : bool
{
	SORTED,
	UNORDERED,
};

export struct TaskScheduler_t final
{
	list<Task> m_List{};
	ESchedulerPolicy m_ExePolicy{ ESchedulerPolicy::SORTED };

	inline void Think(void) noexcept
	{
		switch (m_ExePolicy)
		{
			// Price for ordered execution in the same frame: O(nlogn)
		case ESchedulerPolicy::SORTED:
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
			break;

			// There are some cases that execution order in the same frame does not matter.
		case ESchedulerPolicy::UNORDERED:
			for (auto it = m_List.begin(); it != m_List.end(); /* Does nothing */)
			{
				if (it->ShouldResume())
					it->Resume();

				if (it->Done())
					it = m_List.erase(it);
				else
					++it;
			}
			break;

		default:
			std::unreachable();
			break;
		}
	}

	inline void Enroll(Task &&obj, uint64_t iCoroutineMarker = 0ull, bool bExclusiveMode = false, bool bUsingBits = true) noexcept
	{
		if (bExclusiveMode)
		{
			Delist(iCoroutineMarker, bUsingBits);
		}

		[[likely]]
		if (!obj.Done())
		{
			obj.m_iCoroutineMarker = iCoroutineMarker;

			m_List.emplace_back(std::forward<Task>(obj));
			m_List.sort();
			m_List.remove_if(std::bind_front(&Task::Done));
		}
	}

	inline size_t Delist(uint64_t iCoroutineMarker, bool bUsingBits = true) noexcept
	{
		if (!m_List.empty())
		{
			if (bUsingBits)
				return m_List.remove_if([=](Task const &obj) noexcept { return (obj.m_iCoroutineMarker & iCoroutineMarker) != 0; });
			else
				return m_List.remove_if([=](Task const &obj) noexcept { return obj.m_iCoroutineMarker == iCoroutineMarker; });
		}

		return 0;
	}

	inline size_t Count(uint64_t iCoroutineMarker, bool bUsingBits = true) noexcept
	{
		size_t ret{};

		if (bUsingBits)
		{
			for (auto const &obj : m_List)
			{
				if ((obj.m_iCoroutineMarker & iCoroutineMarker) != 0)
					++ret;
			}
		}
		else
		{
			for (auto const &obj : m_List)
			{
				if (obj.m_iCoroutineMarker == iCoroutineMarker)
					++ret;
			}
		}

		return ret;
	}

	inline bool Exist(uint64_t iCoroutineMarker, bool bUsingBits = true) noexcept
	{
		if (bUsingBits)
		{
			for (auto const &obj : m_List)
			{
				[[unlikely]]
				if ((obj.m_iCoroutineMarker & iCoroutineMarker) != 0)
					return true;
			}
		}
		else
		{
			for (auto const &obj : m_List)
			{
				[[unlikely]]
				if (obj.m_iCoroutineMarker == iCoroutineMarker)
					return true;
			}
		}

		return false;
	}

	inline void Clear(void) noexcept
	{
		m_List.clear();
	}

	inline auto Policy(void) noexcept -> decltype((m_ExePolicy))
	{
		return m_ExePolicy;
	}
};

export namespace TaskScheduler
{
	inline TaskScheduler_t m_GlobalScheduler{};

	// StartFrame_Post
	inline auto Think(void) noexcept -> decltype(m_GlobalScheduler.Think()) { return m_GlobalScheduler.Think(); }

	inline decltype(auto) Enroll(Task &&obj, uint64_t iCoroutineMarker = 0ull, bool bExclusiveMode = false, bool bUsingBits = true) noexcept { return m_GlobalScheduler.Enroll(std::forward<Task>(obj), iCoroutineMarker, bExclusiveMode, bUsingBits); }

	inline decltype(auto) Delist(uint64_t iCoroutineMarker, bool bUsingBits = true) noexcept { return m_GlobalScheduler.Delist(iCoroutineMarker, bUsingBits); }

	inline decltype(auto) Count(uint64_t iCoroutineMarker, bool bUsingBits = true) noexcept { return m_GlobalScheduler.Count(iCoroutineMarker, bUsingBits); }

	inline decltype(auto) Exist(uint64_t iCoroutineMarker, bool bUsingBits = true) noexcept { return m_GlobalScheduler.Exist(iCoroutineMarker, bUsingBits); }

	// ServerDeactivate_Post
	inline auto Clear(void) noexcept -> decltype(m_GlobalScheduler.Clear()) { return m_GlobalScheduler.Clear(); }

	inline decltype(auto) Policy() noexcept { return m_GlobalScheduler.Policy(); }
};

export namespace TaskScheduler::NextFrame
{
	inline constexpr array Rank
	{
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
