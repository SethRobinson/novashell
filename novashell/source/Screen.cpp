#include "AppPrecomp.h"
#include "Screen.h"
#include "MapChunk.h"
#include "Map.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "MovingEntity.h"
#include "AI/NavGraphManager.h"

Screen::Screen(MapChunk *pParent)
{
	m_pParentMapChunk = pParent;
	//LogMsg("initting screen");
	m_vecLayerList.resize(1);
	m_bReCheckIfEmpty = false;
	m_bIsEmpty = true;
	m_version = C_MAP_FILE_VERSION; //it's possible chunks might not be loaded/saved for a while even though the
	//map is, keep tracking per screen gives us flexibility with upgrades
}

void Screen::DeleteTileData()
{
	for (unsigned int i=0; i < m_vecLayerList.size(); i++)
   {
	   DeleteAllTilesOnLayer(i);
   }
}

void Screen::DeleteAllTilesOnLayer(unsigned int layer)
{
	assert(layer < m_vecLayerList.size() && "Do you want to range this or not?");
	
	while (m_vecLayerList[layer].begin() != m_vecLayerList[layer].end())
	{
		RemoveTileByItor(m_vecLayerList[layer].begin(), layer);
	}
}

Screen::~Screen()
{
	//LogMsg("Killing screen");
	//this checks map settings like persistancy and skip next save
	if (GetParentMapChunk()->GetParentMap()->SaveRequested())
	{
 		Save(); //if something changed, it will be written to disk
	}
	DeleteTileData();
}


MapChunk * Screen::GetParentMapChunk()
{
	assert(m_pParentMapChunk && "Huh?!");
	return (m_pParentMapChunk);
}


void Screen::SetRequestIsEmptyRefreshCheck(bool bNew)
{
	m_bReCheckIfEmpty = true;
}

bool Screen::IsEmpty()
{
	if (!m_bReCheckIfEmpty) return m_bIsEmpty;
	//check to see if we're empty or not
	m_bReCheckIfEmpty = false;
	
	m_bIsEmpty = true;

	for (unsigned int i=0; i < m_vecLayerList.size(); i++)
	{
		if (m_vecLayerList[i].size() > 0) 
		{
			m_bIsEmpty = false;
			break;
		}
	}

  return m_bIsEmpty;
}


Tile * Screen::GetTileByPosition(const CL_Vector2 &vecPos, unsigned int layer)
{
	if (layer >= m_vecLayerList.size()) return NULL; //we don't have this layer

	//do any of our tiles match this point?
	tile_list::iterator itor = GetTileList(layer)->begin();

	Tile *pTile;

	while (itor != m_vecLayerList[layer].end())
	{
		pTile = (*itor);
		if (pTile)
		{
			if (pTile->GetType() == C_TILE_TYPE_REFERENCE)
			{
				pTile = pTile->GetTileWereAReferenceFrom();
			}

			if ( pTile->GetPos() == vecPos)
			{
				//found it
				return pTile;
			}
		}
		itor++;
	}
	
	return NULL;
}


string Screen::GetFileName()
{
	return (GetParentMapChunk()->GetParentMap()->GetDirPath() + CL_String::from_int(GetParentMapChunk()->GetScreenID())+".chunk");
}

bool Screen::Save()
{
	//first see if saving is even applicable
	if (!GetParentMapChunk()->GetDataChanged()) return false; //nothing changed, we don't have to save

	//LogMsg("Saving screen %d", GetParentWorldChunk()->GetScreenID());

	if (IsEmpty())
	{
		//instead of saving, we should destroy ourselves..
		g_VFManager.RemoveFile(GetFileName());
	
		return false;
	}

	CL_OutputSource *pFile = g_VFManager.PutFile(GetFileName());
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	//might be helpful to remember these later
	helper.process_const(GetParentMapChunk()->GetParentMap()->GetMapChunkPixelSize());
	helper.process_const(cl_uint32(m_vecLayerList.size()));
	m_version = C_MAP_FILE_VERSION;
	helper.process(m_version);

	tile_list::iterator itor;
	tile_list *pList;
	
	for (unsigned int i=0; i < m_vecLayerList.size(); i++)
	{
		pList = &m_vecLayerList[i];
		
		helper.process_const(cl_uint32(pList->size())); //save how many pieces are in this tile list

		for (itor = pList->begin(); itor != pList->end(); ++itor)
		{
			//these exist for all tile types
			if (  (*itor)->GetBit(Tile::e_notPersistent) || (*itor)->GetType() == C_TILE_TYPE_REFERENCE)
			{
				//we don't want to save this one.. uh.. but we already said how many tiles
				//were here and we don't want to traverse the list twice to count them..
				//just save a dummy
				cl_uint8 byteTemp = C_TILE_TYPE_REFERENCE;
				helper.process_const(byteTemp);
				continue;
			}
			helper.process_const((*itor)->GetType());
			(*itor)->Serialize(helper); //let it save itself out
		}
	}
	
	SAFE_DELETE(pFile);
	GetParentMapChunk()->SetDataChanged(false);
	return true; //success
}

