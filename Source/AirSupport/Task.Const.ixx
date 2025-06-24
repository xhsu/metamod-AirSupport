export module Task.Const;

import std;

export enum ETaskFlags : std::uint64_t
{
	TASK_ANIMATION = (1ull << 0),
	TASK_FADE_OUT = (1ull << 1),
	TASK_FADE_IN = (1ull << 2),
	TASK_SCALING = (1ull << 3),
	TASK_COLOR_DRIFT = (1ull << 4),
	TASK_REFLECTING_FLAME = (1ull << 5),
	TASK_IGNITE = (1ull << 6),
	TASK_SUFFOCATION = (1ull << 7),
	TASK_FLYING = (1ull << 8),

	TASK_QUICK_ANALYZE = (1ull << 9),
	TASK_DEEP_ANALYZE = (1ull << 10),
	TASK_TIME_OUT = (1ull << 11),
	TASK_ACTION = (1ull << 12),
	TASK_ANGLE_INTERPOL = (1ull << 13),
	TASK_MINIATURE = (1ull << 14),

	TASK_RADIO_DEPLOY = (1ull << 16),
	TASK_RADIO_REJECTED = (1ull << 17),
	TASK_RADIO_ACCEPTED = (1ull << 18),
	TASK_RADIO_TARGET = (1ull << 19),
	TASK_RADIO_TEAM_CD = (1ull << 20),
	TASK_RADIO_FORCED_HOLSTER = (1ull << 21),

	TASK_ENTITY_ON_FIRE = (1ull << 24),
	TASK_FLAME_ON_PLAYER = (1ull << 25),
};
