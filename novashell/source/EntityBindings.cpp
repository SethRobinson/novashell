#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "Brain.h"
#include "State.h"
#include "AI/Goal_Think.h"
#include "ListBindings.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#include <luabind/object.hpp>

using namespace luabind;

string EntityToString(BaseGameEntity * pEnt)
{
	return "Entity " + CL_String::from_int(pEnt->ID()) + " (" + pEnt->GetName()+")";
}



string DataManagerToString(DataManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A DataManager with %d entries.", pObj->GetCount());
	return string(stTemp);
}

string BrainManagerToString(BrainManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A BrainManager.");
	return string(stTemp);
}

string BrainToString(Brain * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A %s Brain.", pObj->GetName());
	return string(stTemp);
}

string StateToString(Brain * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A State called %s.", pObj->GetName());
	return string(stTemp);
}
string SpecialEntityToString(BaseGameEntity * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A special entity called %s.", pObj->GetName().c_str());
	return string(stTemp);
}

void luabindEntity(lua_State *pState)
{
	module(pState)
		[

			class_<DataManager>("DataManager")
			.def("Set", &DataManager::Set)
			.def("SetNum", &DataManager::SetNum)
			.def("SetIfNull", &DataManager::SetIfNull)

			.def("Get", &DataManager::Get)
			.def("GetNum", &DataManager::GetNum)

			.def("Exists", &DataManager::Exists)
			.def("ModNum", &DataManager::ModNum)
			.def("GetNumWithDefault", &DataManager::GetNumWithDefault)
			.def("GetWithDefault", &DataManager::GetWithDefault)
			.def("SetNumIfNull", &DataManager::SetNumIfNull)
			.def("Delete", &DataManager::Delete)
			.def("Clear", &DataManager::Clear)
			.def("__tostring", &DataManagerToString)

			,class_<State>("State")
			.def("GetName", &State::GetName)
			.def("__tostring", &StateToString)

			,class_<BrainManager>("BrainManager")
			.def("Add", &BrainManager::Add)
			.def("Remove", &BrainManager::Remove)
			.def("SendToBrainByName", &BrainManager::SendToBrainByName)
			.def("AskBrainByName", &BrainManager::AskBrainByName)
			.def("SendToBrainBase", &BrainManager::SendToBrainBase)
			.def("GetBrainByName", &BrainManager::GetBrainByName)

			.def("SetStateByName", &BrainManager::SetStateByNameNoReturn)
			.def("GetActiveStateName", &BrainManager::GetStateByName)
			.def("LastStateWas", &BrainManager::LastStateWas)
			.def("InState", &BrainManager::InState)
			.def("__tostring", &BrainManagerToString)


			,class_<Brain>("Brain")
			.def("GetName", &Brain::GetName)
			.def("__tostring", &BrainToString)

			,class_<BaseGameEntity>("SpecialEntity")
			.def("GetID", &BaseGameEntity::ID)
			.def("GetName", &BaseGameEntity::GetName)
			.def("SetName", &BaseGameEntity::SetName)
			.def("GetType", &BaseGameEntity::GetType)
			.def("SetType", &BaseGameEntity::SetType)

			.def("GetPriorityLevel", &BaseGameEntity::GetPriorityLevel)
			.def("SetPriorityLevel", &BaseGameEntity::SetPriorityLevel)
			.def("SetDeleteFlag", &BaseGameEntity::SetDeleteFlag)
			.def("Send", &BaseGameEntity::HandleMessageString)
			.def("__tostring", &SpecialEntityToString)


/*

Object: Entity
The Entity object.

About:

The heart of the Novashell system is the flexible <Entity> object.

An <Entity> can represent a patch of ground, an chair, a persistent database, a warp, a health bar, a text overlay or a drooling monster.

Advanced abilities such as:

 * An attached script
 * The BrainManager
 * The GoalManager
 * Collision information
 * Path-finding system
 * Triggers
 
are initialized on demand, only wasting memory if they are actually used.

Scripting:

If a script is assigned to an entity, its <Entity> object can always be reached by accessing "this" from its namespace.

Add the following functions in your script and they will automatically be called at the correct time:

(code)
function OnInit() //run upon initialization, before being inserted on the map.
	//note: this is also run on entities that placed in the copy buffer or undo buffer.
	//this is where you should specify its visuals, so the engine knows how big it is.
	LogMsg("Hey, I'm entity # " .. this:GetID() .. " and I've been initialized.");
end

function OnMapInsert() //run right after they are inserted on the map.
	LogMsg("I was just inserted in the map named " .. this:GetMap():GetName());
end

function OnPostInit() //run once when their first logic cycle happens (for instance, when they come on screen)
end

function OnSave() //run right before the entity is saved or cloned
end

function OnKill() //run right before being destroyed/removed from the map 
end

(end)
You can also request to receive other callbacks for collision, triggers, and updates every logic cycle.


Relative paths:

After a script is loaded, the ~/ character sequence can be used to mean a relative path from the script that is loaded.
For example:

(code)
//this assumes duck_headless.xml and head.col are in the same directory as the script
//that is now running and attached to this entity
this:SetVisualProfile("~/duck_headless.xml", "duck_head");
this:LoadCollisionInfo("~/head.col");
(end)

Script Namespace:

An entity contains it's own lua namespace, with the global namespace used as a fallback if nothing local is found.

(code)
m_myOwnVar = 4; //this var will be unique for each entity using this script

//If a global function or variable exists, you can access and modify it the normal way:
g_dialogCanBeCanceled = true; //write to a global var from an entity's script

//However, to CREATE a global variable that didn't exist from an entity you must do this:
_G.g_myNewGlobal = 3;

The _G forces the global namespace to be used.
(end)

To get local variable data from an entity from another entity or the global level, use <Entity::RunFunction> which gives you access to its local functions and namespace.

Group: General
*/

			,class_<MovingEntity, BaseGameEntity>("Entity")
			.def(constructor<>())
			
			/*
			func: GetID
			(code)
			number GetID()
			(end)

			Returns:

			This entities unique ID.  These are NOT persistent across load/save sessions, only for the life of that entity instance.
			*/

			/*
			func: SetName
			(code)
			nil SetName(string name)
			(end)

			By setting a name, you are requesting that the <TagManager> track this entity.  Only do this with entities you need locatable by name.
			
			This means at any time you can instantly locate this entity's map and position by name only, even without loading its map.
			
			Important Rule:
			
			No two entities can have the same name.  The engine will enforce this rule by adding random letters to a name until it is unique.

			Parameters:

			name - The new name you want associated with this entity.
			*/

			/*
			func: GetName
			(code)
			string GetName()
			(end)

			Returns:

			This entity's tag cache tracking name, or a blank string if none has been set.
			*/




			/*
			func: SetType
			(code)
			nil SetType(number custom)
			(end)
			Allows you to set a number to help you organize entity types.  Check the C_TYPE_* constants as defined in setup_constants.lua.  Purely for scripting ease, not used by the engine internally.

			Defaults to 0.

			Parameters:

			custom - Should be sent one of the C_TYPE_* constants as defined in setup_constants.lua.

			*/

			/*
			func: GetType
			(code)
			number GetType()
			(end)

			Returns:

			One of the C_TYPE_* constants as defined in setup_constants.lua.
			*/
		

			/*
			func: SetPriorityLevel
			(code)
			nil SetPriorityLevel(number level)
			(end)
			Allows you to set a priority level.  Check the C_PRIORITY_LEVEL_* constants as defined in setup_constants.lua.

			If an entity's priority level is lower than the current global priority level, it will not update.

			This isn't currently fully implemented, so for now, just know that if you set this to 100, an entity will keep updating even while 'paused', good for GUI things for instance.

			Parameters:

			level - Should be sent one of the C_PRIORITY_LEVEL_* constants as defined in setup_constants.lua.

			*/

			/*
			func: GetPriorityLevel
			(code)
			number GetPriorityLevel()
			(end)

			Returns:

			One of the C_PriorityLevel_* constants as defined in setup_constants.lua.
			*/



			/*
			func: GetPos
			(code)
			Vector2 GetPos()
			(end)

			Returns:

			A <Vector2> object containing the entity's position.
			*/
			
			.def("GetPos", &MovingEntity::GetPosSafe)

			/*
			func: SetPos
			(code)
			nil SetPos(Vector2 vPos)
			(end)

			Parameters:

			vPos - a <Vector2> object containing the position to move to.
			*/

			.def("SetPos", &MovingEntity::SetPos)
		
			
			
			.def("SetPosAndMap", &MovingEntity::SetPosAndMap)
		
			/*
			func: SetPosAndMap
			(code)
			nil SetPosAndMap(Vector2 vPos, string mapName)
			(end)

			Parameters:

			vPos - a <Vector2> object containing the position to move to. Our own position information is ignored.
			mapName - The name of the <Map> the entity should move to.  Note, the move actually happens at the end of the logic cycle, not instantly.  The map will be loaded if needed.
			
			*/
			.def("SetPosAndMapByTagName", &MovingEntity::SetPosAndMapByTagName)

			/*
			func: SetPosAndMapByTagName
			(code)
			boolean SetPosAndMapByTagName(string tagName)
			(end)

			This can be an easy way to move a player to a specific place.
			
			Parameters:

			tagName - Any entity that has been named can be used as a tagName

			Returns:
			True on success, false if the tag name wasn't found
			*/

			
			.def("GetMap", &MovingEntity::GetMap)
			/*
			func: GetMap
			(code)
			Map GetMap()
			(end)

			Usage:
			(code)
			LogMsg("Hi, I'm in the " .. this:GetMap():GetName() .. " map.);
			(end)
			Returns:

			The <Map> object that the entity is currently in.
			*/

			.def("SetAttach", &MovingEntity::SetAttach)
			/*
			func: SetAttach
			(code)
			nil SetAttach(number entityID, Vector2 vOffset)
			(end)

			Attaching means we will automatically move with the entity we are attached to.

			There are two special things you can use instead of an entity ID:

			* <C_ENTITY_NONE> - This unattaches us if we were previously attached to something.
			* <C_ENTITY_CAMERA> - This attaches us to the camera.  Use this on GUI items, and they will stay "on-screen" without moving.

			Parameters:

			entityID - The entity's ID that we're supposed to attached to.
			vOffset - a <Vector2> containing the offset to attach at.  Use Vector2(0,0) for no offset.

			*/
			
			.def("GetAttach", &MovingEntity::GetAttachEntityID)
			/*
			func: GetAttach
			(code)
			number GetAttach()
			(end)

			Returns:
			The entity ID of who we are attached to.  <C_ENTITY_NONE> if none.
			*/

			
			.def("SetAttachOffset", &MovingEntity::SetAttachOffset)
			/*
			func: SetAttachOffset
			(code)
			nil SetAttachOffset(Vector2 vOffset)
			(end)

			Change the offset position in reference to the entity you're attached to.
			Only useful if you've already used <SetAttach> to attach an entity.

			Parameters:

			vOffset - A <Vector2> object containing the new offset that should be used.
			*/

			.def("GetAttachOffset", &MovingEntity::GetAttachOffset)

			/*
			func: GetAttachOffset
			(code)
			Vector2 SetAttachOffset()
			(end)

			Returns:

			A <Vector2> object containing the current attach offset.
			*/

			.def("SetLayerID", &MovingEntity::SetLayerID)

			/*
			func: SetLayerID
			(code)
			nil SetLayerID(number layerID)
			(end)

			Allows you to change the layer of an entity.

			Note that the actual movement will not happen until the end of that game logic cycle.

			Usage:
			(code)
			this:SetLayerID(this:GetLayerID()+1); //move up one layer
			//Note, this may or may not affect our sorting level as expected, depends on the layer's settings.
			(end)
			*/



			.def("GetLayerID", &MovingEntity::GetLayerID)
		
			/*
			func: GetLayerID
			(code)
			number GetLayerID()
			(end)

			Usage:
			(code)
			local layerID = this:GetLayerID();
			LogMsg("We're on layer " .. layerID);			
			(end)

			Returns:

			The layer ID that we're on.
			*/
			.def("SetLayerByName", &MovingEntity::SetLayerByName)

			/*
			func: SetLayerIDByName
			(code)
			nil SetLayerIDByName(string layerName)
			(end)

			Allows you to change the layer of an entity by using it's name.

			Usage:
			(code)
			this:SetLayerIDByName("Overlay 1"); //move to the Overlay 1 layer.
			(end)
			*/

			.def("SetScale", &MovingEntity::SetScale)
		
			/*
			func: SetScale
			(code)
			nil SetScale(Vector2 vScale)
			(end)

			Allows you to scale up or down an <Entity> in size.  The collision information is automatically scaled with it.  If you'd like to scale the collision information only, use <SetCollisionScale>.

			Parameters:
			
			vScale - A <Vector2> object containing the new scale. A scale of Vector2(2,2) would mean twice the default size, Vector2(0.5,0.5) would mean half size.
			*/

			.def("GetScale", &MovingEntity::GetScale)

			/*
			func: GetScale
			(code)
			Vector2 GetScale()
			(end)

			Returns:

			The current entity scale.
			*/

		
			.def("SetLockedScale", &MovingEntity::SetLockedScale)

			/*
			func: SetLockedScale
			(code)
			nil SetLockedScale(boolean bLocked)
			(end)

			If true, this entity will not scale with the camera.  This is needed for GUI overlays that shouldn't scale when the rest of the game does.  Defaults to off.

			Note:

			This is automatically set to true when using <Entity:SetAttach> to attach something to the camera, as it's probably a GUI item.
			
			Parameters:

			bLocked - Send true if you don't want this scaling with the camera, false for normal behavior.
			*/

		
			.def("GetLockedScale", &MovingEntity::GetLockedScale)

			/*
			func: GetLockedScale
			(code)
			boolean GetLockedScale()
			(end)

			Returns:

			The true if the scale is locked on this entity.  (Meaning it won't scale when the camera zooms in or out)
			*/



			.def("GetWorldRect", &MovingEntity::GetWorldRect)
			
			/*
			func: GetWorldRect
			(code)
			Rectf GetWorldRect()
			(end)

			Usage:
			(code)
			LogMsg("My image position in world coordinates is " .. tostring(this:GetWorldRect());
			(end)
			Returns:

			A <Rectf> object containing the region the entity's image occupies in world coordinates.
			*/



			.def("GetRect", &MovingEntity::GetBoundsRect)

			/*
			func: GetRect
			(code)
			Rectf GetRect()
			(end)

			Returns:

			A <Rectf> object containing the entity's image size in local coordinates.
			*/

			/*
			func: SetDeleteFlag
			(code)
			void SetDeleteFlag(boolean bRequestDelete)
			(end)

			This is the how you delete an entity.  Instead of happening instantly, it actually happens at the end of that logic cycle.

			As always, the entities *OnKill()* script function will be run right before it's destroyed.	(well, if the entity has a script running)

			Parameters:

			bRequestDelete - If true, this entity will be deleted ASAP.
			*/


			.def("GetWorldCollisionRect", &MovingEntity::GetWorldCollisionRect)

			/*
			func: GetWorldCollisionRect
			(code)
			Rectf GetWorldCollisionRect()
			(end)

			Usage:
			(code)
			LogMsg("My collision box in world coordinates is " .. tostring(this:GetWorldCollisionRect());
			(end)
			Returns:

			A <Rectf> object containing the region the entity's collision occupies in world coordinates.
			*/



			.def("GetCollisionRect", &MovingEntity::GetCollisionRect)

			/*
			func: GetCollisionRect
			(code)
			Rectf GetCollisionRect()
			(end)

			Returns:

			A <Rectf> object containing the entity's collision box in local coordinates.
			*/

			/*
			func: SetDeleteFlag
			(code)
			void SetDeleteFlag(boolean bRequestDelete)
			(end)

			This is the how you delete an entity.  Instead of happening instantly, it actually happens at the end of that logic cycle.

			As always, the entities *OnKill()* script function will be run right before it's destroyed.	(well, if the entity has a script running)

			Parameters:

			bRequestDelete - If true, this entity will be deleted ASAP.
			*/


//Group: Image related

.def("SetVisualProfile", &MovingEntity::SetVisualProfile)
/*
func: SetVisualProfile
(code)
boolean SetVisualProfile(string fileName, string profileName)
(end)

A visual profile is an .xml file that contains data about how the entity should look.  An .xml can contain more than one "profile", but only one can be assigned to an entity at a time.

To create a profile, it's recommended you start with an existing one from one of the examples and use it as a template.

A profile can contain data for many directions and many states, such as walking, attacking, idling and dieing.  As well as special animations that are used in a custom way by the script.

Brains automatically communicate with profiles to see what can be displayed and intelligently choose what to display if an animation is missing.
Parameters:

Technical Note:

VisualProfiles and images contained are cached out and shared between Entities.

Parameters:

fileName - Filename of the .xml to load.
profileName - The profile name within the .xml to use.

Returns:

True on success, false if the xml file or profile name wasn't found.
*/


.def("SetBaseColor", &MovingEntity::SetBaseColor)

/*
func: SetBaseColor
(code)
nil SetBaseColor(Color color);
(end)

Using this you can change the color of an entity.  Technically, you're just setting how much of red, green, blue can get through.

So if you start with white, you can make any color by removing the colors you don't want.

A <Color> object also contains an alpha value, this lets you set the translucency.  (This happens on top of whatever per-pixel alpha the image already has)

Usage:
(code)
this:SetBaseColor(Color(255,20,20,255)); //we should now look very red, since we're not
//letting much green or blue through.

//another example

Color c = Color(255,255,255,110); //full color, but only about half the alpha
this:SetBaseColor(c); //we're now about 50% invisible
(end)

Parameters:

color - A <Color> object.
*/


.def("GetBaseColor", &MovingEntity::GetBaseColor)

/*
func: GetBaseColor
(code)
Color GetBaseColor();
(end)

Returns:

A <Color> object containing the entity's base color.  Temporary color changes such as those created by a ColorFlash brain will not affect this value.
*/

.def("GetSizeX", &MovingEntity::GetSizeX)

/*
func: GetSizeX
(code)
number GetSizeX()
(end)

Returns:

The width of the image currently representing the entity.
*/


.def("GetSizeY", &MovingEntity::GetSizeY)
/*
func: GetSizeY
(code)
number GetSizeY()
(end)

Returns:

The height of the image currently representing the entity.
*/

.def("SetAnimByName", &MovingEntity::SetAnimByName)

/*
func: SetAnimByName
(code)
nil SetAnimByName(string animName)
(end)

This is a simple way to cause an animation to be played on an Entity.  Check the barrel.lua script in the Dink Example to see it in action.

Note:

This method won't work for objects that use the BrainManager extensively, as they will set change the state and animation themselves.  For those situations, use the ForceVisual brain instead.

Parameters:

animName - This the name of an animation that has been defined in its Visual Profile.
*/


.def("GetAnimName", &MovingEntity::GetAnimName)

/*
func: GetAnimName
(code)
string GetAnimName()
(end)

Returns:

The name of the current animation.  Returns a blank string if no visual profile is active for the entity.
*/



.def("SetAnimFrame", &MovingEntity::SetAnimFrame)


/*
func: SetAnimFrame
(code)
nil SetAnimFrame(number frame)
(end)

A method to set the current frame of the currently playing animation.

Parameters:

frame - This is a frame number or <C_ANIM_FRAME_LAST> to specify the last frame of the animation.

Parameters:

animName - This the name of an animation that has been defined in its Visual Profile.
*/

.def("GetAnimFrame", &MovingEntity::GetAnimFrame)

/*
func: GetAnimFrame
(code)
frame GetAnimFrame(number frame)
(end)

Returns:

The frame of the animation currently being display.
*/



.def("SetAnimLoopCallback", &MovingEntity::SetAnimLoopCallback)

/*
func: SetAnimLoopCallback
(code)
nil SetAnimLoopCallback(boolean bActive)
(end)

If true, every time the animation playing loops, or reaches the end, the engine will attempt to call a script function named "OnAnimLoop" in this entities namespace.


Parameters:

bActive - If true, enabled.  Send false to turn it off which is the default.
*/

.def("GetAnimLoopCallback", &MovingEntity::GetAnimLoopCallback)

/*
func: GetAnimLoopCallback
(code)
boolean GetAnimLoopCallback()
(end)

Returns:

True if "OnAnimLoop" is set to be called when the current animation loops or reaches the end.
*/



.def("GetLockedScale", &MovingEntity::GetLockedScale)

/*
func: GetLockedScale
(code)
boolean GetLockedScale()
(end)

Returns:

The true if the scale is locked on this entity.  (Meaning it won't scale when the camera zooms in or out)
*/



.def("SetBlendMode", &MovingEntity::SetBlendMode)

/*
func: SetBlendMode
(code)
nil SetBlendMode(number blendConstant)
(end)

Parameters:

blendConstant - Must be one of the <C_BLEND_MODE_CONSTANTS>.
*/


.def("GetBlendMode", &MovingEntity::GetBlendMode)

/*
func: GetBlendMode
(code)
number GetBlendMode()
(end)

Returns:

One of the <C_BLEND_MODE_CONSTANTS>.
*/



.def("SetAnimPause", &MovingEntity::SetAnimPause)

/*
func: SetAnimPause
(code)
nil SetAnimPause(boolean bPause)
(end)

Allows you to pause the currently playing animation.  Used in the TreeWorld example when stopping on ladders.

Parameters:

bPause - true to pause, false to unpause
*/

.def("GetAnimPause", &MovingEntity::GetAnimPause)

/*
func: GetAnimPause
(code)
boolean GetAnimPause()
(end)

Returns:

True if the active animation is currently paused.
*/

.def("SetAlignment", &MovingEntity::SetAlignment)


/*
func: SetAlignment
(code)
nil SetAlignment(number origin, Vector2 vOffset)
(end)

Allows you to set an alignment for images.  Doesn't work with VisualProfile set images, it's more for GUI type stuff that was set with SetImage.
(end)

Parameters:

origin - One of the <C_ORIGIN_CONSTANTS>.
vOffset - A <Vector2> object containing an offset to place the image.
*/



.def("SetSpriteByVisualStateAndFacing", &MovingEntity::SetSpriteByVisualStateAndFacing)


/*
func: SetSpriteByVisualStateAndFacing
(code)
nil SetSpriteByVisualStateAndFacing()
(end)

This sets the current animation based on the facing and visual state.  Normally brains do this for you.
*/

.def("SetVisualState", &MovingEntity::SetVisualState)

/*
func: SetVisualState
(code)
nil SetVisualState(number visualStateID)
(end)

This sets the current animation set from the current profile. Note, brains will set this automatically when states change.  (I.e., state Idle will set it to C_VISUAL_STATE_IDLE)

Parameters:

visualStateID - One of the C_VISUAL_STATE* defines from setup_constants.lua.
*/



.def("GetVisualState", &MovingEntity::GetVisualState)

/*
func: GetVisualState
(code)
number GetVisualState()
(end)

Returns:

The current visual state ID.  Note, if the <ForceVisual> brain is using force_set or <SetVisualStateOverride> was used, this will return that instead.
*/

.def("SetVisualStateOverride", &MovingEntity::SetVisualStateOverride)


/*
func: SetVisualStateOverride
(code)
nil SetVisualStateOverride(number visualStateID)
(end)

Allows you to force a certain visual state regardless of what brains are setting.  It's what the <ForceVisual> brain uses internally when the force_set parm is used.

Parameters:

visualStateID - One of the C_VISUAL_STATE* defines from setup_constants.lua.  Use C_VISUAL_STATE_NONE to disable the override.
*/
.def("GetVisualStateOverride", &MovingEntity::GetVisualStateOverride)


/*
func: GetVisualStateOverride
(code)
number GetVisualStateOverride()
(end)

Returns:

The current visual state ID that is override the real one, or C_VISUAL_STATE_NONE if no override is active.
*/
.def("SetImage", &MovingEntity::SetImage)


/*
func: SetImage
(code)
nil SetImage(string fileName, Rect clipRect)
(end)

Allows you to set a tilepic-style image.  When you click "Convert to entity" on a tilepic, internally, this is how it remembers what to show.

Because the change is saved in the entity's Data() system, it is persistent.  (You will see a _TilePic and _TileRect entry in the entities data, using the editor)

You can also use a regular path and filename, so a normal image sitting in your script directory can also be loaded.  (Internally, <Entity:AddImageToMapCache> is used) (~/ for "current script directory" can be used)

Note:

VisualProfiles will override this image.

Parameters:

fileName - A filename of an image in one of the Map directories.  No path should be included, it will find automatically.  Or, a path and filename to add any image can be used.
clipRect - A <Rect> that contains what portion of the image to show.  Send nil to use the entire image.
*/


.def("SetImageByID", &MovingEntity::SetImageByID)

/*
func: SetImageByID
(code)
nil SetImageByID(number imageHashID)
(end)

Like <SetImage> but Sets the image by its hash directly.  As file names are not stored, you may only have a hash to work with at times.

Parameters:

imageHashID - A number that represents an image.
*/

.def("GetImageID", &MovingEntity::GetImageID)

/*
func: GetImageByID
(code)
number GetImageByID()
(end)

Returns the hashID of the active tilepic style image, if one has been set by script of by the editor when "Convert To Entity" is used.

Returns:

A number representing the image's hashID.
*/


.def("GetImageClipRect", &MovingEntity::GetImageClipRect)


/*
func: GetImageClipRect
(code)
Rect GetImageClipRect()
(end)

Returns:

A <Rect> containing the portion of the tilepic-style image being displayed.
*/


.def("AddImageToMapCache", &MovingEntity::AddImageToMapCache)


/*
func: AddImageToMapCache
(code)
nil AddImageToMapCache(string fileName)
(end)

Allows you to specify any image to add as a tilepic-style image, even if it isn't in a map directory.  This allows a script to insert its own images that can be referenced by only the filename (without extension) from anywhere.

fileName - A filename of an image in one of the Map directories. ~/ for "current script directory" can be used.
*/

//Group: Physics/Collision Related


.def("InitCollisionDataBySize", &MovingEntity::InitCollisionDataBySize)

/*
func: InitCollisionDataBySize
(code)
nil InitCollisionDataBySize(number x, number y)
(end)

Allows you to specify a basic rectangle collision shape.
If you enter 0's for the size, the current image size will be used.

Note:

Only objects with collision data of some kind can fall from gravity or move.  This is used in intro_menu.lua to allow clicked objects to fall.

Parameters:

x - The width of the new collision area
y - The height of the new collision area

*/

.def("LoadCollisionInfo", &MovingEntity::LoadCollisionInfo)


/*
func: LoadCollisionInfo
(code)
nil LoadCollisionInfo(string fileName)
(end)

Loads a .col (collision) file.  If it doesn't exist, it will be created, and any subsequent collision editing done in  the editor will automatically be saved to its collision file.

About Collision Files:

A collision file contains one or more vector shapes and information about which materials are assigned to them.

Note:

It's fine to share a single collision file between multiple kinds of entities.

Parameters:

fileName - The collision file to load.  Relative paths are supposed with ~/.

*/

.def("SetEnableRotationPhysics", &MovingEntity::EnableRotation)

/*
func: SetEnableRotationPhysics
(code)
nil SetEnableRotationPhysics(boolean bEnable)
(end)

Rotation in this sense means the physics of barrels and boxes that can spin on the Z axis.

* In TopView RPG test, nothing has rotation physics turned on, we don't want barrels and boxes twisting around when their corners are pushed
* In TreeWorld Side view test, barrels, boxes and apples have rotation physics on. The player doesn't, otherwise he will tip over and look drunk.

Parameters:
bEnable - True to activate rotation physics
*/

.def("GetEnableRotationPhysics", &MovingEntity::GetEnableRotation)

/*
func: GetEnableRotationPhysics
(code)
boolean GetEnableRotationPhysics()
(end)

Returns:

True if rotational physics are enabled for this entity.
*/


.def("SetRotation", &MovingEntity::SetRotation)

/*
func: SetRotation
(code)
nil SetRotation(number angle)
(end)

Parameters:

angle - The new angle of rotation you want the entity to instantly move to.

Note:

Rotation like this also rotates your collision box, so watch yourself.  You may prefer to use <AddForceAndTorque> if you want to rotate smoothly.

*/


.def("GetRotation", &MovingEntity::GetRotation)

/*
func: GetRotation
(code)
number GetRotation()
(end)

Returns:

The angle of rotation in radians.
*/


.def("SetCategory", &MovingEntity::SetCollisionCategory)
/*
func: SetCategory
(code)
nil SetCategory(number category, boolean bOn)
(end)

By default, an entity is a member of category <C_CATEGORY_ENTITY>. (Defined as 0 in setup_constants.lua)  These categories are useful for controlling which objects collide (<SetCollisionCategory> and what collisions an entity cares about knowing about with <SetCollisionListenCategory>.

(Note, these replace the SetCollision, SetStaticCollision, SetCollisionListen and SetStaticCollisionListen functions from older versions!)

Tilepics are always members of <C_CATEGORY_TILEPIC>, which is defined as 1 in setup_constants.lua.

There are 16 categories from 0 to 15. It's possible to be a member of more than one category at once.

Parameters:

category - Which category you want to set, must be a number between 0 and 15. (Check <C_CATEGORY_CONSTANTS> for the predefined ones)
bOn - If true, this entity is a member of this category, if false, the opposite.

*/

.def("SetCategories", &MovingEntity::SetCategories)
/*
func: SetCategories
(code)
nil SetCategories(number categoryMask)
(end)

Let's you set all 16 states at once with a single number.  
Only use this if you know what you are doing, or use the <C_CATEGORIES_NONE> define to easily turn off all of them.

Parameters:

categoryMask - A single 16 bit number representing each category. 
*/


.def("SetCollisionCategory", &MovingEntity::SetCollisionMask)
/*
func: SetCollisionCategory
(code)
nil SetCollisionCategory(number category, boolean bOn)
(end)

By default, an entity will collide with all category types. (0 to 15 are ON)  
If you would like this entity to be able to pass through certain categories, you must set it to off.

Note:

This doesn't affect the settings of <SetCollisionListenCategory>, it's possible to never collide with anything but still hear about all the (would-be) collisions.


Parameters:

category - Which category you want to set, must be a number between 0 and 15.  (Check <C_CATEGORY_CONSTANTS> for the predefined ones)
bOn - If true, this object will collide with entities of that category type, if false, we will pass through them.
*/

.def("SetCollisionCategories", &MovingEntity::SetCollisionCategories)
/*
func: SetCollisionCategories
(code)
nil SetCollisionCategories(number categoryMask)
(end)

Let's you set all 16 states at once with a single number.  
Only use this if you know what you are doing, or use the <C_CATEGORIES_NONE> define to easily turn off all of them.

Parameters:

categoryMask - A single 16 bit number representing each category. 
*/

.def("SetCollisionListenCategory", &MovingEntity::SetCollisionListenCategory)
/*
func: SetCollisionListenCategory
(code)
nil SetCollisionListenCategory(number category, boolean bOn)
(end)

Similar to <SetCollisionCategory> but rather than affecting what this entity collides with, it controls which collisions you will hear about in a callback.

By default, all categories are set to off, so no calls to an OnCollision() will be made in this entities script/lua namespace.

If you enable listening to any of the categories, your script must include a function with the following name and parameters:
(code)

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

//vNormal is a Vector2 object showing the direction of the impact in a unit vector.
//depth is how much penetration there is, how much overlap there is

LogMsg("Hit Static: Depth: " .. tostring(depth) .. " vNormal: " .. tostring(vNormal));

	if (materialID == C_MATERIAL_VERTICAL_LADDER) then
		LogMsg("You just touched a tile that has a collision line set to the "V Ladder" type!
	end
end
(end)

Parameters:

category - Which category want to set, must be a number between 0 and 15. (Check <C_CATEGORY_CONSTANTS> for the predefined ones)
bOn - If true, this object will collide with entities of that category type, if false, we will pass through them.
*/

.def("SetCollisionListenCategories", &MovingEntity::SetCollisionListenCategories)
/*
func: SetCollisionListenCategories
(code)
nil SetCollisionListenCategories(number categoryMask)
(end)

Let's you set all 16 states at once with a single number.  
Only use this if you know what you are doing, or use the <C_CATEGORIES_NONE> define to easily turn off all of them.

Parameters:

categoryMask - A single 16 bit number representing each category. 
*/

.def("SetCollisionGroup", &MovingEntity::SetCollisionGroup)
/*
func: SetCollisionGroup
(code)
nil SetCollisionGroup(number group)
(end)

In addition to the other methods, another optional way to control what an entity can collide with is a collision group.
By default, this is 0, which means no group.

If two or more objects are set to the same collision group there are two possible outcomes:

If more than 0:  These objects will always collide
If less than 0:  These objects will never collide

Example:

(code)
//inside our bullet.lua, we decide that we don't want it to collide with other bullets

this:SetCollisionGroup(-1);

//Any other entity that also defines this will join our 'never collide' group.
//We could have easily chosen -500 or -666 to be evil, as long as it's negative, they
//will never collide.  1 or 500 would have meant always collide.
(end)

Note:

This takes precendence over the <SetCollisionCategory> function.

Parameters:

group - a number between -64000 and 64000
*/

.def("GetCollisionGroup", &MovingEntity::GetCollisionGroup)
/*
func: GetCollisionGroup
(code)
number GetCollisionGroup(number group)
(end)

Returns:

The current collision group this entity is in, 0 for none.
*/

.def("SetDensity", &MovingEntity::SetDensity)

/*
func: SetDensity
(code)
nil SetDensity(number density)
(end)

Setting density allows you to control how hard it is to push certain objects.  Note that if a _Density data field is found this is automatically set as the objects density.

A density is not the total weight/mass of an object, it's a more like the weight per square inch of collision size.  So a very large object with a small density could still push around a tiny object with a large density.

Note:

Setting density to 0 is a special case that means "Immovable" and saves processor time.  Note, if two things have a density of 0, they will never generate collisions with each other, as they are both immovable.

Parameters:

density - How dense this object is.  0.2 is like Styrofoam, 3 is like a block of steel.

*/


.def("GetMass", &MovingEntity::GetMass)

/*
func: GetMass
(code)
number GetMass()
(end)

Returns:

The total mass of this entity.
*/

.def("SetGravityOverride", &MovingEntity::SetGravityOverride)

/*
func: SetGravityOverride
(code)
nil SetGravityOverride(number gravity)
(end)

This allows you to override gravity settings on a per-entity basis.  So floating monsters can exist along-side ones that fall to the ground.

Set to <C_GRAVITY_OVERRIDE_DISABLED> to return to using the <Map>'s default gravity.

Parameters:

The gravity you'd like applied to this object, instead of the <Map>'s default gravity.
*/

.def("GetGravityOverride", &MovingEntity::GetGravityOverride)

/*
func: GetGravityOverride
(code)
number GetGravityOverride()
(end)

Returns:

The current gravity override, or <C_GRAVITY_OVERRIDE_DISABLED> if not being used.
*/


.def("Stop", &MovingEntity::Stop)

/*
func: Stop
(code)
nil Stop()
(end)

Instantly stops movement of this entity, including both linear and angular velocity. (Stops moving and stops rotating)
*/

.def("StopX", &MovingEntity::StopX)

/*
func: StopX
(code)
nil StopX()
(end)

Instantly stops movement of this entity on the X axis only.  (Stops moving side to side)
*/


.def("StopY", &MovingEntity::StopY)

/*
func: StopY
(code)
nil StopY()
(end)

Instantly stops movement of this entity on the Y axis only.  (Stops moving up and down)
*/

.def("AddForce", &MovingEntity::AddForceBurst)

/*
func: AddForce
(code)
nil AddForce(Vector2 vForce)
(end)

Adds the applied force to the entity.  Useful to make something jump or get "hit" in a certain direction.

Parameters:

vForce - A <Vector2> object containing how much force to add.  this:AddForce(Vector2(20,0)) would send someone to the right.
*/

.def("AddForceAndTorque", &MovingEntity::AddForceAndTorqueBurst)

/*
func: AddForceAndTorque
(code)
nil AddForceAndTorque(Vector2 vForce, Vector2 vTorque)
(end)

Adds a force as well as a rotational torque.  Normally you only need <AddForce> and don't care about torque.

Parameters:

vForce - A <Vector2> object containing how much force to add.  this:AddForce(Vector2(20,0)) would send someone to the right.
vTorque - A <Vector2> containing the torque to add.  This doesn't even make sense, shouldn't it be a number?  Uh, will get back to this.
*/

.def("AddForceConstant", &MovingEntity::AddForce)

/*
func: AddForceConstant
(code)
nil AddForceConstant(Vector2 vForce)
(end)

When applying a steady force over time (like gravity), you should use this function instead of <AddForce> for greater stability across fluctuating framerates. You still need to apply it every frame.

Tech note:

The reason for this is the actual physics simulation runs between 60 and 120 hz to match the current FPS for the smoothest visual movement.  (regardless of FPS, the game will update 60 times a second minimum) Applying a force over time intelligently requires slightly different math.

Parameters:

vForce - A <Vector2> object containing how much force to add.
*/

.def("AddForceAndTorqueConstant", &MovingEntity::AddForceAndTorque)

/*
func: AddForceAndTorqueConstant
(code)
nil AddForceAndTorqueConstant(Vector2 vForce, Vector2 vTorque)
(end)

Like <AddForceConstant> but also accepts a rotational vector.

Parameters:

vForce - A <Vector2> object containing how much force to add.  this:AddForce(Vector2(20,0)) would send someone to the right.
vTorque - A <Vector2> containing the torque to add.  This doesn't even make sense, shouldn't it be a number?  Uh, will get back to this.

*/


.def("GetLinearVelocity", &MovingEntity::GetLinearVelocity)

/*
func: GetLinearVelocity
(code)
Vector2 GetLinearVelocity()
(end)

Usage:
(code)
LogMsg("Whee!  We're moving at " .. tostring(this:GetLinearVelocity());
(end)

Returns:

The speed the entity is actually moving.
*/


.def("GetOnGround", &MovingEntity::IsOnGround)

/*
func: GetOnGround
(code)
boolean GetOnGround()
(end)

Returns:

True if the entity is currently standing on a platform or another entity.

This is useful to check if the player should be able to "jump" or not.

The engine automatically applies a little processing to smooth it and make it 'feel' right.  Use <GetOnGroundAccurate> if you want raw unprocessed data.
*/

.def("SetOnGround", &MovingEntity::SetIsOnGround)

/*
func: SetOnGround
(code)
nil SetOnGround(boolean bOnGround)
(end)

This can hint to the engine that the entity should or shouldn't be processed as if it is 'standing' on top of another object or ground.

TreeWorld uses this to fine tune when the player can jump or not.  Only applicable to side view games?

Parameters:

bOnGround - true if the user is on the ground, false it in the air
*/

.def("GetOnGroundAccurate", &MovingEntity::IsOnGroundAccurate)

/*
func: GetOnGroundAccurate
(code)
bool GetOnGroundAccurate()
(end)

Returns:

True if entity is currently on the ground or sitting on something.  This data is un-smoothed, so small bounces and things may cause it to change quickly.
*/

.def("SetCollisionScale", &MovingEntity::SetCollisionScale)

/*
func: SetCollisionScale
(code)
nil SetCollisionScale(Vector2 vScale);
(end)

When scaling an entity, it's collision data will automatically be scaled with it.

Parameters:

vScale - A <Vector2> object containing the new absolute scale.  Vector2(2,2) would be twice as big, Vector2(0.5,0.5) would be half as big.
*/

.def("GetCollisionScale", &MovingEntity::GetCollisionScale)

/*
func: GetCollisionScale
(code)
Vector2 GetCollisionScale();
(end)

Returns:

The current scale of the entity.
*/


.def("SetDampening", &MovingEntity::SetDampening)

/*
func: SetDampening
(code)
nil SetDampening(number dampening)
(end)

Changing the dampening controls how quickly an entity will slide to a stop.  Think of it like friction.

For things with  their own locomotion to start and stop them, this should probably be set to 0.

Parameters:

dampening - The new dampening amount.  Default is 0.05.
*/


//Group: AI Related

.def("GetBrainManager", &MovingEntity::GetBrainManager)

/*
func: GetBrainManager
(code)
BrainManager GetBrainManager()
(end)

Returns:

A handle to this entity's unique <BrainManager>.  Behind the scenes, this is created upon the first access.
*/

.def("GetGoalManager", &MovingEntity::GetGoalManager)

/*
func: GetGoalManager
(code)
GoalManager GetGoalManager()
(end)

Returns:

A handle to this entity's unique <GoalManager>.  Behind the scenes, this is created upon the first access.
*/

.def("SetDesiredSpeed", &MovingEntity::SetDesiredSpeed)

/*
func: SetDesiredSpeed
(code)
nil SetDesiredSpeed(number desiredSpeed)
(end)

Brain states such as Walk will use this speed to move.

Will be limited by whatever <SetMaxMovementSpeed> is set to.

Default is 2.4.

Parameters:

desiredSpeed - How fast the entity wants to move.
*/

.def("SetMaxMovementSpeed", &MovingEntity::SetMaxWalkSpeed)

/*
func: SetMaxMovementSpeed
(code)
nil SetMaxMovementSpeed(number maxMoveSpeed)
(end)

Limits the internal move force of the entity to this speed.  For instance, even if a walking entity suddenly chose to dodge a bullet, it may move faster than his walk speed, but will still be limited by this speed.

Default is 6.

Parameters:

maxMoveSpeed - The fastest this entity can move from its own power.
*/


.def("SetAcceleration", &MovingEntity::SetAccel)

/*
func: SetAcceleration
(code)
nil SetAcceleration(number accelSpeed)
(end)

Controls how fast an entity can accelerate, when a brain/walk state asks it to.

Default speed is 0.37.

Parameters:

accelSpeed - A higher number means an <Entity> will get up to its desired speed faster.
*/

.def("GetAcceleration", &MovingEntity::SetAccel)

/*
func: GetAcceleration
(code)
float GetAcceleration()
(end)

Returns:

The current acceleration speed.
*/


.def("SetTurnSpeed", &MovingEntity::SetTurnSpeed)

/*
func: SetTurnSpeed
(code)
nil SetTurnSpeed(number turnSpeed)
(end)

The <GoalManager> or manual calls to <SetFacingTarget> will cause the player to turn using this speed.

Parameters:

turnSpeed - How quick an entity can turn.
*/


.def("GetTurnSpeed", &MovingEntity::GetTurnSpeed)

/*
func: GetTurnSpeed
(code)
number GetTurnSpeed()
(end)

Returns:

How fast this entity can turn.
*/
.def("SetFacing", &MovingEntity::SetFacing)

/*
func: SetFacing
(code)
nil SetFacing(number facing)
(end)

Causes an entity to instantly turn to this direction.  <SetFacingTarget> is also set internally, so the entity will keep facing this direction.
For more accuracy, see <SetVectorFacing>.

Parameters:

facing - One of the <C_FACING_CONSTANTS>.
*/



.def("GetFacing", &MovingEntity::GetFacing)

/*
func: GetFacing
(code)
number GetFacing()
(end)

Use <GetVectorFacing> for more accuracy.

Returns:

One of the <C_FACING_CONSTANTS> to best describe the direction the entity is currently facing.

*/


.def("SetFacingTarget", &MovingEntity::SetFacingTarget)

/*
func: SetFacingTarget
(code)
nil SetFacingTarget(number facing)
(end)

Cause an entity to "turn" towards a certain direction.  If you want him to turn instantaneously, use <SetFacing> instead.
For more accuracy, see <SetVectorFacingTarget>.

Parameters:

facing - One of the <C_FACING_CONSTANTS>.
*/


.def("GetFacingTarget", &MovingEntity::GetFacingTarget)

/*
func: GetFacingTarget
(code)
number GetFacingTarget()
(end)

Allows you to see where this entity wants to face.  He may or not be facing there at the time.  See <IsFacingTarget> to check for that.

Use <GetVectorFacing> for more accuracy.

Returns:

One of the <C_FACING_CONSTANTS> to best describe the direction this entity wants to face.
*/

.def("SetVectorFacing", &MovingEntity::SetVectorFacing)

/*
func: SetVectorFacing
(code)
nil SetVectorFacing(Vector2 vDirection)
(end)

Causes an entity to instantly turn to this direction.  <SetVectorFacingTarget> is also set internally, so the entity will keep facing this direction.

Parameters:

vDirection - A <Vector2> object containing the normalized direction vector we should face.
*/


.def("GetVectorFacing", &MovingEntity::GetVectorFacing)

/*
func: GetVectorFacing
(code)
Vector2 GetVectorFacing()
(end)

Returns:

A <Vector2> object containing a unit vector of the entity's current direction.
*/

.def("SetVectorFacingTarget", &MovingEntity::SetVectorFacingTarget)

/*
func: SetVectorFacingTarget
(code)
nil SetVectorFacingTarget(Vector2 vDirection)
(end)

Cause an entity to turn towards a specified direction.

Parameters:

vDirection - A <Vector2> object containing a normalized direction vector we should turn to.
*/
.def("GetVectorFacingTarget", &MovingEntity::GetVectorFacingTarget)

/*
func: GetVectorFacingTarget
(code)
Vector2 GetVectorFacingTarget()
(end)

Returns:

A <Vector2> object containing a unit vector of the entitys facing target.
*/

.def("IsFacingTarget", &MovingEntity::IsFacingTarget)

/*
func: IsFacingTarget
(code)
boolean IsFacingTarget(number toleranceRadians)
(end)

Allows you to test to see if this entity has turned to face it's facing target. (set with <SetFacingTarget> or <SetVectorFacingTarget>)

Usage:
(code)

//Is the entity facing its target yet?

if (this:IsFacingTarget(1.3)) then
		LogMsg("Oh yeah.  We've turned close enough.");
	end
(end)

Parameters:

toleranceRadians - How far off we can be and still think of it as "facing" the direction.

Returns:

True if facing the target
*/

.def("GetVectorToEntity", &MovingEntity::GetVectorToEntity)

/*
func: GetVectorToEntity
(code)
Vector2 GetVectorToEntity(Entity ent)
(end)

Parameters:

ent - The <Entity> you'd like to get a direction to.

Returns:

A <Vector2> object containing a unit vector describing which way to the entity in question.
*/


.def("GetVectorToEntityByID", &MovingEntity::GetVectorToEntityByID)

/*
func: GetVectorToEntityID
(code)
Vector2 GetVectorToEntity(number entityID)
(end)

Parameters:

entityID - An ID of an <Entity>.

Returns:

A <Vector2> object containing a unit vector describing which way to the entity in question.
*/

.def("GetVectorToPosition", &MovingEntity::GetVectorToPosition)

/*
func: GetVectorToPosition
(code)
Vector2 GetVectorToPosition(Vector2 vPos)
(end)

Parameters:

vPos - A <Vector2> holding the map coordinates we want a vector to.

Returns:

A <Vector2> object containing a unit vector describing which way to the position in question.
*/

.def("GetDistanceFromEntityByID", &MovingEntity::GetDistanceFromEntityByID)

/*
func: GetDistanceFromEntityByID
(code)
number GetDistanceFromEntityByID(number entityID)
(end)

Parameters:

entityID - The entity we'd like to measure the distance to.

Returns:

The distance to the entity, -1 if the entity is invalid, <C_DISTANCE_NOT_ON_SAME_MAP> if they are not on the same map. 
*/

.def("GetDistanceFromPosition", &MovingEntity::GetDistanceFromPosition)

/*
func: GetDistanceFromPosition
(code)
number GetDistanceFromPosition(Vector2 vPos)
(end)

Parameters:

vPos - A <Vector2> of the position we want the distance to.

Returns:

The distance to the position.
*/


.def("SetHasPathNode", &MovingEntity::SetHasPathNode)

/*
func: SetHasPathNode
(code)
nil SetHasPathNode(boolean bHasPathNode)
(end)

If an entity has this enabled, the path-finding system will use it as a node.  Nodes are automatically connected to nearby and reachable nodes.

Must be set in an entity's OnInit().

Parameters:

bHasPathNode - If true, it becomes part of the path-finding graph.
*/

.def("SetNavNodeType", &MovingEntity::SetNavNodeType)

/*
func: SetNavNodeType
(code)
nil SetNavNodeType(number nodeType)
(end)

Must be set in an entity's OnInit().  Special entity things like warps and doors use this to let the path-finding system know what's going on.

Parameters:

nodeType - One of the <C_NODE_TYPE_CONSTANTS>.
*/


.def("IsOnSameMapAsEntityByID", &MovingEntity::IsOnSameMapAsEntityByID)

/*
func: IsOnSameMapAsEntityByID
(code)
boolean IsOnSameMapAsEntityByID(number entID)
(end)

A quick way to test if another entity is or isn't on the same map.  Will return false if the entityID is invalid.

Parameters:

entID - The entity ID of who you're checking

Returns:

True if the entity is somewhere on the same map.
*/


.def("IsCloseToEntity", &MovingEntity::IsCloseToEntity)

/*
func: IsCloseToEntity
(code)
boolean IsCloseToEntity(Entity ent, number distance)
(end)

A quick way to test if another entity is within a certain range.

Returns false if entity is not within range of distance or on another map.

Parameters:

ent - A handle to a valid <Entity> object.

distance - How close they have to be to return true.

Returns:

True if the entity is within the range of the distance specified.
*/



.def("IsCloseToEntityByID", &MovingEntity::IsCloseToEntityByID)

/*
func: IsCloseToEntityByID
(code)
boolean IsCloseToEntityByID(number entityID, number distance)
(end)

Like <IsCloseToEntity> but takes an ID.  If the entity ID is invalid, false will be returned.

Parameters:

entityID - The ID of the entity we're interested in.

distance - How close they have to be to return true.

Returns:

True if the entity is within the range of the distance specified.
*/


.def("CanWalkTo", &MovingEntity::CanWalkTo)

/*
func: CanWalkTo
(code)
boolean CanWalkTo(Vector2 vPos, boolean bIgnoreLivingCreatures)
(end)

Lets you know if this entity can walk straight to a specific point without being blocked by any walls or optionally, creatures.

Note:  

This only tests against edges.  So if you're inside of a giant collision shape for some reason, it wouldn't catch it.
Not connected to the path-finding system, this is a very simple check.

Parameters:

vPos - A <Vector2> object specifying the target position.
bIgnoreLivingCreatures - If true, creatures will be ignored during the check.

Returns:

True if the entity can walk to the position in question without bumping into anything.
*/



.def("CanWalkBetween", &MovingEntity::CanWalkBetween)

/*
func: CanWalkBetween
(code)
boolean CanWalkBetween(Map map, Vector2 vFrom, Vector2 vTo, boolean bIgnoreLivingCreatures)
(end)

Like <CanWalkTo> but can check any point on any map, instead of assuming you mean the entity's current location and map.

Note:  

This only tests against edges.  So if you're inside of a giant collision shape for some reason, it wouldn't catch it.
Not connected to the path-finding system, this is a very simple check.

Parameters:

map - A valid <Map> object handle.
vFrom - A <Vector2> object specying the start position.
vTo - A <Vector2> object specifying the end position.
bIgnoreLivingCreatures - If true, creatures will be ignored during the check.

Returns:

True if the entity can walk between the two positions without bumping into anything.
*/

.def("HasLineOfSightToPosition", &MovingEntity::CanWalkTo)
/*
func: HasLineOfSightToPosition
(code)
boolean HasLineOfSightToPosition(Vector2 vPos, boolean bIgnoreLivingCreatures)
(end)

Lets you know if you can see a position from the current point or not.  Internally, this just uses CanWalk to, but this may change later.

Parameters:

vPos - A <Vector2> object specifying the target position.
bIgnoreLivingCreatures - If true, creatures will be ignored during the check.

Returns:

True if the entity can see the position.
*/


.def("IsValidPosition", &MovingEntity::IsValidPosition)

/*
func: IsValidPosition
(code)
boolean IsValidPosition(Map map, Vector2 vPos, boolean bIgnoreLivingCreatures)
(end)

Let's find out if the entity can comfortably fit in this position.

Parameters:

map - The <Map> to check on.
vPos - A <Vector2> object specifying the position to test.
bIgnoreLivingCreatures - If true, creatures will be ignored during the check.

Returns:

True if the entity can be moved here without overlapping any walls or other collidable things.
*/


.def("GetApproachPosition", &MovingEntity::GetApproachPosition)

/*
func: GetApproachPosition
(code)
boolean GetApproachPosition(Entity ent, number distance, Vector2 vOutputPos)
(end)

Query an entity to find a nearby position that the sent entity would fit in.  Useful for figuring out where to programmatically place people during cut-scenes in a 2d overheard RPG.

If the two entities are on the same map, it will automatically try to locate a spot closest to where the <Entity> is already.

Parameters:

ent - The <Entity> that we will need to fit in the space we find.
distance - How far away the spot is.  Use -1 to mean "inside", else, a spot will be chosen so the two entities don't overlap.
vOutputPos - This will hold the chosen position after the function is called.

Returns:

True if successfully located a position and put it in vOutputPos.
*/

.def("SetIsCreature", &MovingEntity::SetIsCreature)

/*
func: SetIsCreature
(code)
nil SetIsCreature(boolean bIsLivingCreature)
(end)

You may have noticed some path-finding related functions such as <CanWalkBetween> have options to ignore "creatures".

This means, even if the path is blocked with 50 barrels or skeletons, the path-finding engine will still assume it's a valid path.  (after all, he can
break the barrels and kill the skeletons, right?)

So what is a creature?:

The engine guesses at what is and isn't a creature.  It's very simple, if you call SetVisualProfile() on an object, it assumes it's a creature.

However, if this is wrong, or you have a single frame creature with no visual profile, you may want want to manually set it with this command.

Parameters:

bIsLivingCreature - If true, this <Entity> will be considered a "living creature" during times that it matters.
*/

.def("GetIsCreature", &MovingEntity::GetIsCreature)

/*
func: GetIsCreature
(code)
boolean GetIsCreature()
(end)

Returns:

True if this <Entity> is currently considered a living creature.
*/


//Group: Scripting Related

.def("SetVisibilityNotifications", &MovingEntity::SetVisibilityNotifications)

/*
func: SetVisibilityNotifications
(code)
nil SetVisibilityNotifications(boolean bEnable)
(end)

By enabling visibility notifications, you will get a script callback when an entity becomes visible and when he loses visibility.

Being visible means he's close enough to the screen to be drawn.

After enabling, you need to create the following script functions to handle the callbacks:
(code)
function OnGetVisible()
end

function OnLoseVisible() //we left the screen
end
(end)

Parameters:

bEnable - true to turn it on, false to disable.
*/

.def("SetRunUpdateEveryFrame", &MovingEntity::SetRunUpdateEveryFrame)

/*
func: SetRunUpdateEveryFrame
(code)
nil SetRunUpdateEveryFrame(boolean bEnable)
(end)

By enabling this option, a script callback is called at every logic cycle. (a minimum of 60 times a second of game-time)

You must add the following function to handle the callback.

(code)
//this is run every logic tick
//step is the delta, sometimes this is useful for math.
//Most functions such as AddForce internally compute the delta into
//the math for you, so you don't have to worry about it.
function Update(step)
end
(end)

Parameters:

bEnable - true to turn it on, false to disable.
*/

.def("GetRunUpdateEveryFrame", &MovingEntity::GetRunUpdateEveryFrame)
/*
func: GetRunUpdateEveryFrame
(code)
boolean GetRunUpdateEveryFrame()
(end)

Returns:

True if we're currently running Update(step) every game tick for this entity.
*/

.def("SetTrigger", &MovingEntity::SetTrigger)

/*
func: SetTrigger
(code)
nil SetTrigger(number triggerType, number typeParm, number triggerBehavior, number behaviorParm)
(end)

A trigger is a way to get a callback when the player enters or leaves a specific area.

They are much cheaper (in terms of CPU cycles) then other methods of notification.

The music and ambient sound objects use triggers to know when to play music.

Only one trigger per entity is allowed.  If you set it again, it wipes out the old one that was there.

After a trigger is created, you need to create functions to handle its callbacks.

Usage:

(code)
function OnInit()

	//setup a trigger that will tell us not only when the player enters and leaves an area, but
	//will also call "OnTriggerInside" every 2000 MS (2 seconds).
	this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_PULSE, 2000);
end

//handle the trigger callbacks

function OnTriggerEnter(ent)
	LogMsg("Entering region!  Entity touched us at " .. tostring(ent:GetPos()));
end

function OnTriggerExit(ent)
	LogMsg("Leaving region");
end

function OnTriggerInside(ent)
	LogMsg("Inside region!");
	//he's still standing on the hot lava, hurt him!  Or whatever.
end

(end)

Parameters:

triggerType - One of the <C_TRIGGER_TYPE_CONSTANTS>, describes what causes the trigger to activate.
typeParm - The meaning varies depending on the trigger type.
triggerBehavior - One of the <C_TRIGGER_BEHAVIOR_CONSTANTS>. Desribes what happens after a trigger activates.
behaviorParm - The meaning varies depending on the trigger behavior.

(end)

Returns:

True if we're currently running Update() every game tick for this entity.
*/

.def("RunFunction", (luabind::object(MovingEntity::*) (const string&)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)

.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)
.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)



//luabind no likey
//.def("RunFunction", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunction)

/*
func: RunFunction
(code)
variable RunFunction(string functionName, parms, ...)
(end)

This runs a function in the entity's namespace. (probably a function you wrote in the entity's script)

Up to seven parameters can be sent.

If you just want to run code from an entity's namespace, rather than a function, use <Schedule> instead.

This function is useful because it lets you easily run functions in OTHER entities and get information via what they return.

Note:

A scripting error will be shown if the function doesn't exist.  If you'd like it to silently fail, use <RunFunctionIfExists> or use <FunctionExists> to check before running.

Parameters:

functionName - The function the entity should run.
variable parms - Up to seven parameters of any type may be sent to match whatever function you're calling.

Returns:

Returns whatever the function you called returns.
*/


.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&)) &MovingEntity::RunFunctionIfExists)
.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object)) &MovingEntity::RunFunctionIfExists)
.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object)) &MovingEntity::RunFunctionIfExists)
.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunctionIfExists)

