
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 29:12:2005   10:21
*/

#ifndef EntWorldCache_HEADER_INCLUDED // include guard
#define EntWorldCache_HEADER_INCLUDED  // include guard

#include "BaseGameEntity.h"
#include "Map.h"
#include "Screen.h"
#include "TileEditOperation.h"

typedef std::vector<MapChunk*> EntWorldChunkVector;
typedef std::vector<Tile*> tile_vector;
typedef std::vector<MovingEntity*> moving_entity_vector;

	
class EntMapCache: public BaseGameEntity
{
public:

    EntMapCache();
    virtual ~EntMapCache();

	virtual void Render(void *pTarget);
	virtual void Update(float step);

	void ClearCache();
	CL_Vector2 WorldToScreen(float x, float y); //returns world coordinates, based on where the camera is
	CL_Vector2 WorldToScreen(const CL_Vector2 &vecWorld); //returns world coordinates, based on where the camera is
	CL_Vector2 ScreenToWorld(const CL_Vector2 &vecScreen); 
	CL_Vector2 WorldToModifiedWorld(const CL_Vector2 &vecWorld, const CL_Vector2 &vecMod);
	CL_Vector2 ModifiedWorldToWorld(const CL_Vector2 &vecWorld, const CL_Vector2 &vecMod);
	

	bool GenerateThumbnail(ScreenID screenID);
	
	void SetDrawWorldChunkGrid(bool bNew) {m_bDrawWorldChunkGrid = bNew;}
	bool GetDrawWorldChunkGrid() {return m_bDrawWorldChunkGrid;}

	void SetDrawCollision(bool bNew);
	bool GetDrawCollision() {return m_bDrawCollisionData;}
	void SetWorld(Map *pWorld);
	void AddTilesByRect(const CL_Rect &recArea, tile_list *pTileList, const vector<unsigned int> &layerIDVect, bool bWithCollisionOnly = false, bool bAllowLoadOnDemand = true, bool bEntitiesOnly = false);
	void CalculateVisibleList(const CL_Rect &recScreen, bool bMakingThumbnail);
	void RenderViewList(CL_GraphicContext *pGC);
	unsigned int GetUniqueDrawID() { return m_uniqueDrawID;}
	int GetTilesRenderedLastFrameCount() {return m_tileLayerDrawList.size();}
	void RenderCollisionOutlines(CL_GraphicContext *pGC);
	void AddActiveTrigger(int entID);
	void ClearTriggers();
	//note, GCC didn't like me sending these as references
	bool IsPathObstructed(CL_Vector2 a, CL_Vector2 b, float radius, Tile *pTileToIgnore= NULL, bool bIgnoreMovingCreatures = false);
	bool IsAreaObstructed(CL_Vector2 pos, float radius, bool bIgnoreMovingCreatures, MovingEntity *pIgnoreEntity); //good for figuring out if an entity can fit here or not
	void PushUndoOperation(const operation_deque &op);
	void PopUndoOperation();
	int GetUndoOpsAvailableCount() {return m_undoDeque.size();};
	
protected:

	void CullScreensNotUsedRecently(unsigned int timeRequiredToKeep);
	void RemoveScreenFromCache(ScreenID screenID);
	void ProcessPendingEntityMovementAndDeletions();
	void ResetPendingEntityMovementAndDeletions();
	void AddSectionToDraw(unsigned int renderID, CL_Rect &viewRect, vector<unsigned int>  & layerIDVec, bool bDontAddToDrawList );
	void OnMapChange();
	void RenderGoalAI(CL_GraphicContext *pGC);
	void RenderGrid(CL_GraphicContext *pTarget);
	Map *m_pWorld; 
	
	//these are rebuilt every frame
	EntWorldChunkVector m_worldChunkVector; //what screens are currently viewable
	tile_vector m_tileLayerDrawList; //what tiles are currently viewable
	moving_entity_vector m_entityList; //for checking for movement/deletions, special things
	unsigned int m_cacheCheckTimer;
	bool m_bDrawWorldChunkGrid;
	bool m_bDrawCollisionData;
	unsigned int m_uniqueDrawID; //so an entity can tell if another entity has already "thought"
	vector<unsigned int> m_activeTriggers; //well, the entities that hold them anyway
	CL_SlotContainer m_slots;


	deque<operation_deque> m_undoDeque; //a deque that contains deques of undos, think of one deque as one undo operation


};

bool compareTileByLayer(const Tile *pA, const Tile *pB) ;
bool compareTileBySortLevelOptimized(Tile *pA, Tile *pB);


extern LayerManager *g_pLayerManager; //used for speeding up the sort
#endif                  // include guard
