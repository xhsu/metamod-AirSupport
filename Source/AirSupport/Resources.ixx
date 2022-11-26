export module Resources;

export import <array>;
export import <string>;
export import <unordered_map>;

export import util;

using std::array;
using std::string;
using std::unordered_map;

export namespace Models
{
	inline constexpr array PLANE =
	{
		"models/AirSupport/F22.mdl",
		"models/AirSupport/F18.mdl",
		"models/AirSupport/B-1B.mdl",
		"models/AirSupport/AC130.mdl",
		"models/AirSupport/F18.mdl",
	};

	inline constexpr array PROJECTILE =
	{
		"models/AirSupport/mq9_missile.mdl",
		"models/AirSupport/mortar-rocket.mdl",
		"models/AirSupport/mortar-rocket.mdl",
		"models/AirSupport/mortar-rocket.mdl",
		"models/AirSupport/mortar-rocket.mdl",
	};

	inline constexpr char V_RADIO[] = "models/AirSupport/v_radio.mdl";
	inline constexpr char P_RADIO[] = "models/AirSupport/p_radio.mdl";

	inline constexpr char GIBS_WALL_BROWN[] = "models/gibs_wallbrown.mdl";
	inline constexpr char GIBS_WOOD[] = "models/gibs_woodplank.mdl";
	inline constexpr char GIBS_BRICK[] = "models/gibs_brickred.mdl";

	inline constexpr char TARGET[] = "models/AirSupport/Test_001.mdl";

	inline constexpr char SPARK[] = "models/AirSupport/m_flash1.mdl";

	inline unordered_map<string, short> m_rgLibrary{};

	namespace v_radio
	{
		enum struct seq
		{
			idle,
			draw,
			holster,
			use,
		};

		namespace time
		{
			inline constexpr float idle = (float)((double)2 / (double)15);
			inline constexpr float draw = (float)((double)31 / (double)30);
			inline constexpr float holster = (float)((double)39 / (double)30);
			inline constexpr float use = (float)((double)124 / (double)45);
		};
	};

	namespace targetmdl
	{
		inline constexpr auto FPS = (2.0 * 256.0 / 17.0);	// the pev->frame must count from 0-255, with all model frame stretch and distributed.

		inline constexpr auto SKIN_GREEN = 0;
		inline constexpr auto SKIN_BLUE = 1;
		inline constexpr auto SKIN_RED = 2;
	}
};

export namespace Sounds
{
	inline constexpr array RADIO =
	{
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_inpos.wav",
		"airsupport/radio/radio_affirm.wav",
	};

	inline constexpr char REQUESTING[] = "airsupport/radio/radio_use.wav";
	inline constexpr char REJECTING[] = "airsupport/radio/radio_negative.wav";
	inline constexpr char NOISE[] = "weapons/RADIO/radio_use.wav";

	inline constexpr char TRAVEL[] = "weapons/missile_travel_06.wav";

	inline constexpr char AMBIENT_FIRE[] = "ambience/fire_loop_1.wav";

	inline constexpr char PLAYER_BREATHE[] = "misc/breathe.wav";
	inline constexpr char PLAYER_EAR_RINGING[] = "misc/earring.wav";
	inline constexpr char PLAYER_HEARTBEAT[] = "misc/heartbeat.wav";

	inline constexpr array EXPLOSION =
	{
		"airsupport/explode/explode_near_1.wav",
		"airsupport/explode/explode_near_2.wav",
		"airsupport/explode/explode_near_3.wav",
		"airsupport/explode/explode_near_4.wav",
		"airsupport/explode/explode_near_5.wav",
		"airsupport/explode/explode_near_6.wav",
	};

	inline constexpr array JET =
	{
		"airsupport/jet/jet_short_1.wav",
		"airsupport/jet/jet_short_2.wav",
		"airsupport/jet/jet_short_3.wav",
		"airsupport/jet/jet_short_4.wav",
		"airsupport/jet/jet_short_5.wav",
		"airsupport/jet/jet_short_6.wav",
		"airsupport/jet/jet_short_7.wav",
		"airsupport/jet/jet_short_8.wav",
		"airsupport/jet/jet_short_9.wav",
		"airsupport/jet/jet_short_10.wav",
		"airsupport/jet/jet_short_11.wav",
		"airsupport/jet/jet_short_12.wav",
	};

	inline constexpr array WHIZZ =
	{
		"misc/whizz1.wav",
		"misc/whizz2.wav",
		"misc/whizz3.wav",
		"misc/whizz4.wav",
		"misc/whizz5.wav",
	};