.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunctionIfExists)
.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunctionIfExists)
.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunctionIfExists)
.def("RunFunctionIfExists", (luabind::object(MovingEntity::*) (const string&, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)) &MovingEntity::RunFunctionIfExists)


/*
func: RunFunctionIfExists
(code)
variable RunFunctionIfExists(string functionName, parms, ...)
(end)

Identical to <RunFunction> except it silently fails if the function doesn't exist (or the entity doesn't have a script loaded) instead of throwing a hissy fit error.

Parameters:

functionName - The function the entity should run.
variable parms - Up to seven parameters of any type may be sent to match whatever function you're calling.

Returns:

Returns whatever the function you called returns.
*/



.def("FunctionExists", &MovingEntity::FunctionExists)

/*
func: FunctionExists
(code)
boolean FunctionExists(string functionName)
(end)

Parameters:

functionName - the function name you want to see if this entity has or not.

Returns:

True if the function exists.
*/

.def("VariableExists", &MovingEntity::VariableExists)

/*
func: VariableExists
(code)
boolean VariableExists(string variableName)
(end)

Parameters:

variableName - the name of a variable or table that you want to see exists in this namespace.

Returns:

True if the variable/table/etc exists in this namespace, or in the global namespace.
*/

.def("DumpScriptInfo", &MovingEntity::DumpScriptInfo)

