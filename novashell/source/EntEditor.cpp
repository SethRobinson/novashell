#include "AppPrecomp.h"
#include "EntEditor.h"
#include "MyEntityManager.h"
#include "GameLogic.h"
#include "EntChooseScreenMode.h"
#include "GeneratorSimple.h"
#include "EntMapCache.h"
#include "EntEditMode.h"
#include "GeneratorDink.h"
#include "Console.h"
#include "AI/WorldNavManager.h"
#include "DataEditor.h"
#include "WorldPackager.h"
#include <algorithm>
#include "MapExportXML.h"

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

EntEditor::EntEditor() : BaseGameEntity(BaseGameEntity::GetNextValidID())
{
   m_bDialogOpen = false;
   m_pWindow = NULL;
   m_pWorldListWindow = NULL;
   m_pLabel = NULL;
   m_pMenu = NULL;
   m_pButton = NULL;
   m_bScrollingMap = false;
   m_pButtonToggleEditMode = NULL;
   m_pShowGridCheckBox = NULL;
   m_pResListBox = NULL;
   m_bGenerateActive = false;
   m_pGenerator = NULL;
   m_pListBoxWorld = NULL;
   m_vecLastMousePos = CL_Point(0,0);
   m_bShowWorldChunkGridLines = g_pMapManager->GetActiveMapCache()->GetDrawWorldChunkGrid();
   m_bShowCollision = g_pMapManager->GetActiveMapCache()->GetDrawCollision();
   m_bShowPathfinding = GetGameLogic()->GetShowPathfinding();
   m_bShowAI = GetGameLogic()->GetShowAI();

   m_slot.connect(CL_Keyboard::sig_key_down(), this, &EntEditor::onButtonDown);
   m_slot.connect(GetGameLogic()->GetMyWorldManager()->sig_map_changed, this, &EntEditor::OnMapChange);

   m_slot.connect(CL_Mouse::sig_move(),this, &EntEditor::OnMouseMove);
   m_slot.connect(CL_Mouse::sig_key_down(),this, &EntEditor::OnMouseDown);
   m_slot.connect(CL_Mouse::sig_key_up(),this, &EntEditor::OnMouseUp);

   m_pLayerLabel = NULL;
   m_pListLayerActive = NULL;
   m_pListLayerDisplay = NULL;
   m_pLayerListWindow = NULL;
   m_bHideMode = false;
   
   GetGameLogic()->SetGamePaused(true);

   for (int i=0; i < e_modeCount; i++)
   {
	   m_modeCheckBoxArray[i] = NULL;
   }
   SetName("editor");
   Init();
   ScheduleSystem(1, ID(), "OnOpenEditor");


}


void EntEditor::Kill()
{
	DisableAllModes();

	SAFE_DELETE(m_pListBoxWorld);
	SAFE_DELETE(m_pWorldListWindow);

	KillLayerListStuff();

	SAFE_DELETE(m_pGenerator);
	SAFE_DELETE(m_pMenu);
	SAFE_DELETE(m_pButtonToggleEditMode);
	SAFE_DELETE(m_pButton);
	SAFE_DELETE(m_pLabel);
	SAFE_DELETE(m_pWindow);

	for (int i=0; i < e_modeCount; i++)
	{
		m_modeCheckBoxArray[i] = NULL;
	}

	m_pLayerLabel = NULL;
	m_pListLayerActive = NULL;
	m_pListLayerDisplay = NULL;
	m_pLayerListWindow = NULL;

}

EntEditor::~EntEditor()
{
	
	Kill();
	SetCameraToTrackPlayer();
	GetGameLogic()->SetGamePaused(false); //unpause the game if it was
	GetGameLogic()->SetEditorActive(false);
	GetGameLogic()->SetShowGrid(false);

	//GetApp()->GetGUI()->close();
	GetScriptManager->RunFunction("OnCloseEditor");

}

void EntEditor::KillLayerListStuff()
{
	SAFE_DELETE(m_pLayerLabel);
	SAFE_DELETE(m_pListLayerDisplay);
	SAFE_DELETE(m_pListLayerActive);
	SAFE_DELETE(m_pLayerListWindow);

}

void EntEditor::OnMouseUp(const CL_InputEvent &key)
{
	switch(key.id)
	{
		case CL_MOUSE_LEFT:
				m_bScrollingMap = false;
		break;
	}
}

void EntEditor::OnScreenChanged()
{
		Init();
}

void EntEditor::OnMouseDown(const CL_InputEvent &key)
{

	switch(key.id)
	{

		case CL_MOUSE_LEFT:
			if (CL_Keyboard::get_keycode(CL_KEY_SPACE))
			{
				//if they hold down space, let's let them scroll the map, photoshop style
				m_bScrollingMap = true;
				return;
			}
		break;
	}

}

void EntEditor::onButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_F:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			GetGameLogic()->ToggleShowFPS();
			m_pMenuShowFPSCheckbox->set_selected(GetGameLogic()->GetShowFPS());
		}
		break;

	case CL_KEY_G:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL) && CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{
			OnToggleWorldChunkGrid();
			return;
		}
		
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnToggleGrid();
		}
		break;

	case CL_KEY_P:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnToggleGamePaused();
		}
		break;

	case CL_KEY_S:
		if (IsControlKeyDown())
		{
			OnSaveMap();
		}
		break;

	case CL_KEY_H:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
			{
				OnToggleShowPathfinding();
			} 
		}
		break;

	

	case CL_KEY_Q:

		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
					OnToggleShowCollision();
		}
		break;

	case CL_KEY_J:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnToggleShowAI();
		}
		break;

	case CL_KEY_1:
	case CL_KEY_2:
	case CL_KEY_3:
	case CL_KEY_4:
	case CL_KEY_5:
	case CL_KEY_6:
	case CL_KEY_7:
	case CL_KEY_8:
	case CL_KEY_9:

		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			if (!m_pWorldListWindow)
			{
				//make the map select active
				BuildWorldListBox();
			}

			int selection = key.id-CL_KEY_1;
			//LogMsg("Chose %d", selection);
			m_pListBoxWorld->set_current_item(selection);

		}
		break;
	}
}

void EntEditor::OnMouseMove(const CL_InputEvent &key)
{
	if (CL_Mouse::get_keycode(CL_MOUSE_MIDDLE) || m_bScrollingMap)
	{
		CL_Point ptOffset = m_vecLastMousePos-CL_Point(CL_Mouse::get_x(), CL_Mouse::get_y());
		
		//LogMsg("Moving %d %d", ptOffset.x, ptOffset.y);

		GetCamera->SetTargetPos(g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(ptOffset.x, ptOffset.y)));
		GetCamera->InstantUpdate();
	}
	m_vecLastMousePos = key.mouse_pos;
}


void EntEditor::validator_numbers(char &character, bool &accept)
{
	// my VC6 don't like std::isdigit with locale :(
	
	if (character != '.' && character != '-' && (character < '0' || character > '9') && character != ' ')
		accept = false;
}

void EntEditor::OnToggleWorldChunkGrid()
{
	m_bShowWorldChunkGridLines = !m_bShowWorldChunkGridLines;
	g_pMapManager->GetActiveMapCache()->SetDrawWorldChunkGrid(m_bShowWorldChunkGridLines);
}

void EntEditor::OnToggleGrid()
{
	GetGameLogic()->SetShowGrid(!GetGameLogic()->GetShowGrid());
	m_pShowGridCheckBox->set_selected(GetGameLogic()->GetShowGrid());
}

void EntEditor::OnToggleGamePaused()
{
	GetGameLogic()->SetGamePaused(!GetGameLogic()->GetGamePaused());
	m_pMenuGamePausedCheckbox->set_selected(GetGameLogic()->GetGamePaused());
	
}

void EntEditor::OnToggleShowCollision()
{
	m_bShowCollision= !m_bShowCollision;
	g_pMapManager->GetActiveMapCache()->SetDrawCollision(m_bShowCollision);
}