bool Screen::Load()
{
	
	SetRequestIsEmptyRefreshCheck(true);
	
	//where is the original?
	string realLocation = GetParentMapChunk()->GetParentMap()->GetDirPath()+"/"+C_MAP_DAT_FILENAME;

	CL_InputSource *pFile;
	
	if (g_VFManager.LocateFile(realLocation))
	{
		//let's load from here
		try
		{
			pFile = new CL_InputSource_File(CL_String::get_path(realLocation)+CL_String::from_int(GetParentMapChunk()->GetScreenID())+".chunk");
		} catch(...)
		{
			return true;
		}


	} else
	{
		//load from wherever
		pFile = g_VFManager.GetFile(GetFileName());
	}
	
	if (!pFile) return true;

	GetGameLogic()->ShowLoadingMessage();

	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving
	try
	{
		int worldChunkPixelSize;
		unsigned int layerCount;
		
		helper.process(worldChunkPixelSize);
	    helper.process(layerCount);
		m_vecLayerList.resize(layerCount);
		helper.process(m_version);

		if (worldChunkPixelSize != GetParentMapChunk()->GetParentMap()->GetMapChunkPixelSize())
		{
			SAFE_DELETE(pFile);
			throw CL_Error("Corrupted map, or the chunk size doesn't match what we're loading");
		}

		unsigned int count;
		cl_uint8 type;
		Tile *pTile = NULL;
		tile_list *pList;

		for (unsigned int i=0; i < m_vecLayerList.size(); i++)
		{
			pList = &m_vecLayerList[i];
			//assert(pList->size() == 0 && "Tilelist should be blank!");
			helper.process(count);

			while(count--)
			{
				helper.process(type);

				switch (type)
				{
				case C_TILE_TYPE_PIC:
					pTile = new TilePic();
					pTile->SetParentScreen(this); //they may need this info during init

					pTile->Serialize(helper);
					AddTile(pTile);
					break;
				case C_TILE_TYPE_ENTITY:
					pTile = new TileEntity();
					pTile->SetParentScreen(this); //they may need this info during init

					pTile->Serialize(helper);
					if (!GetParentMapChunk()->GetParentMap()->IsValidCoordinate(pTile->GetPos()))
					{
						LogMsg("Invalid coordinate in entity, erasing it");
						SAFE_DELETE(pTile);
					} else
					{
						
						AddTile(pTile);
					}
					break;

				case C_TILE_TYPE_REFERENCE:
					//LogMsg("ignoring reference");

					//ignore
					break;
				default:
					SAFE_DELETE(pFile);
					throw CL_Error("Unrecognized tile type in screen");
				}

			}
		}

	}
	catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage("Error loading tile.  Corrupted?", error.message.c_str());
		SAFE_DELETE(pFile);
		return true;
	}

	SAFE_DELETE(pFile);
	//we just did a successful load, so we can't possibly need a save right now
	GetParentMapChunk()->SetDataChanged(false);
	return true; //success
}



void Screen::GetPtrToAllTiles(tile_list *pTileList)
{
	//we use GetTileList at least once so it can create the layer if need be or error on an invalid layer
	Tile *pTile;
	tile_list::iterator itor;

	for (unsigned int i = 0; i < m_vecLayerList.size(); i++)
	{
		itor = m_vecLayerList[i].begin();

		while (itor != m_vecLayerList[i].end())
		{
			pTile = (*itor);

			if (pTile->GetType() == C_TILE_TYPE_REFERENCE)
			{
				pTile = pTile->GetTileWereAReferenceFrom();
			}
			
			pTileList->push_back(pTile);
			
			itor++;
		}
	}

}

