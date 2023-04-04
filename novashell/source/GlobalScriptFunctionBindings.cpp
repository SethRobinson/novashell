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

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;




CL_Vector2 ScreenToWorld(CL_Vector2 v)
{
	if (g_pMapManager->GetActiveMap())
	{
		return g_pMapManager->GetActiveMapCache()->ScreenToWorld(v);
	}	

	LogMsg("ScreenToWorld error: No world active");
	return v;

}

CL_Vector2 WorldToScreen(CL_Vector2 v)
{
	if (g_pMapManager->GetActiveMap())
	{
		return g_pMapManager->GetActiveMapCache()->WorldToScreen(v);
	} 

	LogMsg("WorldToScreen error: No world active");
	return v;
}

MovingEntity * GetEntityByID(int ID)
{
	return (MovingEntity*)EntityMgr->GetEntityFromID(ID);
}


MovingEntity * GetEntityByName(const string &name)
{
	TagObject *t = g_TagManager.GetFromString(name);
	if (!t) return NULL; //can't find it?
	if (t->m_entID == 0)
	{
		//let's load it right now
		g_pMapManager->LoadMap(t->m_pWorld->GetDirPath(), false);
#ifdef _DEBUG
		t = g_TagManager.GetFromString(name);
#endif
		if (t)
		{
			t->m_pWorld->PreloadMap();
		} else
		{
			assert(!"Why did this tag get screwed up?");
		}

		if (!t || t->m_entID == 0)
		{
			LogMsg("GetEntityByName: Entity %s couldn't be located.  Outdated tag? Removing it.", name.c_str());
			if (t)
			{
				g_TagManager.RemoveByHashID(t->m_hashID, 0);
			}
			return NULL; //not loaded yet
		}
	}

	return (MovingEntity*)EntityMgr->GetEntityFromID(t->m_entID);
}

BaseGameEntity * GetSpecialEntityByName(const string &name)
{
	return EntityMgr->GetEntityByName(name);
}

int GetEntityIDByName(const string &name)
{
	MovingEntity *pEnt = GetEntityByName(name);

	if (pEnt) return pEnt->ID();
	return 0;
}

Tile * GetTileByWorldPos(Map *pWorld, CL_Vector2 v, vector<unsigned int> layerIDVec, bool bPixelAccurate)
{
	if (!pWorld)
	{

		LogMsg("GetTileByWorldPos: Error, invalid map");
		return NULL;
	}

	Tile *pTile = NULL;

	CL_Rect recArea(v.x, v.y, v.x + 0.1f, v.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	pWorld->GetMyMapCache()->AddTilesByRect(recArea, &tileList, layerIDVec);

	//LogMsg("Got %d tiles", tileList.size());
	//now we need to sort them
	g_pLayerManager = &pWorld->GetLayerManager();
	tileList.sort(compareTileBySortLevelOptimized);

	tile_list::reverse_iterator itor = tileList.rbegin();

	for( ;itor != tileList.rend(); itor++)
	{
		pTile = (*itor);
		if (pTile->GetType() == C_TILE_TYPE_ENTITY || pTile->GetType() == C_TILE_TYPE_PIC)
		{
			if (!bPixelAccurate || PixelAccurateHitDetection(v, pTile))
			{
				return pTile;
			} else
			{
				continue;
			}

		}
	}

	//couldn't find anything
	return NULL;

}



MovingEntity * GetEntityByWorldPos(CL_Vector2 v, MovingEntity *pEntToIgnore, bool bPixelAccurate)
{
	if (!g_pMapManager->GetActiveMap())
	{

		LogMsg("GetEntityByWorldPos: Error, no map is active right now.");
		return NULL;
	}

	Tile *pTile = NULL;

	CL_Rect recArea(v.x, v.y, v.x + 0.1f, v.y + 0.1f);

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	g_pMapManager->GetActiveMapCache()->AddTilesByRect(recArea, &tileList, g_pMapManager->GetActiveMap()->GetLayerManager().GetDrawList());

	//now we need to sort them
	g_pLayerManager = &g_pMapManager->GetActiveMap()->GetLayerManager();
	tileList.sort(compareTileBySortLevelOptimized);

	tile_list::reverse_iterator itor = tileList.rbegin();
	MovingEntity *pEnt;

	for( ;itor != tileList.rend(); itor++)
	{
		pTile = (*itor);
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			pEnt = ((TileEntity*)pTile)->GetEntity();

			if (pEnt->GetMap()->GetLayerManager().GetLayerInfo(pTile->GetLayer()).GetShowInEditorOnly())
			{
				continue;
			}
			if (pEnt == pEntToIgnore)
			{
				continue;
			}
			//looks good to me!

			if (!bPixelAccurate || PixelAccurateHitDetection(v, pTile))
			{
				return pEnt;
			} else
			{
				continue;
			}
		}
	}

	//couldn't find anything
	return NULL;
}

