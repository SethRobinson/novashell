
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
*/

#pragma once

/*
This world class keeps track of the individual chunks that make up the world.  
*/

#define C_DEFAULT_SCREEN_ID 0
extern const CL_Vector2 g_mapDefaultCenterPos;
#define C_MAP_DAT_FILENAME "map.dat"

#include "main.h"
#include "Screen.h"
#include "Tile.h"
#include "MapChunk.h"
#include "CameraSetting.h"
#include "LayerManager.h"
#include "PhysicsManager.h"

class EntMapCache;
class NavGraphManager;


class Zone
{
public:

	CL_Rectf m_boundingRect;
	CL_Vector2 m_vPos; //add this to the bounding rect to get the rect in world coordinates
	int m_materialID;
	int m_entityID;

};


typedef std::map<ScreenID, MapChunk*> map_chunk_map;

typedef std::list<unsigned int> tag_hash_list;
#define C_MAP_FILE_VERSION 3

enum
{
	C_DATA_UNKNOWN = 0,
	C_DATA_MAP_CHUNK,
	C_DATA_MAP_END_OF_CHUNKS
};

class Map
{
public:

    Map();
    ~Map();

    void Kill();
    void Init(const CL_Rect worldRect = CL_Rect(C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID+1,C_DEFAULT_SCREEN_ID+1)); //you can send in a requested worldrect if you want,
	//other it calculates the middle and puts a screen there to start you off

    const CL_Rect *GetWorldRect(); //rect in screenchunks
	CL_Rect GetWorldRectInPixels(); //full world bounds in rect coordinates, based on chunks, not exact visual data
	CL_Rectf GetWorldRectExact();  //more exact, based on visuals.  Will compute if required, so might be slow to get
    int GetSizeX(){return m_mapRect.get_width();}
    int GetSizeY(){return m_mapRect.get_height();}
    int GetXFromScreenID(ScreenID screenID);
    int GetYFromScreenID(ScreenID screenID);
    MapChunk * DoesWorldChunkExist(ScreenID screenID);
    ScreenID GetScreenID(short x, short y);
	MapChunk * GetMapChunk(ScreenID screenID); //map chunk id
	MapChunk * GetMapChunk(CL_Vector2 vecWorld); //in world coordinates
    Screen * GetScreen(ScreenID screenID); //inits it if needed
    Screen * GetScreen(int x, int y); //like above but automatically converts x/y into a "screenID" for you
	Screen * GetScreen(const CL_Vector2 &vecPos); //like above but from world coords
	CL_Vector2 ScreenIDToWorldPos(ScreenID screenID);
    ScreenID GetScreenIDFromWorld(CL_Vector2 vecPos);
	CL_Vector2 SnapWorldCoords(CL_Vector2 vecWorld, CL_Vector2 vecSnap);
	const string &GetDirPath(){return m_strDirPath;};//it includes the trailing backslash
	void SetDirPath(const string &str);
	bool Load(string dirPath=""); //attempts to load, returns false if nothing was loaded. It also
									//calls SetDirPath on what is sent in
	bool Save(bool bSaveTagCacheAlso);
	map_chunk_map * GetChunkMap() {return &m_chunkMap;}
	int GetMapChunkPixelSize() {return m_worldChunkPixelSize;}
	int GetDefaultTileSizeX() {return m_defaultTileSizeX;}
	int GetDefaultTileSizeY() {return m_uintArray[e_uintDefaultTileSizeY];}
	void DeleteExistingMap();
	bool IsInitted() {return m_defaultTileSizeX != 0;}
	void SetDefaultTileSizeX(int size);
	void SetDefaultTileSizeY(int size);
	void SetMapChunkPixelSize(int widthAndHeight);
	CameraSetting * GetCameraSetting() {return &m_cameraSetting;}
	CL_Color GetBGColor() {CL_Color color; color.color = m_uintArray[e_uintBGColor]; return color;}
	void SetBGColor(CL_Color col){m_uintArray[e_uintBGColor] = col.color;}
	void SetGravity(float grav){m_floatArray[e_floatGravity] = grav; m_bDataChanged = true; m_physicsManager.SetGravity(CL_Vector2(0, grav));}
	float GetGravity() {return m_floatArray[e_floatGravity];}
	void SetThumbnailWidth(int width) {m_intArray[e_intThumbnailWidth] = width; m_bDataChanged = true;}
	void SetThumbnailHeight(int height) {m_intArray[e_intThumbnailHeight] = height; m_bDataChanged = true;}
	int GetThumbnailWidth() {return m_intArray[e_intThumbnailWidth];}
	int GetThumbnailHeight() {return m_intArray[e_intThumbnailHeight];}
	float GetCacheSensitivity() {return m_floatArray[e_floatCacheSensitivity];}
	void SetCacheSensitivity(float sensitivity) {m_floatArray[e_floatCacheSensitivity] = sensitivity; m_bDataChanged = true;}
	void InvalidateAllThumbnails();
	void AddTile(Tile *pTile); //adds a tile automatically based on its pos/layer info
	void GetAllMapChunksWithinThisRect(std::vector<MapChunk*> &wcVector, CL_Rect rec, bool bIncludeBlanks);
	void PreloadMap();
	LayerManager & GetLayerManager() {return m_layerManager;}
	bool IsValidCoordinate(CL_Vector2 vec);
	cl_uint8 GetVersion(){return m_version;}
	const string & GetName(){return m_strMapName;}
	bool GetSnapEnabled() {return m_byteArray[e_byteSnapOn] != 0;}
	void SetSnapEnabled(bool bNew) {m_byteArray[e_byteSnapOn] = bNew; m_bDataChanged = true;}
	void SetMyMapCache(EntMapCache *pWorldCache);
	bool GetPersistent() {return m_byteArray[e_byteNotPersistent] == 0;}
	void SetPersistent(bool bNew) {m_byteArray[e_byteNotPersistent] = !bNew; m_bDataChanged = true;}
	bool GetAutoSave() {return m_byteArray[e_byteAutoSave] == 0;}
	void SetAutoSave(bool bNew) {m_byteArray[e_byteAutoSave] = !bNew; m_bDataChanged = true;}
	bool SaveRequested();
	void ForceSaveNow();
	void RemoveUnusedFileChunks();