void Screen::GetTilesByRect(const CL_Rect &scanRect, tile_list *pTileList, const vector<unsigned int> &layerIntVec, unsigned int scanID, bool bWithCollisionOnly, bool bEntitiesOnly)
{
	//we use GetTileList at least once so it can create the layer if need be or error on an invalid layer
	static Tile *pTile;
    static tile_list::iterator itor;
	
	for (unsigned int i = 0; i < layerIntVec.size(); i++)
	{
	itor = GetTileList(layerIntVec[i])->begin();
	
	while (itor != m_vecLayerList[layerIntVec[i]].end())
	{
		pTile = (*itor);
		
		if (pTile->GetType() == C_TILE_TYPE_REFERENCE)
		{
			pTile = pTile->GetTileWereAReferenceFrom();
		}
		
		if (bEntitiesOnly)
		{
			if (pTile->GetType() != C_TILE_TYPE_ENTITY)
			{
				itor++;
				continue;
			}
		}
		
		
		if (pTile->GetLastScanID() != scanID)
		{

			//first time we've scanned this

			pTile->SetLastScanID(scanID);
			//TODO optimize this rect check.  Maybe precalc its box, or circle in the tile itself
//			assert(m_pParentMapChunk->GetRect().is_overlapped(pTile->GetWorldCombinedRectInt()) && "Tile placed in bad position?");
			
			if (scanRect.is_overlapped(pTile->GetWorldCombinedRectInt()))
			{
				if (bWithCollisionOnly)
				{
					if (!pTile->GetCollisionData() || !pTile->GetCollisionData()->HasData())
					{
						itor++;
						continue;
					}
				}
				
				pTileList->push_back(pTile);
			} else
			{
				
			}
		} else
		{
			//already got this
			
		}
		itor++;
	}
	}

}
tile_list * Screen::GetTileList(unsigned int layer)
{

	if (layer >= m_vecLayerList.size())
	{
		//add a new layer
		m_vecLayerList.resize(layer+1);
	}
	return &m_vecLayerList[layer];
}


bool Screen::RemoveTileByPosition(const CL_Vector2 &vecPos, unsigned int layer)
{
	if (layer >= m_vecLayerList.size()) return false; //we don't have this layer

	//do any of our tiles match this point?
	tile_list::iterator itor = GetTileList(layer)->begin();

	while (itor != m_vecLayerList[layer].end())
	{
		if ( (*itor)->GetPos() == vecPos)
		{
			//found it
			RemoveTileByItor(itor, layer);
			return true;
		}
		itor++;
	}

	return false;
}

bool Screen::RemoveTileBySimilarType(Tile *pSrcTile)
{
	if (pSrcTile->GetLayer() >= m_vecLayerList.size()) return false; //we don't have this layer
	

	tile_list::iterator itor = m_vecLayerList[pSrcTile->GetLayer()].begin();
	Tile *pTile;

	while (itor != m_vecLayerList[pSrcTile->GetLayer()].end())
	{
		pTile = (*itor);

		if (pTile->GetType() == C_TILE_TYPE_REFERENCE)
		{
			pTile = pTile->GetTileWereAReferenceFrom();
		}

		//make add a unique ID or more ways to match later?
		if (
			pTile->GetPos() == pSrcTile->GetPos()
			&& pTile->GetType() == pSrcTile->GetType()
			)
		{
			if (pTile->GetParentScreen() == this)
			{
				RemoveTileByItor(itor, pSrcTile->GetLayer());
			} else
			{
				//we only had a reference, let the real screen delete it
				pTile->GetParentScreen()->RemoveTileBySimilarType(pTile);
			}
			
			return true;
		}
		itor++;
	}

	return false;
}


bool Screen::RemoveTileByPointer(Tile *pSrcTile)
{
	if (pSrcTile->GetLayer() >= m_vecLayerList.size()) return false; //we don't have this layer

	tile_list::iterator itor = m_vecLayerList[pSrcTile->GetLayer()].begin();
	
	while (itor != m_vecLayerList[pSrcTile->GetLayer()].end())
	{
		if ((*itor) == pSrcTile)
		{
			
			//remove it
			RemoveTileByItor(itor, pSrcTile->GetLayer());
			return true;
		}
		itor++;
	}

	LogError("Failed to remove tile by pointer on screen %d", GetParentMapChunk()->GetScreenID());
	return false;
}

void Screen::LinkNavGraph()
{
	Tile *pTile;
	tile_list::iterator itor;

	NavGraphManager *pNav = GetParentMapChunk()->GetParentMap()->GetNavGraph();
	
	for (unsigned int i = 0; i < m_vecLayerList.size(); i++)
	{
		itor = m_vecLayerList[i].begin();

		while (itor != m_vecLayerList[i].end())
		{
			pTile = (*itor);

			if (pTile->GetBit(Tile::e_pathNode))
			{
				pTile->SetGraphNodeID(invalid_node_index); //assume that we've cleared the
				//navgraph before calling this so it's all invalid anyway
				pNav->AddTileNode(pTile);
			}
			itor++;
		}
	}
}

