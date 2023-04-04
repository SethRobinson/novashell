
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/

#pragma once

#include "MyEntityManager.h"
#include "EntityManager.h"
#include "Map.h"
#include "MapManager.h"
#include "AppUtils.h"
#include "ScriptManager.h"
#include "Camera.h"
#include "HashedResourceManager.h"
#include "TagManager.h"
#include "MessageManager.h"
#include "DataManager.h"
#include "GUIStyleBitmap/stylemanager_Bitmap.h"

class EntWorldCache;
class EntEditor;
class MessageManager;
class CL_VirtualFileManager;

#define C_WORLD_INFO_EXTENSION "novashell"
#define C_WORLD_ZIP_EXTENSION "novazip"

class GameLogic
{

public:

    GameLogic();
    ~GameLogic();

      bool Init();
      
	  //note, I use GetMyPlayer instead of GetPlayer because otherwise VS gets confused with its
	  //intellisense parsing of the GetPlayer() #define below that I use to access it, same goes
	  //for other things I have macros for

      void Update(float step);
      void Render();
      void RebuildBuffers();
      MyEntityManager * GetMyEntityManager() {return &m_myEntityManager;}
	  void SaveGlobals();
	  void LoadGlobals();
	  void SetShowGrid(bool bNew){m_bShowGrid = bNew;}
	  bool GetShowGrid() {return m_bShowGrid;}
      void Kill();
	  void SetMyPlayer(MovingEntity * pNew);
	  MovingEntity * GetMyPlayer() {return m_pPlayer;}
	  const string & GetBaseMapPath() {return m_strBaseMapPath;}
	  MapManager * GetMyWorldManager() {return &m_worldManager;}
	  void ClearScreen();
	  void SetShowEntityCollisionData(bool bNew) {m_bShowEntityCollisionData = bNew;}
	  bool GetShowEntityCollisionData() {return m_bShowEntityCollisionData;}
	  bool GetGamePaused() {return m_GamePaused != 0;}
	  int SetGamePaused(bool bNew);
	  void SetEditorActive(bool bNew) {m_bEditorActive = bNew;}
	  bool GetEditorActive() {return m_bEditorActive;}
	  void SetParallaxActive(bool bNew) {m_bParallaxActive = bNew;}
	  bool GetParallaxActive() {return m_bParallaxActive;}
	  const string & GetScriptRootDir() {return m_strScriptRootDir;}
	  bool ToggleEditMode(); //returns true if it was turned on, false if it was turned off
	  void SetMakingThumbnail(bool bNew) {m_bMakingThumbnail = bNew;}
	  bool GetMakingThumbnail() {return m_bMakingThumbnail;}
	  bool SetUserProfileName(const string &name); //returns false if it's an invalid or illegal profile name
	  void ResetUserProfile(string name);
	  const string & GetUserProfilePathWithName() {return m_strUserProfilePathWithName;}
	  const string & GetUserProfileName() {return m_strUserProfileName;}
	  bool UserProfileActive() {return !m_strUserProfileName.empty();}
	  bool UserProfileExists(const string &name);
	  void HandleMessageString(const string &msg);
	  void ClearAllMapsFromMemory();
	  DataManager * Data() {return &m_data;}
	  DataManager * WorldData() {return &m_worldData;}
	  void SetShowMessageActive(bool bNew, CL_Window *pMsgWindow = false);
	  bool GetShowMessageActive() {return m_bShowingMessageWindow;}
	  void SetShowFPS(bool bNew) {m_bShowFPS = bNew;}
	  bool GetShowFPS() {return m_bShowFPS;}
	  void ToggleShowFPS() {m_bShowFPS = !m_bShowFPS;}
	  void SetRestartEngineFlag(bool bNew) { 			m_bRestartEngineFlag = bNew;}
	  bool GetRestartEngineFlag() {return m_bRestartEngineFlag;}
	  void Quit() {GetApp()->OnWindowClose();}
	  bool IsShuttingDown();
	  void OnPlayerDeleted(int id);
	  bool GetShowPathfinding() {return m_bShowPathfinding;}
	  void SetShowPathfinding(bool bNew) {m_bShowPathfinding = bNew;}
	  bool GetShowAI() {return m_bShowAI;}
	  void SetShowAI(bool bNew) {m_bShowAI = bNew;}
	  void ClearModPaths() {m_modPaths.clear(); GetApp()->SetWindowTitle(GetApp()->GetDefaultTitle());}
	  void AddModPath(string s);
	  const string & GetWorldsDirPath() {return m_strWorldsDirPath;}
	  const string & GetActiveWorldName() {return m_activeWorldName;}
	  void InitGameGUI(string xmlFile); //don't make this const, we modify it in place
	  void OneTimeModSetup();
	  CL_StyleManager_Bitmap * GetGUIStyle() {return m_pGUIStyle;}
	  CL_GUIManager * GetGameGUI() {return m_pGUIManager;}
	  void RequestRebuildCacheData();
	  void ShowLoadingMessage();
	  void ResetLastUpdateTimer() { m_lastUpdateTime = CL_System::get_time();} //resets how many MS we've been stuck in a function
	  unsigned int GetTimeSinceLastUpdateMS() {return CL_System::get_time()-m_lastUpdateTime;}  //how many MS we've been stuck in a function
	  bool IsEditorDialogOpen();
	  void OnScreenChanged();
	  bool IsRetail();
	  void AddCallbackOnPostLogicUpdate(const string &callbackFunction, int entityID);
	  void OnRender();
	  vector<string> GetModPaths();

private:

