export module Round;

export import <vector>;

export import edict;

export import Task;

export inline std::vector<edict_t *> g_rgpCTs = {};
export inline std::vector<edict_t *> g_rgpTers = {};

export extern "C++" Task Task_UpdateTeams(void) noexcept;
