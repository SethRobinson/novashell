#include "AppPrecomp.h"
#include "WorldNavManager.h"
#include "TagManager.h"
#include "MapManager.h"
#include "GameLogic.h"
#include "TimeSlicedGraphAlgorithms.h"
#include "GraphAlgorithms.h"
#include "NavGraphManager.h"
#include "PathPlanner.h"


WorldNavManager g_worldNavManager;
WorldNavManager::WorldNavManager()
{
	m_pNavGraph = NULL;
	
}

WorldNavManager::~WorldNavManager()
{
	Kill();
}

void WorldNavManager::Kill()
{
	SAFE_DELETE(m_pNavGraph);
}

void WorldNavManager::Init()
{
	Kill();
	m_pNavGraph = new NavGraph(true);
}

void WorldNavManager::AddNode(TagObject *pTag)
{
	assert(pTag->m_graphNodeID == invalid_node_index);
	pTag->m_graphNodeID = m_pNavGraph->GetNextFreeNodeIndex();
	m_pNavGraph->AddNode(GraphNode(pTag->m_graphNodeID, pTag->m_hashID));
	pTag->m_pWorld->AddWarpTagHashID(pTag->m_hashID);
}

void WorldNavManager::RemoveNode(TagObject *pTag)
{
	assert(pTag->m_graphNodeID != invalid_node_index);
	m_pNavGraph->RemoveNode(pTag->m_graphNodeID);
	pTag->m_pWorld->RemoveWarpTagHashID(pTag->m_hashID);
	pTag->m_graphNodeID = invalid_node_index;
}

bool WorldNavManager::DoNodesConnect(Map *pMap, int a, int b)
{
	//well, there must be some awesome shortcut to figure this out where you don't care about the path, but for now:
	return pMap->GetNavGraph()->DoNodesConnect(ConvertWorldNodeToMapNode(a), ConvertWorldNodeToMapNode(b));
}

void WorldNavManager::LinkTwoNodes(TagObject *pTagSrc, TagObject *pTag)
{
	//OPTIMIZE:  Use sq version?
	//LogMsg("Nodes connect!");
	int cost = Vec2DDistance(pTag->GetPos(), pTagSrc->GetPos());
	
	if (pTag->m_graphNodeID == -1)
	{
		LogError("Uh oh, %s doesn't have a graphNodeID yet.", pTag->m_tagName.c_str());
		return;
	}

	//we're on  the same map as another node, we want them to know it's possible to walk
	//between them.  

	m_pNavGraph->AddEdge(NavGraph::EdgeType(pTagSrc->m_graphNodeID, pTag->m_graphNodeID, cost));
	//m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_graphNodeID, pTagSrc->m_graphNodeID, cost));


}

void WorldNavManager::LinkToConnectedWarpsOnSameMap(TagObject *pTagSrc)
{
	tag_hash_list::iterator itor;
	TagObject *pTag;

	if (!pTagSrc->m_pWorld->IsInitted())
	{
		g_pMapManager->LoadMap(pTagSrc->m_pWorld->GetDirPath(), false);
		pTagSrc->m_pWorld->PreloadMap();
	}
	for (itor = pTagSrc->m_pWorld->GetWarpTagHashList().begin(); itor != pTagSrc->m_pWorld->GetWarpTagHashList().end(); itor++)
	{
		if (*itor == pTagSrc->m_hashID) continue; //don't want to process ourself

		pTag = g_TagManager.GetFromHash(*itor);
		if (pTag)
		{
			//do we actually touch this?
			if (DoNodesConnect(pTagSrc->m_pWorld, pTagSrc->m_graphNodeID, pTag->m_graphNodeID))
			{
				LinkTwoNodes(pTagSrc, pTag);
				LinkTwoNodes(pTag, pTagSrc);
			} else
			{
				//LogMsg("Nodes don't connect");
			}
			
		} else
		{
			LogError("MapNav Hash doesn't exist?!");
		}
	}

}

