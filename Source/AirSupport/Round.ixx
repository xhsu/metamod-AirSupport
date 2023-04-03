export module Round;

export import <vector>;

export import edict;

export import Task;

export inline std::vector<edict_t *> g_rgpPlayersOfCT = {};
export inline std::vector<edict_t *> g_rgpPlayersOfTerrorist = {};

export inline entvars_t *g_pevWorld = nullptr;
export inline edict_t *g_pEdictWorld = nullptr;

export extern "C++" void Task_GetWorld(void) noexcept;
export extern "C++" Task Task_UpdateTeams(void) noexcept;
