#include "AppPrecomp.h"
#include "GameLogic.h"
#include "main.h"
#include "EntEditor.h"
#include "EntMapCache.h"
#include "EntChooseScreenMode.h"
#include "AppUtils.h"
#include "TileEntity.h"
#include "EntEditMode.h"
#include "luabindBindings.h"
#include "MovingEntity.h"
#include "TextManager.h"
#include "Console.h"
#include "MaterialManager.h"
#include "VisualProfileManager.h"
#include "AI/WatchManager.h"
#include "AI/WorldNavManager.h"
#include "AI/WatchManager.h"
#include "GUIStyleBitmap/NS_MessageBox.h"
#include "EntWorldDialog.h"
#include "EffectManager.h"
#include "NovaZip.h"
#include "DrawManager.h"

const char C_GAME_TICK_OF_LAST_LOAD_VAR_NAME[]= "_gameTickOfLastLoad";

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#define C_PROFILE_DAT_FILENAME "profile.dat"
#define C_GLOBAL_DAT_FILENAME "worlddata.dat"

#define C_TIME_TO_WAIT_BEFORE_SHOWING_LOADING_MESSAGE_MS 500
const unsigned int C_PROFILE_DAT_VERSION = 0;

CL_VirtualFileManager g_VFManager;

MessageManager g_MessageManager; 

TagManager g_TagManager;

GameLogic::GameLogic()
{
	m_pGUIResources = NULL;
	m_pGUIStyle = NULL;
	m_pPlayer = NULL;   
	m_pGUIManager= NULL;

	m_slots.connect(CL_Keyboard::sig_key_down(), this, &GameLogic::OnKeyDown);
	m_slots.connect(CL_Keyboard::sig_key_up(), this, &GameLogic::OnKeyUp);
	m_slots.connect(CL_Mouse::sig_key_up(), this, &GameLogic::OnMouseUp);
	m_slots.connect(CL_Mouse::sig_key_down(), this, &GameLogic::OnMouseDown);
    m_editorID = 0;
	m_bRestartEngineFlag = false;
	m_bShowEntityCollisionData = false;
	m_GamePaused = 0;
	m_bEditorActive = false;
	m_bParallaxActive = true;
	m_bMakingThumbnail = false;
	SetShowMessageActive(false);
	m_strBaseMapPath = C_BASE_MAP_PATH+ string("/");
	m_strScriptRootDir = "script";
	m_pShowMessageWindow = false;
	m_bRebuildCacheData = false;
	m_strWorldsDirPath = "worlds";

	CL_Directory::create(m_strBaseUserProfilePath);
	SetShowFPS(false);
	SetShowPathfinding(false);
	SetShowAI(false);
	m_activeWorldName = "base";
	m_bShowGrid = false;
}

bool GameLogic::IsEditorDialogOpen()
{
	if (!GetEditorActive()) return false;

	EntEditMode *pEditMode = (EntEditMode*)EntityMgr->GetEntityByName("edit mode");
	if (pEditMode)
	{
		return pEditMode->IsDialogOpen(true);
	} 

	EntEditor *pEditor = (EntEditor*)EntityMgr->GetEntityFromID(m_editorID);
	if (pEditor) return pEditor->IsDialogOpen();
	
	
	//if we got here, the map manager or something else is probably onscreen
	//LogError("EditorHasDialogOpen can't find it?");
	return false;

}
void GameLogic::OneTimeModSetup()
{
	bool bSentWorldToInstall = false;

	if (GetApp()->GetStartupParms().size() > 0)
	{
		LogMsg("Command line parms received:");
	}

	string worldPath = "";

	for (unsigned int i=0; i < GetApp()->GetStartupParms().size(); i++)
	{
		LogMsg("	%s", GetApp()->GetStartupParms().at(i).c_str());
		string p1 = GetApp()->GetStartupParms().at(i);
		string p2;
		if (i+1 < GetApp()->GetStartupParms().size())
		{
			//this second part might be needed		
			p2 = GetApp()->GetStartupParms().at(i+1);
		}

		if (CL_String::compare_nocase(p1, "-worldpath")  )
		{
			if (p2.empty())
			{
				LogError("Ignored -worldpath in parms, no second part was specified");
			} else
			{
				m_strWorldsDirPath = p2;
			}
		}

		if (CL_String::get_extension(p1) == C_WORLD_INFO_EXTENSION)
		{
			m_strWorldsDirPath = CL_String::get_path(p1);
			worldPath = p1;
			//SetupModPathsFromWorldInfo(p1);
		}

		if (CL_String::get_extension(p1) == C_WORLD_ZIP_EXTENSION)
		{		
			bSentWorldToInstall = true;
			NovaZip nova;
			string worldDir;
			if (nova.InstallWorld(p1, &worldDir))
			{
				//m_strWorldsDirPath = CL_String::get_path(p1);
				worldPath = worldDir;
			}
		}
		

	}

	if (!worldPath.empty())
	{
		SetupModPathsFromWorldInfo(worldPath);
	}


if (!bSentWorldToInstall)
{
	//scan the worlds directory for anything that needs to be installed
	CL_DirectoryScanner scanner;

	scanner.scan(GetApp()->GetBaseDirectory() +GetGameLogic()->GetWorldsDirPath(), string("*.")+C_WORLD_ZIP_EXTENSION);
	while (scanner.next())
	{
		std::string file = scanner.get_name();
			if (scanner.get_name()[0] != '.')
			{

				NovaZip nova;
				string worldDir;
				if (nova.InstallWorld(GetWorldsDirPath() + "/" + scanner.get_name(), &worldDir))
				{
					if (worldDir != "")
					{
						//well, they really did install it
						RemoveFile(GetWorldsDirPath() +"/" +scanner.get_name());
					}
				}
			}
	}
	}
	
}

