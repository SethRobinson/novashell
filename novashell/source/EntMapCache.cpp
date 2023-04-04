#include "AppPrecomp.h"
#include "EntMapCache.h"
#include "GameLogic.h"
#include "EntCollisionEditor.h"
#include "TileEntity.h"
#include "MovingEntity.h"
#include "AI/NavGraphManager.h"
#include "AI/Goal_Think.h"
#include "AI/WatchManager.h"
#include "MaterialManager.h"
#include "State.h"
#include "DrawManager.h"

#define C_TILE_EDIT_UNDO_LEVEL 20

EntMapCache::EntMapCache(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	 SetName("mapcache");
     m_pWorld = NULL;
	 m_cacheCheckTimer = 0;
	 SetDrawWorldChunkGrid(false);
	 SetDrawCollision(false);
}

void EntMapCache::PushUndoOperation(const operation_deque &op)
{
	if (m_undoDeque.size() >= C_TILE_EDIT_UNDO_LEVEL)
	{
		m_undoDeque.pop_back();
	}
	m_undoDeque.push_front(op);
}

void EntMapCache::PopUndoOperation()
{
	if (m_undoDeque.size() == 0) return;
	
	//undo one sequence of events
	
	while (!m_undoDeque.front().empty())
	{
		m_undoDeque.front().front().SetIgnoreParallaxOnNextPaste();
		m_undoDeque.front().front().PasteToWorld(m_undoDeque.front().front().GetUpperLeftPos(), TileEditOperation::PASTE_UPPER_LEFT, NULL);
		m_undoDeque.front().pop_front();
	}

	m_undoDeque.pop_front();
}

void EntMapCache::SetWorld( Map * pWorld)
{
	if (!m_pWorld)
	{
	
	//assert(!m_pWorld && "Um, I didn't plan on reinitting these..");

	m_pWorld = pWorld;
	pWorld->SetMyMapCache(this);

	m_slots.connect(GetGameLogic()->GetMyWorldManager()->sig_map_changed, this, &EntMapCache::OnMapChange);
	} else
	{
		assert(pWorld == m_pWorld);
		ClearCache();
	}

}

EntMapCache::~EntMapCache()
{
    ClearCache();
}

void EntMapCache::ClearCache()
{
	m_worldChunkVector.clear();
	//insure that this isn't run again
	ClearTriggers();

	//don't want to cache old data either
	m_tileLayerDrawList.clear();
	m_entityList.clear();
}

//TODO: these should be rewritten for speed later
CL_Vector2 EntMapCache::WorldToScreen(float x, float y)
{
	return WorldToScreen(CL_Vector2(x,y));
}

CL_Vector2 EntMapCache::WorldToScreen(const CL_Vector2 &vecWorld)
{
	return CL_Vector2( (vecWorld.x-GetCamera->GetPos().x)*GetCamera->GetScale().x , (vecWorld.y-GetCamera->GetPos().y)*GetCamera->GetScale().y);
}

CL_Vector2 EntMapCache::WorldToModifiedWorld(const CL_Vector2 &vecWorld, const CL_Vector2 &vecMod)
{
	CL_Vector2 vCam = GetCamera->GetPosCentered();
	return CL_Vector2(vecWorld.x + vCam.x * vecMod.x, vecWorld.y + vCam.y * vecMod.y);
}

CL_Vector2 EntMapCache::ModifiedWorldToWorld(const CL_Vector2 &vecWorld, const CL_Vector2 &vecMod)
{
	CL_Vector2 vCam = GetCamera->GetPosCentered();
	return CL_Vector2(vecWorld.x - vCam.x * vecMod.x, vecWorld.y - vCam.y * vecMod.y);
}

CL_Vector2 EntMapCache::ScreenToWorld(const CL_Vector2 &vecScreen)
{

/*
#ifdef _DEBUG
	if (!GetWorld->IsValidCoordinate(GetCamera->GetPos()))
	{
		LogMsg("Bad camera coord!");
		assert(0);
		GetCamera->Reset();
	}
#endif
*/

	static CL_Vector2 v;
	v.x =  (GetCamera->GetPos().x)+ (vecScreen.x/GetCamera->GetScale().x);
	v.y =  (GetCamera->GetPos().y)+ (vecScreen.y/GetCamera->GetScale().y);

	return v;
}