	void OnMouseUp(const CL_InputEvent &key);
	void OnMouseDown(const CL_InputEvent &key);
	void Zoom(bool zoomCloser);
   void OnKeyDown(const CL_InputEvent &key);
	void OnKeyUp(const CL_InputEvent &key);
	void RenderGameGUI(bool bDrawMainGUIToo);   
	void DeleteAllCacheFiles();
	void SaveWorldData();
	void LoadWorldData();
	string GetPathToActiveWorld() {return m_pathToActiveWorld;}
	MyEntityManager m_myEntityManager;
    CL_SlotContainer m_slots;

    int m_editorID; //0 if none
	MovingEntity *m_pPlayer;
	string m_strBaseUserProfilePath; //the path without the profile at the end
	string m_strBaseMapPath; //contains something like "maps/" (includes trailing backslash)
	string m_strUserProfileName; //blank if none (empty if none)
	string m_strUserProfilePathWithName; //the path plus the name (empty if none)
	MapManager m_worldManager; //holds all our world data 
	bool m_bShowEntityCollisionData;
	int m_GamePaused; //0 if not paused, 3 if paused three layers deep
	bool m_bEditorActive;
	bool m_bParallaxActive;
	bool m_bMakingThumbnail; //if true we're in the middle of making a thumbnail
	string m_strScriptRootDir;
	DataManager m_data; //to store global variables controlled by Lua per profile
	DataManager m_worldData; //to store global variables controlled per game
	bool m_bShowingMessageWindow;
	bool m_bShowFPS;
	bool m_bShowGrid;
	bool m_bRestartEngineFlag;

	CL_Slot m_playerDestroyedSlot;
	bool m_bShowPathfinding;
	bool m_bShowAI;
	vector<string> m_modPaths; //mount order is important
	string m_activeWorldName;
	string m_strWorldsDirPath;
	bool m_bRebuildCacheData;
	string m_rebuildUserName;
	unsigned int m_lastUpdateTime;
	

	CL_ResourceManager * m_pGUIResources;
	CL_StyleManager_Bitmap * m_pGUIStyle;
	CL_GUIManager *m_pGUIManager;
	CL_Window *m_pShowMessageWindow;

	ScriptCallbackManager m_postLogicUpdateCallback;
	string m_pathToActiveWorld;

};

void MovePlayerToCamera();
void MoveCameraToPlayer();
void SetCameraToTrackPlayer();
void ShowMessage(string title, string msg, bool bForceClassicStyle = true);

extern TagManager g_TagManager;
extern CL_VirtualFileManager g_VFManager;

#define GetPlayer GetApp()->GetMyGameLogic()->GetMyPlayer()
#define GetTagManager (&g_TagManager)

extern MessageManager g_MessageManager; //I should make the class a singleton. uh.. 

void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
void ScheduleSystem(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
bool RunGlobalScriptFromTopMountedDir(const char *pName);