void GameLogic::AddCallbackOnPostLogicUpdate(const string &callbackFunction, int entityID)
{
	m_postLogicUpdateCallback.Add(callbackFunction, entityID);
}

void GameLogic::ShowLoadingMessage()
{
	if (GetTimeSinceLastUpdateMS() > C_TIME_TO_WAIT_BEFORE_SHOWING_LOADING_MESSAGE_MS)
	{
		BlitMessage("... loading ...");
		ResetLastUpdateTimer();
		GetApp()->ClearTimingAfterLongPause();

	}
}

void GameLogic::RequestRebuildCacheData()
{
	SetRestartEngineFlag(true);
	m_bRebuildCacheData = true;
	m_rebuildUserName = GetUserProfileName();
}

GameLogic::~GameLogic()
{
	Kill();
}

int GameLogic::SetGamePaused(bool bNew)
{
	if (bNew)
	{
		m_GamePaused++;
	} else
	{
		m_GamePaused--;
	}

	m_GamePaused = max(0, m_GamePaused);
	
	return m_GamePaused;
}

void GameLogic::SaveGlobals()
{
	if (UserProfileActive())
	{
		m_data.Set(C_GAME_TICK_OF_LAST_LOAD_VAR_NAME, CL_String::from_int(GetApp()->GetGameTick()));

		//save out our globals
		CL_OutputSource *pSource = g_VFManager.PutFile(C_PROFILE_DAT_FILENAME);
		CL_FileHelper helper(pSource);

		helper.process_const(C_PROFILE_DAT_VERSION);
		m_data.Serialize(helper);
		SAFE_DELETE(pSource);
	}
}

void GameLogic::LoadGlobals()
{

	m_data.Clear();
	
	if (UserProfileActive())
	{
	
		CL_InputSource *pSource = g_VFManager.GetFile(C_PROFILE_DAT_FILENAME);
		if (pSource)
		{
			CL_FileHelper helper(pSource);

			unsigned int version;
			helper.process(version);
			m_data.Serialize(helper);
			SAFE_DELETE(pSource);
			GetApp()->SetGameTick(m_data.GetNum(C_GAME_TICK_OF_LAST_LOAD_VAR_NAME));
			return;
		}
	}

	//if we got here, let's init the defaults
	//init defaults if we couldn't load them
	
	m_data.Set(C_GAME_TICK_OF_LAST_LOAD_VAR_NAME, "0");
	GetApp()->SetGameTick(0);
}

void GameLogic::SaveWorldData()
{
	if (GetActiveWorldName() == "base") return; //we don't care about the base
	
		
	
	//save out our globals
		CL_OutputSource *pSource = g_VFManager.PutFileRaw(GetPathToActiveWorld()+"/"+C_GLOBAL_DAT_FILENAME );
		if (pSource)
		{
			CL_FileHelper helper(pSource);
			helper.process_const(C_PROFILE_DAT_VERSION);
			m_worldData.Serialize(helper);
			SAFE_DELETE(pSource);
		} else
		{
			LogMsg("Error saving worlddata.dat");
		}
	
}


void GameLogic::LoadWorldData()
{

	m_worldData.Clear();
	
		CL_InputSource *pSource = g_VFManager.GetFileRaw(GetPathToActiveWorld()+"/"+C_GLOBAL_DAT_FILENAME  );
		if (pSource)
		{
			CL_FileHelper helper(pSource);

			unsigned int version;
			helper.process(version);
			m_worldData.Serialize(helper);
			SAFE_DELETE(pSource);
			return;
		}
	
	//if we got here, let's init the defaults

}
void GameLogic::ClearScreen()
{
	if (g_pMapManager->GetActiveMap())
	{
		CL_Display::clear(g_pMapManager->GetActiveMap()->GetBGColor());
	} else
	{
		CL_Display::clear(CL_Color(0,0,0,255));
	}
}


string StripDangerousFromString(const string &name)
{
	string safe;

	for (unsigned int i=0; i < name.size(); i++)
	{
		if (name[i] != '\\'
			&& name[i] != '/'
			&& name[i] != '.'
			)
		{
			safe.push_back(name[i]);
		}
	}
	
	return safe;
}

bool GameLogic::UserProfileExists(const string &name)
{

	string stPath = m_strBaseUserProfilePath + "/" + StripDangerousFromString(name) + "/"+ m_activeWorldName;
	
	//TODO:  Replace with real DoesFileExist function, this one is slow and dumb

	CL_DirectoryScanner scanner;

	if (!scanner.scan(stPath, "*")) return false;
	while (scanner.next())
	{
		if (scanner.get_name() == ".") return true;
	}

	return false;
}