void WorldNavManager::LinkNode(TagObject *pTag)
{
	
	TagObject *pTarget;
	const int cost = 100;

	//one way link FROM its maps hub to itself
	//m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_pWorld->GetMasterNavMapID(), pTag->m_graphNodeID, cost));

	//also we need to link to its real target
	if (pTag->m_warpTarget != "none")
	{

	
	pTarget = g_TagManager.GetFromString(pTag->m_warpTarget);
	if (pTarget)
	{
		if (pTarget->m_graphNodeID == invalid_node_index)
		{
			LogError("Warp %s cannot warp to %s because it's not also a warp.  (it needs GetTagManager:RegisterAsWarp in its script)",
				pTag->m_tagName.c_str(), pTarget->m_tagName.c_str());
		} else
		{
			m_pNavGraph->AddEdge(NavGraph::EdgeType(pTag->m_graphNodeID, pTarget->m_graphNodeID, cost));

			//the target may want to link back, let's figure out its entity

			if (pTarget->m_warpTarget != "none")
			{
				//let him warp back to us, usually overkill because it's already one (the call will do nothing in that case)
				//but sometimes it's needed to fix an error
				m_pNavGraph->AddEdge(NavGraph::EdgeType(pTarget->m_graphNodeID, pTag->m_graphNodeID,cost));
			}
		}
	} else
	{
		LogError("Warp %s Couldn't find warp target %s. (map might not be loaded yet, load it, then warp info will get cached)", pTag->m_tagName.c_str(), pTag->m_warpTarget.c_str());
		//return;
	}


	}
	//and a two way link to any other warp nodes it connects with

	 LinkToConnectedWarpsOnSameMap(pTag);
	
}

void WorldNavManager::LinkMap(Map *pMap)
{
	
	//scan through and link all nodes to its master node, and also the node it warps to
	tag_hash_list::iterator itor;
	TagObject *pTag;

	for (itor = pMap->GetWarpTagHashList().begin(); itor != pMap->GetWarpTagHashList().end(); itor++)
	{
		pTag = g_TagManager.GetFromHash(*itor);
		if (pTag) //empty warp target means it's on the receiving end of a one way warp
		{
			LinkNode(pTag);//link to the map hub
		} 
	}
}

void WorldNavManager::LinkEverything()
{
	//go through each map and link them all

	map_info_list * pList = GetGameLogic()->GetMyWorldManager()->GetMapInfoList();
	map_info_list::iterator itor = pList->begin();

	while(itor != pList->end())
	{
		LinkMap(&(*itor)->m_world);
		//LogMsg("Linking navmap for %s. (its hub is node %d)", (*itor)->m_world.GetName().c_str(), (*itor)->m_world.GetMasterNavMapID());
		itor++;
	}

}

//certain kinds of map nodes like warps have extra data so we can figure out what their node ID is on the map nav graph (the
//map nav graph is the thing that helps us figure out where doors lead to quickly)

int WorldNavManager::ConvertMapNodeToWorldNode(Map *pMap, int mapNode)
{

	NavGraphNodeExtra &n = pMap->GetNavGraph()->GetGraph().GetNode( mapNode);
	MovingEntity *pNodeOwnerEnt = (MovingEntity*)EntityMgr->GetEntityFromID(n.GetEntID());
	assert(pNodeOwnerEnt && "didn't you know this should always exist?  Maybe the original node wasn't a named entity");
	TagObject *pTag = g_TagManager.GetFromHash(pNodeOwnerEnt->GetNameHash());
	if (!pTag)
	{
			LogError("ConvertMapNodeToWorldNode: %s missing tagcache data.  It should be a named entity", pNodeOwnerEnt->GetName().c_str());
		return invalid_node_index;
	}

	assert(pTag->m_graphNodeID != invalid_node_index && "Huh?");
	return pTag->m_graphNodeID;

};

int WorldNavManager::ConvertWorldNodeToMapNode(int nodeID)
{
	MovingEntity *pEnt = ConvertWorldNodeToOwnerEntity(nodeID, false);
	if (pEnt)
	{
		return pEnt->GetTile()->GetGraphNodeID();
	} else
	{
		LogError("ConvertWorldNodeToMapNode failed.  NodeID %d doesn't have a mapID equivalent.", nodeID);
	}

	return invalid_node_index;
}

