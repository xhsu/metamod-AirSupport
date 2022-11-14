export module Task;

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
	unsigned long m_iCoroutineMarker{ 0ul };	// LUNA: use for marking the removal of coroutine

	explicit Task(promise_type *ppt) noexcept : m_handle{ Handle_t::from_promise(*ppt) } {}	// Get handle and form the promise
	Task(Task &&rhs) noexcept : m_handle{ std::exchange(rhs.m_handle, nullptr) }, m_iCoroutineMarker{ rhs.m_iCoroutineMarker } {}	// Move only
	~Task() noexcept { if (m_handle) m_handle.destroy(); }

	__forceinline bool Done(void) const noexcept { return m_handle.done(); }
	__forceinline bool ShouldResume(void) const noexcept { return m_handle.promise().m_flNextThink < gpGlobals->time; }
	__forceinline void Resume(void) const noexcept { return m_handle.resume(); }
	__forceinline auto operator<=> (Task const &rhs) const noexcept { return this->m_handle.promise().m_flNextThink <=> rhs.m_handle.promise().m_flNextThink; }
};

export namespace TaskScheduler
{
	inline list<Task> m_List{};

	inline void Think(void) noexcept
	{
		if (m_List.empty())
			return;

		while (!m_List.empty())
		{
			m_List.sort();

			while (m_List.front().Done())
			{
				m_List.pop_front();
			}

			[[likely]]
			if (m_List.front().ShouldResume())
				m_List.front().Resume();
			else
				break;	// if the first one in queue is not going to resume, then nor should anyone else.
		}
	}

	inline void Enroll(Task &&obj, unsigned long iCoroutineMarker = 0ul) noexcept
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

	inline unsigned int Delist(unsigned long iCoroutineMarker) noexcept
	{
		if (!m_List.empty())
			return m_List.remove_if([=](Task const &obj) noexcept { return obj.m_iCoroutineMarker == iCoroutineMarker; });

		return 0;
	}

	inline unsigned int Count(unsigned long iCoroutineMarker) noexcept
	{
		unsigned int ret{};

		for (auto const& obj : m_List)
		{
			if (obj.m_iCoroutineMarker == iCoroutineMarker)
				++ret;
		}

		return ret;
	}

	inline bool Exist(unsigned long iCoroutineMarker) noexcept
	{
		for (auto const &obj : m_List)
		{
			[[unlikely]]
			if (obj.m_iCoroutineMarker == iCoroutineMarker)
				return true;
		}

		return false;
	}

	inline void Clear(void) noexcept
	{
		m_List.clear();
	}
};
