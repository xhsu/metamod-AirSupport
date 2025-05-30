module;

//#define CONSERVE_SFX_RES_SLOT

export module Resources;

export import std;

#ifdef PACKING_RESOURCES
export import Transpiler;
#elif defined CREATING_ENFORCING_TABLE
export import Enforcer;
#else
export import hlsdk;
export import :Const;
#endif

using std::array;
using std::string_view;
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
		"models/AirSupport/F18.mdl",	// #SHOULD_DO_ON_FREE what plane shooting phosphorus munition?
	};

	inline constexpr char PROJECTILE[] = "models/AirSupport/projectiles.mdl";

	inline constexpr char V_RADIO[] = "models/AirSupport/v_radio.mdl";
	inline constexpr char P_RADIO[] = "models/AirSupport/p_radio.mdl";

	inline constexpr char GIBS_CONCRETE[] = "models/gibs_wallbrown.mdl";
	inline constexpr char GIBS_METAL[] = "models/gibs_stairsmetal.mdl";
	inline constexpr char GIBS_RUBBLE[] = "models/gibs_rubble.mdl";
	inline constexpr char GIBS_WOOD[] = "models/gibs_woodplank.mdl";

	inline constexpr char TARGET[] = "models/AirSupport/Test_001.mdl";

	inline constexpr char SPARK[] = "models/AirSupport/m_flash1.mdl";

	inline unordered_map<string_view, short> m_rgLibrary{};	// MUST be use with local string!!

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
	inline constexpr array ACCEPTING =
	{
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_inpos.wav",
		"airsupport/radio/radio_affirm.wav",
		"airsupport/radio/radio_affirm.wav",
	};

	inline constexpr char REQUESTING[] = "airsupport/radio/radio_use.wav";
	inline constexpr array REJECTING = { "airsupport/radio/radio_negative.wav", "airsupport/radio/radio_negative-2.wav" };
	inline constexpr char NOISE[] = "weapons/RADIO/radio_talking.wav";

	inline constexpr char ALERT_AC130[] = "airsupport/radio/radio_enemy_ac130.wav";
	inline constexpr char ALERT_AIRSTRIKE[] = "airsupport/radio/radio_enemy_airstrike.wav";
	inline constexpr char ALERT_APACHE[] = "airsupport/radio/radio_enemy_apache.wav";

	inline constexpr char TRAVEL[] = "weapons/cruise_missile_travel_02.wav";

	inline constexpr char PLAYER_BREATHE[] = "misc/breathe.wav";
	inline constexpr char PLAYER_EAR_RINGING[] = "misc/earring.wav";
	inline constexpr char PLAYER_HEARTBEAT[] = "misc/heartbeat.wav";
	inline constexpr char PLAYER_HB_AND_ER[] = "misc/breathe_heartbeat.wav";

	inline constexpr array PLAYER_COUGH =
	{
		"misc/cough1.wav",
		"misc/cough2.wav",
		"misc/cough3.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"misc/cough4.wav",
		"misc/cough5.wav",
		"misc/cough6.wav",
		"misc/cough7.wav",
#endif
	};

	inline constexpr array EXPLOSION =
	{
		"airsupport/explode/explode_near_1.wav",
		"airsupport/explode/explode_near_2.wav",
		"airsupport/explode/explode_near_3.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"airsupport/explode/explode_near_4.wav",
		"airsupport/explode/explode_near_5.wav",
		"airsupport/explode/explode_near_6.wav",
		"airsupport/explode/explode_near_7.wav",
		"airsupport/explode/explode_near_8.wav",
#endif
	};

	inline constexpr array EXPLOSION_SHORT =
	{
		"airsupport/explode/m67_detonate_01.wav",
		"airsupport/explode/m67_detonate_02.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"airsupport/explode/m67_detonate_03.wav",
		"airsupport/explode/m67_detonate_04.wav",
#endif
	};

	inline constexpr array JET =
	{
		"airsupport/jet/jet_short_1.wav",
		"airsupport/jet/jet_short_2.wav",
		"airsupport/jet/jet_short_3.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"airsupport/jet/jet_short_4.wav",
		"airsupport/jet/jet_short_5.wav",
		"airsupport/jet/jet_short_6.wav",
		"airsupport/jet/jet_short_7.wav",
		"airsupport/jet/jet_short_8.wav",
		"airsupport/jet/jet_short_9.wav",
		"airsupport/jet/jet_short_10.wav",
		"airsupport/jet/jet_short_11.wav",
		"airsupport/jet/jet_short_12.wav",
#endif
	};

	inline constexpr array WHIZZ =
	{
		"misc/whizz1.wav",
		"misc/whizz2.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"misc/whizz3.wav",
		"misc/whizz4.wav",
		"misc/whizz5.wav",
#endif
	};

	inline constexpr array BOMBER =
	{
		"airsupport/bomber/b1b_01.wav",
		"airsupport/bomber/b1b_02.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"airsupport/bomber/b1b_03.wav",
		"airsupport/bomber/b1b_04.wav",
		"airsupport/bomber/b1b_05.wav",
#endif
	};

	inline constexpr array HIT_METAL =
	{
		"debris/metal1.wav",
		"debris/metal2.wav",
		"debris/metal3.wav",
#ifndef CONSERVE_SFX_RES_SLOT
		"debris/metal4.wav",
		"debris/metal5.wav",
		"debris/metal6.wav",
#endif
	};

	inline constexpr array EXPLOSION_BIG =
	{
		"airsupport/explode/bigexplosion_01.wav",
		"airsupport/explode/bigexplosion_02.wav",
	};

	inline constexpr char CLUSTER_BOMB_DROP[] = "weapons/missile_travel_01.wav";

	inline constexpr array GRENADE_BOUNCE =
	{
		"weapons/he_bounce-1.wav",
		"weapons/grenade_hit1.wav",
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
#ifndef CONSERVE_SFX_RES_SLOT
			"airsupport/radio/ac130_fco_goodkill_01.wav",
			"airsupport/radio/ac130_fco_hotdamn3_02.wav",
			"airsupport/radio/vo_mp_announcer_english_en_40.wav",
#endif
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
		inline constexpr char RESELECT_TARGET[] = "airsupport/radio/radio_attacking.wav";
		inline constexpr char TARGET_RAN_TO_COVER[] = "airsupport/radio/ac130_tvo_coverbywall1_01.wav";
	}

	namespace FuelAirBomb
	{
		inline constexpr char GAS_LEAK_LOOP[] = "airsupport/explode/steam_pipe_burst_loop.wav";
		inline constexpr char GAS_LEAK_FADEOUT[] = "airsupport/explode/steam_pipe_burst_loop_end.wav";

		inline constexpr array GAS_EXPLO =
		{
			"airsupport/explode/steam_pipe_burst2.wav",
			"airsupport/explode/steam_pipe_burst3.wav",
		};
	};

	namespace Flame
	{
		inline constexpr array FLAME =
		{
			"ambience/fire_woodtrash_small1v2res.wav",
			"ambience/fire_woodtrash_small2v2res.wav",
		};

		inline constexpr char FLAME_FADEOUT[] = "ambience/fire_loop_fadeout_01.wav";
	};

	namespace Thermite
	{
		inline constexpr char COMPOSED[] = "weapons/an_m14_burn.wav";
		inline constexpr char BURNING_LOOP[] = "weapons/an_m14_burn_loop.wav";
		inline constexpr char BURNING_END[] = "weapons/an_m14_burn_loop_end.wav";
		inline constexpr char DETONATE[] = "weapons/an_m14_detonate.wav";
	};
};