//returns true if actually generated a new thumb nail, false if the screen was blank
bool EntMapCache::GenerateThumbnail(ScreenID screenID)
{
	assert(m_pWorld);
	MapChunk *pWorldChunk = m_pWorld->GetMapChunk(screenID);

	if (!pWorldChunk->GetNeedsThumbnailRefresh())
	{
		assert(!"Don't need a thumbnail generated, why was this called?");
		return false;
	}

	pWorldChunk->SetThumbNail(NULL);
	pWorldChunk->SetNeedsThumbnailRefresh(false);

	if (pWorldChunk->IsEmpty()) 
	{
		//um.. we don't REALLY need this... although we should fill it with black I guess.
		pWorldChunk->KillThumbnail();
		return false;
	}

	Map *pWorld = pWorldChunk->GetParentMap();
	short width = pWorld->GetThumbnailWidth();
	short height = pWorld->GetThumbnailHeight();

	if (width == 0)
	{
		//thumbnails are disabled
		return false; 
	}

	bool bWasOriginallyInMem = pWorldChunk->IsScreenLoaded();
	//Screen *pScreen = pWorldChunk->GetScreen();

	//make thumbnail

	CL_PixelBuffer *pbuf = new CL_PixelBuffer(width, height, width*3, C_THUMBNAIL_FORMAT);
	//make thumbnail
	CL_Rect rectSrc = CL_Rect(0,0, width,height);

	GetApp()->GetBackgroundCanvas()->get_gc()->clear();
	
	CameraSetting oldCamSettings = GetCamera->GetCameraSettings();
	
	//setup cam to render stuff in the perfect position
	GetCamera->SetTargetPos(CL_Vector2(pWorldChunk->GetRect().left, pWorldChunk->GetRect().top));
	GetCamera->InstantUpdate();
	CL_Vector2 vecScale(float(GetScreenX)/float(pWorld->GetMapChunkPixelSize()),
		float(GetScreenY)/float(pWorld->GetMapChunkPixelSize()));

	//hack it up to only draw to the upper left at the size we want
	vecScale.x = float(rectSrc.right)/float(pWorld->GetMapChunkPixelSize());
	vecScale.y = float(rectSrc.bottom)/float(pWorld->GetMapChunkPixelSize());
	GetCamera->SetScaleRaw(vecScale);


	//render out our stuff to it
	CalculateVisibleList(pWorldChunk->GetRect(), true);

	//ugly, but I need a way for each sprite to know when to use special behavior like ignoring
	//parallax repositioning when making the thumbnail during its draw phase

	GetGameLogic()->SetMakingThumbnail(true);
	RenderViewList(GetApp()->GetBackgroundCanvas()->get_gc());
	GetGameLogic()->SetMakingThumbnail(false);

	GetApp()->GetBackgroundCanvas()->get_pixeldata(rectSrc).convert(*pbuf);

	pWorldChunk->SetThumbNail(pbuf);

	if (bWasOriginallyInMem)
	{
		//RemoveScreenFromCache(screenID);
		//pWorldChunk->UnloadScreen();
	}

	//set camera back to how it was
	GetCamera->SetCameraSettings(oldCamSettings);

	return true;
}

bool compareTileByLayer(const Tile *pA, const Tile *pB) 
{
	LayerManager *pLayerManager = &g_pMapManager->GetActiveMap()->GetLayerManager();

	return pLayerManager->GetLayerInfo(pA->GetLayer()).GetSort() <
		pLayerManager->GetLayerInfo(pB->GetLayer()).GetSort();
}

void EntMapCache::ProcessPendingEntityMovementAndDeletions()
{
}

void EntMapCache::ResetPendingEntityMovementAndDeletions()
{
	m_entityList.clear();
}