	EntMapCache * GetMyMapCache();
	void ReInitEntities(); //reinits all cached entities in this world, useful after 
	//changing a script
	void ReInitCollisionOnTilePics(); 
	NavGraphManager * GetNavGraph();
	bool NavGraphDataExists() {return m_pNavGraphManager != 0;}

	void AddWarpTagHashID(unsigned int hashID);
	void RemoveWarpTagHashID(unsigned int hashID);
	tag_hash_list & GetWarpTagHashList() {return m_warpTagHashIDList;}
	void SaveAndKill();
	bool IsWorldCacheInitted() {return m_pWorldCache != NULL;}
	void SetDataChanged(bool bNew);
	void BuildNavGraph();
	bool GetModified(); //this checks the world and all screesn to see if anything needs to be saved
	void SetModified(bool bModified);
	bool IsKillingMapNow() { return m_bKillingMap;}
	CL_Rectf ComputeWorldRect(int reserved); //very slow!
	void SetWorldRectExact(CL_Rectf r);
	PhysicsManager * GetPhysicsManager() {return &m_physicsManager;}
	Zone GetZoneByRectAndType(const CL_Rectf rectInput, int matType);
	Zone GetZoneByPointAndType(const CL_Vector2 &vPos, int matType);
	DataManager * GetDB() {return &m_db;} //generic user db

private:

	bool TestCoordPacker(int x, int y);
	void MarkAllMapPiecesAsNeedingToSave();
	//you can add new variables by adding enums, the datafile will stay compatible automatically
	enum
	{
		e_intThumbnailWidth = 0,
		e_intThumbnailHeight,
	
		//add more above here
		e_intCount
	};
	enum
	{
		e_byteSnapOn = 0,
		e_byteNotPersistent,
		e_byteAutoSave,
		
		//add more above here
		e_byteCount
	};

	enum
	{
		e_floatCacheSensitivity = 0,
		e_floatViewSensitivity,
		e_floatGravity,

		//add more above here
		e_floatCount
	};

	enum
	{
		e_uintBGColor = 0, //what color the background clear should be
		e_uintDefaultTileSizeY,
		//add more above here
		e_uintCount
	};