void EntEditor::OnToggleShowPathfinding()
{
	m_bShowPathfinding= !m_bShowPathfinding;
	GetGameLogic()->SetShowPathfinding(m_bShowPathfinding);
}

void EntEditor::OnToggleShowAI()
{
	m_bShowAI= !m_bShowAI;
	GetGameLogic()->SetShowAI(m_bShowAI);
}

void EntEditor::OnToggleParallax()
{
	GetGameLogic()->SetParallaxActive(!GetGameLogic()->GetParallaxActive());
}

void EntEditor::MapOptionsDialog()
{
	m_bDialogOpen = true;
	// Creating InputDialog
	CL_InputDialog dlg("Map Properties", "Ok", "Cancel","", GetApp()->GetGUI());
	dlg.set_event_passing(false);
	// Save pointers to input_boxes to connect signals for character validating
	CL_InputBox *pGrid = dlg.add_input_box("Map Chunk Size In Pixels:", CL_String::from_int(g_pMapManager->GetActiveMap()->GetMapChunkPixelSize()));
	CL_InputBox *pCache = dlg.add_input_box("Cache Sensitivity (0 for none, 1 for double the viewable area):", CL_String::from_float(g_pMapManager->GetActiveMap()->GetCacheSensitivity()));
	CL_InputBox *pThumb = dlg.add_input_box("Auto Thumbnail Size: (0 for none):", CL_String::from_int(g_pMapManager->GetActiveMap()->GetThumbnailWidth()));
	CL_InputBox *pBGColor = dlg.add_input_box("BG Color: (in the format of r g b a)", ColorToString(g_pMapManager->GetActiveMap()->GetBGColor()));
	CL_InputBox *pGravity = dlg.add_input_box("Gravity:", SimplifyNumberString(CL_String::from_float(g_pMapManager->GetActiveMap()->GetGravity())));
	CL_InputBox *pZoom = dlg.add_input_box("Current Camera Zoom (1 1 for normal):", VectorToString(&GetCamera->GetScale()));
	CL_CheckBox *pPersistent = dlg.add_check_box("Persistent (changes saved for each player profile)", g_pMapManager->GetActiveMap()->GetPersistent(), 270);
	CL_CheckBox *pAutoSave = dlg.add_check_box("Auto Save (if false, must use File->Save)", g_pMapManager->GetActiveMap()->GetAutoSave(), 200);

	CL_SlotContainer slots;

	// Connecting signals, to allow only numbers
	slots.connect(pGrid->sig_validate_character(), this, &EntEditor::validator_numbers);
	slots.connect(pCache->sig_validate_character(), this, &EntEditor::validator_numbers);
	slots.connect(pThumb->sig_validate_character(), this, &EntEditor::validator_numbers);
	slots.connect(pBGColor->sig_validate_character(), this, &EntEditor::validator_numbers);
	slots.connect(pGravity->sig_validate_character(), this, &EntEditor::validator_numbers);

	slots.connect(pZoom->sig_validate_character(), this, &EntEditor::validator_numbers);
	// Run dialog
	dlg.run();
	m_bDialogOpen = false;

	// Show some feedback to user
	std::string clicked_button;
	switch(dlg.get_result_button())
	{
	case 0:
		clicked_button = "Ok"; break;
	case 1:
		clicked_button = "Cancel"; break;
	default:
		return;
	}
	
	tile_list tileListOfPointersOnly;
	tile_list tileList;

	if (clicked_button == "Ok")
	{
		bool requireClear = false;

		int grid = CL_String::to_int(pGrid->get_text());
		int thumbSize = CL_String::to_int(pThumb->get_text());
		if (grid != g_pMapManager->GetActiveMap()->GetMapChunkPixelSize()) requireClear = true;
	
		if (requireClear)
		{
			//this change so we need to be careful here
			if (!ConfirmMessage("Warning!", "These changes might take a while to convert the map.\nAre you sure?")) return;
		BlitMessage("Converting map...");
		if (
			(g_pMapManager->GetActiveMap()->GetWorldRectInPixels().right / grid > SHRT_MAX)
			||
			(g_pMapManager->GetActiveMap()->GetWorldRectInPixels().bottom / grid > SHRT_MAX)
			)
		{
			if (!ConfirmMessage("Warning!", "This would create out of range chunks.  If you continue, we're going to clear the map.\n\n(Try a larger number to avoid this error)."))
			{
				return;
			}

			//we're going to have to actually clear everything
			g_pMapManager->GetActiveMap()->DeleteExistingMap();
			g_pMapManager->GetActiveMap()->SetMapChunkPixelSize(grid);
			GetCamera->Reset();
			GetCamera->InstantUpdate();
		}

		vector<unsigned int> layerIDVec;
		g_pMapManager->GetActiveMap()->GetLayerManager().PopulateIDVectorWithAllLayers(layerIDVec);
		g_pMapManager->GetActiveMapCache()->AddTilesByRect(g_pMapManager->GetActiveMap()->GetWorldRectInPixels(), &tileListOfPointersOnly, layerIDVec);
		//make a copy of each tile
		CloneTileList(tileListOfPointersOnly, tileList);
		}

		//set all the defaults
		g_pMapManager->GetActiveMap()->SetMapChunkPixelSize(grid);
		g_pMapManager->GetActiveMap()->SetCacheSensitivity(CL_String::to_float(pCache->get_text()));
		g_pMapManager->GetActiveMap()->SetBGColor(StringToColor(pBGColor->get_text()));
		g_pMapManager->GetActiveMap()->SetGravity(CL_String::to_float(pGravity->get_text()));
		g_pMapManager->GetActiveMap()->SetPersistent(pPersistent->is_checked());
		g_pMapManager->GetActiveMap()->SetAutoSave(pAutoSave->is_checked());
		GetCamera->SetScale(StringToVector(pZoom->get_text()));

		if (requireClear)
		{
			//reinit the world		
			g_pMapManager->GetActiveMap()->DeleteExistingMap();
			g_pMapManager->GetActiveMap()->Init();
			g_pMapManager->GetActiveMapCache()->ClearCache();

			//re-add all the tiles we had deleted
			tile_list::iterator itor = tileList.begin();
			
			while(itor != tileList.end())
			{
				g_pMapManager->GetActiveMap()->AddTile(*itor);
				itor++;
			}

		} else
		{
			if (thumbSize != g_pMapManager->GetActiveMap()->GetThumbnailWidth())
			{
				g_pMapManager->GetActiveMap()->SetThumbnailWidth(thumbSize);
				g_pMapManager->GetActiveMap()->SetThumbnailHeight(thumbSize);
				g_pMapManager->GetActiveMap()->InvalidateAllThumbnails();

			}
		}

		GetGameLogic()->GetMyWorldManager()->sig_map_changed();
	}

}

void EntEditor::OnClose()
{
  	SetDeleteFlag(true);
}

void EntEditor::OnCloseButton(CL_SlotParent_v0 &parent_handler)
{
	OnClose(); 
}

void EntEditor::SetTipLabel(std::string tip)
{
	m_pLabel->set_text(tip);
}

void EntEditor::SetDefaultTipLabel()
{
   m_pLabel->set_text("Use the mouse wheel or +/- to zoom in/out.\n\nHold and drag the middle mouse button to pan.");
}

void EntEditor::OnSaveMap()
{

	BlitMessage("Saving " + g_pMapManager->GetActiveMap()->GetName());
	g_pMapManager->GetActiveMap()->ForceSaveNow();
}

void EntEditor::OnExportXML()
{
	m_bDialogOpen = true;
	MapExportXML w;

	w.Init();
	m_bDialogOpen = false;

}

void EntEditor::OnExit()
{
	GetApp()->RequestAppExit();
}