void LogErrorLUA(const char *pMessage)
{
	string s = pMessage;
	if (s.length() > C_LOGGING_BUFFER_SIZE)
	{
		s.resize(C_LOGGING_BUFFER_SIZE);
	}
	LogError(s.c_str());
}

bool RunScript(const string &scriptName)
{

	string fileName = C_DEFAULT_SCRIPT_PATH + scriptName;

	if (!g_VFManager.LocateFile(fileName))
	{
		fileName = ChangeFileExtension(fileName, ".luac");
		if (!g_VFManager.LocateFile(fileName))
		{
			LogError("Unable to locate script file %s.", fileName.c_str());
			return false;
		}
	}

	GetScriptManager->LoadMainScript(fileName.c_str());
	return true;
}


bool VariableExists(const string &varName)
{
	return GetScriptManager->VariableExists(varName.c_str());
}

bool FunctionExists(const string &varName)
{
	return GetScriptManager->VariableExists(varName.c_str());
}


float LerpNumber(float src, float target, float amount)
{
	float l = Lerp(src, target, amount);
	set_float_with_target(&l, target, amount); //cheat to avoid round-off errors
	return l;
}


/*
Title: Global Functions
These functions are global and can be run from anywhere, they aren't attached to any specific object.
*/

bool LUAExists(const string & name)
{
	GetScriptManager->SetStrict(false);
	bool bTemp = luabind::type(luabind::globals(GetScriptManager->GetMainState())[name]) != LUA_TNIL;
	GetScriptManager->SetStrict(true);
	return bTemp; //doesn't exist
}


//Section: Miscellaneous