/*
func: DumpScriptInfo
(code)
nil DumpScriptInfo()
(end)

This is a diagnostic function that will send all functions and variables in this entity's scripting namespace to the System Console.

You'll need to bring up the System Console to see the output, or check the log.txt file when you're done.
*/

//.def("SetGarbageCollectionOnKill", &MovingEntity::SetGarbageCollectionOnKill)


//Group: Audio Related
.def("PlaySound", &MovingEntity::PlaySound)

/*
func: PlaySound
(code)
number PlaySound(string soundFile)
(end)

Similar to using <SoundManager::Play> but lets you use paths relative to this entity's main script.

Parameters:

soundFile - The file name to play.

Returns:

The soundID handle.  This can be used with the <SoundManager> for additional controls if needed.

*/


.def("PlaySoundPositioned", &MovingEntity::PlaySoundPositioned)

/*
func: PlaySoundPositioned
(code)
number PlaySoundPositioned(string soundFile)
(end)

If the entity is near the screen then the sound will be heard with appropriate panning and volume.

If the entity is too far from the camera or on a different map, the sound isn't played.

Internally, it just calls <UpdateSoundByPosition> with default parameters (100, 500, 1).

Note:

Panning not implemented yet.

Parameters:

soundFile - The file name to play.

Returns:

The soundID handle.  This can be used with the <SoundManager> for additional controls if needed.
*/