void EntEditor::OnRevertChanges()
{
	if (ConfirmMessage("Revert Changes", "Undo all changes made to map " + g_pMapManager->GetActiveMap()->GetName()+" since it was last saved?"))
	{
		LogMsg("Reverting changes to map.");
		g_pMapManager->GetActiveMap()->SetAutoSave(false);
		g_pMapManager->GetActiveMap()->SetPersistent(false);
		
		string mapName = g_pMapManager->GetActiveMap()->GetName();
		//clear it from mem
		g_pMapManager->UnloadMapByName(mapName);

		//load it back up
		g_pMapManager->SetActiveMapByName(mapName); //reload it

		
	}
}

void EntEditor::OnToggleShowEntityCollision()
{
	GetGameLogic()->SetShowEntityCollisionData(!GetGameLogic()->GetShowEntityCollisionData());
}

void EntEditor::OnToggleLockAtRefresh()
{
	if (GetApp()->GetRefreshType() == App::FPS_AT_REFRESH)
	{
		GetApp()->SetRefreshType(App::FPS_UNLIMITED);
		GetApp()->ClearTimingAfterLongPause();
	} else 
	{
		GetApp()->SetRefreshType(App::FPS_AT_REFRESH);
		GetApp()->ClearTimingAfterLongPause();
}

	//update our checkbox
	m_pMenuLockAtRefreshCheckbox->set_selected(GetApp()->GetRefreshType() == App::FPS_AT_REFRESH);

}

void EntEditor::OnResetCamera()
{
	GetScriptManager->RunFunction("OnEditorResetCamera", true);
}

void EntEditor::OnResetCameraScale()
{
	GetScriptManager->RunFunction("OnEditorResetCameraScale", true);
}


void EntEditor::OnDumpEngineStatistics()
{
	LogMsg("**** Engine statistics ****");
	g_TagManager.PrintStatistics();
	GetHashedResourceManager->PrintStatistics();
	g_inputManager.PrintStatistics();
	LogMsg("");
	LogMsg("*******");
	LogMsg("Press ` (backtick/unshifted tilde) to close this window.");
	g_Console.SetOnScreen(true);
}

void EntEditor::OnRestart()
{
	/*
	if (GetWorld && !GetWorld->GetPersistent() && GetWorld->GetModified())
	{
		if (!ConfirmMessage("Restart Engine", "Map has unsaved changed.  Are you sure you wish to quit?")) return;

	}
	*/

	GetScriptManager->RunFunction("OnRestartEngine", true);
}

void OpenScriptForEditing(string scriptName)
{
	string file = scriptName;

	if (!FileExists(file))
	{
		if (!ConfirmMessage("Script doesn't exist", file + " not found.\n\nWould you like to create this script?")) return;
		
		CL_OutputSource *pFile = g_VFManager.PutFile(file);
		
		if (!pFile)
		{
			LogError("Unable to create the file %s.  Bad directory maybe?", file.c_str());
			return;
		}
		string text;
		
		text = "\nfunction OnInit() //run upon initialization\r\n\r\nend\r\n\r\n"; pFile->write(text.c_str(), text.size());
		text = "function OnPostInit() //run during the entity's first logic update\r\n\r\nend\r\n\r\n"; pFile->write(text.c_str(), text.size());
		text = "function OnKill() //run when removed\r\n\r\nend\r\n\r\n"; pFile->write(text.c_str(), text.size());
		SAFE_DELETE(pFile);
		
		g_VFManager.LocateFile(file);
		
	}

	LogMsg("Launching %s...", file.c_str());

#ifdef WIN32
	
	//for some reason, under Vista, we suddenly need full paths which sort of confuses things as sometimes we are sent links that already
	//have one..

	if (file.length()>4 && file[1]!=':')
	{
		//prepend the full path
		file = CL_Directory::get_current() + "/"+file;
	}
	std::replace(file.begin(),file.end(),'\\','/');
	open_file(GetApp()->GetHWND(), file.c_str());
#elif __APPLE__
	
	if (CL_Display::is_fullscreen())
	{
		LogMsg("Switching to windowed mode, otherwise you won't see the text editor pop up.");
		GetApp()->ToggleWindowedMode();
	}
	
	FSRef theRef;
	int err = FSPathMakeRef((StringPtr)file.c_str(), &theRef, NULL);
	if (err)
	{
		LogError("FSPathMakRef reported error an error converting the path.");
		return;
	}
	
	int ret = LSOpenFSRef(&theRef, NULL);
	if (ret == 0)
			{
				LogError("Unable to open %s.  Invalid path maybe??", file.c_str());
			}
	
	FSRef application;
	LSGetApplicationForItem( &theRef, kLSRolesAll, &application, NULL);

	if (ret != noErr)
	{
		ret = 	LSGetApplicationForInfo( 'TEXT', kLSUnknownCreator, CFSTR("txt"), kLSRolesAll, &application, NULL);
	}
	
	if (ret == noErr)
	{
		LSLaunchFSRefSpec launchSpec = { &application, 1, &theRef, NULL, kLSLaunchAndDisplayErrors | kLSLaunchDontAddToRecents | kLSLaunchAsync, NULL};
		ret = LSOpenFromRefSpec( &launchSpec, NULL);
	}

	if (ret != noErr)
	{
		LogError("Unable to open editor associated with .lua or .txt files");
	}

#else
   char *parms[2];
        parms[1] = 0;
        parms[0] = (char*)file.c_str();
    

		if (CL_Display::is_fullscreen())
		{
			LogMsg("Switching to windowed mode, otherwise you won't see the text editor pop up.");
			GetApp()->ToggleWindowedMode();
		}
		
//linux
	
	pid_t pid;
	pid=fork();



         if (pid==0)
        {
			string editor = GetApp()->Data()->Get("linux_editor");
			if (execlp(editor.c_str(),editor.c_str(), file.c_str(), NULL)<0)
                {
                           LogError("Unknown error trying to execv %s %s.  Check the editor settings in Options->Novashell System Preferences.",
							   editor.c_str(), file.c_str());
                }
                else
                {
                
                }
	  exit(0);
                                      
        } else
        {
              // LogError("Unknown error trying to fork.");
        }
          
#endif


}


void EntEditor::OnOpenScript()
{
	string original_dir = CL_Directory::get_current();
	string dir = original_dir + "/" + GetGameLogic()->GetScriptRootDir();
	CL_Directory::change_to(dir);
	CL_FileDialog dlg("Open LUA Script", "", "*.lua", GetApp()->GetGUI());

	//dlg.set_behavior(CL_FileDialog::quit_file_selected);
	dlg.set_event_passing(false);
	dlg.run();
	CL_Directory::change_to(original_dir);

	if (dlg.get_pressed_button() == CL_FileDialog::button_ok)
	{
		string fname = dlg.get_path() +"/" + dlg.get_file();

		if (fname.size() < dir.size())
		{
			//imprecise way to figure out if they went to a wrong directory and avoids an STL crash.  really, I
			//need to convert the slashes to forward slashes in both strings and check if dir is in fname.

			LogMsg("Can't open this file, outside of our media dir.");
			return;
		}

		OpenScriptForEditing(fname);
	}
}

void EntEditor::SetHideMode(bool bHide)
{
	LogMsg("Toggling hide mode");
	m_bHideMode = bHide;
	
	m_pWindow->show(!bHide);
	if (m_pWorldListWindow) m_pWorldListWindow->show(!bHide);
	if (m_pLayerListWindow) m_pLayerListWindow->show(!bHide);
	
	sig_hide_mode_changed(bHide); //broadcast this to anybody who is interested
} 

bool EntEditor::GetHideMode()
{
	return m_bHideMode;
}

string EntEditor::HandleMessageString(const string &msg)
{
	vector<string> words = CL_String::tokenize(msg, "|");

	if (words[0] == "toggle_hide")
	{
		SetHideMode(!GetHideMode());

	} else
		if (msg == "OnOpenEditor")
		{
			GetScriptManager->RunFunction("OnOpenEditor");

		} else
		
		{
			LogMsg("Don't know how to handle message %s", msg.c_str());
		}

	return "";
}



