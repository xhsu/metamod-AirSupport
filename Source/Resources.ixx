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
		"models/F18.mdl",
		"models/F18.mdl",
		"models/F18.mdl",
		"models/AC130_fly.mdl",
		"models/F18.mdl",
	};

	inline constexpr array PROJECTILE =
	{
		"models/mq9_missile.mdl",
		"models/mortar-rocket.mdl",
		"models/mortar-rocket.mdl",
		"models/mortar-rocket.mdl",
		"models/mortar-rocket.mdl",
	};

	inline constexpr char V_RADIO[] = "models/v_radio.mdl";
	inline constexpr char P_RADIO[] = "models/p_radio.mdl";

	inline constexpr char GIBS_WALL_BROWN[] = "models/gibs_wallbrown.mdl";
	inline constexpr char GIBS_WOOD[] = "models/gibs_woodplank.mdl";
	inline constexpr char GIBS_BRICK[] = "models/gibs_brickred.mdl";

	inline unordered_map<string, int> m_rgLibrary{};

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

	inline constexpr char TRAVEL[] = "weapons/law_travel.wav";

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
};

export namespace Sprite
{
	inline constexpr char SMOKE[] = "sprites/exsmoke.spr";
	inline constexpr char SMOKE2[] = "sprites/rockeexfire.spr";
	inline constexpr char ROCKET_EXPLO[] = "sprites/rockeexplode.spr";
	inline constexpr char ROCKET_EXPLO2[] = "sprites/zerogxplode-big1.spr";
	inline constexpr char SMOKE_TRAIL[] = "sprites/tdm_smoke.spr";
	inline constexpr char FIRE[] = "sprites/rockefire.spr";
	inline constexpr char FIRE2[] = "sprites/hotglow.spr";
	inline constexpr char FIRE3[] = "sprites/flame.spr";
	inline constexpr char SMOKE_1[] = "sprites/gas_smoke1.spr";
	inline constexpr char SMOKE_2[] = "sprites/wall_puff1.spr";

	inline constexpr char BEAM[] = "sprites/laserbeam.spr";
	inline constexpr char AIM[] = "sprites/targetsign.spr";

	inline unordered_map<string, int> m_rgLibrary{};
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
