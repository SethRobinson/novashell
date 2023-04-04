//  ***************************************************************
//  EntCreationUtils - Creation date: 09/15/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntCreationUtils_h__
#define EntCreationUtils_h__

class MovingEntity;
class Tile;
class Map;
class TileEditOperation;

#include "Screen.h"

MovingEntity * CreateEntity(Map *pMap, CL_Vector2 vecPos, string scriptFileName);
BaseGameEntity *  CreateEntitySpecial(const string &EntName, const string &parms);
void AddShadowToParam1(CL_Surface_DrawParams1 &params1, Tile *pTile);

void InitEntity(MovingEntity *pEntity);
void ReInitTileList(tile_list &t);
void ReInitEntity(MovingEntity *pEnt);
void GetPointersToSimilarTilesOnMap(Map *pMap, tile_list &t, Tile *pTile);
#endif // EntCreationUtils_h__