void EntEditor::OnAddNewMap()
{
	
	CL_InputDialog dlg("Add New Map Dialog", "Create New Map Dir", "Cancel", "",GetApp()->GetGUI());
	dlg.set_event_passing(false);

	CL_InputBox *pName = dlg.add_input_box("Map name", "A Cool Map", 600);
	pName->set_tab_id(0);

	dlg.get_button(0)->set_tab_id(1);
	dlg.get_button(1)->set_tab_id(2);

	dlg.set_focus();
	pName->set_focus();
	dlg.run();

	if (dlg.get_result_button() == 0)
	{
		string mapName = GetGameLogic()->GetBaseMapPath() +  pName->get_text();
		
		if (pName->get_text().size() > 0)
		{
			if (CL_Directory::create(g_VFManager.GetLastMountedDirectory() + "/" + mapName))
			{
				CL_MessageBox::info("Created", "Map directory "+mapName+" created.  Note, this just makes a directory in the maps dir.  You can cut/paste/copy from file explorer too.", GetApp()->GetGUI());
			
				g_pMapManager->AddMap(mapName);
				BuildWorldListBox();

				//activate the new map now?
				g_pMapManager->SetActiveMapByPath(mapName);
	
			} else
			{
				CL_MessageBox::info("Failed?", "Couldn't create "+mapName+".  Maybe it already existed?", GetApp()->GetGUI());
			}

		} else
		{
			LogMsg("Bad directory name");
		}
	}
}

void EntEditor::OnEditStartupLua()
{
	string file = GetGameLogic()->GetScriptRootDir();
	file += "/system_start.lua";
	g_VFManager.LocateFile(file);
	OpenScriptForEditing(file);
}

void EntEditor::OnEditGameStartLua()
{
	string file = GetGameLogic()->GetScriptRootDir();
	file += "/game_start.lua";
	g_VFManager.LocateFile(file);
	OpenScriptForEditing(file);
}

void EntEditor::OnEditSetupConstants()
{
	string file = GetGameLogic()->GetScriptRootDir();
	file += "/system/setup_constants.lua";
	g_VFManager.LocateFile(file);
	OpenScriptForEditing(file);

}
void EntEditor::OnDumpWorldNavStatistics()
{
	g_worldNavManager.DumpStatistics();
	g_Console.SetOnScreen(true);

}

void EntEditor::OnRebuildNavigationMaps()
{
	
	if (!ConfirmMessage("Rebuild tag/world navigation cache data", "This may clear up any weird data problems from the result of a crash or merging maps.\n\nMake sure the world/mod/saved game is loaded that you would like this applied to.\n\nThis may take a long time, are you sure?")) return;

	GetGameLogic()->RequestRebuildCacheData();
	
}

void EntEditor::OnSetScreenSize()
{
	int index = m_pResListBox->get_current_item();
	CL_ListItem *pItem = m_pResListBox->get_item(index);
	
	//let's extract the res and change to it
	vector<string> word =  CL_String::tokenize(pItem->str.c_str()," ", true);
	GetApp()->SetScreenSize(CL_String::to_int(word[0]), CL_String::to_int(word[2]));
}

void EntEditor::PopulateResolutionList()
{
	m_pResListBox->clear();
	std::vector<CL_DisplayMode> modes = CL_DisplayMode::get_display_modes();

	for(unsigned int i=0; i < modes.size(); ++i)
	{
		if (modes[i].get_bpp() == 32)
		{
			//if this was essentially the same as the last mode, skip it
			if (i > 0 && modes[i-1].get_bpp() == modes[i].get_bpp())
			{
				if (modes[i-1].get_resolution() == modes[i].get_resolution())
				{
					continue;
				}
			}

			m_pResListBox->insert_item(CL_String::from_int(modes[i].get_resolution().width) + " by " + CL_String::from_int(
				modes[i].get_resolution().height) + " pixels");
		}
	}

	if (m_pResListBox->get_count() == 0)
	{
		//well, OSX doesn't enumerate video modes yet.  Let's do a hack..
		m_pResListBox->insert_item("640 by 480 pixels");
		m_pResListBox->insert_item("800 by 600 pixels");
		m_pResListBox->insert_item("1024 by 768 pixels");
		m_pResListBox->insert_item("1440 by 900 pixels");
		m_pResListBox->insert_item("1600 by 1200 pixels");
		m_pResListBox->insert_item("1680 by 1050 pixels");
		m_pResListBox->insert_item("1920 by 1080 pixels");
		m_pResListBox->insert_item("1920 by 1200 pixels");

	}
}

