export module Task;

export import <functional>;
export import <list>;
export import <tuple>;

export import progdefs;

using std::function;
using std::list;
using std::pair;
using std::tuple;

export struct Tasks
{
	static void Think(void) noexcept
	{
		if (m_List.empty())
			return;

		[[unlikely]]
		if (std::get<0>(m_List.front()) < gpGlobals->time)
		{
			std::get<2>(m_List.front())();
			m_List.pop_front();
		}
	}

	static void Add(float flDelayExecute, unsigned int iTaskIndex, const function<void()> &fn) noexcept
	{
		m_List.emplace_back(
			flDelayExecute + gpGlobals->time,
			iTaskIndex,
			fn
		);

		m_List.sort([](auto const &lhs, auto const &rhs) noexcept { return std::get<0>(lhs) < std::get<0>(rhs); });
	}

	static void Remove(unsigned int iTaskIndex) noexcept
	{
		m_List.remove_if(
			[=](auto const &elem) noexcept
			{
				return std::get<1>(elem) == iTaskIndex;
			}
		);
	}

private:
	static inline list<tuple<float, unsigned int, function<void()>>> m_List{};
};
