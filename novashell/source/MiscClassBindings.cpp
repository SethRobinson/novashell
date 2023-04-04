
#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"

#include "VisualProfile.h"
#include "MaterialManager.h"
#include "MessageManager.h"
#include "TextManager.h"
#include "TagManager.h"
#include "InputManager.h"
#include "VisualProfileManager.h"
#include "AI/Goal_Think.h"
#include "AI/WatchManager.h"
#include "EntCreationUtils.h"
#include "ListBindings.h"
#include "DrawManager.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif


using namespace luabind;




enum
{
	//don't modify the order, the scripting counts on it
	C_RAY_ENTITIES = 0,
	C_RAY_TILE_PIC,
	C_RAY_EVERYTHING,
	C_RAY_DEBUG_MODE
};



Zone GetCollisionByRay(Map * pMap, CL_Vector2 vStartPos, CL_Vector2 vFacing, int actionRange, int raySpread, MovingEntity *pEntToIgnore, int mode, bool bIgnoreCreatures)
{

	int tileScanMode = C_TILE_TYPE_BLANK; //everything

	Tile *pTileToIgnore = NULL;

	if (pEntToIgnore) pTileToIgnore = pEntToIgnore->GetTile();
	bool bDebug = false;

	switch (mode)
	{
	case C_RAY_EVERYTHING: break;
	case C_RAY_ENTITIES: tileScanMode = C_TILE_TYPE_ENTITY; break;
	case C_RAY_TILE_PIC: tileScanMode = C_TILE_TYPE_PIC; break;
	case C_RAY_DEBUG_MODE:
		bDebug = true;
		break;
	default:
		LogError("GetCollisionByRay reports unknown mode: %d", mode);
	}

	CL_Vector2 vEndPos;
	vEndPos = vStartPos + (vFacing * actionRange);

	CL_Vector2 vCross = vFacing.cross();
	CL_Vector2 vColPos;

	Tile *pTile = NULL;

	tile_list tilelist;

	CL_Vector2 scanRectTopLeft = vStartPos;
	CL_Vector2 scanRectBottomRight = vEndPos;


	if (raySpread != 0) 
	{
		switch (  VectorToFacing(vFacing) )
		{
		case VisualProfile::FACING_DOWN:
		case VisualProfile::FACING_UP:
		case VisualProfile::FACING_LEFT:
		case VisualProfile::FACING_RIGHT:

			scanRectTopLeft -= vCross * raySpread*4;
			scanRectBottomRight += vCross * raySpread*4;
			break;

		}
	}


	CL_Rect r(scanRectTopLeft.x, scanRectTopLeft.y, scanRectBottomRight.x, scanRectBottomRight.y);

	//expand it to cover the other places we're going to scan

	pMap->GetMyMapCache()->AddTilesByRect(r, &tilelist,pMap->GetLayerManager().GetCollisionList(), true);
	GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode, bIgnoreCreatures );

	if (bDebug)
	{
		DrawRectFromWorldCoordinates(CL_Vector2(r.left, r.top), CL_Vector2(r.right, r.bottom),  CL_Color::white, CL_Display::get_current_window()->get_gc());
		DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
	}

	if (raySpread != 0)
	{
		if (!pTile)
		{
			//try again
			vEndPos += vCross * raySpread;

			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode , bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}

		if (!pTile)
		{
			//try again
			vEndPos += vCross * -(raySpread*2);
			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode , bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}



		//try even more

		if (!pTile)
		{
			//try again
			vEndPos += vCross * (raySpread*3);
			vStartPos += vCross * raySpread;
			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode , bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}

		if (!pTile)
		{
			//try again
			vEndPos += vCross * -(raySpread*4);
			vStartPos += vCross * (-raySpread*2);
			GetTileLineIntersection(vStartPos, vEndPos, tilelist, &vColPos, pTile, pTileToIgnore, tileScanMode, bIgnoreCreatures);
		}

		if (bDebug)
		{
			DrawLineWithArrowWorld(vStartPos, vEndPos, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
		}

	}

	if (bDebug)
	{
		CL_Display::flip(2); //show it now
	}

	if (pTile)
	{

		Zone z;
		z.m_entityID = C_ENTITY_NONE;
		z.m_materialID = CMaterial::C_MATERIAL_TYPE_NONE;


		z.m_vPos = vColPos;
		z.m_boundingRect = pTile->GetWorldColRect();
		line_list::iterator litor = pTile->GetCollisionData()->GetLineList()->begin();

		z.m_materialID = litor->GetType(); //kind of a hack, we're assuming there is only one type here.. bad us

		if (z.m_materialID == CMaterial::C_MATERIAL_TYPE_DUMMY)
		{
			//fine, we'll do one more check..
			if (pTile->GetCollisionData()->GetLineList()->size() > 1)
			{
				litor++;
				z.m_materialID = litor->GetType(); 
			}
		}

		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			z.m_entityID = ((TileEntity*)pTile)->GetEntity()->ID();
		}
		//LogMsg("Found tile at %.2f, %.2f", vColPos.x, vColPos.y);
		return z;
	}

	return g_ZoneEmpty;

}