bool EntEditor::Init()
{

	Kill();
	assert(!m_pWindow);

string windowLabel = ""; //"Editor Tools Palette";

if (GetGameLogic()->GetUserProfileName().empty())
{
	windowLabel +=" Editing ** MASTER MAP FILES **, be careful!";
} else
{
	windowLabel += "Editing map data in profile '"+GetGameLogic()->GetUserProfileName() + "'.";
}

	m_pWindow = new CL_Window(CL_Rect(0, 0, GetScreenX, C_EDITOR_MAIN_MENU_BAR_HEIGHT), windowLabel, CL_Window::close_button, GetApp()->GetGUI()->get_client_area());
   // m_slotClose = m_pWindow->sig_close().connect(this, &EntEditor::OnClose);
    m_pWindow->set_event_passing(false);
	m_pWindow->set_topmost_flag(true);
	m_slotClose = m_pWindow->sig_close_button_clicked().connect_virtual(this, &EntEditor::OnCloseButton);

    m_pMenu = new CL_Menu(m_pWindow->get_client_area());
	m_pMenu->set_event_passing(false);
	
	CL_MenuNode *pItem;
	pItem = m_pMenu->create_item("File/Toggle Edit Mode (F1)");
    m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnClose);

	pItem = m_pMenu->create_item("File/Open Script/Choose");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnOpenScript);

	
	pItem = m_pMenu->create_item("File/Open Script/system_start.lua");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnEditStartupLua);

	pItem = m_pMenu->create_item("File/Open Script/game_start.lua");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnEditGameStartLua);

	pItem = m_pMenu->create_item("File/Open Script/setup_constants.lua");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnEditSetupConstants);

	
	pItem = m_pMenu->create_item("File/Save Active Map Now (Ctrl+S)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnSaveMap);

	pItem = m_pMenu->create_item("File/Export Map/Export as .xml (roughly Ogmo .eol format)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnExportXML);

	pItem = m_pMenu->create_item("File/Revert Changes On Active Map");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnRevertChanges);

	pItem = m_pMenu->create_item("File/Quick Exit (Alt+F4)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnExit);

	pItem = m_pMenu->create_item("File/Restart Engine (Ctrl+Shift+R)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnRestart);

	pItem = m_pMenu->create_item("File/World Packaging & Distribution");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnWorldPackage);

	pItem = m_pMenu->create_toggle_item("Mode/Map Info");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnChooseScreenMode);
	m_modeCheckBoxArray[e_modeMap] = static_cast<CL_MenuItem*>(pItem->get_data());
	
	pItem = m_pMenu->create_toggle_item("Mode/Tile Edit");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleEditMode);
	m_modeCheckBoxArray[e_modeTileEdit] = static_cast<CL_MenuItem*>(pItem->get_data());

	pItem = m_pMenu->create_item("Options/Map Properties");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::MapOptionsDialog);

	pItem = m_pMenu->create_item("Options/Map User Properties");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnMapUserProperties);

	pItem = m_pMenu->create_item("Options/World Global Properties");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnGameGlobalProperties);

	pItem = m_pMenu->create_item("Options/Profile Global Properties");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnWorldProperties);

	pItem = m_pMenu->create_item("Options/Novashell System Preferences");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnPreferences);


	pItem = m_pMenu->create_item("Utilities/Clear Map");
	m_slot.connect(pItem->sig_clicked(), this, &EntEditor::OnClearMap);

	pItem = m_pMenu->create_item("Utilities/Add New Map");
	m_slot.connect(pItem->sig_clicked(), this, &EntEditor::OnAddNewMap);

	pItem = m_pMenu->create_item("Utilities/Rebuild World Navigation Cache");
	m_slot.connect(pItem->sig_clicked(), this, &EntEditor::OnRebuildNavigationMaps);

	pItem = m_pMenu->create_item("Display/Video Options/Toggle Fullscreen (Alt+Enter, Command+F)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleFullScreen);

	pItem = m_pMenu->create_toggle_item("Display/Video Options/Toggle Lock At Refresh Rate");
	m_pMenuLockAtRefreshCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuLockAtRefreshCheckbox->set_selected(GetApp()->GetRefreshType() == App::FPS_AT_REFRESH);
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleLockAtRefresh);


	// Create a listbox within menu
	CL_MenuNode *list_node = m_pMenu->create_node("Display/Video Options/Set Screen Size/Resolutions");
	list_node->set_close_on_click(false);
	m_pResListBox = new CL_ListBox(CL_Rect(17,0,140,400), list_node );
	
	PopulateResolutionList();

	//slots.connect( menu_list->sig_mouse_dblclk(), this, &App::OnListDoubleClick);
	
	m_slot.connect( m_pResListBox->sig_selection_changed(), this, &EntEditor::OnSetScreenSize);

	pItem = m_pMenu->create_toggle_item("Display/Video Options/Toggle FPS Display (Ctrl+F)");
	m_pMenuShowFPSCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuShowFPSCheckbox->set_selected(GetGameLogic()->GetShowFPS());
	m_slot.connect(pItem->sig_clicked(),GetGameLogic(), &GameLogic::ToggleShowFPS);

	pItem = m_pMenu->create_toggle_item("Display/Show Map Collision Data (Ctrl+Q)");
	m_pMenuShowCollisionCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuShowCollisionCheckbox->set_selected(m_bShowCollision);
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleShowCollision);

	pItem = m_pMenu->create_toggle_item("Display/Show Map Pathfinding Data (Ctrl+Shift+H)");
	m_pMenuShowPathfindingCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuShowPathfindingCheckbox->set_selected(m_bShowPathfinding);
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleShowPathfinding);

	pItem = m_pMenu->create_toggle_item("Display/Show Entity Goal and AI Data (Ctrl+J)");
	m_pMenuShowAICheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuShowAICheckbox->set_selected(m_bShowAI);
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleShowAI);

	pItem = m_pMenu->create_toggle_item("Display/Parallax Scrolling");
	m_pMenuParallaxCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuParallaxCheckbox->set_selected(GetGameLogic()->GetParallaxActive());
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleParallax);

	pItem = m_pMenu->create_toggle_item("Display/Show Entity Collision Processing");
	m_pMenuShowEntityCollisionCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuShowEntityCollisionCheckbox->set_selected(GetGameLogic()->GetShowEntityCollisionData());
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleShowEntityCollision);

	pItem = m_pMenu->create_toggle_item("Display/Show Grid Lines (Ctrl+G)");
	m_pShowGridCheckBox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pShowGridCheckBox->set_selected(GetGameLogic()->GetShowGrid());
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleGrid);

	pItem = m_pMenu->create_toggle_item("Display/Show Map Chunk Grid Lines (Ctrl+Shift+G)");
	m_pMenuWorldChunkCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuWorldChunkCheckbox->set_selected(m_bShowWorldChunkGridLines);
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleWorldChunkGrid);



	pItem = m_pMenu->create_item("Display/Show\\Hide Editor Palettes (Tab)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnShowHidePalettes);


	pItem = m_pMenu->create_item("Display/Dump Engine Statistics");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnDumpEngineStatistics);

	pItem = m_pMenu->create_item("Display/Dump World Nav Statistics");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnDumpWorldNavStatistics);

	pItem = m_pMenu->create_item("Display/ ");
	pItem = m_pMenu->create_item("Display/(backtick to toggle the System Console");
	pItem->enable(false);


	//pItem = m_pMenu->create_item("MapGen/Generate Small Map (disabled right now)");
	//m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnGenerateSmall);

	//pItem = m_pMenu->create_item("MapGen/Generate Large Map (disabled right now)");
	//m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnGenerateLarge);
	
	//too touchy to include right now
	//pItem = m_pMenu->create_item("Utilities/Import from Dink Map");
	//m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnGenerateDink);

	pItem = m_pMenu->create_item("Utilities/Reset Camera Scale to 1:1 (R)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnResetCameraScale);
	pItem = m_pMenu->create_item("Utilities/Reset Camera Scale to 1:1 and position to 0,0 (Shift+R)");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnResetCamera);


	pItem = m_pMenu->create_item("Tool Window/Map Switcher");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::BuildWorldListBox);

	pItem = m_pMenu->create_item("Tool Window/Layer Control");
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::BuildLayerControlBox);

	m_pLabel = new CL_Label (CL_Point(5,25), "", m_pWindow->get_client_area());

	pItem = m_pMenu->create_toggle_item("Game/Game Paused (Ctrl+P)");
	m_pMenuGamePausedCheckbox = static_cast<CL_MenuItem*>(pItem->get_data());
	m_pMenuGamePausedCheckbox->set_selected(GetGameLogic()->GetGamePaused());
	m_slot.connect(pItem->sig_clicked(),this, &EntEditor::OnToggleGamePaused);
	DisableAllModes();
	SetDefaultTipLabel();

	/*
	int buttonPosY = 30;
	int buttonSizeY = 22;

    m_pButton = new CL_Button(CL_Point(10,buttonPosY), "Map Info Mode", m_pWindow->get_client_area());
    m_pButton->set_size(110,buttonSizeY);
    m_slot.connect(m_pButton->sig_clicked(),this, &EntEditor::OnChooseScreenMode);
	
    m_pButtonToggleEditMode = new CL_Button(CL_Point(130,buttonPosY), "Tile Edit Mode", m_pWindow->get_client_area());
    m_pButtonToggleEditMode->set_size(110,buttonSizeY);
    m_slot.connect(m_pButtonToggleEditMode->sig_clicked(),this, &EntEditor::OnToggleEditMode);
	*/
	
	GetGameLogic()->SetEditorActive(true);

	OnToggleEditMode(); //turn on the edit submode

	
#ifndef _DEBUG
	//these slow down my debug too much
	BuildWorldListBox();
	BuildLayerControlBox();

#endif

#ifdef _DEBUG
	//OnWorldPackage();
#endif

	return true;
}

void EntEditor::OnShowHidePalettes()
{
	SetHideMode(true);
}

void EntEditor::OnSelectMap()
{
	if (!g_pMapManager->GetMapInfoByPath(m_pListBoxWorld->get_current_text())->m_world.IsInitted())
	{
		BlitMessage("Loading map...");
	}
	g_pMapManager->SetActiveMapByPath(m_pListBoxWorld->get_current_text());
}

int EntEditor::GetWorldInfoListHeight()
{
	int height =(g_pMapManager->GetMapInfoList()->size()*12) +40;
	if (height > 500) height = 500;
	return height;
}