export namespace Sprites
{
	inline constexpr char ROCKET_EXPLO[] = "sprites/rockeexplode.spr";
	inline constexpr char ROCKET_EXPLO2[] = "sprites/zerogxplode-big1.spr";
	inline constexpr char ROCKET_EXHAUST_FLAME[] = "sprites/hotglow.spr";
	inline constexpr char MINOR_EXPLO[] = "sprites/zerog-frag1.spr";
	inline constexpr char AIRBURST[] = "sprites/exploeffect1.spr";
	inline constexpr char CARPET_FRAGMENT_EXPLO[] = "sprites/m79grenadeex.spr";
	inline constexpr char SHOCKWAVE[] = "sprites/shockwave.spr";
	inline constexpr char SPARK[] = "sprites/metal_sparks1.spr";
	inline constexpr char LIFTED_DUST[] = "sprites/smoke_loop.spr";
	inline constexpr char GROUNDED_DUST[] = "sprites/bettyspr2.spr";
	inline constexpr char GIGANTIC_EXPLO[] = "sprites/bunkerbuster_explosion.spr";
	inline constexpr char STATIC_SMOKE_THIN[] = "sprites/static_smoke_thin_1.spr";
	inline constexpr char STATIC_SMOKE_THICK[] = "sprites/static_smoke_thick_1.spr";
	inline constexpr char PHOSPHORUS_TRACE_HEAD[] = "sprites/xflare1.spr";
	inline constexpr char PHOSPHORUS_FLAME[] = "sprites/flame6.spr";
	inline constexpr char PHOSPHORUS_SMOKE[] = "sprites/bettyspr1.spr";
	inline constexpr char PHOSPHORUS_MINOR_SPARK[] = "sprites/w_lightgranate.spr";
	inline constexpr char PHOSPHORUS_MAJOR_SPARK[] = "sprites/fusion_p_engage.spr";
	inline constexpr char FLAME_ON_PLAYER[] = "sprites/zombieriot/flame.spr";

