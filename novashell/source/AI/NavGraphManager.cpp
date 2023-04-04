#include "AppPrecomp.h"
#include "NavGraphManager.h"
#include "AppUtils.h"
#include "GameLogic.h"
#include "GraphAlgorithms.h"
#include "AStarHeuristicPolicies.h"
#include "PathPlanner.h"

NavGraphManager::NavGraphManager(Map *pParent)
{
	m_pWorld = pParent;
	m_pNavGraph = NULL;
	
	Clear();
}

NavGraphManager::~NavGraphManager()
{
	Kill();
}
void NavGraphManager::Clear()
{
	Kill();

	m_pNavGraph = new NavGraph(false);
	m_bPerformLinkOnAdd = true;

}

void NavGraphManager::Kill()
{
	SAFE_DELETE(m_pNavGraph);
}

void NavGraphManager::ExamineNodesForLinking(Tile *pA, Tile *pB)
{
	assert(pA->GetGraphNodeID() != invalid_node_index);
	assert(pB->GetGraphNodeID() != invalid_node_index);
	
	assert(pA != pB && "Why the heck would you link to yourself?");
	//if these two things don't have a wall between them, they should be linked.

	//let's just assume they don't
	static CL_Vector2 a, b; 

	if (pA->GetType() == C_TILE_TYPE_ENTITY)
	{
		a = pA->GetPos();
		b = pB->GetPos();
	
	} else
	{
		//center it so it appears like it's coming from the center of the tilepic
		 a = pA->GetPos()+ (pA->GetBoundsSize()/2);
		 b = pB->GetPos()+ (pB->GetBoundsSize()/2);
	}

	float dist = Vec2DDistanceSq(a, b);

	if (!m_pWorld->GetMyMapCache()->IsPathObstructed(a, b, 10, NULL, true))
	{
		m_pNavGraph->AddEdge(NavGraph::EdgeType(pA->GetGraphNodeID(), pB->GetGraphNodeID(), dist));
		//link the other way too
		m_pNavGraph->AddEdge(NavGraph::EdgeType(pB->GetGraphNodeID(), pA->GetGraphNodeID(), dist));
	}

}

void NavGraphManager::AddNeighborLinks(Tile *pTile)
{

	//first get all surrounding nodes that we might want to connect to
	CL_Vector2 pos; 

	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		pos = pTile->GetPos();
	
	} else
	{
		//center it
		pos = pTile->GetPos() + (pTile->GetBoundsSize()/2);

	}


	float halfRange = GetNodeMaxLinkDistance()/2;

	CL_Rect recArea(CL_Point(pos.x-halfRange, pos.y-halfRange), CL_Size(halfRange*2, halfRange*2));

	//returns a list of tile pointers, we shouldn't free them!
	tile_list tileList;

	m_pWorld->GetMyMapCache()->AddTilesByRect(recArea, &tileList, m_pWorld->GetLayerManager().GetCollisionList(), false, false);

	GraphNode* pN;
	int nodeID;

	tile_list::iterator itor = tileList.begin();

	for (; itor != tileList.end(); itor++)
	{
		
		if (pTile == (*itor))
		{
			//don't want to link to ourself!
			continue;
		}
		nodeID = (*itor)->GetGraphNodeID();

		if (nodeID != invalid_node_index)
		{
			//LogMsg("Node %d found node %d to test against", pTile->GetGraphNodeID(), nodeID );
			pN = &m_pNavGraph->GetNode(nodeID);
			ExamineNodesForLinking(pTile, (*itor));
		}
	}

}

void NavGraphManager::AddTileNode(Tile *pTile)
{
	if (!GetPerformLinkOnAdd())
	{
		return; //we'll do it later
	}
	
	pTile->SetGraphNodeID(m_pNavGraph->GetNextFreeNodeIndex());

	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();
		//already centered
		//LogMsg("Adding ent %s (entID %d)", pEnt->GetName().c_str(),pEnt->ID() );
		m_pNavGraph->AddNode(GraphNode(pTile->GetGraphNodeID(), pTile->GetPos(), pEnt->GetNavNodeType(), pEnt->ID()) );

	} else
	{
		//center it
		m_pNavGraph->AddNode(GraphNode(pTile->GetGraphNodeID(), pTile->GetPos() + (pTile->GetBoundsSize()/2), C_NODE_TYPE_NORMAL, -1));
	}

	AddNeighborLinks(pTile);

}

