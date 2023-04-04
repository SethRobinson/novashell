
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 20:2:2006   21:11
*/


#ifndef ScriptManager_HEADER_INCLUDED // include guard
#define ScriptManager_HEADER_INCLUDED  // include guard

//a few macros to reduce typing

#define LUABIND_CATCH(a) catch (luabind::error &e) { ShowLUAMessagesIfNeeded(e.state(), 1); \
	LogError(a.c_str());} catch (...) {LogError("Unknown LUA error, invalid return type maybe? : %s", a.c_str());}
//this works inside of MovingEntity
#define LUABIND_ENT_CATCH(a) catch (luabind::error &e) { ShowLUAMessagesIfNeeded(e.state(), 1); \
	LogError("Entity %d (%s) : %s (Entity's main script is %s)", ID(), GetName().c_str(), a, GetMainScriptFileName().c_str());} catch (...) {LogError("Unknown LUA error in Entity %d (%s) : %s.  Invalid return type maybe?", ID(), GetName().c_str(), a);}

//this works inside of brains
#define LUABIND_ENT_BRAIN_CATCH(a) catch (luabind::error &e) { ShowLUAMessagesIfNeeded(e.state(), 1); \
	LogError("Entity %d (%s) : %s", m_pParent->ID(), m_pParent->GetName().c_str(), a);} catch (...) {LogError("Brain: Unknown LUA error (Invalid return type maybe?) in Entity %d (%s) : %s", m_pParent->ID(), m_pParent->GetName().c_str(), a);}

#include "InputManager.h"

class ScriptObject
{
public:

	ScriptObject();
	~ScriptObject();
	bool Load(const char *pFileName);
	void RunFunction(const char *pFuncName);

	void SetGlobal(const string& key, int value);
	void RunString(const char *pString);
	bool FunctionExists(const char *pFuncName);
	bool FunctionExists(string &funcName);
	bool VariableExists(const char *pVarName);

	lua_State * GetState() {return m_pLuaState;}
	void SetGarbageCollectOnKill(bool bGarbageCollectOnKill){m_bGarbageCollectOnKill = bGarbageCollectOnKill;}
	
protected:

	lua_State* m_pLuaState;
	int m_threadReference;
	bool m_bGarbageCollectOnKill;
};

class ScriptManager
{
public:

    ScriptManager();
    virtual ~ScriptManager();

	bool Init();
	void Kill();
	void LoadMainScript(const char *pScriptName);
    void RunFunction(const char *pFuncName);
	void RunFunction(const char *pFuncName, bool bBool);
	void RunFunction(const char *pFuncName, BaseGameEntity *pBaseGameEntity);
	void RunString(const char *pString);
	void SetGlobal(const char * pGlobalName, int value);
	void SetGlobalBool(const char * pGlobalName, bool value);
	void SetStrict(bool bStrict);
	bool VariableExists(const char *pFuncName);
	bool FunctionExists(const char *pFuncName);
	bool FunctionExists(string &funcName);

	lua_State * GetMainState() {return m_pMainState;}
	void UpdateAfterScreenChange(bool bActuallyChanged);
	bool CompileLuaIfNeeded(string filename);
	void CompileAllLuaFilesRecursively(string dir);
protected:

lua_State * m_pMainState;

};

extern InputManager g_inputManager;

int luaPrint(lua_State *L);
int luaPrintError(lua_State *L);
void ShowLUAMessagesIfNeeded(lua_State *pState, int result);
void DumpTable( lua_State *L, const char *pTableName = NULL, int tableIndex = LUA_GLOBALSINDEX);

#endif                  // include guard
