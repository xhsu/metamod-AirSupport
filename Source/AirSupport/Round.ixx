export module Round;

export import <vector>;

export import edict;

export import Task;

export inline std::vector<edict_t *> g_rgpPlayersOfCT = {};
export inline std::vector<edict_t *> g_rgpPlayersOfTerrorist = {};

export extern "C++" Task Task_UpdateTeams(void) noexcept;