.def("UpdateSoundByPosition", &MovingEntity::UpdateSoundByPosition)

/*
func: UpdateSoundByPosition
(code)
number UpdateSoundByPosition(number soundID, number hearingMin, number hearingMax, number volMod)
(end)

If the entity is near the screen then the sound will be heard with appropriate panning and volume.

If the entity is too far from the camera or on a different map, the sound isn't played.

Parameters:

soundID - The ID of an existing sound (for instance, what <PlaySound> returns)
hearingMin - If the entity is closer than this distance to listner, it will play maximum
hearingMax - From the distance of hearingMin to the distance of hearingMax, the sound will gradually fade out.
*/



//Group: Text Related


.def("SetText", &MovingEntity::SetText)

/*
func: SetText
(code)
nil SetText(string text)
(end)

Setting this causes text to be drawn over the entity.

Use \n for a line break.

If you try to use other text functions before using <SetText>, they will fail and tell you to call <SetText> first, as this also initializes the data structures required for text-related functionality.

To position and orient the text, use <SetTextAlignment>.

If <SetTextRect> is also set, the text will automatically be wrapped. (not implemented yet)

Note:

Text is sorted along with the entity, meaning it can be obscured by other objects, unlike text drawn through the <TextManager>.

Text is normally scaled up and down when the camera zooms out, unlike text drawn through the <TextManager>.  The exception to this is when you <SetAttach> to <C_ENTITY_CAMERA>, in this case it will not scale with the camera.  (more useful for GUI things)

Parameters:

text - The text you'd like to display.
*/