void EntMapCache::AddSectionToDraw(unsigned int renderID, CL_Rect &viewRect, vector<unsigned int>  & layerIDVec, bool bDontAddToDrawList )
{
	
	/*
	//slower way I was testing with
	
	//get all screens that we'll be able to see

	tile_list tileList;
	AddTilesByRect(viewRect, &tileList, layerIDVec);

	static tile_list::iterator itor;

	itor = tileList.begin();

	while (itor != tileList.end())
	{
		if (!bDontAddToDrawList)
		{
			m_tileLayerDrawList.push_back(*itor);
		}

		if ( (*itor)->GetType() == C_TILE_TYPE_ENTITY)
		{
			m_entityList.push_back(   ((TileEntity*)(*itor))->GetEntity()    );
		} 

		itor++;
	}
	*/

	//get all screens that we'll be able to see

	m_worldChunkVector.clear();
	m_pWorld->GetAllMapChunksWithinThisRect(m_worldChunkVector, viewRect, false);

	tile_list *pTileList;
	tile_list::iterator tileListItor;
	Tile *pTile;
	Layer *pLayer;

	//build the tile list
	for (EntWorldChunkVector::iterator itor = m_worldChunkVector.begin(); itor != m_worldChunkVector.end(); itor++)
	{
		for (unsigned int i=0; i < layerIDVec.size(); i++)
		{
			pTileList =  (*itor)->GetScreen()->GetTileList(layerIDVec[i]);
			pLayer = &m_pWorld->GetLayerManager().GetLayerInfo(layerIDVec[i]);
			tileListItor = pTileList->begin();

			while (tileListItor != pTileList->end())
			{
				//add our tiles?
				pTile = *tileListItor;
				if (pTile->GetType() == C_TILE_TYPE_REFERENCE)
				{
					pTile = pTile->GetTileWereAReferenceFrom();
				}

				if (pTile->GetType() == C_TILE_TYPE_ENTITY)
				{
					if (((TileEntity*)pTile)->GetEntity()->GetDeleteFlag())
					{
						tileListItor++;
						((TileEntity*)pTile)->GetEntity()->ProcessPendingMoveAndDeletionOperations();
						continue;
					}
				}
				
				if (pTile->GetLastScanID() != renderID)
				{
					pTile->SetLastScanID(renderID); //helps us not draw something twice, due to ghost references

					if (!pTile->GetWorldCombinedRectInt().is_overlapped(viewRect))
					{
						tileListItor++;
						continue;
					} 

					if (
						(!GetGameLogic()->GetEditorActive() && pLayer->GetShowInEditorOnly())
						|| !pLayer->IsDisplayed() )
					{
						//don't render this						
					} else
					{
						
						m_tileLayerDrawList.push_back(pTile);
					}

					if (pTile->GetType() == C_TILE_TYPE_ENTITY)
					{
						m_entityList.push_back( ((TileEntity*)pTile)->GetEntity());
					}
				} else
				{
					//already rendered it, don't do so again

				}

				tileListItor++;
			}
		}
	}
}


//erm, never do this

LayerManager *g_pLayerManager;

bool compareTileBySortLevelOptimized(Tile *pA, Tile *pB) 
{
	static int aLayer, bLayer;
	static CollisionData *pCol;
	aLayer = g_pLayerManager->GetLayerInfo(pA->GetLayer()).GetSort();
	bLayer = g_pLayerManager->GetLayerInfo(pB->GetLayer()).GetSort();
	static float aBottom, bBottom;

	if (aLayer == bLayer)
	{
		if (!g_pLayerManager->GetLayerInfo(pA->GetLayer()).GetDepthSortWithinLayer()) return aLayer < bLayer;
		//extra checking for depth queuing.  We depth cue by the bottom of the
		//collision box when possible, it's more accurate than the picture.
		pCol = pA->GetCollisionData();

		if (pCol && pCol->GetLineList()->size() > 0)
		{
			
			aBottom = pA->GetPos().y + (pCol->GetLineList()->begin()->GetRect().bottom);

			/*
			if (pA->GetType() == C_TILE_TYPE_ENTITY)
			{
				aBottom = pA->GetPos().y + (pCol->GetLineList()->begin()->GetRect().get_height()/2);
			} else
			{
				aBottom = pA->GetPos().y + (pCol->GetLineList()->begin()->GetRect().bottom);
			}
			*/
		} else
		{
			aBottom = pA->GetWorldRect().bottom;
		}
		pCol = pB->GetCollisionData();
	
		if (pCol && pCol->GetLineList()->size() > 0)
		{
			
				bBottom = pB->GetPos().y + (pCol->GetLineList()->begin()->GetRect().bottom);
			/*
			if (pB->GetType() == C_TILE_TYPE_ENTITY)
			{
				bBottom = pB->GetPos().y + (pCol->GetLineList()->begin()->GetRect().get_height()/2);
			} else
			{
				bBottom = pB->GetPos().y + (pCol->GetLineList()->begin()->GetRect().bottom);
			}
			*/
		} else
		{
			bBottom = pB->GetWorldRect().bottom;
		}

		return aBottom < bBottom;
	} else
	{
		return aLayer < bLayer;
	}
}