	cl_uint8 m_byteArray[e_byteCount];
	cl_int32 m_intArray[e_intCount];
	cl_uint32 m_uintArray[e_uintCount];
	float m_floatArray[e_floatCount];

	//stuff we actually save/load, don't change the save/load order, ok? Just add more
	//at the end.
	cl_uint8 m_version;
	CL_Rect m_mapRect;
	int m_worldChunkPixelSize;
	int m_defaultTileSizeX; //default is 64
	//stuff we don't
	map_chunk_map m_chunkMap;

	string m_strDirPath; //where we're located, not saved, set upon loading, I mean, we'd know if we know where to load it, right?
	string m_strMapName; //we get this by cutting off the last part of the dir name
	CameraSetting m_cameraSetting; //to remember out last camera position
	LayerManager m_layerManager;
	EntMapCache *m_pWorldCache; //cached here for speed

	bool m_bDataChanged; //only applicable to what is in this file
	NavGraphManager *m_pNavGraphManager;
	int m_masterNavMapID; //a central node that connects to all warps on this map
	tag_hash_list m_warpTagHashIDList; //keep track of the hash's of named tagobjects WARPS that exist and belong to this map (note, only warps!)
	bool m_bKillingMap;
	CL_Rectf m_worldBounds; //the exact size of the map
	bool m_bNeedToComputeBounds;
	PhysicsManager m_physicsManager;
	DataManager m_db; //generic database per map for user data

};

void RemoveWorldFiles(const string &path); //util for deleting stuff