.def("GetText", &MovingEntity::GetText)

/*
func: GetText
(code)
string GetText()
(end)

Returns:

The text that was set with <SetText>.
*/

.def("SetFont", (void(MovingEntity::*)(const std::string&))&MovingEntity::SetFont)
.def("SetFont", (void(MovingEntity::*)(int))&MovingEntity::SetFont)

/*
func: SetFont
(code)
nil SetFont(string resourceID)
nil SetFont(number fontID)
(end)

Set the font to be used by the entity.

You can use the resourceID directly from the resource file or you can use the fontID loaded by GameLogic::InitGameGUI(string xmlFile).

Parameters:

string version
resourceID - the value of the name property of the font tag in a resource file.

number version
fontID - A valid fontID or one of the <C_FONT_CONSTANTS>.
*/


.def("GetTextBounds", &MovingEntity::GetTextBounds)

/*
func: GetTextBounds
(code)
Vector2 GetTextBounds()
(end)

Returns:

A <Vector2> object containing the size the text will take when drawn.
*/


.def("SetTextAlignment", &MovingEntity::SetTextAlignment)

/*
func: SetTextAlignment
(code)
nil SetTextAlignment(number origin, Vector2 vOffset)
(end)

Allows you to position text more accurately.  For instance, to put a name hovering over a monster, do this in his script:
(code)
this:SetText("Gary");
this:SetTextAlignment(C_ORIGIN_CENTER, Vector2(0,50)); //50 units above
(end)

Parameters:

origin - One of the <C_ORIGIN_CONSTANTS>.
vOffset - A <Vector2> object containing an offset to place the text.  Use Vector2(0,0) for no offset.
*/