	namespace Gunship
	{
		inline constexpr array AC130_AMBIENT =
		{
			"airsupport/airgunship/ac130_01.wav",
			"airsupport/airgunship/ac130_02.wav",
			"airsupport/airgunship/ac130_03.wav",
			"airsupport/airgunship/ac130_06.wav",
		};
		inline constexpr array AC130_DEPARTURE =
		{
			"airsupport/airgunship/ac130_04.wav",
			"airsupport/airgunship/ac130_05.wav",
		};
		inline constexpr char AC130_IS_IN_AIR[] = "airsupport/radio/ns_1mc_use_ac130_02.wav";
		inline constexpr char UAV_IS_ONLINE[] = "airsupport/radio/ns_1mc_use_uav_01.wav";
		inline constexpr array KILL_CONFIRMED =
		{
			"airsupport/radio/ns_gst_inform_killfirm_generic_02_R.wav",
			"airsupport/radio/ns_gst_inform_killfirm_generic_04_R.wav",
			"airsupport/radio/ns_gst_inform_killfirm_generic_05_R.wav",
		};
		inline constexpr char NOISE_PILOT[] = "airsupport/radio/radio_pilot.wav";
		inline constexpr array AC130_FIRE_25MM =
		{
			"airsupport/airgunship/ac130_fire_01.wav",
			"airsupport/airgunship/ac130_fire_02.wav",
			"airsupport/airgunship/ac130_fire_03.wav",
			"airsupport/airgunship/ac130_fire_04.wav",
		};
		inline constexpr array AC130_FIRE_40MM =
		{
			"airsupport/airgunship/ac130_fire_05.wav",
			"airsupport/airgunship/ac130_fire_06.wav",
		};
		inline constexpr char AC130_FIRE_125MM[] = "airsupport/airgunship/ac130_fire_07.wav";
		inline constexpr array AC130_RELOAD =
		{
			"airsupport/airgunship/ac130_reloading_01.wav",
			"airsupport/airgunship/ac130_reloading_02.wav",
			"airsupport/airgunship/ac130_reloading_03.wav",
		};
	}

	namespace Length::Gunship
	{
		inline constexpr array AC130_RELOAD = { 2.2f, 4.2f, 3.2f };
		inline constexpr array AC130_DEPARTURE = { 6.2f, 6.2f };
	}
};

export namespace Sprites
{
	inline constexpr char SMOKE[] = "sprites/exsmoke.spr";
	inline constexpr char ROCKET_EXPLO[] = "sprites/rockeexplode.spr";
	inline constexpr char ROCKET_EXPLO2[] = "sprites/zerogxplode-big1.spr";
	inline constexpr char FIRE[] = "sprites/rockefire.spr";
	inline constexpr char FIRE2[] = "sprites/hotglow.spr";
	inline constexpr char SMOKE_1[] = "sprites/gas_smoke1.spr";
	inline constexpr char SMOKE_2[] = "sprites/wall_puff1.spr";
	inline constexpr char PERSISTENT_SMOKE[] = "sprites/ballsmoke.spr";
	inline constexpr char MINOR_EXPLO[] = "sprites/zerog-frag1.spr";
	inline constexpr char AIRBURST[] = "sprites/fexplo1.spr";
	inline constexpr char CARPET_FRAGMENT_EXPLO[] = "sprites/m79grenadeex.spr";
	inline constexpr char SHOCKWAVE[] = "sprites/shockwave.spr";

	inline constexpr char BEAM[] = "sprites/laserbeam.spr";
	inline constexpr char AIM[] = "sprites/targetsign.spr";

	inline constexpr char TRAIL[] = "sprites/smoke.spr";

	inline constexpr array FLAME =
	{
		"sprites/flame1.spr",
		"sprites/flame2.spr",
		"sprites/flame3.spr",
	};

	inline constexpr array BLACK_SMOKE =	// MUST be used with TE_SMOKE or you can see the effect.
	{
		"sprites/black_smoke1.spr",
		"sprites/black_smoke2.spr",
		"sprites/black_smoke3.spr",
		"sprites/black_smoke4.spr",
	};

	inline constexpr array WALL_PUFF =
	{
		"sprites/wall_puff1.spr",
		"sprites/wall_puff2.spr",
		"sprites/wall_puff3.spr",
		"sprites/wall_puff4.spr",
	};

	inline unordered_map<string, short> m_rgLibrary{};

	namespace Frames
	{
		inline constexpr array FLAME = { 17, 16, 16 };
		inline constexpr auto WALL_PUFF = 30;
	};

	static_assert(FLAME.size() == Frames::FLAME.size());
};

export namespace Decal
{
	struct Decal_t final
	{
		const char *m_pszName{};
		int m_Index{};

		inline void Initialize(void) noexcept { m_Index = g_engfuncs.pfnDecalIndex(m_pszName); }
	};

	inline array GUNSHOT =
	{
		Decal_t{"{shot1", 0},
		Decal_t{"{shot2", 0},
		Decal_t{"{shot3", 0},
		Decal_t{"{shot4", 0},
		Decal_t{"{shot5", 0},
	};

	inline array SCORCH =
	{
		Decal_t{"{scorch1", 0},
		Decal_t{"{scorch2", 0},
	};
}

export namespace HUD
{
	inline constexpr char RADIO[] = "weapon_radio";
};