//natural docs stuff
/*
Object: Map
A map is a single area that can be any size and contain any amount of tiles, including <Entities> and <TilePic>'s.

Most of these map settings are also available in the editor under Map Properties.

If the map is saved (by defaults maps auto-save and are persistent), changes made are remembered.

A game may have an unlimited number of maps.

Each map contains its own <LayerManager> which can be accessed to see what kind of layers it has.

See also:

<MapManager>

<LayerManager>

Group: Member Functions

func: SetPersistent
(code)
nil SetPersistent(boolean bPersistent)
(end)
By default, maps are persistent, meaning all changes are remembered and automatically saved in the player's profile. (the original is not changed, only the player's modified version of it)

This only has meaning when a player profile is active.

Parameters:

bPersistant - True if the map should be persistent and changes remembered, false if not

func: GetPersistent
(code)
boolean GetPersistent()
(end)

Returns:

True if the map is persistent and changes will be remembered on a per-player-profile basis.

func: SetAutoSave
(code)
nil SetAutoSave(boolean bAutoSave)
(end)

This only applies when a user profile is not yet loaded.

For title screens, auto-save is usually turned off because no player profile is loaded, yet we don't want to save after we mess up the screen.

Parameters:

bAutoSave - True if the map should automatically be saved without needing to choose "Save map now" from the editor.

func: GetAutoSave
(code)
boolean GetAutoSave()
(end)

Returns:

True if auto-save is currently enabled.

func: GetName
(code)
string GetName()
(end)

Returns:

The map name.  (the name of its directory)


func: GetWorldRect
(code)
Rectf GetWorldRect()
(end)

Returns:

The exact size of the map in world coordinates, based on visual data.  Will be computed the first time it is needed.
If chunks of the map are erased, you must call <ComputeWorldRect> yourself if you want this to be updated.

Returns:

a <Rectf> containing the location of the used map.

func: SetWorldRect
(code)
nil SetWorldRect(Rectf viewArea)
(end)

Allows you to manually set the map view rect, allowing more control when the camera is setup to respect these boundries.

Parameters:

viewArea - a <Rectf> object containing the boundries of this map in world coordinates.

func: ComputeWorldRect
(code)
Rect ComputeWorldRect(number reserved)
(end)

Computes the actual size of the entire map in world coordinates by looking at the size of each entity/tile.  Slow.  <GetWorldRect> will now return this cached data.

Parameters:

reserved - Reserved for future features, you must always pass it 0.

Returns:

a <Rect> containing the location of the used map.


func: GetLayerManager
(code)
LayerManager GetLayerManager()
(end)

Returns:

This map's <LayerManager>, containing all information about its layers and layer settings.

func: BuildLocalNavGraph
(code)
nil BuildLocalNavGraph()
(end)

Rebuilds the navigational graph for this map.

In general, this isn't needed, as navigational graphs are grown/destroyed fluidly on the fly as the map changes.

func: GetCollisionByRay
(code)
Zone GetCollisionByRay(Vector2 vStartPos, Vector2 vDir, number rayRange, number raySpread,
		Entity entToIgnore, number mode, boolean bIgnoreCreatures)
(end)

Allows you to shoot a ray from any point in this map and see what it hits.

The returned <Zone> object's *vPos* member will contain the exact position of the hit, its other values will contain additional information about the collision.

Usage:
(code)
//let's shoot a ray in front of this entity and see if it detects any other entities)

local rayRange = 80;
local raySpread = 8; //causes 5 rays to be shot in a spread formation 8 units apart, easier to detect hits.  0 to use
					 //1 ray only
local entToIgnore = this;
local hitZone = this:GetMap():GetCollisionByRay(this:GetPos(), this:GetVectorFacing(), rayRange, raySpread,
				entToIgnore, C_RAY_ENTITIES, false);

if (hitZone.entityID != C_ENTITY_NONE) then
//we have an entity in front of us!

local ent = GetEntityByID(hitZone.entityID);
LogMsg("Entity " .. tostring(ent) .. " is sitting in front of us.");

end

(end)

Parameters:

vStartPos - A <Vector2> object containing the start position of the ray.
vDir - A unit vector containing the direction of the ray.
rayRange - How far the ray can reach.
raySpread - 0 for a single ray, otherwise will shoot five rays in a spread formation, this distance apart from each one.
entToIgnore - An <Entity> we should ignore during the check, otherwise nil
mode - One of the <C_RAY_CONSTANTS>.  Use <C_RAY_DEBUG> to visually see the rays being shot.
bIgnoreCreature - If true, creatures are ignored during the check.

Returns:

A <Zone> object containing information on what was hit.  The Zone's materialID will be <C_MATERIAL_TYPE_NONE> if no collision occured.


func: GetTilesByRect
(code)
TileList GetTilesByRect(Rect rect, LayerList layers, boolean bWithCollisionOnly)
(end)
This allows you to grab a list of all the entities and tiles in a rectangular area of the map.

You can later cycle through the list examining each one, or move or copy the list to a new place. (well, the moving/pasting as a group isn't accessible yet in script, coming soon?)

Usage:
(code)

//grab all tiles/entities we're touching

local layerList = this:GetMap():GetLayerManager():GetVisibleLayers(); //what layers we'll scan
local tileList = this:GetMap():GetTilesByRect( Rect(this:GetWorldCollisionRect()), layerList, false); //grab them

LogMsg("Found " .. tileList:GetCount() .. " tiles after scanning the " .. layerList:GetCount() .. " layers.");

//ok, now we have our list and need to run through and look at each one

local tile; 

while true do

	tile = tileList:GetNext();
	if (tile == nil) then break; end;

	LogMsg("Found tile type " .. tile:GetType());

	if (tile:GetType() == C_TILE_TYPE_ENTITY and tile:GetAsEntity():GetID() != this:GetID()) then
		LogMsg("We are standing near an entity that isn't us! Its ID is " ..  tile:GetAsEntity():GetID());
	end
end
(end)

Parameters:

rect - A <Rect> object containing the area we should grab tiles from.  Any tile/entity that overlays this area will be included.
layers - A <LayerList> object containing which layers should be included for the search.
bWidthCollisionOnly - If true, entities/tiles without collision information will be ignored.

Returns:

A <TileList> object containing handles to all the tiles/entities found. If <TileList::GetCount> is 0, nothing was found.

Section: Related Constants

Group: C_RAY_CONSTANTS
Used with <Map::GetCollisionByRay>.

constant: C_RAY_ENTITIES
The ray will hit only entities.

constant: C_RAY_TILE_PIC
The ray will hit only tile pics.

constant: C_RAY_EVERYTHING
The ray will hit entities as well as tile pics.

constant: C_RAY_DEBUG
The rays will be visually drawn on the screen, helps to figure out problems.  Slow, so don't leave this on. Hits everything.

*/