void EntMapCache::CalculateVisibleList(const CL_Rect &recScreen, bool bMakingThumbnail)
{
	CL_Rect viewRect;

	//viewable area in world coords
	if (bMakingThumbnail)
	{
		viewRect = recScreen;

	} else
	{
		viewRect = GetCamera->GetViewRectWorldInt();
		viewRect.right += 100;

	}
	//hack so we don't get shadow popping

	
	if (viewRect.right > 60000)
	{
		assert(0);
	}
	m_tileLayerDrawList.clear();
	ResetPendingEntityMovementAndDeletions();
	unsigned int renderID = GetApp()->GetUniqueNumber();

	//add the actual tile/entity data to our draw list
	LayerManager &layerMan = m_pWorld->GetLayerManager();
	CL_Vector2 scrollMod;

	static vector<unsigned int> layerIdVecStandard; //default layers clumped together to process faster
	static vector<unsigned int> layerIdVecSpecial; //special layers does separately
	
	layerIdVecStandard.clear();

	static bool bNoParallax;
	static bool bDontAddToDrawList;
	
	for (unsigned int i=0; i < layerMan.GetLayerCount(); i++)
	{
		Layer &layer = layerMan.GetLayerInfo(i);

		if (bMakingThumbnail)
		{
			if (!layer.GetUseInThumbnail()) continue;
		} else
		{
			//this way it's possible to have something ONLY rendered in the thumbnail
			if (!layer.IsDisplayed() && !layer.GetShowInEditorOnly()) continue; //we're told not to show it
		}

		bNoParallax = true;
		bDontAddToDrawList = false;
	
		scrollMod = layer.GetScrollMod();
			 
		if ( GetGameLogic()->GetParallaxActive() && (scrollMod !=  CL_Vector2::ZERO))
		{
			bNoParallax = false; //we need special handling
		}
	
		if (layer.GetShowInEditorOnly())
		{
			if (!GetGameLogic()->GetEditorActive() || !layer.IsDisplayed())
			{
				//well, we don't want to show this, but we do want its entities to get processed.
				bNoParallax = false;
				bDontAddToDrawList = true;
			}
		}
		
		if (bMakingThumbnail)
		{
			if (!layer.GetUseParallaxInThumbnail())
			{
				bNoParallax = true;
			}
		}

		if (bNoParallax)
		{
			layerIdVecStandard.push_back(i);
		} else
		{
			//requires special handling
			layerIdVecSpecial.clear();
			layerIdVecSpecial.push_back(i);
			//map rect to modified position
			CL_Rect modViewRect = viewRect;
			CL_Vector2 vOffset = WorldToModifiedWorld(CL_Vector2(0,0), scrollMod);
			modViewRect.apply_alignment(origin_top_left, vOffset.x, vOffset.y);
			AddSectionToDraw(renderID, modViewRect, layerIdVecSpecial, bDontAddToDrawList );
			continue;
		}
	}

	//LogMsg("Specials gave us %d tiles.", m_tileLayerDrawList.size());
	//add all the normals
	AddSectionToDraw(renderID, viewRect, layerIdVecStandard, false);

	//LogMsg("Sorting %d tiles.  %d entities found. %d normal layers.", m_tileLayerDrawList.size(), m_entityList.size(),
	//	layerIdVecStandard.size());
	//LogMsg("Viewrect is %s.  Drawing %d tiles", PrintRect(viewRect).c_str(), m_tileLayerDrawList.size());
	//sort the tiles/entities we're going to draw by layer
	
	g_pLayerManager = &g_pMapManager->GetActiveMap()->GetLayerManager();
	
	std::sort(m_tileLayerDrawList.begin(), m_tileLayerDrawList.end(), compareTileBySortLevelOptimized);

	if (m_bDrawWorldChunkGrid)
	{
		m_worldChunkVector.clear();
		m_pWorld->GetAllMapChunksWithinThisRect(m_worldChunkVector, viewRect, false);
	}

}

