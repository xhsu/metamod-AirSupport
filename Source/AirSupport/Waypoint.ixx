export module Waypoint;

export import <span>;
export import <vector>;

export import vector;

using std::span;
using std::vector;

export inline constexpr size_t TOTAL_SCANS = (8192 / 32) * (8192 / 32) * (8192 / 72) + 4;
export inline constexpr size_t WAYPOINT_FMT_VER = 1;

export inline vector<Vector> g_rgvecValidJetSpawn = {};

export inline struct WaypointMgr_t
{
	size_t m_iVersion = WAYPOINT_FMT_VER;
	size_t m_iCount = 0;
	Vector *m_prgvecOrigins = nullptr;
	bool m_bPointerOwnership = false;
	span<Vector> m_rgvecOrigins = g_rgvecValidJetSpawn;
}
g_WaypointMgr = {};