void GameLogic::ResetUserProfile(string name)
{
	name = StripDangerousFromString(name);
	
	//we have to go to the directory and delete everything.  Uh.  this could be potentially very dangerous, so let's
	//walk with care here.

    string stPath = m_strBaseUserProfilePath + "/" + name +"/" +m_activeWorldName+ string("/") + string(C_BASE_MAP_PATH);

	//also delete all

	LogMsg("Resetting!");
	g_textManager.Reset();

	{
		//we want the scanner to deconstruct otherwise we can't remove the path it was working with
		CL_DirectoryScanner scanner;
		if (scanner.scan(stPath, "*"))
		{
			while (scanner.next())
			{
				std::string file = scanner.get_name();
				if (scanner.is_directory() && scanner.get_name() != "." && scanner.get_name() != "..")
				{
					//no underscore at the start, let's show it
					RemoveWorldFiles(stPath+"/"+scanner.get_name()+"/");
					//now remove the directory
					CL_Directory::remove(stPath+"/"+scanner.get_name(), false, false);
				}
			}
		}

	}
	stPath = m_strBaseUserProfilePath + "/" + name + string("/") + m_activeWorldName +"/";

	RemoveFile(stPath+string(C_WORLD_NAV_FILENAME));
	RemoveFile(stPath+string(C_PROFILE_DAT_FILENAME));

	CL_Directory::remove(m_strBaseUserProfilePath + "/" + name + string("/") + m_activeWorldName+"/"+ string(C_BASE_MAP_PATH));
	CL_Directory::remove(m_strBaseUserProfilePath + "/" + name + string("/") + m_activeWorldName);
	m_data.Clear();
}

bool GameLogic::SetUserProfileName(const string &name)
{

	if (UserProfileActive())
	{
		LogError("Cannot change user profiles on the fly - you must use SetRestartEngineFlag() first and load it after the initialization.");
		return false;
	}
	ClearAllMapsFromMemory();

	//first check security stuff
	if ( (name.find_last_of("/\\")) != -1)
	{
		LogMsg("Can't use %s as a profile name",name.c_str());
		return false;
	}

	//first close down any open map files

	m_strUserProfileName = name;
	CL_Directory::create( m_strBaseUserProfilePath); //just in case it wasn't created yet

	CL_Directory::create(m_strBaseUserProfilePath + "/" + m_strUserProfileName);

	m_strUserProfilePathWithName = m_strBaseUserProfilePath + "/" + m_strUserProfileName+"/" + m_activeWorldName;

	//make all dirs that will be needed
	CL_Directory::create(m_strUserProfilePathWithName);
	
	CL_Directory::create(m_strUserProfilePathWithName + "/maps");

	LogMsg("User profile path now %s", m_strUserProfilePathWithName.c_str());
	
	//set as new save/load path, with fallback still on the default path
	g_VFManager.MountDirectoryPath(m_strUserProfilePathWithName);
	GetScriptManager->CompileAllLuaFilesRecursively(m_strUserProfilePathWithName);
	m_worldManager.ScanMaps(m_strBaseMapPath);

	//load globals
	LoadGlobals();	
	return true;
}

bool IsOnReadOnlyDisk()
{
	const char *pfName = "readonlychk.tmp";
	FILE *fp = fopen(pfName, "wb");
	if (!fp)
	{
		return true;
	}

	//successfully opened it, disk is not read only
	fclose(fp);
	RemoveFile(pfName);
	
	return false;
}

void GameLogic::AddModPath(string s)
{
//	m_modPaths.push_back(m_strWorldsDirPath+ "/"+ StripDangerousFromString(s));
	
	if (CL_String::get_extension(s) == C_WORLD_INFO_EXTENSION)
	{
		//convert this into the directory path
		s = s.substr(0, s.size()- (strlen(C_WORLD_INFO_EXTENSION)+1));
	}

	m_modPaths.push_back(s);
}