void EntMapCache::CullScreensNotUsedRecently(unsigned int timeRequiredToKeep)
{
	
}

//break this rect down into chunks to feed into the screens to get tile info
void EntMapCache::AddTilesByRect(const CL_Rect &recArea, tile_list *pTileList, const vector<unsigned int> &layerIDVect,  bool bWithCollisionOnly /*= false*/, bool bAllowLoadOnDemand /* = false*/, bool bEntitiesOnly)
{

	assert(recArea.right < 20000);
	CL_Rect rec(recArea);
	rec.normalize();
    static int startingX;
	unsigned int scanID = GetApp()->GetUniqueNumber();
	static CL_Rect scanRec;
	static MapChunk *pWorldChunk;
	static CL_Rect screenRec;
	static bool bScanMoreOnTheRight, bScanMoreOnTheBottom;
	Screen *pScreen;

	startingX = rec.left;

	while (1)
	{
		scanRec = rec;

		assert(rec.top <= rec.bottom && "this should never happen");
		//get the screen the upper left is on
		pWorldChunk = m_pWorld->GetMapChunk(CL_Vector2(scanRec.left, scanRec.top));
		screenRec = pWorldChunk->GetRect();
		//truncate to the screen size if we need to
		
		if (scanRec.right > screenRec.right)
		{
			//we have more on our right to scan after this, truncate it for now
			scanRec.right = screenRec.right;
			bScanMoreOnTheRight = true;
		} else
		{
			bScanMoreOnTheRight = false;
		}

		if (scanRec.bottom > screenRec.bottom)
		{
			//we have more on our bottom to scan after this, truncate it for now
			scanRec.bottom = screenRec.bottom;
			bScanMoreOnTheBottom = true;
		} else
		{
			bScanMoreOnTheBottom = false;
		}
	
		pScreen = pWorldChunk->GetScreen(bAllowLoadOnDemand);

		if (pScreen) pScreen->GetTilesByRect(scanRec, pTileList, layerIDVect, scanID, bWithCollisionOnly, bEntitiesOnly);

		//do we have more to scan on our right after this?

		if (bScanMoreOnTheRight)
		{
			rec.left = scanRec.right;
		} else if (bScanMoreOnTheBottom)
			{
				rec.top = scanRec.bottom;
				rec.left = startingX;
			} else
		{
			//all done
			break;
		}
	}
}



void EntMapCache::RenderCollisionOutlines(CL_GraphicContext *pGC)
{
	Tile *pTile;

	for (unsigned int i=0; i < m_tileLayerDrawList.size(); i++)
	{
		
		pTile = m_tileLayerDrawList.at(i);
		if (!pTile) continue;

		if (pTile->GetCollisionData() && pTile->GetCollisionData()->HasData())
		{
			CollisionData *pCol = pTile->GetCollisionData();
		
			if (pTile->GetCustomBody())
			{
				b2Shape *pShape = pTile->GetCustomBody()->GetShapeList();
				for (;pShape; pShape = pShape->GetNext())
				{
					assert(g_pMapManager->GetActiveMap() == pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap());
					ShapeUserData *pShapeUserData = (ShapeUserData*)pShape->GetUserData();
					
					CL_Color c;
					c.color = g_materialManager.GetMaterial(pShapeUserData->materialID)->GetColor();
				
					g_pMapManager->GetActiveMap()->GetPhysicsManager()->DrawShape(pShape,
					pTile->GetCustomBody()->GetXForm(), b2Color(c.get_red(), c.get_green(), c.get_blue()),
					false);
					
				}
			} else
			{
			
			/*
				if (pTile->UsesTileProperties() && pTile->GetType() == C_TILE_TYPE_PIC)
			{ 
				CollisionData col;
				//we need a customized version
				CreateCollisionDataWithTileProperties(pTile, col);
				RenderVectorCollisionData(pTile->GetPos(), col, pGC, false, NULL);
			} else
			{
			
				RenderVectorCollisionData(pTile->GetPos(), *pCol, pGC, false, NULL);
			}
			*/
				//assert(0);

			
			}
		}
	}

	
}