.def("SetTextColor", &MovingEntity::SetTextColor)

/*
func: SetTextColor
(code)
nil SetTextColor(Color color)
(end)

Parameters:

color - A <Color> object containing the new color to use for text.
*/

.def("GetTextColor", &MovingEntity::GetTextColor)

/*
func: GetTextColor
(code)
Color GetTextColor()
(end)

Returns:

A <Color> object containing the current text color.  Defaults to white.
*/

.def("SetTextScale", &MovingEntity::SetTextScale)

/*
func: SetTextScale
(code)
nil SetTextScale(Vector2 vScale)
(end)

Parameters:

vScale - A <Vector2> object containing the new text scale.  Vector2(2,2) would mean twice as big.
*/


.def("GetTextScale", &MovingEntity::GetTextScale)

/*
func: GetTextScale
(code)
Vector2 GetTextScale()
(end)

Returns:

A <Vector2> object containing the current text scale.
*/

.def("SetTextRect", &MovingEntity::SetTextRect)

/*
func: SetTextRect
(code)
nil SetTextRect(Rect rect)
(end)
Note:

This isn't implemented yet, has no effect!

Parameters:

rect - A <Rect> object containing a region to limit text to.  Text automatically wraps around it.
*/


.def("SetDefaultTalkColor", &MovingEntity::SetDefaultTextColor)

