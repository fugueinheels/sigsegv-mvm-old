#include "mod.h"
#include "sm/detours.h"
#include "stub/stub.h"
#include "util/util.h"


#if defined __GNUC__
#warning TODO: move cvars into class
#endif


#ifdef EXPERIMENTAL

namespace Mod_Visualize_AirblastBox
{
	ConVar cvar_duration("sigsegv_visualize_airblastbox_duration", "0.1", FCVAR_NOTIFY,
		"Visualization: box draw duration");
	
	ConVar cvar_color_r("sigsegv_visualize_airblastbox_color_r", "255", FCVAR_NOTIFY,
		"Visualization: box color (red)");
	ConVar cvar_color_g("sigsegv_visualize_airblastbox_color_g", "255", FCVAR_NOTIFY,
		"Visualization: box color (green)");
	ConVar cvar_color_b("sigsegv_visualize_airblastbox_color_b", "255", FCVAR_NOTIFY,
		"Visualization: box color (blue)");
	ConVar cvar_color_a("sigsegv_visualize_airblastbox_color_a", "255", FCVAR_NOTIFY,
		"Visualization: box color (alpha)");
	
	
	RefCount rc_CTFWeaponBase_DeflectProjectiles;
	DETOUR_DECL_MEMBER(bool, CTFWeaponBase_DeflectProjectiles)
	{
		SCOPED_INCREMENT(rc_CTFWeaponBase_DeflectProjectiles);
		return DETOUR_MEMBER_CALL(CTFWeaponBase_DeflectProjectiles)();
	}
	
	/* UTIL_EntitiesInBox forwards call to partition->EnumerateElementsInBox */
	RefCount rc_EnumerateElementsInBox;
	DETOUR_DECL_MEMBER(void, ISpatialPartition_EnumerateElementsInBox, SpatialPartitionListMask_t listMask, const Vector& mins, const Vector& maxs, bool coarseTest, IPartitionEnumerator *pIterator)
	{
		SCOPED_INCREMENT(rc_EnumerateElementsInBox);
		if (rc_CTFWeaponBase_DeflectProjectiles > 0 && rc_EnumerateElementsInBox <= 1) {
			NDebugOverlay::Box(vec3_origin, mins, maxs,
				cvar_color_r.GetInt(),
				cvar_color_g.GetInt(),
				cvar_color_b.GetInt(),
				cvar_color_a.GetInt(),
				cvar_duration.GetFloat());
		}
		
		return DETOUR_MEMBER_CALL(ISpatialPartition_EnumerateElementsInBox)(listMask, mins, maxs, coarseTest, pIterator);
	}
	
	
	class CMod : public IMod
	{
	public:
		CMod() : IMod("Visualize_AirblastBox")
		{
			MOD_ADD_DETOUR_MEMBER(CTFWeaponBase,     DeflectProjectiles);
			MOD_ADD_DETOUR_MEMBER(ISpatialPartition, EnumerateElementsInBox);
		}
		
		void SetEnabled(bool enable)
		{
			this->ToggleAllDetours(enable);
		}
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sigsegv_visualize_airblastbox_enable", "0", FCVAR_NOTIFY,
		"Visualization: draw box used for airblast deflection of projectiles",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue) {
			ConVarRef var(pConVar);
			s_Mod.SetEnabled(var.GetBool());
		});
}

#endif
