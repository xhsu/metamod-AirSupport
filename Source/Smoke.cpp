import Effects;
import Math;
import Resources;

import UtlRandom;

Task CSmoke::Task_EmitSmoke() noexcept
{
	for (;;)
	{
		co_await UTIL_Random(0.1f, 0.3f);

		Vector const vecNoise = get_spherical_coord(m_flRadius, UTIL_Random(70.0, 80.0), UTIL_Random(0.0, 359.9));

		MsgPVS(SVC_TEMPENTITY, pev->origin + vecNoise);
		WriteData(TE_SPRITE);
		WriteData(pev->origin + vecNoise);
		WriteData((short)Sprite::m_rgLibrary[Sprite::PERSISTENT_SMOKE]);
		WriteData((byte)UTIL_Random<short>(45, 55));
		WriteData((byte)UTIL_Random<short>(40, 60));
		MsgEnd();
	}
}

Task CSmoke::Task_Remove() noexcept
{
	for (;;)
	{
		co_await 0.1f;

		bool bAnySurvived = false;

		for (const auto &flame : m_rgpFlamesDependent)
		{
			[[likely]]
			if (flame)
			{
				bAnySurvived = true;
				break;
			}
		}

		[[unlikely]]
		if (!bAnySurvived)
		{
			pev->flags |= FL_KILLME;
			co_return;
		}
	}
}

void CSmoke::Spawn() noexcept
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	g_engfuncs.pfnSetSize(edict(), Vector(-0.2, -0.2, -0.2), Vector(0.2, 0.2, 0.2));
	g_engfuncs.pfnDropToFloor(edict());

	m_Scheduler.Enroll(Task_EmitSmoke());
	m_Scheduler.Enroll(Task_Remove());
}