bool GameLogic::Init()
{

	LogMsg("");
	LogMsg("Initializing GameLogic...");
	m_pathToActiveWorld.clear();

	assert(!g_pMapManager->GetActiveMap());
	GetApp()->SetCursorVisible(true); //the default condition

	if (g_pSoundManager && !GetApp()->ParmExists("-nosound"))
	{
		g_pSoundManager->Init();
	}
	GetApp()->OnGameReset();
	g_VFManager.Reset();
	g_VFManager.MountDirectoryPath(CL_Directory::get_current()+"/base");
	GetScriptManager->CompileAllLuaFilesRecursively(CL_Directory::get_current()+"/base");

	m_activeWorldName = "base";
	ClearScreen();
	CL_Display::flip(2); //show it now
	string modInfoFile;
	//now mount any mod paths
	for (unsigned int i=0; i < m_modPaths.size(); i++)
	{
		if (LocateWorldPath(m_modPaths[i], modInfoFile))
		{
			//m_strWorldsDirPath = CL_String::get_path(modInfoFile);
			m_modPaths[i] = modInfoFile;
			LogMsg("Mounting world path %s.", m_modPaths[i].c_str());
			g_VFManager.MountDirectoryPath(m_modPaths[i]);
			GetScriptManager->CompileAllLuaFilesRecursively(m_modPaths[i]);
			m_pathToActiveWorld = modInfoFile;
			m_activeWorldName = CL_String::get_filename(m_modPaths[i]);
		}

	}
	
	LoadWorldData();

	m_worldManager.ScanMaps(m_strBaseMapPath);
	m_pPlayer = NULL;
	//calculate our user profile base path, later, this could be in the windows user dir or whatever is correct
	m_strBaseUserProfilePath = CL_Directory::get_current() + "/profiles";
	GetGameLogic()->ShowLoadingMessage();

	if (IsOnReadOnlyDisk())
	{
		ShowMessage("Hold on!", "You must drag this out of the DMG before running it.\n\n(all prefs and saves are contained inside for easy deletion for now)");
		return false;
	}

	g_textManager.Reset();

	if (!GetHashedResourceManager->Init()) throw CL_Error("Error initting hashed resource manager");

	GetScriptManager->Init();
	//bind app specific functions at the global level
	RegisterLuabindBindings(GetScriptManager->GetMainState());
	
	if (! RunGlobalScriptFromTopMountedDir("system_start.lua"))
	{
		LogError("Unable to locate script file system_start.lua");
		return false;
	}
	
	if (m_bRebuildCacheData)
	{
		m_bRebuildCacheData = false;

		if (!m_rebuildUserName.empty())
		{
			SetUserProfileName(m_rebuildUserName);
		}
		//load all maps
		CL_Display::clear(CL_Color(0,0,0));
		BlitMessage("... loading all maps, please wait ...", (GetScreenY/2) -30);

		m_worldManager.PreloadAllMaps();

		CL_Display::clear(CL_Color(0,0,0));
		BlitMessage("... linking all navigation graphs ...", (GetScreenY/2) -30);

		//then rebuild node data
		g_worldNavManager.LinkEverything();
		CL_Display::clear(CL_Color(0,0,0));
		BlitMessage("... saving out navgraph and tagcache data ...", (GetScreenY/2) -30);

		//now save everything, nothing will have changes except tagcache data
		m_worldManager.SaveAllMaps();

		SetRestartEngineFlag(true);
		return true;
	}

#ifdef _DEBUG
	//NovaZip nova;
	//nova.InstallWorld("/Users/Seth/Desktop/Dink.novazip");
	//nova.InstallWorld(GetWorldsDirPath() + "/Dink7zip.novazip");
	//nova.InstallWorld(GetWorldsDirPath() + "/Dink.novazip");

	//CL_Zip_Archive z;
	//z.add_file("readme.txt");
	//z.save("crap.zip");

#endif
	
	if (! RunGlobalScriptFromTopMountedDir("game_start.lua"))
	{
		LogError("Unable to locate script file game_start.lua");
		return false;
	}

	return true;
}


void GameLogic::OnScreenChanged()
{
	EntEditor * pEnt = (EntEditor *)EntityMgr->GetEntityByName("editor");

	if (pEnt)
	{
		//the editor is open, lets reset it..
		pEnt->OnScreenChanged();
	}
}


bool GameLogic::ToggleEditMode() //returns true if the it just turned ON the editor
{
	EntEditor * pEnt = (EntEditor *)EntityMgr->GetEntityByName("editor");
	
	if (!pEnt)
	{
		if (m_modPaths.empty())
		{
			//assume they want to edit something in the base
			if (!g_pMapManager->GetActiveMap())
			{
				BaseGameEntity *pEnt = EntityMgr->GetEntityByName("ChooseWorldDialog");
				if (pEnt) pEnt->SetDeleteFlag(true);
				GetMyWorldManager()->SetActiveMapByName("Generic Palette");

			} else
			{
				BaseGameEntity *pEnt = EntityMgr->GetEntityByName("ChooseWorldDialog");
				if (pEnt) pEnt->SetDeleteFlag(true);
			}
		}
		
		if (!g_pMapManager->GetActiveMap())
		{
			ShowMessage("Oops", "Can't open world editor, no map is loaded.");
			return NULL;
		} 

		if (!GetApp()->GetFont(C_FONT_GRAY))
		{
			ShowMessage("Oops", "Can't use the editor, GetGameLogic:InitGameGUI was not called from the game_start.lua, need this for the fonts.");
			return NULL;
		}
		//take the focus off the player
		GetCamera->SetEntTracking(0);
		pEnt = (EntEditor *) m_myEntityManager.Add( new EntEditor);
		return true;
	} 
	
	//guess we need to turn it off
	pEnt->SetDeleteFlag(true);
	return false;
}

void GameLogic::OnPlayerDeleted(int id)
{
	if (m_pPlayer)
	{
		if (m_pPlayer->ID() == id)
		{
			LogMsg("Player was removed?  Be careful.  Use GetGameLogic:SetPlayer(NIL);");
			m_pPlayer = 0;
		}
	}
	
}