void luabindGlobalFunctions(lua_State *pState)
{
	module(pState)
		[

			//stand alone functions

			/*
			func: LogMsg
			(code)
			nil LogMsg (string message)
			(end)

			Adds a message to the log.  You can see it by bringing up the console, or checking the log.txt file.
			Lua's native "print" function is routed to use LogMsg also.
			
			Parameters:
			message - Any text you want to display or log.
			*/

			def("Exists", &LUAExists),

		
			/*
			func: LogError
			(code)
			nil LogError (string message)
			(end)

			Like <LogMsg> but forces the console to pop-up and prints in red.  Should be used for serious errors that you want to know about.

			Logged to log.txt.

			Parameters:
			message - Any text.  Note that it prepends the "Error: " to your message automatically.
			*/

		
			/*
			func: Exists
			(code)
			nil Exists (string funcOrVarName)
			(end)

			Allows you to check the global namespace to see if a variable or function name exists.  To improve readability, you should probably use <FunctionExists> or <VariableExists> instead.

			If you'd like to check a specific entity namespace, use <Entity:FunctionExists> and <Entity:VariableExists> instead.
			
			Parameters:
			funcOrVarName - The exact name of the function or variable.
			*/

			def("FunctionExists", &FunctionExists),

			/*
			func: FunctionExists
			(code)
			nil FunctionExists (string funcName)
			(end)

			Allows you to check the global namespace to see if a certain function exists.

			If you'd like to check a specific entity namespace, use <Entity:FunctionExists> instead.

			Parameters:
			funcName - The exact name of the function.
			*/
			def("VariableExists", &FunctionExists),

			/*
			func: VariableExists
			(code)
			nil VariableExists (string varName)
			(end)

			Allows you to check the global namespace to see if a certain function exists.

			If you'd like to check a specific entity namespace, use <Entity:VariableExists> instead.

			Parameters:
			varName - The exact name of the variable.
			*/



			/*
			func: CreateEntity
			(code)
			Entity CreateEntity(Map map, Vector2 vPos, string scriptFileName)
			(end)
			
			Creates an entity on the specified map at vPos and inits it using the script file name specified.
		
			There is an <Entity::CreateEntity> version that is identical except it allows relative paths (relative to the entity's script) to be used.
			
			Parameters:
			map - The map to initialize the entity on.  If nil is passed in, the currently active map will be used.
			vPos - The position to create the entity at.
			scriptFileName - The script it should be initialized with.

			Returns:
			A handle to the <Entity> created.
			*/

			def("RunScript", &RunScript),

			/*
			func: RunScript
			(code)
			boolean RunScript (string name)
			(end)

			Loads a lua script.  If loaded by an entity, keep in mind the script will only exist in the entity's name space, not globally.

			Because lua's other native script loading functions are disabled for security reasons this is the only way to load a script within a script.

			Like images, sounds, and other data, scripts are searched for in mounted worlds in reverse order, so "mods" can override scripts if needed.

			Usage:
			(code)
			RunScript("system/sound.lua"); //load the sound module so everybody can use its functions
			(end)

			Parameters:
			name - Path and file name.  Paths trying to reference outside of the world directory tree will be invalid.

			Returns:
			True if the script was found and loaded.
			*/

			def("CreateEntity", &CreateEntity),

			/*
			func: Schedule
			(code)
			Schedule (number deliveryMS, number targetID, string message)
			(end)

			Schedules a line of lua script to be run after a delay.

			If an entity ID is specified, the script is run from its namespace.

			Internally, <App::GetGameTick> is used, so messages are paused/modified by the current game speed.

			You can use backslashes to escape quotes or apostrophes if needed.

			If an entity ID is specified and it doesn't exist at the time of delivery, the message is discarded.

			Usage:

			(code)
			//delete the entity in 100 milliseconds
			Schedule(100, this:GetID(), "this:SetDeleteFlag(true);"); 
			
			//add the fade out and delete brain in 800 ms to "ent"
			Schedule(800, ent:GetID(), "this:GetBrainManager():Add(\"FadeOutAndDelete\",'');"); 

			(end)

			Parameters:
			deliveryMS - How long to wait before delivering in milliseconds.  500 would mean half a second later.
			targetID - Entity ID of who should run the script, or C_ENTITY_NONE for the global namespace.
			message - The line of lua code that should be executed.
			*/
	
			def("CreateEntitySpecial", &CreateEntitySpecial),

			/*
			func: CreateEntitySpecial
			(code)
			BaseEntity CreateEntitySpecial(string name, string parms)
			(end)

			Special kinds of entities like dialogs must be created with this function.
			
			For simplicity, their real class types are not made available to lua, only the <BaseEntity> interface is accessible.
			
			So how can you control them?  Primarily through <BaseEntity::Send> with text.

			List of special entity types:

			* ChoiceDialog
			* WorldChooseDialog

			Uh.. Scan the example script files for these words for examples.
			
			Parameters:
			name - The kind of special entity you want to create.  
			parms - Same as using <BaseEntity::Send> but gets sent during initialization.

			Returns:
			A handle to the <BaseEntity> created.
			*/

			def("Schedule", &Schedule),
			
			/*
			func: ScheduleSystem
			(code)
			nil Schedule (number deliveryMS, number targetID, string message)
			(end)

			Identical to <Schedule> except <GetTick> is used for timing.  This means messages will be delivered after the delay takes place, regardless of game speed or if the game is paused for dialog.
			*/
			
			def("ScheduleSystem", &ScheduleSystem),
			
			

			def("GetEntityByWorldPos", &GetEntityByWorldPos),
			
			
			/*
			func: GetEntityByWorldPos
			(code)
			Entity GetEntityByWorldPos (Vector2 vPos, Entity entityToIgnore, boolean bPixelAccurate)
			(end)

			Returns the <Entity> handle at the specified position.  If there is more than one, it returns the one "on top".

			This function uses all visible layers and the active map for its check.
			
			Usage:

			(code)
			local pt = ScreenToWorld(GetInputManager:GetMousePos());
			local ent = GetEntityByWorldPos(pt, this, true); //get all entities but ignore ourself, be pixel accurate

			if (ent != nil) then
				LogMsg("The mouse is over entity # " .. ent:GetID());
			else
				LogMsg("Didn't find anybody.");
			end
			(end)

			Parameters:
			vPos - The point in the world we should check,
			entityToIgnore - If this isn't nil, we'll ignore this entity during our checks.
			bPixelAccurate - If true, we'll ignore entities that are transparent at the pixel location clicked. (slower)

			Returns:
			The <Entity> found, or nil if nothing was found.
			*/

			
			def("GetEntityByID", &GetEntityByID),
			
			/*
			func: GetEntityByID
			(code)
			Entity GetEntityByID (number entityID)
			(end)

			Converts an entityID to an <Entity> handle.  Very fast.

			Usage:

			(code)
			LogMsg("This entity's ID is " .. GetEntityById(ent:GetID()):GetID()); // a ridiculous example
			(end)

			Parameters:
			entityID - Every entity has a unique number it can be referenced by. (unique to that gaming session, not saved/loaded)
			
			Returns:
			The <Entity> found, or nil if the ID is invalid.
			*/
			def("GetEntityByName", &GetEntityByName),
			
			/*
			func: GetEntityByName
			(code)
			Entity GetEntityByName (string name)
			(end)

			Converts a name to an Entity. Very fast. The entity's map will be loaded if needed.  

			Usage:

			(code)
			local entGun = GetEntityByName("Gun"); //if an entity is named Gun anywhere in the world, we'll get it.
			(end)

			Parameters:
			name - The name set in the editor or with <Entity::SetName>.  Names are always unique.

			Returns:
			The <Entity> found, or nil if the entity can't be located.
			*/
			
		
			
			def("GetEntityIDByName", &GetEntityIDByName),
			
			/*
			func: GetEntityIDByName
			(code)
			EntityID GetEntityIDByName (string name)
			(end)

			Like <GetEntityByName> except it returns the entityID instead of the entity.

			Usage:

			(code)
			local entID = GetEntityIDByName("player");
			if (entID != C_ENTITY_NONE) then
				LogMsg("Found entityID " .. entID .. ".  His name is " .. GetEntityByID(entID):GetName());
			end
			(end)

			Parameters:
			name - The entity's name set in the editor or with <Entity::SetName>.  Names are always unique.

			Returns:
			The entity's ID if found, or <C_ENTITY_NONE> if the entity can't be located.
			*/

			def("GetSpecialEntityByName", &GetSpecialEntityByName),

			/*
			func: GetSpecialEntityByName
			(code)
			BaseEntity GetSpecialEntityByName (string name)
			(end)

			Certain non-entity objects can exist and be manipulated at times using the <BaseEntity> handle.

			Usage:

			(code)
			local ent = GetSpecialEntityByName("editor");
			if (ent != nil) then ent:SetDeleteFlag(true); end; //kill the editor window if it exists

			if (GetSpecialEntityByName("ChoiceDialog") != nil) then LogMsg("A dialog window is active."); end;
			(end)

			Parameters:
			name - Special entities usually have pre-defined names.

			Returns:
			The <BaseEntity> found, or nil if nothing is found.
			*/

			
			def("ShowMessage", &ShowMessage),
			
			/*
			func: ShowMessage
			(code)
			nil ShowMessage (string title, string msg, boolean bForceClassicStyle)
			(end)

			Quick way to pop-up a message box to show the user something.  The user must hit OK to continue.

			Usage:

			(code)
			ShowMessage("Error", "You shall not pass!", true);
			(end)

			Parameters:
			title - The text at the top of the message box
			msg - The main text.  Use \n to add a line feed
			bForceClassicStyle - Normally the style will uses the active game dialog style, but by passing true here you can force the boring silver style.
			
			*/

			/*
			func: Lerp
			(code)
			number Lerp (number originalNum, number targetNum, number lerpAmount)
			(end)

			Lerp (a quasi-acronym for linear interpolation) is a method to make one number change into another number over time.

			If you want an effect where a player's score "counts up" as he gets points, but don't want it to take two hours when he scores a million, lerping is for you!

			Usage:

			(code)
			displayScore = Lerp(displayScore, actualScore, 0.1); //lerp by ten percent, call every frame
			LogMsg("You have " .. displayScore .. " Points!"); //watch as the number counts up
			(end)

			Parameters:
			originalNum - The number to start with
			targetNum - the number we want to eventually end up with
			lerpAmount - the percent of the difference to modify the number by
			
			Returns:

			The modified number.
			*/

			def("Lerp", &LerpNumber),

	
			//Section: Conversion
			
			def("ScreenToWorld", &ScreenToWorld),

			/*
			func: ScreenToWorld
			(code)
			Vector2 ScreenToWorld (Vector2 vScreenPos)
			(end)

			Converts a screen position (0,0 being the upper left of your monitor) to a world position based on the active map and camera position.

			Usage:

			(code)
			//figure out where the mouse is pointing, in world coordinates
			local vPos = ScreenToWorld(GetInputManager:GetMousePos());
			LogMsg("The mouse is currently hovering over " .. tostring(vPos) .. " in world coordinates.");
			(end)

			Parameters:
			vScreenPos - The screen position you need converted to world coordinates.

			Returns:
			The position converted to world coordinates.

			*/




			def("WorldToScreen", &WorldToScreen),

			/*
			func: WorldToScreen
			(code)
			Vector2 WorldToScreen (Vector2 vWorldPos)
			(end)

			The opposite of <ScreenToWorld>.

			Parameters:
			vWorldPos - The world position you want converted to screen coordinates.  May be negative or larger than your monitor size.

			Returns:
			The position converted to screen coordinates.
			*/


			def("FacingToVector", &FacingToVector),
			/*
			func: FacingToVector
			(code)
			Vector2 FacingToVector (number facing)
			(end)

			Converts a directional constant ( <C_FACING_CONSTANTS> ) into a unit vector.

			Usage:

			(code)
			LogMsg("The down direction is unit vector " .. FacingToVector(C_FACING_DOWN));
			(end)

			Parameters:
			facing - One of the <C_FACING_CONSTANTS> constants.
			
			Returns:
			A <Vector2> object containing a unit vector of the direction.
			
			*/
			
			def("VectorToFacing", &VectorToFacing),
			
			/*
			func: VectorToFacing
			(code)
			facing VectorToFacing (Vector2 vDirection)
			(end)

			Converts a unit vector into a <C_FACING_CONSTANTS> facing.

			Parameters:
			vDirection - A <Vector2> object containing a unit vector.

			Returns:
			The <C_FACING_CONSTANTS> that bests fits the direction.
			*/

			
		
		
			def("ColorToString", &ColorToString),
		
			/*
			func: ColorToString
			(code)
			string ColorToString (Color color)
			(end)

			Converts a <Color> to a string format.  Useful for saving it through <Entity:Data>.

			Usage:
			(code)
			local colorString = ColorToString(Color(255,255,255,255));
			LogMsg("Neat.  In string format, that color is " .. colorString);
			local myColor = StringToColor(colorString);
			LogMsg("We converted it back!");
			(end)

			Parameters:
			color - A <Color> object.

			Returns:
			A string formatted with the color data.
			*/

			def("StringToColor", &StringToColor),
			
			/*
			func: StringToColor
			(code)
			Color StringToColor (string colorText)
			(end)

			Converts a color formatted like "255 255 255 255" to a <Color> object.

			Parameters:
			colorText - Colors in a string.

			Returns:
			A <Color> object.
			*/

			def("VectorToString", &VectorToString),
			
			/*
			func: VectorToString
			(code)
			string VectorToString (Vector2 v)
			(end)

			Converts a <Vector2> to a string format.  Useful for saving it through <Entity:Data>.

			Parameters:
			v - A <Vector2> that you want converted.

			Returns:
			A string containing something like "343.2343 54.23"
			*/

			def("StringToVector", &StringToVector),

			/*
			func: StringToVector
			(code)
			Vector2 StringToVector (string textVector)
			(end)

			Converts a string like "23.4 5" to a <Vector2>.

			Parameters:
			textVector - A string with two numbers in it.

			Returns:
			A <Vector2> object.
			*/


		
			def("RectToString", &RectToString),
			
			/*
			func: RectToString
			(code)
			string RectToString (Rect r)
			(end)

			Converts a <Rect> to a string format.  Useful for saving it through <Entity:Data>.

			Parameters:
			r - A <Rect> that you want converted.

			Returns:
			A string containing something like "0 0 100 100"
			*/

			
			def("StringToRect", &StringToRect)
		
			/*
			func: StringToRect
			(code)
			Vector2 StringToRect (string textRect)
			(end)

			Converts a string like "0 0 10 10" to a <Rect>.

			Parameters:
			textRect - A string with four numbers in it.

			Returns:
			A <Rect> object.
			*/


			//Section: Related Constants

			/*

			group: C_FACING_CONSTANTS

			constant: C_FACING_NONE
			Means no facing at all.

			constant: C_FACING_LEFT

			constant: C_FACING_UP_LEFT

			constant: C_FACING_UP

			constant: C_FACING_UP_RIGHT

			constant: C_FACING_RIGHT

			constant: C_FACING_DOWN_RIGHT

			constant: C_FACING_DOWN

			constant: C_FACING_DOWN_LEFT

			constant: C_FACING_COUNT

			How many directions we support, this will be 8.
			
			
			group: C_ORIGIN_CONSTANTS

			constant: C_ORIGIN_TOP_LEFT

			constant: C_ORIGIN_CENTER_LEFT

			constant: C_ORIGIN_CENTER

			constant: C_ORIGIN_CENTER_RIGHT
			
			constant: C_ORIGIN_BOTTOM_CENTER
			
			*/
	];
}