	inline constexpr char BEAM[] = "sprites/laserbeam.spr";

	inline constexpr char TRAIL[] = "sprites/tdm_smoke.spr";

	inline constexpr array FLAME =
	{
		"sprites/flame1.spr",
		"sprites/flame2.spr",
	};

	inline constexpr array BLACK_SMOKE =	// MUST be used with TE_SMOKE or you can see the effect.
	{
		"sprites/black_smoke1.spr",
		"sprites/black_smoke2.spr",
		"sprites/black_smoke3.spr",
		"sprites/black_smoke4.spr",
	};

	inline constexpr array ROCKET_TRAIL_SMOKE =
	{
		//"sprites/exsmoke.spr",
		"sprites/wall_puff1.spr",
		"sprites/wall_puff2.spr",
		"sprites/wall_puff3.spr",
		"sprites/wall_puff4.spr",
	};

	inline constexpr array GAS_EXPLO =
	{
		//"sprites/exploeffect3.spr",
		//"sprites/exploeffect4.spr",
		//"sprites/exploeffect5.spr",
		//"sprites/exploeffect7.spr",
		"sprites/fire_explosion_1.spr",
		"sprites/fire_explosion_2.spr",
	};

	inline unordered_map<string_view, short> m_rgLibrary{};	// MUST be use with local string!!

	namespace Frames
	{
		inline constexpr array FLAME = { 17, 16 };
		inline constexpr auto BLACK_SMOKE = 30;
		//inline constexpr array GAS_EXPLO = { 15, 20, 15, 20 };
		inline constexpr array GAS_EXPLO = { 50, 50 };
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

	inline array BIGSHOT =
	{
		Decal_t{"{bigshot1", 0},
		Decal_t{"{bigshot2", 0},
		Decal_t{"{bigshot3", 0},
		Decal_t{"{bigshot4", 0},
		Decal_t{"{bigshot5", 0},
	};

	inline array SCORCH =
	{
		Decal_t{"{scorch1", 0},
		Decal_t{"{scorch2", 0},
		Decal_t{"{scorch3", 0},
	};

	inline array SMALL_SCORCH =
	{
		Decal_t{"{smscorch1", 0},
		Decal_t{"{smscorch2", 0},
		Decal_t{"{smscorch3", 0},
	};
}

export namespace HUD
{
	inline constexpr char RADIO[] = "weapon_radio";
};
