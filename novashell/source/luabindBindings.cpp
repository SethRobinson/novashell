#include "AppPrecomp.h"
#include "luabindBindings.h"
#include "main.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

#include "VectorBindings.h"
#include "EntityBindings.h"
#include "MiscClassBindings.h"
#include "GlobalScriptFunctionBindings.h"
#include "MaterialManager.h"
#include "TextManager.h"
#include "GameLogic.h"
#include "AI/WatchManager.h"
#include "ListBindings.h"
#include "TileBindings.h"
#include "LoopingSoundBindings.h"
#include "EffectManagerBindings.h"
#include "EffectManager.h"
#include "DrawManager.h"

void RegisterLuabindBindings(lua_State *pLuaState)
{
	luabindVector(pLuaState);
	luabindTile(pLuaState);
	luabindEntity(pLuaState);
	luabindMisc(pLuaState);
	luabindGlobalFunctions(pLuaState);
	luabindList(pLuaState);
	luabindLoopingSound(pLuaState);
	luabindEffectManager(pLuaState);

	//add some global functions
	luabind::globals(pLuaState)["GetMaterialManager"] = &g_materialManager;
	luabind::globals(pLuaState)["GetApp"] = GetApp();
	luabind::globals(pLuaState)["GetSoundManager"] = g_pSoundManager;
	luabind::globals(pLuaState)["GetTextManager"] = &g_textManager;
	luabind::globals(pLuaState)["GetMapManager"] = GetGameLogic()->GetMyWorldManager();

	luabind::globals(pLuaState)["GetGameLogic"] = GetGameLogic();
	luabind::globals(pLuaState)["GetTagManager"] = &g_TagManager;
	luabind::globals(pLuaState)["GetDrawManager"] = &g_drawManager;
	luabind::globals(pLuaState)["GetCamera"] = GetCamera;
	luabind::globals(pLuaState)["GetInputManager"] = &g_inputManager;
	luabind::globals(pLuaState)["GetWatchManager"] = &g_watchManager;
	luabind::globals(pLuaState)["GetEffectManager"] = &g_EffectManager;

	luabind::globals(pLuaState)["g_PlayerID"] = 0;	 //will be set later
#ifdef _DEBUG
	luabind::globals(pLuaState)["g_isDebug"] = true;	 //will be set later
#else
	luabind::globals(pLuaState)["g_isDebug"] = false;	 //will be set later
#endif
	luabind::globals(pLuaState)["g_isRetail"] = false;	 //will be set later


}