void GameLogic::SetMyPlayer(MovingEntity *pNew)
{
	
	if (pNew && !pNew->GetTile()->GetParentScreen())
	{
		LogError("Error, this that doesn't exist on map is trying to say it's a player?");
		return;
	}
	if (m_pPlayer && pNew != m_pPlayer)
	{
		//tell the entity that currently holds the players focus he just lost it
		int newID = 0;
		if (pNew != 0) newID = pNew->ID();
		
		/*
		if (m_pPlayer->GetBrainManager()->GetBrainBase())
		{
			m_pPlayer->GetBrainManager()->SendToBrainBase("lost_player_focus="+CL_String::from_int(newID));
		}
		*/
	}
	
	//also let's tell the script about it
	if (pNew)
	{
		m_pPlayer = pNew;
		luabind::globals(GetScriptManager->GetMainState())["g_playerID"] = m_pPlayer->ID();

		/*
		//no longer needed
		if (m_pPlayer->GetBrainManager()->GetBrainBase())
		{
			m_pPlayer->GetBrainManager()->SendToBrainBase("got_player_focus");
		}
		*/

		//also setup ourselves to be notified when it is destroyed
		m_playerDestroyedSlot = pNew->sig_delete.connect(this, &GameLogic::OnPlayerDeleted);
		
	} else
	{

		if (m_pPlayer)
		{
			if (GetCamera->GetEntTracking() == m_pPlayer->ID())
			{
				//shouldn't track this anymore		
				GetCamera->SetEntTracking(0);
			}
		}
		luabind::globals(GetScriptManager->GetMainState())["g_playerID"] = 0;
		m_pPlayer = pNew;
	}

	luabind::globals(GetScriptManager->GetMainState())["GetPlayer"] = m_pPlayer;

}

void GameLogic::OnKeyUp(const CL_InputEvent &key)
{
		if (g_inputManager.HandleEvent(key, false) )
		{
			//they handled it, let's ignore it
			return;
		}
}

bool GameLogic::IsRetail()
{
	if (!GetApp()->GetMyScriptManager()) return false;

	return (luabind::globals(GetApp()->GetMyScriptManager()->GetMainState())["g_isRetail"] == true);
}
void GameLogic::OnKeyDown(const CL_InputEvent &key)
{
		
	if (GetShowMessageActive())
	{
		if (key.id == CL_KEY_ESCAPE || key.id == CL_KEY_ENTER)
			
			if (m_pShowMessageWindow)
			{
				m_pShowMessageWindow->quit();
			}
	}
	
	if (g_inputManager.HandleEvent(key, true))
		{
			//they handled it, let's ignore it
			return;
		}
	
	if (!GetEditorActive())
	{

		switch (key.id)
		{

		case CL_KEY_F:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				ToggleShowFPS();
			}
			break;


		case CL_KEY_S:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				if (!IsRetail())
				SetShowEntityCollisionData(!GetShowEntityCollisionData());
			}
			break;

		case CL_KEY_H:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
				{
					if (!IsRetail())
						SetShowPathfinding(!GetShowPathfinding());
				} 
			}
			break;

		case CL_KEY_Q:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
					if (g_pMapManager->GetActiveMap())
					{
						if (!IsRetail())
						g_pMapManager->GetActiveMapCache()->SetDrawCollision(!g_pMapManager->GetActiveMapCache()->GetDrawCollision());
					}

			}
			break;

	
		
		case CL_KEY_J:
			if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
			{
				if (!IsRetail())
				SetShowAI(!GetShowAI());
			}
			break;


		}

	}
}

void GameLogic::OnMouseDown(const CL_InputEvent &key)
{
		if (g_inputManager.HandleEvent(key, true))
		{
			return;
		}
}

void GameLogic::OnMouseUp(const CL_InputEvent &key)
{

		if (g_inputManager.HandleEvent(key, false))
		{
			return;
		}

}

void GameLogic::RebuildBuffers()
{
    //note, currently, this can get called before Init() is called!
    GetApp()->GetBackgroundCanvas()->get_gc()->clear(CL_Color(0,255,255));
}

void GameLogic::Kill()
{
	//BlitMessage("Saving data...");


	if (g_pMapManager->GetActiveMap())
	{
		//the main world is active, let's save out the current player position I guess
		*g_pMapManager->GetActiveMap()->GetCameraSetting() = GetCamera->GetCameraSettings();
	}
	
	if (EntityMgr->GetEntityByName("coleditor"))
	{
		//we were editing collision data, let's kill it without saving
		EntityMgr->GetEntityByName("coleditor")->SetDeleteFlag(true);
	}

	//g_EntEditModeCopyBuffer.ClearSelection();
	
	//save out warp cache
	g_worldNavManager.Save();

	
	m_worldManager.Kill();
	SaveWorldData();
	SaveGlobals();
	g_EffectManager.Reset();
	m_myEntityManager.Kill();

	SAFE_DELETE(m_pGUIStyle);
	SAFE_DELETE(m_pGUIResources);
	SAFE_DELETE(m_pGUIManager);

	if (g_pSoundManager)
	{
		g_pSoundManager->Kill();
	}

	g_MessageManager.Reset();
	
	m_strUserProfileName.clear();
	g_materialManager.Init();
	g_watchManager.Clear();
	GetVisualProfileManager->Kill();
	m_postLogicUpdateCallback.Reset();
	//flush key input
	CL_System::keep_alive(); 


}