TileList GetTileListByRect(Map *pMap, const CL_Rect &r, vector<unsigned int> &layerList, bool bWithCollisionOnly)
{
	tile_list tilelist;
	pMap->GetMyMapCache()->AddTilesByRect(r, &tilelist,layerList, bWithCollisionOnly);

	return (TileList(&tilelist));
}

string CameraToString(Camera * pCam)
{
	char stTemp[256];
	sprintf(stTemp, "A Camera at %s", PrintVector(pCam->GetPos()).c_str());
	return string(stTemp);
}

string MapToString(Map * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A Map named %s.", pObj->GetName().c_str());
	return string(stTemp);
}

string MapManagerToString(MapManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A MapManager. %d maps located.", pObj->GetTotalMapsAvailable());
	return string(stTemp);
}

string LayerManagerToString(LayerManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A LayerManager with %d layers.", pObj->GetLayerCount());
	return string(stTemp);
}

string SoundManagerToString(ISoundManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A SoundManager.");
	return string(stTemp);
}

string WatchManagerToString(WatchManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A WatchManager with %d entities being watched.", pObj->GetWatchCount());
	return string(stTemp);
}

string TagManagerToString(TagManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A TagManager with %d entries.", pObj->GetCount());
	return string(stTemp);
}

string TagToString(TagObject * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A Tag at %s on %s. (ID# %d)", PrintVector(pObj->GetPos()).c_str(), pObj->GetMapName().c_str(), pObj->GetID());
	return string(stTemp);
}

string ZoneToString(Zone * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "Zone");
	return string(stTemp);
}

string CameraSettingsToString(CameraSetting * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "CameraSettings");
	return string(stTemp);
}

string JoystickToString(JoystickInfo * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "Joystick %d: %s", pObj->GetID(), pObj->GetName().c_str());
	return string(stTemp);
}


string InputManagerToString(InputManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "InputManager");
	return string(stTemp);
}


string GoalManagerToString(Goal_Think * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "GoalManager with %d goals.", pObj->GetGoalCount());
	return string(stTemp);
}

string MaterialManagerToString(MaterialManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A MaterialManager with %d materials.", pObj->GetCount());
	return string(stTemp);
}
string MaterialToString(MaterialManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A Material.", pObj->GetCount());
	return string(stTemp);
}

string GameLogicToString(GameLogic * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "GameLogic");
	return string(stTemp);
}

string AppToString(App * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "App");
	return string(stTemp);
}

string TextManagerToString(TextManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "TextManager");
	return string(stTemp);
}


