export module Ray;

export import eiface;
export import util;

export struct trace_hull_functor_t final
{
	trace_hull_functor_t(Vector const &vecMin, Vector const &vecMax) noexcept : m_ent(g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target")))
	{
		g_engfuncs.pfnSetSize(m_ent, vecMin, vecMax);
		g_engfuncs.pfnSetModel(m_ent, "models/w_galil.mdl");
		m_ent->v.effects = EF_NODRAW;
	}
	trace_hull_functor_t(trace_hull_functor_t const &) noexcept = delete;
	trace_hull_functor_t(trace_hull_functor_t &&) noexcept = delete;
	trace_hull_functor_t &operator=(trace_hull_functor_t const &) noexcept = delete;
	trace_hull_functor_t &operator=(trace_hull_functor_t &&) noexcept = delete;
	~trace_hull_functor_t(void) noexcept { g_engfuncs.pfnRemoveEntity(m_ent); m_ent = nullptr; }

	inline operator edict_t *() const noexcept { return m_ent; }

	inline edict_t *operator-> () const noexcept { return m_ent; }
	inline edict_t &operator* () const noexcept { return *m_ent; }

	inline void operator() (Vector const &vecSrc, Vector const &vecEnd, int const iIgnore, edict_t *const pEdict, TraceResult *ptr) const noexcept
	{
		g_engfuncs.pfnTraceMonsterHull(m_ent, vecSrc, vecEnd, iIgnore, pEdict, ptr);
	}

	edict_t *m_ent{};
};

export struct trace_arc_functor_t final
{
	trace_arc_functor_t(void) noexcept : m_ent(g_engfuncs.pfnCreateNamedEntity(MAKE_STRING("info_target"))) {}
	trace_arc_functor_t(trace_arc_functor_t const &) noexcept = delete;
	trace_arc_functor_t(trace_arc_functor_t &&) noexcept = delete;
	trace_arc_functor_t &operator=(trace_arc_functor_t const &) noexcept = delete;
	trace_arc_functor_t &operator=(trace_arc_functor_t &&) noexcept = delete;
	~trace_arc_functor_t(void) noexcept { g_engfuncs.pfnRemoveEntity(m_ent); m_ent = nullptr; }

	inline operator edict_t *() const noexcept { return m_ent; }

	inline edict_t *operator-> () const noexcept { return m_ent; }
	inline edict_t &operator* () const noexcept { return *m_ent; }

	inline bool operator() (Vector const &vecSrc, Vector const &vecEnd, int const iIgnore, edict_t *const pEdict = nullptr) const noexcept
	{
		auto const vecDir = vecEnd.Make2D() - vecSrc.Make2D();
		auto const H = vecSrc.z - vecEnd.z;
		auto const S = vecDir.Length();
		auto const G = 386.08858267717;
		auto const vecVelocityInit = vecDir.Normalize() * (S * std::sqrt(G / (2 * H)));
		static constexpr auto dx = 0.1;

		auto flLastDist = (vecSrc - vecEnd).LengthSquared();
		auto flCurDist = (vecSrc - vecEnd).LengthSquared() - 1;
		auto vecVel = Vector{ vecDir, 0 };
		auto vecCur = vecSrc;
		auto vecStep = vecCur + vecVel * dx;

		for (TraceResult tr{};
			flCurDist < flLastDist;
			vecVel.z -= float(G * dx), vecCur = vecStep, vecStep += vecVel * dx, flLastDist = flCurDist, flCurDist = (vecCur - vecEnd).LengthSquared()
			)
		{
			g_engfuncs.pfnTraceMonsterHull(m_ent, vecCur, vecStep, iIgnore, pEdict, &tr);

			if (tr.flFraction < 0.99 || tr.fStartSolid || tr.fAllSolid)
				return false;
		}

		return true;
	}

	edict_t *m_ent{};
};