void EntMapCache::RenderGoalAI(CL_GraphicContext *pGC)
{
	Tile *pTile;

	for (unsigned int i=0; i < m_tileLayerDrawList.size(); i++)
	{
		pTile = m_tileLayerDrawList.at(i);
		if (!pTile) continue;

			if (pTile->GetType() == C_TILE_TYPE_ENTITY)
				{
					MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();

					if (pEnt->IsGoalManagerActive())
					{
						//render goal names in  the stack
						CL_Vector2 pos = WorldToScreen(pEnt->GetPos());
						
						//first render some general info about the entity:

						if (pEnt->GetBrainManager()->GetState())
						{

						
							string info = pEnt->GetName() + " ("+ CL_String::from_int(pEnt->ID())+") " +
							string(pEnt->GetBrainManager()->GetState()->GetName());
							
							ResetFont(GetApp()->GetFont(C_FONT_GRAY));
							CL_Color col = CL_Color(30,255,30);
							DrawWithShadow(pos.x, pos.y, info, col);
							
							pEnt->GetGoalManager()->RenderAtPos(pos);
							//render any custom debug data a goal wants to
							pEnt->GetGoalManager()->Render();
						}
					}
				}
	}
}


void EntMapCache::RenderViewList(CL_GraphicContext *pGC)
{
	Tile *pTile;

	//we render them in sections by layer, in two passes.  The first pass is for shadows

	int renderStart = 0;
	int curSort = -99999;
	LayerManager *pLayerManager = &g_pMapManager->GetActiveMap()->GetLayerManager();
	int sort;
	g_drawManager.RenderUpToLayerID(-1, pGC); 

	unsigned int i;
	for (i=0; i < m_tileLayerDrawList.size(); i++)
	{
		pTile = m_tileLayerDrawList.at(i);
		if (!pTile) continue;
	
		if ( (sort = pLayerManager->GetLayerInfo(pTile->GetLayer()).GetSort()) != curSort)
		{
				//actually changing layer
				curSort = sort;
				//now render the tiles
				for (unsigned int r=renderStart; r < i; r++)
				{
					pTile = m_tileLayerDrawList.at(r);
					if (pTile)
					{
						if (pTile->GetBit(Tile::e_castShadow) && pTile->GetBit(Tile::e_sortShadow))
						{
							pTile->RenderShadow(pGC);
						}
						pTile->Render(pGC);
					}
				}

				//setup for next shadow pass
				renderStart = i;

				i--; //do the current one again
				continue;
		} else
		{
			if (pTile->GetBit(Tile::e_castShadow) && !pTile->GetBit(Tile::e_sortShadow))
			{
				pTile->RenderShadow(pGC);
			}
			g_drawManager.RenderUpToLayerID(curSort, pGC); 

		}
	} 

	//render last batch if applicable

	//now render the tiles
	for (unsigned int r=renderStart; r < m_tileLayerDrawList.size(); r++)
	{
		pTile = m_tileLayerDrawList.at(r);
		if (pTile)
		{
			if (pTile->GetBit(Tile::e_castShadow) && pTile->GetBit(Tile::e_sortShadow))
			{
				pTile->RenderShadow(pGC);
			}
			pTile->Render(pGC);

		}
	}

	g_drawManager.RenderUpToLayerID(99999999, pGC);  //make sure we got everything

}

void EntMapCache::ClearTriggers()
{
	m_activeTriggers.clear();
}

void EntMapCache::AddActiveTrigger(int entID)
{
	m_activeTriggers.push_back(entID);
}



void EntMapCache::Update(float step)
{
}