MovingEntity * WorldNavManager::ConvertWorldNodeToOwnerEntity(int nodeID, bool bLoadWorldIfRequired)
{
	GraphNode &n =m_pNavGraph->GetNode(nodeID);

	TagObject *pTag = g_TagManager.GetFromHash(n.GetTagHashID());
	if (!pTag)
	{
		return NULL;
	} 

	if (bLoadWorldIfRequired)
	{
		if (!pTag->m_pWorld->IsInitted())
		{
			//uh oh, we're probably going to need this
			g_pMapManager->LoadMap(pTag->m_pWorld->GetDirPath(), false);
			pTag->m_pWorld->PreloadMap();
		}
	}

	return (MovingEntity*)EntityMgr->GetEntityFromID(pTag->m_entID);
}

void WorldNavManager::DumpStatistics()
{
	LogMsg("** World Nav Graph Info **");

	if (m_pNavGraph->isEmpty())
	{
		LogMsg("It's empty right now.")	;
		return;
	}

	NavGraph::ConstNodeIterator NodeItr(*m_pNavGraph);
	for (const NavGraph::NodeType* pN=NodeItr.begin();
		!NodeItr.end();
		pN=NodeItr.next())
	{

		TagObject *pTag = g_TagManager.GetFromHash(pN->GetTagHashID());

		string extra = "Bad Tag Hash";
		if (pTag)
		{
			extra =  pTag->m_tagName + " (" + CL_String::from_int(pTag->m_entID)+ ") ";
			if (pTag->m_graphNodeID != pN->Index())
			{
				extra += "ID Mismatch!";
			} else
			{
			
				MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(pTag->m_entID);
				if (pEnt)
				{
					if (pEnt->GetNameHash() == pTag->m_hashID)
					{
						extra += "Match OK";

					} else
					{
						extra += "Ent " + CL_String::from_int(pEnt->ID())+"'s hash wrong: " + CL_String::from_int(pEnt->GetNameHash());
					}
				} else
				{
					if ( pTag->m_entID > 0)
					{
						extra = "Ent ID " + CL_String::from_int(pTag->m_entID)+ " doesn't exist?";

					} else
					{
						extra = "Ent ID " + CL_String::from_int(pTag->m_entID)+ " not active yet.";

					}
				}
			}
		}
		LogMsg("Node %d - Hash: %u - %s", pN->Index(), pN->GetTagHashID(), extra.c_str());

		string temp = "    Connections: ";

		NavGraph::ConstEdgeIterator EdgeItr(*m_pNavGraph, pN->Index());
		for (const NavGraph::EdgeType* pE=EdgeItr.begin();
			pE && !EdgeItr.end();
			pE=EdgeItr.next())
		{
			temp += CL_String::from_int(m_pNavGraph->GetNode(pE->To()).Index()) + "  ";
		}
		LogMsg(temp.c_str());
	}

}

