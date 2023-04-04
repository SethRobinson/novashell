
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 7:1:2006   9:40
*/


#pragma once

#include "Tile.h"


/*
A screen is a small chunk of the world containing a tilegrid.  These are stitched together
to make up the world.  It's a member of "WorldChunk" which the World data structure keeps
track of in a giant map.

A tilegrid uses a linked list for each grid of the tile so it can contain more than one object.
An example use of layering tiles would be to draw a background tile, a custom collision object, a
flower and an "on-top" overlay.  They are ordered by the sorted by the layer parm.

*/

#define C_INVALID_SCREEN INT_MAX
class Tile;
class MapChunk;

typedef std::list<Tile*> tile_list;
typedef std::vector <tile_list> layer_list;

class Screen
{
public:

    Screen(MapChunk *pParent);
    ~Screen();

	Tile * GetTileByPosition(const CL_Vector2 &vecPos, unsigned int layer); //get the tile that is exactly at this position
	//I'm giving full access to these but DON'T add/remove tiles this way!
	//I should const these but it screwed up some other access I needed.. uh..
	tile_list * GetTileList(unsigned int layer); //get tiles on this layer
	layer_list * GetLayerList() {return &m_vecLayerList;}
	bool IsEmpty(); //returns true if there is absolutely nothing in the screen
	MapChunk * GetParentMapChunk();
	bool Save();
	bool Load();
	std::string GetFileName(); //the filename of our datafile (if we've saved)
	void SetRequestIsEmptyRefreshCheck(bool bNew);
	void GetTilesByRect(const CL_Rect &scanRect, tile_list *pTileList, const std::vector<unsigned int> &layerIntVec, unsigned int scanID, bool bWithCollisionOnly = false, bool bEntitiesOnly = false);
	bool RemoveTileBySimilarType(Tile *pSrcTile);
	bool RemoveTileByPointer(Tile *pSrcTile);
	bool RemoveTileByPosition(const CL_Vector2 &vecPos, unsigned int layer);
    void AddTile(Tile *pTile);
	void DeleteAllTilesOnLayer(unsigned int layer);
	cl_uint8 GetVersion() {return m_version;}
	void GetPtrToAllTiles(tile_list *pTileList); //this adds pointers to all this screen's tiles to the list sent in
	void LinkNavGraph(); //expects the navgraph to have been recently cleared!

	
private:

  void DeleteTileData();
  void RemoveTileByItor(tile_list::iterator itor, unsigned int layer); //the itor will contain the next valid itor after the erase

  MapChunk *m_pParentMapChunk; //the worldchunk that owns us, not saved, but set when  loaded
  layer_list m_vecLayerList;

  bool m_bIsEmpty;
  bool m_bReCheckIfEmpty;
  cl_uint8 m_version;
};

bool CloneTileList(const tile_list &srcList, tile_list &destList);
void ClearTileList(tile_list &tileList);