//more class definitions
void luabindMisc(lua_State *pState)
{
	module(pState)
		[

/*
Object: Color
The color object stores R G B A color information.

About:
Internally the color is stored as a 32 bit integer.


The range for the colors and alpha is between 0 and 255.

Usage:

(code)
myColor = Color(); //instantiate it

myColor:Set(255,255,255,255); //we just set it to white.

//Or we can save time by doing:

myColor = Color(255,255,255,255);

(end)

*/
		class_<CL_Color>("Color")
	
		//Group: Initialization
		
		/*
		func: Color()
		(code)
		Color(number red, number green, number blue, number alpha)
		(end)
		Creates the object with default values. (everything will be 0)
		*/

		.def(constructor<>())

		//func: Color(r,g,b,a)
		
		.def(constructor<unsigned int, unsigned int, unsigned int, unsigned int>())

		//Group: Member Functions

		//func: GetRed
		//(code)
		//number GetRed()
		//(end)
		.def("GetRed", &CL_Color::get_red)

		//func: GetGreen
		//(code)
		//number GetGreen()
		//(end)
		.def("GetGreen", &CL_Color::get_green)

		//Function: GetBlue
		//(code)
		//number GetBlue()
		//(end)
		.def("GetBlue", &CL_Color::get_blue)

		/*
		Function: GetAlpha
		(code)
		number GetAlpha()
		(end)
		
		Returns:
		the alpha component. (0 is transparent, 255 is opaque/fully visible)
		*/

		.def("GetAlpha", &CL_Color::get_alpha)

		//func: SetRed
		//(code)
		//nil SetRed(number red)
		//(end)
		.def("SetRed", &CL_Color::set_red)
		//func: SetGreen
		//(code)
		//nil SetGreen(number green)
		//(end)
		.def("SetGreen", &CL_Color::set_green)
		//func: SetBlue
		//(code)
		//nil SetBlue(number blue)
		//(end)
		.def("SetBlue", &CL_Color::set_blue)
		//func: SetAlpha
		//(code)
		//nil SetAlpha(number alpha)
		//(end)
		.def("SetAlpha", &CL_Color::set_alpha)

		/*
		func: Set
		(code)
		nil Set(number red, number green, number blue, number alpha)
		(end)
		Sets all the values at once.
		*/
		
		.def("Set", &CL_Color::set_color)
		.def("__tostring", &ColorToString)

		//Group: -=-=-=-=-=-
	
		,class_<MaterialManager>("MaterialManager")
		
		.def("AddMaterial", &MaterialManager::AddMaterial)
		.def("GetMaterial", &MaterialManager::GetMaterial)
		.def("__tostring", &MaterialManagerToString)


		,class_<CMaterial>("Material")
		.def("SetType", &CMaterial::SetType)
		.def("GetType", &CMaterial::GetType)
		.def("SetSpecial", &CMaterial::SetSpecial)
		.def("GetSpecial", &CMaterial::GetSpecial)
		.def("__tostring", &MaterialToString)

		,class_<GameLogic>("GameLogic")
		.def("ToggleEditMode", &GameLogic::ToggleEditMode)
		.def("SetUserProfileName", &GameLogic::SetUserProfileName)
		.def("GetUserProfileName", &GameLogic::GetUserProfileName)
		.def("ResetUserProfile", &GameLogic::ResetUserProfile)
		.def("UserProfileExists", &GameLogic::UserProfileExists)
		.def("SetRestartEngineFlag", &GameLogic::SetRestartEngineFlag)
		.def("IsShuttingDown", &GameLogic::IsShuttingDown)
		.def("UserProfileActive", &GameLogic::UserProfileActive)
		.def("SetPlayer", &GameLogic::SetMyPlayer)
		.def("Quit", &GameLogic::Quit)
		.def("ClearModPaths", &GameLogic::ClearModPaths)
		.def("AddModPath", &GameLogic::AddModPath)
		.def("Data", &GameLogic::Data)
		.def("WorldData", &GameLogic::WorldData)
		.def("InitGameGUI", &GameLogic::InitGameGUI)
		.def("__tostring", &GameLogicToString)
		.def("AddCallbackOnPostLogicUpdate", &GameLogic::AddCallbackOnPostLogicUpdate)

		,class_<App>("App")
//		.def("SetGameLogicSpeed", &App::SetGameLogicSpeed)
	//	.def("SetGameSpeed", &App::SetGameSpeed)
		.def("GetGameTick", &App::GetGameTick)
		.def("GetTick", &App::GetTick)
		.def("ParmExists", &App::ParmExists)
		.def("SetSimulationSpeedMod", &App::SetSimulationSpeedMod)
		.def("GetSimulationSpeedMod", &App::GetSimulationSpeedMod)
		.def("SetScreenSize", &App::SetScreenSize)
		.def("GetScreenSize", &App::GetScreenSize)
		.def("GetPreLaunchScreenSize", &App::GetPrelaunchScreenSize)

		.def("GetIsWindowed", &App::GetIsWindowed)
		.def("SetIsWindowed", &App::SetIsWindowed)


		.def("GetEngineVersion", &App::GetEngineVersion)
		.def("GetEngineVersionAsString", &App::GetEngineVersionAsString)
		.def("SetWindowTitle", &App::SetWindowTitle)
		.def("SetCursorVisible", &App::SetCursorVisible)
		.def("GetCursorVisible", &App::GetCursorVisible)
		.def("GetPlatform", &App::GetPlatform)
		.def("__tostring", &AppToString)
		.def("AddCallbackOnResolutionChange", &App::AddCallbackOnResolutionChange)

		,class_<ISoundManager>("SoundManager")
		//.def("PlayMusic", &ISoundManager::PlayMusic)
		//.def("KillMusic", &ISoundManager::KillMusic)
		//.def("PlayMixed", &ISoundManager::PlayMixed)
		.def("__tostring", &SoundManagerToString)

		.def("Play", &ISoundManager::PlayMixed)
		.def("PlayLooping", &ISoundManager::PlayLooping)
		.def("SetSpeedFactor", &ISoundManager::SetSpeedFactor)
		.def("SetVolume", &ISoundManager::SetVolume)
		.def("SetPan", &ISoundManager::SetPan)
		.def("SetPaused", &ISoundManager::SetPaused)
		.def("SetPriority", &ISoundManager::SetPriority)
		.def("IsPlaying", &ISoundManager::IsSoundPlaying)
		.def("AddEffect", &ISoundManager::AddEffect)
		.def("RemoveAllEffects", &ISoundManager::RemoveAllEffects)
		.def("Kill", &ISoundManager::KillChannel)

		.def("MuteAll", &ISoundManager::MuteAll)

		,class_<TextManager>("TextManager")
		.def("__tostring", &TextManagerToString)
		.def("Add", &TextManager::Add)
		.def("AddCustom", &TextManager::AddCustom)
		.def("AddCustomScreen", &TextManager::AddCustomScreen)

		,class_<LayerManager>("LayerManager")
		.def("__tostring", &LayerManagerToString)
		.def("GetLayerIDByName", &LayerManager::GetLayerIDByName)
		.def("GetCollisionLayers", &LayerManager::GetCollisionListNoConst)
		.def("GetVisibleLayers", &LayerManager::GetDrawListNoConst)
		.def("GetAllLayers", &LayerManager::GetAllListNoConst)
		
		,class_<Map>("Map")
		.def("__tostring", &MapToString)
	
		.def("SetPersistent", &Map::SetPersistent)
		.def("GetPersistent", &Map::GetPersistent)
		.def("SetAutoSave", &Map::SetAutoSave)
		.def("GetAutoSave", &Map::GetAutoSave)
		.def("GetName", &Map::GetName)
		.def("GetLayerManager", &Map::GetLayerManager)
		.def("BuildLocalNavGraph", &Map::BuildNavGraph)
		.def("GetCollisionByRay", &GetCollisionByRay)
		.def("GetTilesByRect", &GetTileListByRect)
		.def("GetWorldRect", &Map::GetWorldRectExact)
		.def("SetWorldRect", &Map::SetWorldRectExact)
		.def("ComputeWorldRect", &Map::ComputeWorldRect)

		,class_<MapManager>("MapManager")
		.def("__tostring", &MapManagerToString)

		.def("SetActiveMapByName", &MapManager::SetActiveMapByName)
		.def("GetActiveMap", &MapManager::GetActiveMap)
		.def("UnloadMapByName", &MapManager::UnloadMapByName)
		.def("LoadMapByName", &MapManager::LoadMapByName)
		
		,class_<TagManager>("TagManager")
		.def("GetFromString", &TagManager::GetFromString)
		.def("GetFromHash", &TagManager::GetFromHash)
		.def("GetPosFromName", &TagManager::GetPosFromName)
		.def("RegisterAsWarp", &TagManager::RegisterAsWarp)

		,class_<DrawManager>("DrawManager")
		.def("DrawLine", &DrawManager::DrawLine)
		.def("DrawPixel", &DrawManager::DrawPixel)
		.def("DrawFilledRect", &DrawManager::DrawFilledRect)
		.def("DrawRect", &DrawManager::DrawRect)


		,class_<Camera>("Camera")
		
		.def("__tostring", &CameraToString)

		//Position And Scale
		.def("SetPos", &Camera::SetPos)
		.def("GetPos", &Camera::GetPos)
		.def("GetPosCentered", &Camera::GetPosCentered)
		.def("SetPosCentered", &Camera::SetPosCentered)
		.def("SetScale", &Camera::SetScale)
		.def("GetScale", &Camera::GetScale)
		
		//Tracking and control
		.def("SetEntityTrackingByID", &Camera::SetEntTracking)
		.def("GetEntityTrackingByID", &Camera::GetEntTracking)
		.def("SetPosTarget", &Camera::SetTargetPos)
		.def("SetPosCenteredTarget", &Camera::SetTargetPosCentered)
		.def("SetScaleTarget", &Camera::SetScaleTarget)
		.def("SetEntityTrackingOffset", &Camera::SetEntityTrackingOffset)
		.def("SetLimitToMapArea", &Camera::SetLimitToMapArea)
		.def("GetLimitToMapArea", &Camera::GetLimitToMapArea)

		//Interpolation Settings
		.def("SetMoveLerp", &Camera::SetMoveLerp)
		.def("SetScaleLerp", &Camera::SetScaleLerp)

		//Miscellaneous
		.def("Reset", &Camera::Reset)
		.def("InstantUpdate", &Camera::InstantUpdate)
		.def("GetCameraSettings", &Camera::GetCameraSettings)
		.def("SetCameraSettings", &Camera::SetCameraSettings)

		,class_<TagObject>("Tag")
		.def("__tostring", &TagToString)
		.def("GetMapName", &TagObject::GetMapName)
		.def("GetID", &TagObject::GetID)
		.def("GetPos", &TagObject::GetPos)

		,class_<Zone>("Zone")
		.def("__tostring", &ZoneToString)
		.def_readwrite("boundingRect", &Zone::m_boundingRect)
		.def_readwrite("entityID", &Zone::m_entityID)
		.def_readwrite("materialID", &Zone::m_materialID)
		.def_readwrite("vPos", &Zone::m_vPos)

		,class_<CameraSetting>("CameraSettings")
		.def("__tostring", &CameraSettingsToString)

		,class_<JoystickInfo>("Joystick")
		.def("__tostring", &JoystickToString)
		.def("GetButtonCount", &JoystickInfo::GetButtonCount)
		.def("GetName", &JoystickInfo::GetName)
		.def("GetID", &JoystickInfo::GetID)
		.def("GetLeftStickPosition", &JoystickInfo::GetLeftStickPosition)
		.def("GetRightStickPosition", &JoystickInfo::GetRightStickPosition)
		.def("SetRightStickAxis", &JoystickInfo::SetRightStickAxis)


		,class_<InputManager>("InputManager")
		.def("__tostring", &InputManagerToString)
		.def("AddBinding", &InputManager::AddBinding)
		.def("RemoveBinding", &InputManager::RemoveBinding)
		.def("RemoveBindingsByEntity", &InputManager::RemoveBindingsByEntity)
		.def("GetMousePos", &InputManager::GetMousePos)
		.def("SetMousePos", &InputManager::SetMousePos)
		.def("GetJoystickCount", &InputManager::GetJoystickCount)
		.def("GetJoystick", &InputManager::GetJoystick)

		,class_<Goal_Think>("GoalManager")

		.def("__tostring", &GoalManagerToString)
		.def("AddNewGoal", &Goal_Think::AddNewGoal)
		.def("AddDelay", &Goal_Think::AddDelay)
		.def("AddMoveToPosition", &Goal_Think::AddMoveToPosition)
		
		.def("AddMoveToMapAndPosition", &Goal_Think::AddMoveToMapAndPosition)

		//.def("AddMoveToTag", &Goal_Think::AddMoveToTag)
		.def("AddApproach", &Goal_Think::AddApproach)
		.def("AddApproachAndSay", &Goal_Think::AddApproachAndSay)
		.def("AddSay", ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::AddSay)
		.def("AddSay", ( void(Goal_Think::*)(const string&, int)) &Goal_Think::AddSay)
		.def("AddSayByID",  ( void(Goal_Think::*)(const string&, int,int,int)) &Goal_Think::AddSayByID)
		.def("AddSayByID",  ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::AddSayByID)
		.def("AddRunScriptString", &Goal_Think::AddRunScriptString)

		.def("PushSay",  ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::PushSay)
		.def("PushSay",  ( void(Goal_Think::*)(const string&, int)) &Goal_Think::PushSay)

		.def("PushApproach", &Goal_Think::PushApproach)
		.def("PushApproachAndSay", &Goal_Think::PushApproachAndSay)
		.def("PushDelay", &Goal_Think::PushDelay)

		.def("PushMoveToPosition", &Goal_Think::PushMoveToPosition)
		.def("PushMoveToMapAndPosition", &Goal_Think::PushMoveToMapAndPosition)
		.def("PushSayByID",  ( void(Goal_Think::*)(const string&, int,int,int)) &Goal_Think::PushSayByID)
		.def("PushSayByID",  ( void(Goal_Think::*)(const string&, int,int)) &Goal_Think::PushSayByID)
		.def("PushRunScriptString", &Goal_Think::PushRunScriptString)
		.def("PushMoveToTag", &Goal_Think::PushMoveToTag)
		.def("PushNewGoal", &Goal_Think::PushNewGoal)

		.def("RemoveAllSubgoals", &Goal_Think::RemoveAllSubgoals)
		.def("IsGoalActiveByName", &Goal_Think::IsGoalActiveByName)
		.def("GetGoalCountByName", &Goal_Think::GetGoalCountByName)
		.def("GetGoalCount", &Goal_Think::GetGoalCount)
		.def("GetGoalByName", &Goal_Think::GetGoalByName)
		.def("GetActiveGoal", &Goal_Think::GetActiveGoal)
		.def("Kill", &Goal_Think::Kill)

		,class_<WatchManager>("WatchManager")
		.def("__tostring", &WatchManagerToString)
		.def("Add", &WatchManager::Add)
		.def("Remove", &WatchManager::Remove)
		.def("GetWatchCount", &WatchManager::GetWatchCount)
		
		];
}