void EntEditor::BuildWorldListBox()
{
	SAFE_DELETE(m_pListBoxWorld);
	SAFE_DELETE(m_pWorldListWindow);

	int width = 300;
	int height =  GetWorldInfoListHeight();

	CL_Rect rectSize = CL_Rect(GetScreenX-width, C_EDITOR_MAIN_MENU_BAR_HEIGHT, 0, 0);
	rectSize.set_size(CL_Size(width,height));

	m_pWorldListWindow = new CL_Window(rectSize, "Map Switcher (Ctrl+1-9)" , CL_Window::close_button, GetApp()->GetGUI()->get_client_area());
	m_pWorldListWindow->set_event_passing(false);

	CL_Rect r =m_pWorldListWindow->get_children_rect();
	r.set_size(r.get_size() - CL_Size(5,28));
	m_pListBoxWorld = new CL_ListBox(r, m_pWorldListWindow->get_client_area());

	//add all maps
	
	map_info_list * pList = g_pMapManager->GetMapInfoList();
	map_info_list::iterator itor = pList->begin();

	while(itor != pList->end())
	{
		bool showIt = true;
		if ( (*itor)->m_world.GetDirPath().find("hidden_") == string::npos
			|| g_pMapManager->GetActiveMap()->GetDirPath() == (*itor)->m_world.GetDirPath())
		{
			int id = m_pListBoxWorld->insert_item((*itor)->m_world.GetDirPath());

			if (g_pMapManager->GetActiveMap()->GetDirPath() == (*itor)->m_world.GetDirPath())	
			{
				//this should be highlighted now
				m_pListBoxWorld->set_current_item(id);
			}

		}
		itor++;
	}
	
	m_pListBoxWorld->sort();
	m_slot.connect( m_pListBoxWorld->sig_selection_changed(), this, &EntEditor::OnSelectMap);
	m_slot.connect( m_pListBoxWorld->sig_mouse_dblclk(), this, &EntEditor::OnWorldDoubleClick);

}


void EntEditor::PopulateLayerListMenu()
{
	if (!m_pListLayerDisplay) return;

	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();
	
	m_pListLayerDisplay->clear();
	m_pListLayerActive->clear();

	unsigned int layers = layerMan.GetLayerCount();
	//we also want to do rudementry sorting
	vector<unsigned int> layerVec;
	layerMan.PopulateIDVectorWithAllLayers(layerVec);

	//sort it
	std::sort(layerVec.begin(), layerVec.end(), compareLayerBySort);
	
	int index;
	for (unsigned int i=0; i < layers; i++)
	{
		index = m_pListLayerDisplay->insert_item(layerMan.GetLayerInfo(layerVec[i]).GetName());
		m_pListLayerDisplay->get_item(i)->user_data = layerVec[i];
		m_pListLayerDisplay->set_selected(i, layerMan.GetLayerInfo(layerVec[i]).IsDisplayed() != 0);

		index = m_pListLayerActive->insert_item(layerMan.GetLayerInfo(layerVec[i]).GetName());
		m_pListLayerActive->set_selected(i, layerMan.GetLayerInfo(layerVec[i]).IsEditActive() != 0);
		m_pListLayerActive->get_item(i)->user_data = layerVec[i];
	}

	layerMan.BuildLists();
}

void EntEditor::PopUpLayerPropertiesDialog(int layerID)
{
	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();

	Layer *pLayer = &layerMan.GetLayerInfo(layerID);
	m_bDialogOpen = true;
	// Creating InputDialog
	string stTitle(CL_String::format("%1 Layer Properties", layerMan.GetLayerInfo(layerID).GetName()));

	CL_InputDialog dlg(stTitle, "Ok", "Cancel","Delete", GetApp()->GetGUI());
	dlg.set_event_passing(false);
	
	CL_InputBox *pName = dlg.add_input_box("Name", pLayer->GetName(), 100);
	
	// Save pointers to input_boxes to connect signals for character validating
	CL_InputBox *pScrollX = dlg.add_input_box("Parallax scroll modifier X: (0 for normal):", 
		CL_String::from_float(pLayer->GetScrollMod().x));
	CL_InputBox *pScrollY = dlg.add_input_box("Parallax scroll modifier Y: (0 for normal):", 
		CL_String::from_float(pLayer->GetScrollMod().y));

	CL_InputBox *pSort = dlg.add_input_box("Sorting level (Lower # is drawn behind higher #):",
		CL_String::from_int(pLayer->GetSort()) );

	CL_CheckBox *pCheckBoxEditorOnly = dlg.add_check_box("Show in editor only", 
		layerMan.GetLayerInfo(layerID).GetShowInEditorOnly() != 0);

	CL_CheckBox *pCheckBoxUseThumbnail = dlg.add_check_box("Show in thumbnail", 
		layerMan.GetLayerInfo(layerID).GetUseInThumbnail() != 0);

	CL_CheckBox *pCheckBoxUseParallaxInThumb = dlg.add_check_box("Use parallax in thumbnail", 
		layerMan.GetLayerInfo(layerID).GetUseParallaxInThumbnail() != 0, 150);

	CL_CheckBox *pCheckBoxHasCollisionData = dlg.add_check_box("Has Collision Data", 
		layerMan.GetLayerInfo(layerID).GetHasCollisionData() != 0, 150);

	CL_CheckBox *pCheckBoxDepthSortWithinLayer = dlg.add_check_box("Depth sort", 
		layerMan.GetLayerInfo(layerID).GetDepthSortWithinLayer() != 0, 150);

	CL_SlotContainer slots;

	// Connecting signals, to allow only numbers
	slots.connect(pScrollX->sig_validate_character(), this, &EntEditor::validator_numbers);
	slots.connect(pScrollY->sig_validate_character(), this, &EntEditor::validator_numbers);
	slots.connect(pSort->sig_validate_character(), this, &EntEditor::validator_numbers);

	// Run dialog
	dlg.run();
	m_bDialogOpen = false;

	// Show some feedback to user
	std::string clicked_button;
	switch(dlg.get_result_button())
	{
	case 0:
		clicked_button = "Ok"; break;
	case 1:
		clicked_button = "Cancel"; break;
	case 2:
		clicked_button = "Delete"; break;
	default:
		assert(!"Dumbass!");
	}

	if (clicked_button == "Delete")
	{
		if (ConfirmMessage("Delete layer "+pLayer->GetName(), "Are you sure you wish to delete this Layer?"))
		{
		    layerMan.Remove(layerID);
		}
	} else
	if (clicked_button == "Ok")
	{
		pLayer->SetScrollMod(
			CL_Vector2(CL_String::to_float(pScrollX->get_text()), CL_String::to_float(pScrollY->get_text()) )
			);

		pLayer->SetShowInEditorOnly(pCheckBoxEditorOnly->is_checked());
		pLayer->SetUseInThumbnail(pCheckBoxUseThumbnail->is_checked());
		pLayer->SetUseParallaxInThumbnail(pCheckBoxUseParallaxInThumb->is_checked());
		pLayer->SetHasCollisionData(pCheckBoxHasCollisionData->is_checked());
		pLayer->SetDepthSortWithinLayer(pCheckBoxDepthSortWithinLayer->is_checked());

		pLayer->SetName(pName->get_text());
		pLayer->SetSort(CL_String::to_int(pSort->get_text()));
	}

	//in case something changed
	PopulateLayerListMenu();
}

void EntEditor::OnLayerDoubleClick(const CL_InputEvent &input)
{
	int index = m_pListLayerActive->get_item(input.mouse_pos);
	if (index == -1) return;
	
	//LogMsg("Double click!");
	//m_pListLayerActive->	
	m_pListLayerActive->release_mouse();
	PopUpLayerPropertiesDialog(m_pListLayerActive->get_item(index)->user_data);
}

void EntEditor::OnLayerAdd()
{
	Layer l;
	l.SetName("New Layer");
	g_pMapManager->GetActiveMap()->GetLayerManager().Add(l);
	PopulateLayerListMenu();
}

void EntEditor::OnLayerHighlight()
{
	if (EntityMgr->GetEntityByName("edit mode"))
	{
		LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();
		EntEditMode *pEdit = dynamic_cast<EntEditMode*>(EntityMgr->GetEntityByName("edit mode"));
		const vector<unsigned int> &layerVec = layerMan.GetEditActiveList();
		pEdit->SelectByLayer(layerVec);
	} else
	{
		LogMsg("Must be in edit mode to highlight tiles.");
	}
}

