export module Round;

export import std;
export import hlsdk;

export import Task;

export inline std::vector<edict_t *> g_rgpPlayersOfCT = {};
export inline std::vector<edict_t *> g_rgpPlayersOfTerrorist = {};

export inline entvars_t *g_pevWorld = nullptr;
export inline edict_t *g_pEdictWorld = nullptr;

export extern "C++" void Task_GetWorld() noexcept;
export extern "C++" void Task_UpdateTeams(void) noexcept;
export extern "C++" Task Task_TeamwiseAI(cvar_t const* pcvarEnable, std::vector<edict_t*> const* pTeamMembers, std::vector<edict_t*> const* pEnemies) noexcept;
