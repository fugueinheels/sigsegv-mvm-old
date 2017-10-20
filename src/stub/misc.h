#ifndef _INCLUDE_SIGSEGV_STUB_MISC_H_
#define _INCLUDE_SIGSEGV_STUB_MISC_H_


#include "link/link.h"


class CTFBotMvMEngineerTeleportSpawn;
class CTFBotMvMEngineerBuildTeleportExit;
class CTFBotMvMEngineerBuildSentryGun;

class CFlaggedEntitiesEnum;

class CRConClient;

class CBaseEntity;


CRConClient& RCONClient();


#define DECL_FT_WRAPPER(ret, name) \
	template<typename... ARGS> ret name(ARGS&&... args) { return ft_ ## name(std::forward<ARGS>(args)...); }


extern StaticFuncThunk<void, const Vector&, trace_t&, const Vector&, const Vector&, CBaseEntity *> ft_FindHullIntersection;
DECL_FT_WRAPPER(void, FindHullIntersection);


#if 0
extern StaticFuncThunk<const char *, const char *, int> TranslateWeaponEntForClass;
//const char *TranslateWeaponEntForClass(const char *name, int classnum);
#endif


// TODO: move to common.h
#include <gamestringpool.h>


class CMapListManager
{
public:
	void RefreshList()                      {        ft_RefreshList(this); }
	int GetMapCount() const                 { return ft_GetMapCount(this); }
	int IsMapValid(int index) const         { return ft_IsMapValid (this, index); }
	const char *GetMapName(int index) const { return ft_GetMapName (this, index); }
	
private:
	static MemberFuncThunk<      CMapListManager *, void>              ft_RefreshList;
	static MemberFuncThunk<const CMapListManager *, int>               ft_GetMapCount;
	static MemberFuncThunk<const CMapListManager *, int, int>          ft_IsMapValid;
	static MemberFuncThunk<const CMapListManager *, const char *, int> ft_GetMapName;
};

extern GlobalThunk<CMapListManager> g_MapListMgr;


#endif