bool EntMapCache::IsAreaObstructed(CL_Vector2 pos, float radius, bool bIgnoreMovingCreatures, MovingEntity *pIgnoreEntity)
{
	tile_list tilelist;
	CL_Rect r(pos.x - radius/2, pos.y - radius/2, pos.x + radius/2, pos.y + radius/2);

	CollisionData *pCol;

	AddTilesByRect(r, &tilelist, m_pWorld->GetLayerManager().GetCollisionList(), true);

	if (tilelist.empty()) return false;

	tile_list::iterator listItor = tilelist.begin();

	Tile *pTile;
	CL_Rect tempRect;
	static CollisionData col;

	for (; listItor != tilelist.end(); listItor++)
	{
		pTile = *listItor;

		pCol = pTile->GetCollisionData();
		if (pCol && pCol->HasData())
		{
			if (pTile == pIgnoreEntity->GetTile()) continue; //skip this

			if (bIgnoreMovingCreatures && pTile->GetType() == C_TILE_TYPE_ENTITY)
			{
				//the path-finding may be building nodes, we don't want it to build an incorrect base path because
				//the player or monster was sitting in the way. 

				if (  ((TileEntity*)(*listItor))->GetEntity()->GetIsCreature())
				{
					continue; //skip this
				}
			}
			
			
			if (pTile->UsesTileProperties() && pTile->GetType() == C_TILE_TYPE_PIC )
			{
				//we need a customized version
				CreateCollisionDataWithTileProperties(pTile, col);
				pCol = &col;
			}
			

			//do we overlap his collision info?
			//we can't just check GetWorldColRect because it's not accurate enough, let's check each line and do the rect
			//from that
			line_list::iterator lineListItor = pCol->GetLineList()->begin();

			while (lineListItor != pCol->GetLineList()->end())
			{
				if (lineListItor->GetPointList()->size() > 0 && g_materialManager.GetMaterial(lineListItor->GetType())->GetType()
					== CMaterial::C_MATERIAL_TYPE_NORMAL)
				{
					CL_Vector2 offset = pTile->GetPos();
					tempRect = CL_Rect(lineListItor->GetRect());
					tempRect += CL_Point(offset.x, offset.y);
					//if ( tempRect.is_overlapped(r))
					{
						CL_Vector2 picOffset;

						if (pTile->GetType() == C_TILE_TYPE_PIC)
						{
							picOffset = lineListItor->GetOffset();
						} else
						{
							picOffset = CL_Vector2::ZERO;
						}
						CL_Vector2 worldColCenter = offset+picOffset;
						CL_Vector2 facingAwayFromCollision = pos-worldColCenter;
						facingAwayFromCollision.unitize();
						CL_Vector2 worldColOutsideTarget = worldColCenter+ (facingAwayFromCollision* 10000);

						//DrawLineWithArrowWorld(pos, worldColOutsideTarget, 5, CL_Color::white, CL_Display::get_current_window()->get_gc());
						
						//found a potential, let's do further checking for more accuracy?
						//DrawBullsEyeWorld(offset+picOffset, CL_Color::white, 10, CL_Display::get_current_window()->get_gc());
						//CL_Display::flip(2); //show it now
						
						//this check figures out if the center is inside the poly or not by shooting a giant
						//ray out from the center and seeing if it hits anything
						CL_Vector2 localPos = pos- (offset+picOffset);
						if (lineListItor->GetLineIntersection(localPos, worldColOutsideTarget- (offset+picOffset)))
						{
							//we hit it.
							return true;
						}

						//now we need to do an additional check for situations where we're too close to the edge
						if (lineListItor->GetCircleIntersection(localPos, radius))
						{
							return true;
						}
					
					}
				}

				lineListItor++;
			}

		}
	}

	return false;
}

bool EntMapCache::IsPathObstructed(CL_Vector2 a, CL_Vector2 b, float radius, Tile *pTileToIgnore, bool bIgnoreMovingCreatures)
{

	//LogMsg("Checking path with %.2f bounds", radius);
	//first grab all entities that could possibly by between these two locations
	tile_list tilelist;
	CL_Rect r(a.x,a.y, b.x, b.y);
	r.normalize();

	AddTilesByRect(r, &tilelist, m_pWorld->GetLayerManager().GetCollisionList(), true);

	//LogMsg("Found %d tiles", tilelist.size());
	static Tile *pTile;
	static CL_Vector2 vHitPoint;

	//do three checks
	bool obstructed;
	if (obstructed = GetTileLineIntersection(a, b, tilelist, &vHitPoint, pTile, pTileToIgnore, C_TILE_TYPE_BLANK, bIgnoreMovingCreatures)) return true;

	if (radius > 1)
	{
		//extra checks
		CL_Vector2 vHeading = a-b;
		vHeading.unitize();
		
		CL_Vector2 vOffset = Vector2Perp(vHeading) * radius/2;
		
		//move the line a little and shoot another ray

		a += vOffset;
		b += vOffset;

		if (obstructed = GetTileLineIntersection(a, b, tilelist, &vHitPoint, pTile, pTileToIgnore, C_TILE_TYPE_BLANK, bIgnoreMovingCreatures)) return true;

		//now go the other way
		vOffset = - (vOffset*2);

		a += vOffset;
		b += vOffset;

		if (obstructed = GetTileLineIntersection(a, b, tilelist, &vHitPoint, pTile, pTileToIgnore, C_TILE_TYPE_BLANK, bIgnoreMovingCreatures)) return true;
	}

	return false;
}