/*
func: SetDefaultTalkColor
(code)
nil SetDefaultTalkColor(Color color)
(end)

Lets you set the default color of the text when the <TextManager> is used with this entity.  Not related to <SetTextColor> or <SetBaseColor> at all.

Parameters:

color - a <Color> object containing the default color <TextManager> should use when writing text attached to this entity.
*/

		
//Group: Data/Cloning Related

.def("Data", &MovingEntity::GetData)

/*
func: Data
(code)
DataManager Data()
(end)
This should really be called GetDataManager, but in this one case I think a smaller name is worth it for convenience as commands can get very long with this.

Any data is automatically saved and loaded with its entity.

Data can be viewed/edited from the editor as well.

Returns:

A <DataManager> handle allowing you to store/retrieve persistent entity-specific custom data.

*/


.def("Clone", &MovingEntity::Clone)

/*
func: Clone
(code)
Entity Clone(Map map, Vector2 vPos)
(end)

Cloning is exactly like copying and pasting an entity in the editor.

If you clone an entity with a name, such as "Gun", the new entity created may be called "GunA" or something like that, as two things may not have the exact same name.

Parameters:

map - The <Map> that the new entity should be created on or nil to use the active <Map>.
vPos - The position the new entity should be created on.

Returns:

A handle to the new <Entity> created.

*/