void GameLogic::InitGameGUI(string xmlFile)
{
	if (m_pGUIResources)
	{
		LogError("Can't init game GUI twice");
		return;
	}

	if (g_VFManager.LocateFile(xmlFile))
	{
		m_pGUIResources = new CL_ResourceManager(xmlFile);
		m_pGUIStyle = new CL_StyleManager_Bitmap(m_pGUIResources);
		m_pGUIManager = new CL_GUIManager(m_pGUIStyle);

		// Everytime the GUI draws, let's draw the game under it
		m_slots.connect(m_pGUIManager->sig_paint(), this, &GameLogic::OnRender);

		GetApp()->SetFont(C_FONT_GRAY,  new CL_Font("font_gray", m_pGUIResources));
		GetApp()->SetFont(C_FONT_NORMAL, new CL_Font("font_dialog", m_pGUIResources));

	} else
	{
		LogError("Unable to find GUI xml %s in base or mounted paths.", xmlFile.c_str());
	}


}

void ClearCacheDataFromWorldPath(string worldPath)
{
	string fileToDelete = worldPath+"/"+string(C_WORLD_NAV_FILENAME);

	//first kill it's main navgraph file
	RemoveFile(fileToDelete);
	
	//next scan each map path and remove cached data

	//scan map directory for available maps
	CL_DirectoryScanner scanner;

	scanner.scan(worldPath+"/"+string(C_BASE_MAP_PATH), "*");
	
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (scanner.is_directory())
		{
			if (scanner.get_name()[0] != '_')
				if (scanner.get_name()[0] != '.')
				{
					//no underscore at the start, let's show it
					fileToDelete = worldPath+"/"+string(C_BASE_MAP_PATH)+"/"+scanner.get_name()+"/"+string(C_TAGCACHE_FILENAME);
					RemoveFile(fileToDelete);
				}

		}
	}
}

void GameLogic::DeleteAllCacheFiles()
{
		LogMsg("Deleting all tagcache files..");
		ClearCacheDataFromWorldPath(g_VFManager.GetLastMountedDirectory());
}

bool GameLogic::IsShuttingDown()
{
	return m_bRestartEngineFlag;
}

void GameLogic::Update(float step)
{

	ResetLastUpdateTimer(); //this is checked to see if we need to show a "please wait, loading" type thing
	g_drawManager.Update(step);
	if (m_bRestartEngineFlag)
	{
		LogMsg("Restarting engine...");
		g_Console.SetOnScreen(false);
		Kill();
		
		if (m_bRebuildCacheData)
		{
			DeleteAllCacheFiles();
		}
		m_bRestartEngineFlag = false;

		GetApp()->BuildCommandLineParms();
		Init();
	}
	
	if (g_pSoundManager)
		g_pSoundManager->UpdateSounds();

	g_MessageManager.Update();
//	LogMsg("Cam zoom is %s", VectorToString(&GetCamera->GetScale()).c_str());
	m_myEntityManager.Update(step); //only big entities are here, like the the choice menu, world select dialog, ent cache manager, not each little one
	
	if (!GetGamePaused())
	{
		m_postLogicUpdateCallback.OnActivate(step);
	}
	GetCamera->Update(step);

	m_worldManager.PrepareUpdate(step);
	m_worldManager.Update(step);
	g_textManager.Update(step);

	
}


void GameLogic::RenderGameGUI(bool bDrawMainGUIToo)
{
	if (GetApp()->GetRenderedGameGUI() == false)
	{
		GetApp()->SetRenderedGameGUI(true);
		ClearScreen();

		m_worldManager.Render();
		m_myEntityManager.Render(); 
		g_textManager.Render();

		g_Console.Render();

		if (bDrawMainGUIToo)
		{
			GetApp()->GetGUI()->show(); //this will trigger its OnRender() which happens right before
		}

		g_Console.RenderPostGUI();

		if (GetShowFPS()) 
		{
			CL_Font *pFont = GetApp()->GetFont(C_FONT_NORMAL);
			if (!pFont)
			{
				pFont = GetApp()->GetConsoleFont();
			}
			ResetFont(pFont);

			int tiles = 0;
			if (g_pMapManager->GetActiveMapCache())
			{
				tiles = g_pMapManager->GetActiveMapCache()->GetTilesRenderedLastFrameCount();
			}
			static char buff[256];
			sprintf(buff, "FPS:%d E:%d T:%d: W:%d", GetApp()->GetFPS(),g_pMapManager->GetUpdateEntityCount(),  tiles, g_watchManager.GetWatchCount());
			pFont->draw(GetScreenX-265,0, buff);
			
			
		}
	}

}

void GameLogic::OnRender()
{
	//when the game GUI is in a modal dialog, this is the only thing causes everything else to get
	//rendered
	RenderGameGUI(true);
}


//when the main GUI is in a modal dialog box, this is the only thing causes the game gui and game screens
//to get rendered underneath

