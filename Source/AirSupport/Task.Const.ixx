export module Task.Const;

export import <cstdint>;

export enum ETaskFlags : uint64_t
{
	TASK_ANIMATION = (1 << 0),
	TASK_FADE_OUT = (1 << 1),
	TASK_FADE_IN = (1 << 2),
	TASK_SCALING = (1 << 3),
	TASK_COLOR_DRIFT = (1 << 4),
	TASK_REFLECTING_FLAME = (1 << 5),
	TASK_IGNITE = (1 << 6),
	TASK_SUFFOCATION = (1 << 7),
	TASK_FLYING = (1 << 8),

	TASK_QUICK_ANALYZE = (1 << 9),
	TASK_DEEP_ANALYZE = (1 << 10),
	TASK_TIME_OUT = (1 << 11),
	TASK_ACTION = (1 << 12),

	TASK_RADIO_DEPLOY = (1 << 16),
	TASK_RADIO_REJECTED = (1 << 17),
	TASK_RADIO_ACCEPTED = (1 << 18),
	TASK_RADIO_TARGET = (1 << 19),
	TASK_RADIO_TEAM_CD = (1 << 20),

	TASK_ENTITY_ON_FIRE = (1 << 24),
	TASK_FLAME_ON_PLAYER = (1 << 25),
};