void EntMapCache::OnMapChange()
{
	//let any triggers register that the player has left the map
	MovingEntity *pEnt = NULL;
	//use a temp copy to avoid circular problems

	vector<unsigned int> activeTriggers =  m_activeTriggers;

	for (unsigned int i=0; i < activeTriggers.size(); i++)
	{
		pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(activeTriggers[i]);
		if (pEnt)
		{
			pEnt->UpdateTriggers(0);
		}
	}
	
	
	ClearCache();
}

void EntMapCache::RenderGrid(CL_GraphicContext *pTarget)
{

	CL_Color col = CL_Color(255,255,255,36);
	CL_Vector2 vSnap = CL_Vector2(m_pWorld->GetDefaultTileSizeX(), m_pWorld->GetDefaultTileSizeY());
	
	if (vSnap.x <= 1 || vSnap.y <= 1)
	{
		//grid is too small to draw..
		return;
	}

	//modify the grid if we're zoomed out really far...looks better
	while (vSnap.x * GetCamera->GetScale().x < 5)
	{
		vSnap *= 2;
	}

	CL_Rectf rWorld = GetCamera->GetViewRectWorld();
	
	CL_Vector2 vPos = m_pWorld->SnapWorldCoords(GetCamera->GetPos(), vSnap);

	for (; vPos.x < rWorld.right+vSnap.x; vPos.x += vSnap.x)
	{
		DrawLineFromWorldCoordinates(vPos, vPos + CL_Vector2(0,rWorld.get_height() + vSnap.y), col, pTarget);
	}
	
	vPos.x = rWorld.left; //reset this to do another pass for the horizontal lines

	for (; vPos.y < rWorld.bottom+vSnap.y; vPos.y += vSnap.y)
	{
		DrawLineFromWorldCoordinates(vPos, vPos + CL_Vector2(rWorld.get_width() + vSnap.x, 0), col, pTarget);
	}

}


void EntMapCache::Render(void *pTarget)
{
	
	//ProcessPendingEntityMovementAndDeletions();
	CalculateVisibleList(CL_Rect(0,0,GetScreenX,GetScreenY), false);
	ClearTriggers();

	CL_GraphicContext* pGC = (CL_GraphicContext*)pTarget;

	if (m_bDrawCollisionData)
	{
		RenderCollisionOutlines(pGC);
	}

	RenderViewList(pGC);

	if (GetGameLogic()->GetShowPathfinding())
	{
		if (m_pWorld->NavGraphDataExists())
			m_pWorld->GetNavGraph()->Render(true, pGC);
	}


	if (GetGameLogic()->GetShowEntityCollisionData())
	{
		//show extra debug stuff
		//RenderCollisionLists(pGC);
	}

	if (GetGameLogic()->GetShowAI())
	{
		RenderGoalAI(pGC);
	}

	if (m_bDrawWorldChunkGrid)
	{
		CL_Rect recScreen;
		for (EntWorldChunkVector::iterator itor = m_worldChunkVector.begin(); itor != m_worldChunkVector.end(); itor++)
		{
			recScreen = (*itor)->GetRect();
			DrawRectFromWorldCoordinates(CL_Vector2(recScreen.left, recScreen.top),
				CL_Vector2(recScreen.right, recScreen.bottom), CL_Color(0,255,255,255), pGC);
		}
	}

	if (GetGameLogic()->GetShowGrid())
	{
		RenderGrid(pGC);
	}

}

void EntMapCache::SetDrawCollision( bool bNew )
{
	m_bDrawCollisionData = bNew;
	
	if (m_pWorld)
	m_pWorld->GetPhysicsManager()->SetDrawDebug(m_bDrawCollisionData);

}


