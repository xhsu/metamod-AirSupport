#include <cassert>

import <cmath>;
import <cstdint>;
import <cstdio>;

import <algorithm>;
import <array>;
import <filesystem>;
import <format>;
import <numbers>;
import <span>;
import <vector>;

import eiface;
import progdefs;
import util;

import Task;
import Waypoint;

namespace fs = std::filesystem;

using std::array;
using std::span;
using std::vector;

Task Waypoint_Scan(void) noexcept
{
//	TimedFnMgr::Delist(RADIO_KEY * 2);

	Vector vecOrigin{};
	TraceResult tr{};
	size_t iCounter = 0;

	g_rgvecValidJetSpawn.clear();
	g_rgvecValidJetSpawn.reserve(TOTAL_SCANS);

	if (g_WaypointMgr.m_bPointerOwnership)
	{
		free(g_WaypointMgr.m_prgvecOrigins);
		g_WaypointMgr.m_prgvecOrigins = nullptr;

		g_WaypointMgr.m_iCount = 0;
		g_WaypointMgr.m_bPointerOwnership = false;
	}

	for (float X = -4096.f; X <= 4096.f; X += 32.f)
	{
		for (float Y = -4096.f; Y <= 4096.f; Y += 32.f)
		{
			for (float Z = -4096.f; Z <= 4096.f; Z += 72.f)
			{
				++iCounter;
				vecOrigin = Vector{ X, Y, Z };

				[[unlikely]]
				if (!(iCounter % 4096))
				{
					g_engfuncs.pfnServerPrint(std::format("[Waypoint_Scan] {:.2f}% done. {} valid spawn point found.\n", (double)iCounter / (double)TOTAL_SCANS * 100.0, g_rgvecValidJetSpawn.size()).c_str());
					co_await TaskScheduler::NextFrame::Rank[9];
				}

				if (g_engfuncs.pfnPointContents(vecOrigin) != CONTENTS_EMPTY)
					continue;

				g_engfuncs.pfnTraceLine(vecOrigin, Vector(X, Y, 4096.f), ignore_monsters | ignore_glass, nullptr, &tr);

				[[likely]]
				if (tr.fAllSolid || g_engfuncs.pfnPointContents(tr.vecEndPos) != CONTENTS_SKY)
					continue;

				g_rgvecValidJetSpawn.emplace_back(tr.vecEndPos);
			}
		}
	}

	std::ranges::sort(g_rgvecValidJetSpawn, [](const Vector &lhs, const Vector &rhs) noexcept { return lhs.LengthSquared() < rhs.LengthSquared(); });
	auto const [itFirst, itLast] = std::ranges::unique(g_rgvecValidJetSpawn);
	g_rgvecValidJetSpawn.erase(itFirst, itLast);

	g_engfuncs.pfnServerPrint(std::format("[Waypoint_Scan] {} valid spawn point survived after de-duplication.\n", g_rgvecValidJetSpawn.size()).c_str());
	co_await TaskScheduler::NextFrame::Rank[9];

	iCounter = 0;	// Got to use it later.

	for (auto it = g_rgvecValidJetSpawn.begin(); it != g_rgvecValidJetSpawn.end(); ++iCounter)
	{
		[[unlikely]]
		if (!(iCounter % 512))
		{
			g_engfuncs.pfnServerPrint(std::format("[Waypoint_Scan] {:.2f}% done. {} valid spawn point survived.\n", (double)std::distance(g_rgvecValidJetSpawn.begin(), it) / (double)g_rgvecValidJetSpawn.size() * 100.0, g_rgvecValidJetSpawn.size()).c_str());
			co_await TaskScheduler::NextFrame::Rank[9];
		}

		g_engfuncs.pfnTraceLine(*it, Vector(it->x, it->y, -8192.f), ignore_monsters | ignore_glass, nullptr, &tr);

		Vector const vecGround = tr.vecEndPos;
		array const vecDirs =
		{
			Vector(it->x + 32.0, it->y, vecGround.z),
			Vector(it->x - 32.0, it->y, vecGround.z),
			Vector(it->x, it->y + 32.0, vecGround.z),
			Vector(it->x, it->y - 32.0, vecGround.z),
		};

		for (auto &&vec : vecDirs)
		{
			g_engfuncs.pfnTraceLine(vecGround, vec, ignore_monsters | ignore_glass, nullptr, &tr);

			if (tr.flFraction < 1.f)
			{
				auto const flAngleRad = std::acos(DotProduct(tr.vecPlaneNormal, Vector(0, 0, 1))/* Originally it should divided by |a|*|b|, but they have their length gurenteed 1. */);
				auto const flAngleDeg = flAngleRad / std::numbers::pi * 180.0;

				if (flAngleDeg > 45)	// This indicates that this is an actual wall instead of something like a walkable incline.
					goto LAB_PASS_TEST;
			}
		}

		it = g_rgvecValidJetSpawn.erase(it);	// Four sides are empty. In the 'middle' of map.
		continue;

	LAB_PASS_TEST:;
		++it;
	}

	// This is costly. But we are not going to modify it ANYMORE after this point, so..
	g_rgvecValidJetSpawn.shrink_to_fit();

	//if (auto f = fopen(fmt::format("OUTPUT_{}.txt", STRING(gpGlobals->mapname)).c_str(), "wt"); f != nullptr)
	//{
	//	for (const auto &vec : g_rgvecValidJetSpawn)
	//		fmt::print(f, "{1:.{0}f}\t{2:.{0}f}\t{3:.{0}f}\t\n", std::numeric_limits<float>::max_digits10, vec.x, vec.y, vec.z);
	//	fclose(f);
	//}
	//else
	//{
	//	vecOrigin = {};
	//}

	char szGameDir[32]{};
	g_engfuncs.pfnGetGameDir(szGameDir);
	fs::path hBinFilePath = std::format("{}/addons/metamod/AirSupport/Waypoint/{}.bin", szGameDir, STRING(gpGlobals->mapname));

	if (!fs::exists(hBinFilePath))
		fs::create_directories(hBinFilePath.parent_path());

	if (auto f = fopen(hBinFilePath.string().c_str(), "wb"); f != nullptr)
	{
		auto const iCount = g_rgvecValidJetSpawn.size();

		fwrite(&WAYPOINT_FMT_VER, sizeof(size_t), 1, f);
		fwrite(&iCount, sizeof(iCount), 1, f);
		fwrite(g_rgvecValidJetSpawn.data(), sizeof(Vector), g_rgvecValidJetSpawn.size(), f);

		fclose(f);
		g_engfuncs.pfnServerPrint(std::format("[Waypoint_Scan] {} spawn origin(s) saved to {}.\n", iCount, hBinFilePath.string()).c_str());
	}

	g_WaypointMgr.m_bPointerOwnership = false;
	g_WaypointMgr.m_iCount = g_rgvecValidJetSpawn.size();
	g_WaypointMgr.m_iVersion = WAYPOINT_FMT_VER;
	g_WaypointMgr.m_prgvecOrigins = g_rgvecValidJetSpawn.data();
	g_WaypointMgr.m_rgvecOrigins = g_rgvecValidJetSpawn;
}

