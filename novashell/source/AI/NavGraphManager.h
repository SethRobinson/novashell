//  ***************************************************************
//  NavGraphManager - Creation date: 08/30/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//Each map has its own NavGraphManager to keep track of its local pathfinding system.
//the WorldNavManager keeps track of the connection between maps.

#ifndef NavGraphManager_h__
#define NavGraphManager_h__

#include "SparseGraph.h"
#include "GraphEdgeTypes.h"
#include "GraphNodeTypes.h"

class Map;
class Tile;
class MovingEntity;

class NavGraphManager
{
public:

	typedef NavGraphNodeExtra         GraphNode;
	typedef SparseGraph<GraphNode, NavGraphEdge>      NavGraph;

	NavGraphManager(Map *pParent);
	virtual ~NavGraphManager();
	void Kill();
	void Render(bool bDrawNodeIDs, CL_GraphicContext *pGC);

	NavGraph&                          GetGraph()const{return *m_pNavGraph;}

	enum eStatus
	{
		OK,
		NO_NODE_CLOSE,
		NO_SPECIAL_NODE_FOUND
	};

	void AddTileNode(Tile *pTile);
	void RemoveTileNode(Tile *pTile);
	float GetNodeMaxLinkDistance() {return 400;}
	Map * GetParent() {return m_pWorld;}
	int GetClosestSpecialNode(MovingEntity *pEnt, Map *pMap, const CL_Vector2 pos, int nodeType,  NavGraphManager::eStatus *pStatus);
	bool DoNodesConnect(int a, int b);
	void SetPerformLinkOnAdd(bool bNew) {m_bPerformLinkOnAdd = bNew;}
	bool GetPerformLinkOnAdd(){return m_bPerformLinkOnAdd;}
	void Clear();

	
protected:
	

private:

	void AddNeighborLinks(Tile *pTile);
	void ExamineNodesForLinking(Tile *pA, Tile *pB); //see if two specific tiles should be linked, if so, it links them

	//this map's accompanying navigation graph
	NavGraph*                          m_pNavGraph;  
	Map *m_pWorld;
	bool m_bPerformLinkOnAdd; //allows us to delay linking until the whole work is loaded
	
};

#endif // NavGraphManager_h__