void Screen::RemoveTileByItor(tile_list::iterator itor, unsigned int layer)
{

	GetParentMapChunk()->SetNeedsThumbnailRefresh(true);

	if ( (*itor)->GetType() != C_TILE_TYPE_REFERENCE )
	{

		GetParentMapChunk()->SetDataChanged(true);
		
		if ((*itor)->GetType() == C_TILE_TYPE_ENTITY)
		{
			g_TagManager.Remove(((TileEntity*)(*itor))->GetEntity());
		} 
		

		if ((*itor)->GetBit(Tile::e_pathNode))
		{
			GetParentMapChunk()->GetParentMap()->GetNavGraph()->RemoveTileNode((*itor));
		}

		//if (GetParentMapChunk()->GetParentMap()->IsWorldCacheInitted())
		//GetParentMapChunk()->GetParentMap()->GetMyMapCache()->RemoveTileFromList( (*itor) );
		
	} else
	{
		
		SetRequestIsEmptyRefreshCheck(true);
	}


	delete (*itor);
	itor = m_vecLayerList[layer].erase(itor);
}


void Screen::AddTile(Tile *pTile)
{
		
	pTile->SetParentScreen(this); //who knows when they'll need this data
	GetTileList(pTile->GetLayer())->push_back(pTile);

	GetParentMapChunk()->SetNeedsThumbnailRefresh(true);
	m_bIsEmpty = false; //can't be empty anymore

	if (pTile->GetType() == C_TILE_TYPE_REFERENCE) 
	{
				return; //references don't need the edge cases figured out
	}

	GetParentMapChunk()->SetDataChanged(true);

	//we may also need to index info about this sprite if it has a tag name

	

	if (pTile->GetBit(Tile::e_pathNode))
	{
		GetParentMapChunk()->GetParentMap()->GetNavGraph()->AddTileNode(pTile);
		//LogMsg("Added graphid %d", pTile->GetGraphNodeID());
	}
	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		//LogMsg("Adding tile %s. Layer %d now has %d tiles. Rect is %s", ((TileEntity*)pTile)->GetEntity()->GetName().c_str(),
		//	pTile->GetLayer(), GetTileList(pTile->GetLayer())->size(), PrintRectInt(tileRect).c_str());

		g_TagManager.Update(GetParentMapChunk()->GetParentMap(), ((TileEntity*)pTile)->GetEntity());
	}
	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		((TileEntity*)pTile)->GetEntity()->RunOnMapInsertIfNeeded();
	}
	
	//early rejection to see if this tile's bounds are totally within this, otherwise it's an edge case and takes
	//special preparation h

	//TODO do this a faster way
	static CL_Rect tileRect; 
	static CL_Rect unionRect;

	tileRect = pTile->GetWorldCombinedRectInt();

	
	unionRect = tileRect.calc_union(m_pParentMapChunk->GetRect());
	if (unionRect == tileRect)
	{
		return;
	}

	//we expand beyond the bounds of this screen.  We'll notify the other screens about this with a reference that
	//points to this one, the only real instance. When references are deleted, this tile will be updated and vice-versa.

	std::vector <MapChunk*> wcVector;
	m_pParentMapChunk->GetParentMap()->GetAllMapChunksWithinThisRect(wcVector, pTile->GetWorldCombinedRectInt(), true);


	//LogMsg("Got %d screens.", wcVector.size());
	for(unsigned int i=0; i <wcVector.size(); i++)
	{
		if (wcVector[i]->GetScreen() != this)
			wcVector[i]->GetScreen()->AddTile(pTile->CreateReference(wcVector[i]->GetScreen()));
	}
}


//misc utilities
bool CloneTileList(const tile_list &srcList, tile_list &destList)
{
	ClearTileList(destList); //just in case

	tile_list::const_iterator itor = srcList.begin();
	while (itor != srcList.end())
	{
		destList.push_back((*itor)->CreateClone()); //add the tile
		itor++;
	}
	return true; //no errors
}

void ClearTileList(tile_list &tileList)
{
	tile_list::iterator itor;

	for (itor = tileList.begin(); itor != tileList.end(); ++itor)
	{
		assert( (*itor)->GetType() != C_TILE_TYPE_REFERENCE && "These should only be in screens!");
		assert( !(*itor)->IsEdgeCase() && "These should only be in screens!");
		delete (*itor);
	}
	tileList.clear();
}