void Waypoint_Read(void) noexcept
{
//	TimedFnMgr::Delist(RADIO_KEY * 2);

	char szGameDir[32]{};
	g_engfuncs.pfnGetGameDir(szGameDir);
	fs::path hBinFilePath = std::format("{}/addons/metamod/AirSupport/Waypoint/{}.bin", szGameDir, STRING(gpGlobals->mapname));

	if (auto f = fopen(hBinFilePath.string().c_str(), "rb"); f != nullptr)
	{
		if (g_WaypointMgr.m_bPointerOwnership)
		{
			free(g_WaypointMgr.m_prgvecOrigins);
			g_WaypointMgr.m_prgvecOrigins = nullptr;

			g_WaypointMgr.m_iCount = 0;
			g_WaypointMgr.m_bPointerOwnership = false;
		}

		fread(&g_WaypointMgr.m_iVersion, sizeof(size_t), 1, f);
		assert(g_WaypointMgr.m_iVersion == WAYPOINT_FMT_VER);

		fread(&g_WaypointMgr.m_iCount, sizeof(size_t), 1, f);
		assert(g_WaypointMgr.m_iCount > 0);

		g_WaypointMgr.m_prgvecOrigins = (Vector *)malloc(g_WaypointMgr.m_iCount * sizeof(Vector)); assert(g_WaypointMgr.m_prgvecOrigins != nullptr);
		fread(g_WaypointMgr.m_prgvecOrigins, sizeof(Vector), g_WaypointMgr.m_iCount, f);

		g_WaypointMgr.m_rgvecOrigins = span<Vector>(g_WaypointMgr.m_prgvecOrigins, g_WaypointMgr.m_iCount);
		g_WaypointMgr.m_bPointerOwnership = true;

#ifdef _DEBUG
		auto const iCurPos = ftell(f);
		fseek(f, 0, SEEK_END);
		auto const iEndPos = ftell(f);
		assert(iCurPos == iEndPos);
#endif
		g_engfuncs.pfnServerPrint(std::format("[Waypoint_Read] {} spawn origin(s) loaded.\n", g_WaypointMgr.m_iCount).c_str());
	}
	else
	{
		g_engfuncs.pfnServerPrint("[Waypoint_Read] Waypoint file no found.\n\t\tOne may generate such file via console command: airsupport_scanjetspawn\n");
	}
}
/*
TimedFn Waypoint_Show(EHANDLE<CBasePlayer> pPlayer) noexcept
{
LAB_RESTART:;

	[[unlikely]]
	if (!pPlayer || !pPlayer->IsAlive())
		co_return;

	for (const auto &vecOrigin : g_WaypointMgr.m_rgvecOrigins)
	{
		if ((vecOrigin - pPlayer->pev->origin).Length2D() < 700)
		{
			MsgSend(pPlayer->pev, SVC_TEMPENTITY);
			WriteData(TE_BEAMPOINTS);
			WriteData(vecOrigin);	// coord coord coord (start position) 
			WriteData(Vector(vecOrigin.x, vecOrigin.y, vecOrigin.z - 128.f));	// coord coord coord (end position) 
			WriteData((short)Sprite::m_rgLibrary[Sprite::BEAM]);	// short (sprite index) 
			WriteData((byte)0);	// byte (starting frame) 
			WriteData((byte)150);	// byte (frame rate in 0.1's) 
			WriteData((byte)5);	// byte (life in 0.1's) 
			WriteData((byte)10);	// byte (line width in 0.1's) 
			WriteData((byte)0);	// byte (noise amplitude in 0.01's) 
			WriteData((byte)255);	// byte (R)
			WriteData((byte)255);	// byte (G)
			WriteData((byte)255);	// byte (B)
			WriteData((byte)255);	// byte (brightness)
			WriteData((byte)0);	// byte (scroll speed in 0.1's)
			MsgEnd();

			co_await (gpGlobals->frametime * 0.5f);
		}
	}

	co_await 0.5f;
	goto LAB_RESTART;
}
*/