void GameLogic::Render()
{

	RenderGameGUI(false);
	GetApp()->SetRenderedGameGUI(false);

	if (GetApp()->GetGUI()->get_modal_component())
	{
		//need to jump start so it renders every frame
		GetApp()->GetGUI()->update();

		if (GetApp()->GetRequestedQuit())
		{
			GetApp()->GetGUI()->get_modal_component()->quit();
		}
	}
}

//generally, this means a script sent us a text message through the message scheduler. 
//Which we'll let the main script instance run..

void GameLogic::HandleMessageString(const string &msg)
{
  //LogMsg("Gamelogic got %s", msg.c_str());
  GetScriptManager->RunString(msg.c_str());
}

void SetCameraToTrackPlayer()
{
	MovingEntity * pEnt = (MovingEntity *)GetGameLogic()->GetMyPlayer();

	if (pEnt)
	{
		if (pEnt->GetTile()->GetParentScreen()->GetParentMapChunk()->GetParentMap() == g_pMapManager->GetActiveMap())
		{
			GetCamera->SetEntTracking(pEnt->ID());
			GetCamera->InstantUpdate();
		}  else
		{
			//he's not from our world
		}
	}
}

void MovePlayerToCamera()
{
	MovingEntity * pEnt = (MovingEntity *)GetGameLogic()->GetMyPlayer();

	if (pEnt)
	{
		CL_Vector2 playerPos = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(GetScreenX/2, GetScreenY/2));
		pEnt->SetPos(playerPos);
		
		GetCamera->SetEntTracking(pEnt->ID());
		GetCamera->InstantUpdate();
		assert(pEnt->GetPos() == playerPos);
	} else
	{
		//assert(!"Can't find the player");
	}
}

void GameLogic::ClearAllMapsFromMemory()
{
   LogMsg("Clearing all maps");
   g_textManager.Reset();

   m_worldManager.Kill();
   
}

void GameLogic::SetShowMessageActive( bool bNew, CL_Window *pMsgWindow )
{
	m_bShowingMessageWindow = bNew;
	m_pShowMessageWindow = pMsgWindow;
}

vector<string> GameLogic::GetModPaths()
{
	return m_modPaths;
}

void MoveCameraToPlayer()
{
	MovingEntity * pEnt = (MovingEntity *)GetGameLogic()->GetMyPlayer();

	if (pEnt)
	{
		GetCamera->SetEntTracking(0);
		GetCamera->SetTargetPos(CL_Vector2(GetPlayer->GetPos().x, GetPlayer->GetPos().y));
		GetCamera->InstantUpdate();
	}
}


void ShowMessage(string title, string msg, bool bForceClassicStyle)
{
	CL_GUIManager *pStyle = GetApp()->GetGUI();

	GetGameLogic()->SetGamePaused(true);

	if (!bForceClassicStyle && GetGameLogic()->GetGameGUI())
	{
			NS_MessageBox m(GetGameLogic()->GetGameGUI(), title, msg, "Ok", "", "");
			GetGameLogic()->SetShowMessageActive(true, &m); //so it knows not to send mouse clicks to the engine while
			m.run();
			GetGameLogic()->SetShowMessageActive(false);
			GetGameLogic()->SetGamePaused(false);
			GetApp()->ClearTimingAfterLongPause();
			return;
	}
	
	CL_MessageBox message(title, msg, "Ok", "", "", pStyle);
	GetGameLogic()->SetShowMessageActive(true, &message); //so it knows not to send mouse clicks to the engine while

	//we're showing this
	message.run();
	GetGameLogic()->SetShowMessageActive(false);
	GetGameLogic()->SetGamePaused(false);
	GetApp()->ClearTimingAfterLongPause();

}


void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg)
{
	g_MessageManager.Schedule(deliveryMS, targetID, pMsg);
}

void ScheduleSystem(unsigned int deliveryMS, unsigned int targetID,const char * pMsg)
{
	g_MessageManager.ScheduleSystem(deliveryMS, targetID, pMsg);
}

bool RunGlobalScriptFromTopMountedDir(const char *pName)
{
	string s = GetGameLogic()->GetScriptRootDir()+"/";
	s += pName;

	StringReplace("\\", "/", s);
	if (s[s.length()-1] == 'a') s+="c"; //try to grab the compiled version first
	if (!g_VFManager.LocateFile(s))
	{
		//try again
		s = s.substr(0, s.length()-1); //cut the .c off and try again
		if (!g_VFManager.LocateFile(s))
		{
			return false;
		}
	}


	//run a global script to init anything that needs doing
	GetScriptManager->LoadMainScript( s.c_str());
	return true;
}


//lua doc info for natural docs processor.  Be careful how you move it, it will break..
/*

Object: GameLogic

Contains general game-related functions.

About:

This is a global object that can always be accessed.

Usage:

(code)
GetGameLogic:Quit(); //quits the game
(end)

Group: Profile Related

func: SetUserProfileName
(code)
boolean SetUserProfileName(string name)
(end)

Re-initializes everything and attempts to load the profile sent in.  Think of this as loading a saved game.  If no profile exists, it is created.

Dangerous and illegal characters are stripped automatically, so the profile name may be slightly different than the one you sent in.

Parameters:

name - the name of the profile.  If you say, "Player", it will create/load/save profiles/Player/(active world name).

Returns:
False on error (for instance, the name passed in is not a valid path, or tries to access somewhere illegal), true on success
*/