void NavGraphManager::RemoveTileNode(Tile *pTile)
{
	assert(pTile->GetGraphNodeID() != invalid_node_index);
	m_pNavGraph->RemoveNode(pTile->GetGraphNodeID());
}


void NavGraphManager::Render(bool bDrawNodeIDs, CL_GraphicContext *pGC)
{
	if (!m_pNavGraph) return;
	ResetFont(GetApp()->GetFont(C_FONT_GRAY));

	//draw the nodes 
	NavGraph::ConstNodeIterator NodeItr(*m_pNavGraph);
	for (const NavGraph::NodeType* pN=NodeItr.begin();
		!NodeItr.end();
		pN=NodeItr.next())
	{
		CL_Vector2 a = g_pMapManager->GetActiveMapCache()->WorldToScreen(pN->Pos());
	
		DrawCenteredBoxWorld(pN->Pos(), 4, CL_Color::white, pGC);

		if (bDrawNodeIDs)
		{
			
			DrawWithShadow(a.x-3, a.y-17,  CL_String::from_int(pN->Index()));
			if (pN->GetEntID() != -1)
			{
				DrawWithShadow(a.x-40, a.y-29,  "Type: " + CL_String::from_int(pN->GetType())+ " Parent: "+ CL_String::from_int(pN->GetEntID()));
			}

			//gdi->TextColor(200,200,200);
			//gdi->TextAtPos((int)pN->Pos().x+5, (int)pN->Pos().y-5, ttos(pN->Index()));
		}

		NavGraph::ConstEdgeIterator EdgeItr(*m_pNavGraph, pN->Index());
		for (const NavGraph::EdgeType* pE=EdgeItr.begin();
			pE && !EdgeItr.end();
			pE=EdgeItr.next())
		{
				CL_Vector2 b = g_pMapManager->GetActiveMapCache()->WorldToScreen( m_pNavGraph->GetNode(pE->To()).Pos() );

			pGC->draw_line(a.x, a.y, b.x, b.y, CL_Color(255,255,255,100));
		}
	}

}

bool NavGraphManager::DoNodesConnect(int a, int b)
{

	if (a == invalid_node_index || b == invalid_node_index)
	{
		LogMsg("DoNodesConnect was given a bad world node.");
		return false;
	}

	assert(a != b && "They are the same node!");


	//OPTIMIZE  can't this be optimized somehow?  We don't actually care about the path taken
	typedef Graph_SearchAStar<NavGraph, Heuristic_Euclid> AStar;
	AStar search(*m_pNavGraph,a,b);
	return search.IsPathValid();
}

int NavGraphManager::GetClosestSpecialNode(MovingEntity *pEnt, Map *pMap, const CL_Vector2 pos, int nodeType,  NavGraphManager::eStatus *pStatus)
{
	int a = pEnt->GetPathPlanner()->GetClosestNodeToPosition(pMap, pos, true);
	
	if (a == invalid_node_index)
	{
		//LogMsg("Ent %d: %s in %s at %s: No pathfinding node close by, can't use pathmanager here", pEnt->ID(), pEnt->GetName().c_str(), pMap->GetName().c_str(),
		//	PrintVector(pos).c_str());
		*pStatus = NavGraphManager::NO_NODE_CLOSE;

		return invalid_node_index;
	}

	typedef Graph_SearchDijkstras_TS<NavGraphManager::NavGraph, FindSpecialNode> DijSearch;
	Graph_SearchTimeSliced<NavGraph::EdgeType>*  pCurrentSearch =  new DijSearch(pMap->GetNavGraph()->GetGraph(),
		a,nodeType);

	int result;
	do {result = pCurrentSearch->CycleOnce();} while(result == search_incomplete);

	if (result == target_not_found)
	{
		delete pCurrentSearch;
				LogMsg("Unable to locate any connected nodes of type %d", nodeType);
		return invalid_node_index; //they can query this object to see it failed
		*pStatus = NavGraphManager::NO_SPECIAL_NODE_FOUND;

	}

	int chosenNode = pCurrentSearch->GetFinalNode();
	SAFE_DELETE(pCurrentSearch);
	*pStatus = NavGraphManager::OK;

	return chosenNode;
}