void EntEditor::SetAllLayersActive(bool bNew)
{
	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();

	for (unsigned int i=0; i < layerMan.GetLayerCount(); i++)
	{
		layerMan.GetLayerInfo(i).SetIsEditActive(bNew);
	}

	PopulateLayerListMenu();

}
void EntEditor::OnLayerAll()
{
	SetAllLayersActive(true);
}

void EntEditor::OnLayerNone()
{
	SetAllLayersActive(false);
}

void EntEditor::BuildLayerControlBox()
{
	KillLayerListStuff();

	int boxWidth = 100;
	int boxHeight = 160;
	int spacingBetweenBoxes = 12;
	int boxTopOffset = 15;


	int width = (boxWidth*2)+spacingBetweenBoxes+7;
	int height = boxHeight+boxTopOffset+16+25;

	CL_Rect rectSize = CL_Rect(GetScreenX-width, C_EDITOR_MAIN_MENU_BAR_HEIGHT+GetWorldInfoListHeight(), 0, 0);
	rectSize.set_size(CL_Size(width,height));

	m_pLayerListWindow = new CL_Window(rectSize, "Layer Control (Dbl click to edit)" , CL_Window::close_button, GetApp()->GetGUI()->get_client_area());
	m_pLayerListWindow->set_event_passing(false);

	CL_Rect r =m_pLayerListWindow->get_children_rect();
	r.top += boxTopOffset;

	r.right = boxWidth;
	r.bottom = boxHeight;
	
	m_pListLayerDisplay= new CL_ListBox(r, m_pLayerListWindow->get_client_area());
	m_pListLayerDisplay->set_multi_selection(true);
	
	//setup position of the box on the right
	r.left = r.right+spacingBetweenBoxes;
	r.right = r.left + boxWidth;
	m_pListLayerActive = new CL_ListBox(r, m_pLayerListWindow->get_client_area());
	m_pListLayerActive->set_multi_selection(true);

	m_pLayerLabel = new CL_Label (CL_Point(37,0), "Display            Active For Edit", m_pLayerListWindow->get_client_area());

	CL_Point pt(3, m_pListLayerDisplay->get_client_y()+m_pListLayerDisplay->get_height());
	pt.y += 5; //spacing

	//add some buttons
	CL_Button *pHighlight = new CL_Button(pt, "Highlight", m_pLayerListWindow->get_client_area());
	int button_spacer = 15;
	pt.x = pHighlight->get_client_x()+pHighlight->get_width()+button_spacer;
	CL_Button *pAdd = new CL_Button(pt, "Add Layer", m_pLayerListWindow->get_client_area());
	pt.x = pAdd->get_client_x()+pAdd->get_width()+button_spacer;

	CL_Button *pAll = new CL_Button(pt, "All", m_pLayerListWindow->get_client_area());
	pt.x = pAll->get_client_x()+pAll->get_width()+button_spacer;
	CL_Button *pNone = new CL_Button(pt, "None", m_pLayerListWindow->get_client_area());

	PopulateLayerListMenu();

	m_slot.connect( m_pListLayerDisplay->sig_selection_changed(), this, &EntEditor::OnLayerChange);
	m_slot.connect( m_pListLayerActive->sig_selection_changed(), this, &EntEditor::OnLayerChange);
	m_slot.connect( m_pListLayerActive->sig_mouse_dblclk(), this, &EntEditor::OnLayerDoubleClick);
	m_slot.connect( pAdd->sig_clicked(), this, &EntEditor::OnLayerAdd);
	m_slot.connect( pHighlight->sig_clicked(), this, &EntEditor::OnLayerHighlight);
	m_slot.connect( pAll->sig_clicked(), this, &EntEditor::OnLayerAll);
	m_slot.connect( pNone->sig_clicked(), this, &EntEditor::OnLayerNone);

}

void EntEditor::OnLayerChange()
{
	
	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();

	//copy all settings to the real world
	for (int i=0; i < m_pListLayerActive->get_count();i++)
	{
		if ( (layerMan.GetLayerInfo(m_pListLayerDisplay->get_item(i)->user_data).IsDisplayed()!= NULL) != m_pListLayerDisplay->is_selected(i))
		{
			//a change was made to the display layer.  Force the edit layer to match, less confusing for the user
			m_pListLayerActive->set_selected(i, m_pListLayerDisplay->is_selected(i));
		}

		layerMan.GetLayerInfo(m_pListLayerDisplay->get_item(i)->user_data).SetIsDisplayed(m_pListLayerDisplay->is_selected(i));
		layerMan.GetLayerInfo(m_pListLayerActive->get_item(i)->user_data).SetIsEditActive(m_pListLayerActive->is_selected(i));
	}

   layerMan.BuildLists();
}

void EntEditor::OnToggleFullScreen()
{
	GetApp()->RequestToggleFullscreen();
}



void EntEditor::OnGenerateSmall()
{
	if (!ConfirmMessage("Confirmation that you aren't crazy", "This will destroy your current map.  Are you sure?")) return;
	
	int startX = C_DEFAULT_SCREEN_ID;
	int startY = C_DEFAULT_SCREEN_ID;

	BlitMessage("Deleting old map...");
	g_pMapManager->GetActiveMap()->DeleteExistingMap();
	g_pMapManager->GetActiveMap()->Init(CL_Rect(startX,startY,startX+8,startY+8));
  	
	UpdateAll();
    SimpleGenerateInit();
}

void EntEditor::SimpleGenerateInit()
{
	SAFE_DELETE(m_pGenerator);
	GetCamera->Reset();
	GetCamera->InstantUpdate();
    g_pMapManager->GetActiveMapCache()->ClearCache();
    m_pGenerator = new GeneratorSimple;
   
    int divider = 128;
    divider = min(g_pMapManager->GetActiveMap()->GetSizeX()*g_pMapManager->GetActiveMap()->GetSizeY()-1, divider);
    m_generateStepSize = (g_pMapManager->GetActiveMap()->GetSizeX()*g_pMapManager->GetActiveMap()->GetSizeY()) /divider;
    m_bGenerateActive = true;
    m_pGenerator->GenerateInit();
	ForceChooseScreenMode(true); //switch to view screen mode if it isn't already active
}


void EntEditor::UpdateAll()
{
    EntChooseScreenMode *pChoose = (EntChooseScreenMode*) EntityMgr->GetEntityByName("choose screen mode");
    
    if (pChoose)
    {
        pChoose->QuickInit();
    }
}

void EntEditor::OnGenerateLarge()
{
	if (!ConfirmMessage("Confirmation that you aren't crazy", "This will destroy your current map.  Are you sure?")) return;
	int startX = C_DEFAULT_SCREEN_ID;
	int startY = C_DEFAULT_SCREEN_ID;

	BlitMessage("Deleting old map...");
	g_pMapManager->GetActiveMap()->DeleteExistingMap();
	g_pMapManager->GetActiveMap()->Init(CL_Rect(startX,startY,startX+32,startY+32));
	UpdateAll();
    SimpleGenerateInit();
 }

void EntEditor::OnGenerateDink()
{
	if (!ConfirmMessage("Import From Dink Map", "The dink.dat, map.dat and ts*.bmp files need to be in the maps\\_base_graphics dir but as PNG, JPG or PCX.\n\nThis will destroy your current map.\n\nAre you sure?")) return;
	
	SAFE_DELETE(m_pGenerator);
	BlitMessage("Deleting old map...");
	g_pMapManager->GetActiveMap()->DeleteExistingMap();
	g_pMapManager->GetActiveMap()->SetDefaultTileSizeX(50);
	g_pMapManager->GetActiveMap()->SetDefaultTileSizeY(50);
	g_pMapManager->GetActiveMap()->SetMapChunkPixelSize(50*12); //match how dink screens where
	g_pMapManager->GetActiveMap()->Init();
	GetCamera->Reset();
	GetCamera->InstantUpdate();

	g_pMapManager->GetActiveMapCache()->ClearCache();

	UpdateAll();

	m_pGenerator = new GeneratorDink;
	m_generateStepSize = 1;
	m_bGenerateActive = true;
	m_pGenerator->GenerateInit();
	ForceChooseScreenMode(true); //switch to view screen mode if it isn't already active
}