void WorldNavManager::StripUnrequiredNodesFromPath(MacroPathInfo &m)
{
	
	//first the starting ones..
	
	if (m.m_path.size() == 1) 
	{
		//it looks like the path is so simple we didn't need the world manager at all...
		return;
	}
	MovingEntity *pNodeEntA, *pNodeEntB;
	
	while (1)
	{
		if (m.m_path.size() < 2)
		{
			//assert(!"Huh?!");
			//LogMsg("WorldNav node stripping error?");		
			return;
			
		};
		//get first node
		pNodeEntA = ConvertWorldNodeToOwnerEntity(m.m_path.front(), false);
		pNodeEntB = ConvertWorldNodeToOwnerEntity( *(++m.m_path.begin()), false );

		if (!pNodeEntA || !pNodeEntB)
		{
			//well, the map isn't even loaded so let's assume we can't optimize the path right now, afterall, it's more for looks
			break;
		}
		if (pNodeEntA->GetMap() == pNodeEntB->GetMap())
		{
			//they are on the same map, this MIGHT be ok, but only if the nodes are not connected.
			//(NPC's won't use warps/doors if the path is directly walkable)
			
			//do these actually connect?  If so, screw 'em
			if (pNodeEntA->GetMap()->GetNavGraph()->DoNodesConnect(pNodeEntA->GetTile()->GetGraphNodeID(),
				pNodeEntB->GetTile()->GetGraphNodeID()))
			{
				//they are actually linked making this one unneeded
				m.m_path.pop_front();
			} else
			{
				//they don't connect, it's a warp.  carry on
				break;
			}
		} else
		{
			//two nodes on different maps? yeah, we're done scanning here
			break;
		}
	}
	
	//same thing for the other side of the list, the destination

	while (1)
	{
		assert(m.m_path.size() > 1 && "Huh?!");
		//get first node
		pNodeEntA = ConvertWorldNodeToOwnerEntity(m.m_path.back(), false);
		pNodeEntB = ConvertWorldNodeToOwnerEntity( *( (-- (--m.m_path.end()) )), false);

		if (!pNodeEntA || !pNodeEntB)
		{
			//well, the map isn't even loaded so let's assume we can't optimize the path right now, afterall, it's more for looks
			break;
		}

		if (pNodeEntA->GetMap() == pNodeEntB->GetMap())
		{
			//they are on the same map, this MIGHT be ok, but only if the nodes are not connected.
			//(NPC's won't use warps/doors if the path is directly walkable)

			//do these actually connect?  If so, screw 'em
			if (pNodeEntA->GetMap()->GetNavGraph()->DoNodesConnect(pNodeEntA->GetTile()->GetGraphNodeID(),
				pNodeEntB->GetTile()->GetGraphNodeID()))
			{
				//they are actually linked making this one unneeded
				m.m_path.pop_back();
			} else
			{
				//they don't connect, it's a warp.  carry on
				break;
			}
		} else
		{
			//two nodes on different maps? yeah, we're done scanning here
			break;
		}
	}



}

MacroPathInfo WorldNavManager::FindPathToMapAndPos(MovingEntity *pEnt, Map *pDestMap, CL_Vector2 vDest)
{
	//first we need to locate a door, any door, it doesn't have to  be the closest, it just has to be reachable
	NavGraphManager::eStatus status;
	int startWarpID = pEnt->GetMap()->GetNavGraph()->GetClosestSpecialNode(pEnt, pEnt->GetMap(), pEnt->GetPos(), C_NODE_TYPE_WARP, &status);

	MacroPathInfo m;

	if (status == NavGraphManager::NO_NODE_CLOSE)
	{
		m.m_status = MacroPathInfo::NO_NODE_CLOSE;
		return m;
	}

	if (startWarpID == invalid_node_index)
	{ 
		LogMsg("Can't reach door on starting map");
		return m;
	}


	//now we need to find a door on the goal map that definitely can connect to

	int destWarpID = pDestMap->GetNavGraph()->GetClosestSpecialNode(pEnt,pDestMap,  vDest, C_NODE_TYPE_WARP, &status);

	if (status == NavGraphManager::NO_NODE_CLOSE)
	{
		m.m_status = MacroPathInfo::NO_NODE_CLOSE;
		return m;
	}

	if (destWarpID == invalid_node_index)
	{
		LogMsg("Can't reach warp on target map.");
		return m;
	}

	//the unique thing about nodes that are warps is we can get their worldnav node id from their mapnav id.  The worldnav is
	//a sparse graph that connects all the doors in the world
	
	int worldNavNodeStart = ConvertMapNodeToWorldNode(pEnt->GetMap(), startWarpID);

	if (worldNavNodeStart == invalid_node_index)
	{
		LogError("Unable to locate starting warp %d's path node.", startWarpID);
		return m;
	}
	int worldNavNodeEnd = ConvertMapNodeToWorldNode(pDestMap, destWarpID);

	if (worldNavNodeEnd == invalid_node_index)
	{
		LogError("Unable to locate destination warp %d's path node.", worldNavNodeEnd);
		return m;
	}

	//LogMsg("Start worldNavID: %d  End: %d",worldNavNodeStart, worldNavNodeEnd);
	//first we need to locate a doable pathway

	typedef Graph_SearchDijkstras_TS<NavGraph, FindNodeIndex> DijSearch;
	Graph_SearchTimeSliced<NavGraph::EdgeType>*  pCurrentSearch =  new DijSearch(*m_pNavGraph,
		worldNavNodeStart, worldNavNodeEnd);

	int result;
	do {result = pCurrentSearch->CycleOnce();} while(result == search_incomplete);

	if (result == target_not_found)
	{
		return m; //they can query this object to see it failed
	}
	
	//we've located a way through that we know will work.

	m.m_path = pCurrentSearch->GetPathToTarget();

	//before we send it back, we need to clean up the entry and exit part of the path, it may have
	//extra nodes due to the trick we use to find a valid door to start/dest spot on both ends.
	//(the paths between doors are precomputed and linked in the graph, but extra work is done to
	//find a valid door that connects to the start and another one that can reach the dest pos.
	
	StripUnrequiredNodesFromPath(m);

	return m;
}