.def("CreateEntity", &MovingEntity::CreateEntity)

/*
func: CreateEntity
(code)
Entity CreateEntity(Map map, Vector2 vPos, string scriptName)
(end)

Exactly like using the global version of CreateEntity except supporting entity-script file relative paths.

Parameters:

map - The <Map> that the new entity should be created on or nil to use the Entity's <Map>.
vPos - The position the new entity should be created on.
scriptName - The filename of the script to initialize the Entity with.

Returns:

A handle to the new <Entity> created.

*/

.def("SetPersistent", &MovingEntity::SetPersistent)

/*
func: SetPersistent
(code)
nil SetPersistent(boolean bPersistent)
(end)

Persistence in this case means "will be saved and loaded with the map".

Note:

If the <Map> this entity exists on is not persistent, nothing is going to be saved in any case.

Parameters:

bPersistent - Set to false and this Entity will not be saved with the map.  (Default is true)
*/

.def("GetPersistent", &MovingEntity::GetPersistent)

/*
func: GetPersistent
(code)
boolean GetPersistent()
(end)

Returns:

True if this entity will be saved with the map.  If false, it won't exist after saving and reloading the map.
*/


.def("IsPlaced", &MovingEntity::IsPlaced)

/*
func: IsPlaced
(code)
boolean IsPlaced()
(end)

Internally, when entities are placed in a copy buffer, they will run *OnInit()* and but not *OnMapInsert()*, *OnPostInit()*, or *OnKill()*.  This function is a way to see if an entity was actually placed on the map or not.

Returns:

True if this entity has actually been placed on a map.
*/

.def("AddEffect", &MovingEntity::AddEffect)



.def("__tostring", &EntityToString)
			
		//Group: Zone/Scanning Related
		
	.def("InZoneByMaterialType", &MovingEntity::InZoneByMaterialType)
	
	/*
	func: InZoneByMaterialType
	(code)
	boolean InZoneByMaterialType(number materialType)
	(end)

	Let's you check to see if you're standing on a certain type of collision/zone by material type, such as a ladder or warp.

	The method used to determine if you're making contact with any collision shape of the requested material is by checking to see if its rect and your collision rect overlap.
	
	An entity should have a collision shape to use this function.

	Parameters:

	materialType - One of the <C_MATERIAL_TYPE_CONSTANTS> or a custom material type that was created in script.
	
	Returns:

	True if the entity is touching a collision shape with this material type.
	*/

	
	.def("InNearbyZoneByMaterialType", &MovingEntity::InNearbyZoneByMaterialType)
	
	/*
	func: InNearbyZoneByMaterialType
	(code)
	boolean InNearbyZoneByMaterialType(Vector2 vPos, number materialType)
	(end)

	Similar to <InZoneByMaterialType> but lets you enter an exact point to check.
	
	It's useful for seeing if there is a ladder an inch below your entity's foot, for instance.

	An entity should have a collision shape to use this function.

	Parameters:

	vPos - A <Vector2> position that is very close to the entity.
	materialType - One of the <C_MATERIAL_TYPE_CONSTANTS> or a custom material type that was created in script.

	Returns:

	True if the entity is touching a collision shape with this material type.
	*/
	
	.def("GetActiveZoneByMaterialType", &MovingEntity::GetNearbyZoneByCollisionRectAndType)
	
	/*
	func: GetActiveZoneByMaterialType
	(code)
	Zone GetActiveZoneByMaterialType(number materialType)
	(end)

	Similar to <InZoneByMaterialType> but returns a <Zone> data object that describes the collision better. 

	Usage:

	(code)
	local z = this:GetActiveZoneByMaterialType(C_MATERIAL_TYPE_WARP);

	if (z.materialID != C_MATERIAL_TYPE_NONE) then
		LogMsg("We collided with something of the material type we asked about!");

		if (z.entityID != C_ENTITY_NONE) then
			//It must be a simple tilepic, it isn't an entity.
		else
			LogMsg("We collided with entity # " .. z.entityID .. "!");
		end
	end
	(end)
	
	The method used to determine if you're making contact with any collision shape of the requested material is by checking to see if its rect and your collision rect overlap.

	An entity should have a collision shape to use this function.

	Parameters:

	materialType - One of the <C_MATERIAL_TYPE_CONSTANTS> or a custom material type that was created in script.

	Returns:

	A <Zone> object describing the results.  If the Zone's materialID variable is not <C_MATERIAL_TYPE_NONE>, the zone type was found and contains valid data.
	*/
	
	.def("GetNearbyZoneByMaterialType", &MovingEntity::GetNearbyZoneByPointAndType)
	
	/*
	func: GetNearbyZoneByMaterialType
	(code)
	Zone GetNearbyZoneByMaterialType(Vector2 vPos, number materialType)
	(end)

	Similar to <InNearbyZoneByMaterialType> but returns a <Zone> data object that describes the collision better. 

	The method used to determine if you're making contact with any collision shape of the requested material is by checking to see if its rect and your collision rect overlap.

	An entity should have a collision shape to use this function.

	Parameters:

	vPos - a position close to the entity.
	materialType - One of the <C_MATERIAL_TYPE_CONSTANTS> or a custom material type that was created in script.

	Returns:

	A <Zone> object describing the results.  If the Zone's materialID variable is not <C_MATERIAL_TYPE_NONE>, the zone type was found and contains valid data.
	*/
	
			
	];

/*

Section: Entity Related Constants

Group: C_BLEND_MODE_CONSTANTS

constants: C_BLEND_MODE_NORMAL
Default blending mode.

constants: C_BLEND_MODE_ADDITIVE
Colors overlay to get brighter.

constants: C_BLEND_MODE_NEGATIVE
Colors are removed to make things darker.


Group: C_ANIM_FRAME_CONSTANTS

constants: C_ANIM_FRAME_LAST
Means the last frame of the animation.


Group: C_GRAVITY_CONSTANTS
Used with <Entity::SetGravityOverride>.

constants: C_GRAVITY_OVERRIDE_DISABLED
Use the default <Map>'s gravity on this entity instead of a user defined one.

Group: C_DISTANCE_CONSTANTS
Various uses.

constants: C_DISTANCE_TALK
How close someone should be to talk.

constants: C_DISTANCE_CLOSE
The closest you can get to someone/something without trying to go inside him.

constants: C_DISTANCE_INSIDE
Means actually try to go inside the target when used with GoalManager::AddApproach.  Good for monsters that need to cause touch damage.

constants: C_DISTANCE_NOT_ON_SAME_MAP
If <Entity::GetDistanceFromEntityByID> returns this, you know the entity in question has left the map.


Group: C_NODE_TYPE_CONSTANTS
Use with <Entity::SetNavNodeType> to describes special properties related to path-finding.

constants: C_NODE_TYPE_NORMAL
A normal node, nothing special.

constants: C_NODE_TYPE_WARP
A warp or door.  Path-finding engine needs to track it in a special way.

Group: C_TRIGGER_TYPE_CONSTANTS
Use with <Entity::SetTrigger> to describe what activates the trigger.

constants: C_TRIGGER_TYPE_NONE
Disables the trigger. triggerParm is ignored.

constants: C_TRIGGER_TYPE_REGION_IMAGE
Trigger is enabled when active player overlaps this entity's image.  triggerParm is ignored.

Group: C_TRIGGER_BEHAVIOR_CONSTANTS
Use with <Entity::SetTrigger> to describe what happens when a trigger is activated.

constants: C_TRIGGER_BEHAVIOR_NORMAL
OnTriggerEnter(ent) and OnTriggerExit(ent) are called. behavior parm is ignored.

constants: C_TRIGGER_BEHAVIOR_PULSE
In addition to OnTriggerEnter(ent) and OnTriggerExit(ent) being called, OnTriggerInside(ent) is called at the interval set by behaviorParm.


Group: C_CATEGORY_CONSTANTS
Default constants used with <Entity::SetCategory> or <Entity::SetCollisionCategory> to control turning on and off specific categories. (Setup in setup_constants.lua)

constants: C_CATEGORY_ENTITY
All entities start as being a member of this.

constants: C_CATEGORY_TILEPIC
All tilepics are marked as this category.

constants: C_CATEGORY_PLAYER
A default category setup for your own use.

constants: C_CATEGORY_DAMAGE
A default category setup for your own use.

constants: C_CATEGORY_PLATFORM
A default category setup for your own use.

constants: C_CATEGORY_ENEMY
A default category setup for your own use.

constants: C_CATEGORY_CHARACTER
A default category setup for your own use.

constants: C_CATEGORY_BULLET
A default category setup for your own use.

constants: C_CATEGORY_POWER_UP
A default category setup for your own use.

Group: C_CATEGORIES_CONSTANTS
Default constants used with <Entity::SetCategories> or <Entity::SetCollisionCategories> to control setting all 16 bits at once. (Setup in setup_constants.lua)

constants: C_CATEGORIES_NONE
Sets all to off.

constants: C_CATEGORIES_ALL
Sets all to on.


*/


}


		