void EntEditor::OnClearMap()
{
	if (!ConfirmMessage("Confirmation that you aren't crazy", "This will destroy your current map.  Are you sure?")) return;
	
	int startX = C_DEFAULT_SCREEN_ID;
	int startY = C_DEFAULT_SCREEN_ID;
    m_bGenerateActive = false;
	BlitMessage("Deleting old map...");
	g_pMapManager->GetActiveMap()->DeleteExistingMap();
	g_pMapManager->GetActiveMap()->Init(CL_Rect(startX,startY,startX+1,startY+1));
	GetCamera->Reset();
	GetCamera->InstantUpdate();
	g_pMapManager->GetActiveMapCache()->ClearCache();
}

void EntEditor::DisableAllModes()
{
	GetMyEntityMgr->TagEntityForDeletionByName("choose screen mode");
	GetMyEntityMgr->TagEntityForDeletionByName("edit mode");

	
	for (int i=0; i < e_modeCount; i++)
	{
		if (m_modeCheckBoxArray[i])
		{
			m_modeCheckBoxArray[i]->set_selected(false);
		}
	}
	
}

void EntEditor::OnChooseScreenMode()
{
	m_pMenu->collapse_submenus();

	if (!EntityMgr->GetEntityByName("choose screen mode"))
    {
        //it doesn't exist, so let's create this mode
        DisableAllModes();
		BaseGameEntity *pEnt= GetMyEntityMgr->Add(new EntChooseScreenMode() );
		m_modeCheckBoxArray[e_modeMap]->set_selected(true);

		//when its killed, let's get notified.
		m_slot.connect(pEnt->sig_delete, this, &EntEditor::OnModeEnded);

    } else
	{
		//GetMyEntityMgr->TagEntityForDeletionByName("choose screen mode");
		//m_bGenerateActive = false;
		m_modeCheckBoxArray[e_modeMap]->set_selected(true);
	}
}


void EntEditor::OnToggleEditMode()
{
	m_pMenu->collapse_submenus();

	if (!EntityMgr->GetEntityByName("edit mode"))
	{
		DisableAllModes();
		//it doesn't exist, so let's create this mode
		BaseGameEntity *pEnt= GetMyEntityMgr->Add(new EntEditMode() );
		//when it's killed, let's get notified.
		m_slot.connect(pEnt->sig_delete, this, &EntEditor::OnModeEnded);
		m_modeCheckBoxArray[e_modeTileEdit]->set_selected(true);

	} else
	{
		//no reason to allow no modes

		//GetMyEntityMgr->TagEntityForDeletionByName("edit mode");
		//m_bGenerateActive = false;
		m_modeCheckBoxArray[e_modeTileEdit]->set_selected(true);
	}
	
}

void EntEditor::OnModeEnded(int id)
{
	SetDefaultTipLabel();
}

void EntEditor::ForceChooseScreenMode(bool bOn)
{
	if (!EntityMgr->GetEntityByName("choose screen mode"))
	{
		//it doesn't exist, so toggle it on
		if (bOn) OnChooseScreenMode();
	} else
	{
		if (!bOn) OnChooseScreenMode();
	}
}


void EntEditor::Render(void *pTarget)
{
	string profile = "** Master Files **";

	if (!GetGameLogic()->GetUserProfileName().empty())
	{
		profile = GetGameLogic()->GetUserProfileName();
	}
	
	if (m_bHideMode)
	{
		//draw bar
		DrawTextBar(0, CL_Color(0,0,0,80),  profile + " - TAB to restore"); 
	}
}

void EntEditor::Update(float step)
{

    if (m_bGenerateActive)
    {
        for (int i=0; i < m_generateStepSize; i++)
        {
          if (m_bGenerateActive)
          {
              if (!m_pGenerator->GenerateStep())
              {
				  //all done with the generator
				  m_bGenerateActive = false;
				  SAFE_DELETE(m_pGenerator);

				  EntChooseScreenMode *pChoose = (EntChooseScreenMode*) EntityMgr->GetEntityByName("choose screen mode");

				  if (pChoose)
				  {
					  pChoose->GenerateThumbnailsIfNeeded();
				  }
			  }
		  }
		}
	}

}

void EntEditor::OnMapChange()
{
	if (g_pMapManager->GetActiveMapCache())
	{
		g_pMapManager->GetActiveMapCache()->SetDrawWorldChunkGrid(m_bShowWorldChunkGridLines);
		g_pMapManager->GetActiveMapCache()->SetDrawCollision(m_bShowCollision);
		PopulateLayerListMenu();
	}

}

void EntEditor::OnWorldDoubleClick( const CL_InputEvent &input )
{
	int index = m_pListBoxWorld->get_item(input.mouse_pos);
	if (index == -1) return;

	//LogMsg("Double click!");
	//m_pListLayerActive->	
	//m_pWorldListWindow->release_mouse();

	MapOptionsDialog();
}

void EntEditor::OnWorldProperties()
{
	if (!GetGameLogic()->UserProfileActive())
	{
		CL_MessageBox::info("No user profile is active.  First, start a game.", GetApp()->GetGUI());
		return;
	}
	
	DataEditor d;
	
	m_bDialogOpen = true;
	d.Init("User Profile Global Properties", "This data area is global and automatically saved and loaded with the player profile.\r\n"\
		"\r\nTo read or write this data by script, use GetGameLogic:Data() to return this as a DataManager object.\r\n"
		"\r\nKey names starting with an underscore mean the engine needs these for its own use.",
		GetGameLogic()->Data());
	m_bDialogOpen = false;
}

void EntEditor::OnGameGlobalProperties()
{
m_bDialogOpen = true;
	DataEditor d;
	d.Init("World Global Properties", "This data area is global and automatically saved and loaded with the game.\r\n"\
		"\r\nTo read or write this data by script, use GetGameLogic:WorldData() to return this as a DataManager object.\r\n"
		"\r\nKey names starting with an underscore mean the engine needs these for its own use.",
		GetGameLogic()->WorldData());
	m_bDialogOpen = false;
}

void EntEditor::OnMapUserProperties()
{
	if (!g_pMapManager->GetActiveMap())
	{
		CL_MessageBox::info("Load/create a map first.", GetApp()->GetGUI());
		return;
	}
	m_bDialogOpen = true;
	DataEditor d;
	d.Init("Map User Properties", "This data area is automatically saved and loaded with the current map.\r\n"\
		"\r\nTo read or write this data by script, guilt Seth into making a way to access its DataManager object.\r\n"
		"\r\nKey names starting with an underscore mean the engine needs these for its own use.",
		g_pMapManager->GetActiveMap()->GetDB());
	m_bDialogOpen = false;
}

void EntEditor::OnPreferences()
{
	m_bDialogOpen = true;
	DataEditor d;
	d.Init("Novashell System Preferences", "This data is stored in the prefs.dat file.\r\n"\
		"\r\nBy editing values, you can set the default video mode. (or desktop for the desktop's size)\r\n" \
		"\r\nParms starting with linux_, osx_, or windows_ mean they are only applicable to that particular system.\r\n" \
		"\r\nValid command_line_parms are -nosound, -nomusic, MyWorld.novashell, etc.",
		GetApp()->Data());
	m_bDialogOpen = false;
	GetApp()->BuildCommandLineParms();
}

void EntEditor::OnWorldPackage()
{
	m_bDialogOpen = true;
	WorldPackager w;

	w.Init();
	m_bDialogOpen = false;

}