/*
func: GetUserProfileName
(code)
string GetUserProfileName()
(end)

Returns:
The active user profile name. Will return a blank string if empty. (ie, "", not nil)
*/

/*
func: ResetUserProfile
(code)
nil ResetUserProfile(string name)
(end)

This completely deletes a profile, if it existed.

Input is stripped of dangerous/illegal characters.

Parameters:

name - The profile name you want to delete.

Note:

Novashell never uses a dangerous deltree type way to delete things, it carefully verifies and scans the directory deletes only valid novashell data.

*/

/*
func: UserProfileExists
(code)
boolean UserProfileExists(string name)
(end)

Parameters:

name - The profile name you want to check for.  Is stripped for dangerous/illegal characters.

Returns:

True if the profile exists.
*/

/*
func: UserProfileActive
(code)
boolean UserProfileActive()
(end)

Returns:

True if a user profile is currently loaded/active.


Group: Miscellaneous Functions

func: ToggleEditMode

(code)
boolean ToggleEditMode()
(end)
Enable/Disable the in-game editor.

Returns:
True if the it just turned ON the editor, false if it was just turned off
*/


/*
func: SetRestartEngineFlag
(code)
nil SetRestartEngineFlage(boolean bRestart)
(end)

Parameters:

bRestart - If true, the engine will save all modified data and restart as soon as cybernetic ally possible.
*/


/*


func: AddCallbackOnPostLogicUpdate
(code)
nil AddCallbackOnPostLogicUpdate(string callbackName, number entityID)
(end)

Allows you to add a function to be called after the entities logic updates happen.  I added this as an easy way to make a game loop without needing to connect it with an entity.

The callback is automatically deleted when an <Entity> dies. In the case of a global function, there is no way to unregister it.

Note:

It must accept a single parm, this "Step" is a number that contains the 'step-size' used in the logic update, similar to an entities Update(step).

Usage:
(code)
//Example to make a global game loop function, run this somewhere at the start:
GetGameLogic:AddCallbackOnPostLogicUpdate("OnGameLoop", C_ENTITY_NONE);

//this is run every logic tick

function OnGameLoop(step)
	LogMsg("Game loop update");

	//shake the camera randomly.. earthquake!
	GetCamera:SetPos(GetCamera:GetPos() + Vector2(10-Random(20), 10-Random(20)));

end

(end)

Parameters:

string callBackName - The name of the callback function that should be called. (Must exist in your code)
entityID - If it's an <Entity> function and not a global function, you'll need to enter the <Entity>'s ID here.  Otherwise, <C_ENTITY_NONE>.


func: IsShuttingDown
(code)
boolean IsShuttingDown()
(end)

When a game is closing or restarting all maps are 'shutdown'.  During this time, each entity has its OnKill() run, during times like this is may be useful to know if the engine is currently shutting down or not, to avoid trying to access things that might already be deleted.

Returns:

True if the engine is currently shutting down.  (Either to exit, or to restart, both have the same shutdown process)
*/

/*
func: ClearModPaths
(code)
nil ClearModPaths()
(end)

Removes all overlaid paths.  This should be done right before doing a SetRestartEngineFlag() call.  Allows you to re-mount directories or go back to the world selection dialog. (that's default if no mods are mounted)
*/

/*
func: AddModPath
(code)
nil AddModPath(string pathName)
(end)

This lets you mount a world path.  Scripts, images, and resources are loaded in reverse order of mounting, ie, the last thing you mounted gets checked first.

Note:
"Base" is always mounted first automatically.

Worlds are also automatically mounted if specified in its .novashell configuration file.

Parameters:

pathName - A name of a world to mount.  Example: "Test World".  
*/

/*
func: InitGameGUI
(code)
nil InitGameGUI(string xmlFileName)
(end)

This xml file contains data about fonts, sizes, and colors that should be used for game dialogs and menus.

By editing this, you can use your own custom bitmap fonts.

Check the default one in base/game_gui/standard.xml for an example.

Note:

A world can only initialize this once.

Parameters:

xmlFileName - A path and filename to an xml file.
*/

/*
func: Data
(code)
DataManager Data()
(end)

This let's you access a global <DataManager> that is automatically saved and loaded with the game per user profile.

Note:

Never use <DataManager::Clear> on this, the save-load game system uses it to store certain things as well, such as the game time.

Returns:

A global <DataManager> object to store/retrieve anything you wish per profile.
*/

/*
func: WorldData
(code)
DataManager WorldData()
(end)

This let's you access a global <DataManager> that is automatically saved and loaded with the game.

Note:

Unlike <DataManager::Data> because it is shared between profiles.  Useful for specifying global game tweaking variables that will never change for instance.

Returns:

A global <DataManager> object to store/retrieve anything you wish.
*/


/*
func: Quit
(code)
nil Quit()
(end)

Immediately closes the application.  Changed game data is auto-saved.
*/