//load and save navigational connection data.  The tagmanager will have already loaded the named
//tags and set them up as nodes, the time consuming thing we want to avoid by saving and loading
//this data is figuring out which warps can connect to which.

void WorldNavManager::Serialize(CL_FileHelper &helper)
{
	
	//write/read the total amount we're going to save/load

	unsigned int chunkType;

	if (helper.IsWriting())
	{

		//write out data for each node
		NavGraph::ConstNodeIterator NodeItr(*m_pNavGraph);
		for (const NavGraph::NodeType* pN=NodeItr.begin();
			!NodeItr.end();
			pN=NodeItr.next())
		{

			helper.process(chunkType = CHUNK_NODE);
			//write out the hashID it's connected with (this is built from the warps tag name)
			helper.process_const(pN->GetTagHashID());
		
			NavGraph::ConstEdgeIterator EdgeItr(*m_pNavGraph, pN->Index());
			for (const NavGraph::EdgeType* pE=EdgeItr.begin();
				pE && !EdgeItr.end();
				pE=EdgeItr.next())
			{
				//write out all the links this node is connected to, by hashID only
				helper.process(chunkType = CHUNK_LINK);
				helper.process_const( m_pNavGraph->GetNode(pE->To()).GetTagHashID()) ;
			}
		}

		//signal EOF
		helper.process(chunkType = CHUNK_DONE);
	
	} else
	{

		unsigned int curNodeHash;
		unsigned int nodeToLink;

		TagObject *pTagSrc, *pTagLink;

		while (1)
		{
			helper.process(chunkType);

			switch(chunkType)
			{

				case CHUNK_NODE:
					helper.process(curNodeHash);
					pTagSrc = g_TagManager.GetFromHash(curNodeHash);
					if (!pTagSrc)
					{
						LogError("WorldNavCache Loader: Unable to find tag %d.  Rebuild maybe?", curNodeHash);
					}
					break;

				case CHUNK_LINK:
					helper.process(nodeToLink);

					pTagLink = g_TagManager.GetFromHash(nodeToLink);
					if (!pTagLink)
					{
						LogError("WorldNavCache Loader: Unable to find tag %d to link to.  Rebuild maybe?", nodeToLink);
					} else
					{
						if (pTagSrc)
						{
							LinkTwoNodes(pTagSrc, pTagLink);
						}
					}
					break;

				case CHUNK_DONE:
					return;

				default:

					LogError("%s appears to be corrected, %d chunk unknown.", C_WORLD_NAV_FILENAME, chunkType);
					return;
			}

		}

	}
}


void WorldNavManager::Save()
{
	CL_OutputSource *pFile =g_VFManager.PutFile(C_WORLD_NAV_FILENAME);
	
	if (pFile)
	{
		CL_FileHelper helper(pFile); 
		Serialize(helper);
		SAFE_DELETE(pFile);
	}
}


void WorldNavManager::Load()
{
	CL_InputSource *pFile = g_VFManager.GetFile(C_WORLD_NAV_FILENAME);

	if (!pFile)
	{
		return;
	}

	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	Serialize(helper);
	SAFE_DELETE(pFile);
}
