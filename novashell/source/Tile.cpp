#include "AppPrecomp.h"
#include "Tile.h"
#include "Screen.h"
#include "CollisionData.h"
#include "MovingEntity.h"
#include "LayerManager.h"
#include "MaterialManager.h"

#define C_DEFAULT_LAYER 1

void RenderTilePicShadow(TilePic *pTile, CL_GraphicContext *pGC);

Tile::Tile()
{
	m_lastScanID = 0;
	m_pParentScreen = NULL;
	m_type = C_TILE_TYPE_BLANK;
	m_layer = C_DEFAULT_LAYER;
	m_vecScale = CL_Vector2(1,1);
	m_pEdgeCaseList = NULL; //if not null, we're an edge case
	m_pFatherTile = NULL; //if not null, our type must be C_TILE_REFERENCE
	m_bitField.clear();
	m_pCollisionData = NULL;
	m_color = CL_Color(255,255,255);
	m_bUsingCustomCollision = false;
	m_graphNodeID = invalid_node_index;
	m_origin = origin_center;
	m_alignX = m_alignY = 0;

}

CL_Rect Tile::GetWorldRectInt()
{
	static CL_Rectf r;
	r = GetWorldRect();
	
/*		
	CL_Rect c(r);
	if (c.left < -20000 || c.left > 20000)
	{
		LogMsg("BEFORE:  rect is %s",  PrintRect(r).c_str());
		
		LogMsg("AFTER:  rect is %s", PrintRectInt(c).c_str());
	}
*/

	return CL_Rect(r.left, r.top, r.right, r.bottom);
}

Tile::~Tile()
{
	if (m_pFatherTile)
	{
		//we need to remove outself from our parent, he exists because we still exist. (if
		//he was deleted, he would have deleted us already)
		m_pFatherTile->RemoveReference(this);
	} else
	{
		if (m_pEdgeCaseList)
		{
			//we have references floating around, eliminate them, mr jones
			std::list<Tile*>::iterator itor = m_pEdgeCaseList->begin();

			while (itor != m_pEdgeCaseList->end())
			{
				//by setting its parent blank, it won't attempt to notify us
				//that it is being deleted which would probably screw things up to no end
				(*itor)->m_pFatherTile = NULL;
				(*itor)->m_pParentScreen->RemoveTileByPointer(*itor);
				//no need to delete each link, will happen in a sec anyway
				itor++;
			}

			SAFE_DELETE(m_pEdgeCaseList);
		}
	}

	assert(!m_pEdgeCaseList);

	if (m_bUsingCustomCollision)
	{
		delete m_pCollisionData;
	}
}

const CL_Rectf & Tile::GetWorldColRect()
{
	static CL_Rectf r;
	if (!GetCollisionData())
	{
		assert(!"Huh?  This doesn't have collision data!");
		r = CL_Rectf(0,0,0,0);
		return r;
	}
	r = GetCollisionData()->GetCombinedCollisionRect();
	r += *(const CL_Pointf*)(&GetPos());
	
	if (GetType() == C_TILE_TYPE_ENTITY)
	{
		//I'm a little confused here, TILE_PICS are screwing up with this, but shouldn't their
		//offsets be set at 0 anyway?  
		r -= *(const CL_Pointf*)(&GetCollisionData()->GetCombinedOffsets());
	}
	return r;
}



CL_Rectf Tile::GetWorldCombinedRect()
{
	
	if (GetType() == C_TILE_TYPE_ENTITY)
	{
		if (m_pCollisionData && m_pCollisionData->HasData())
		return CombineRects(GetWorldColRect(), GetWorldRect());
	} else
	{
		//tilepics have a more generic collision...
		return CombineRects(GetWorldColRect(), GetWorldRect());
	}

	return GetWorldRect();
}



CL_Rect Tile::GetWorldCombinedRectInt()
{
	CL_Rectf r = GetWorldCombinedRect();
	
	return CL_Rect(r.left,r.top,r.right,r.bottom);
}

const CL_Rect & Tile::GetBoundsRect()
{
	static CL_Rect r(0,0,64,64);
	return r;
}

Screen * Tile::GetParentScreen()
{
	return m_pParentScreen;
}

bool Tile::UsesTileProperties()
{
	CollisionData *pColData = GetCollisionData();
	if (pColData && pColData->GetLineList()->size() == 0) return false; //doesn't matter
	if ( GetBit(Tile::e_flippedX)  || GetBit(Tile::e_flippedY) || GetScale() != CL_Vector2(1,1))
	{
		return true;
	}

	return false;
}

void Tile::RemoveReference(Tile *pTileRef)
{
	assert(m_type != C_TILE_TYPE_REFERENCE && m_pEdgeCaseList && m_pEdgeCaseList->size() > 0);

	std::list<Tile*>::iterator itor = m_pEdgeCaseList->begin();

	while (itor != m_pEdgeCaseList->end())
	{
		if ((*itor) == pTileRef)
		{
			//bingo
			m_pEdgeCaseList->erase(itor);
			//all done
			break;
		}
		itor++;
	}

}

void Tile::SerializeBase(CL_FileHelper &helper)
{
	helper.process(m_layer);
	helper.process( *(cl_uint32*)&m_bitField);

	if (helper.IsWriting())
	{
		helper.process_const(GetPos());
	} else
	{
		helper.process(m_vecPos);
		
		//total hack to avoid the normal SetPos confusing the "did the map change" flag
		if (GetType() == C_TILE_TYPE_ENTITY)
		{
			((TileEntity*)this)->SetPos(m_vecPos);
			((TileEntity*)this)->GetEntity()->SetMovedFlag(false);
		}
		
	}
	helper.process(m_vecScale);
	helper.process(*(cl_uint32*)&m_color);

	cl_uint8 ver = GetParentScreen()->GetVersion();

	if (helper.IsWriting() || ver > 2)
	{
		CL_Origin o;
		if (helper.IsWriting())
		{
			//get latest data
			GetAlignment(o, m_alignX, m_alignY);
			m_origin = o;
		}
		helper.process(m_origin);
		helper.process(m_alignX);
		helper.process(m_alignY);
	}

}

//sometimes with blank/placeholders we need copy a blank tile
Tile * Tile::CreateClone()
{
	Tile *pNew = new Tile(); //this sets the type for us
	CopyFromBaseToClone(pNew);
	return pNew;
}

Tile * Tile::CopyFromBaseToClone(Tile *pNew)
{
	pNew->m_layer = m_layer;
	pNew->m_bitField = m_bitField;
	pNew->m_vecPos = m_vecPos;
	pNew->m_vecScale = m_vecScale;
	pNew->m_color = m_color;
	pNew->m_origin = m_origin;
	pNew->m_alignX = m_alignX;
	pNew->m_alignY = m_alignY;
	return pNew;
}

void Tile::SetScale(const CL_Vector2 &v)
{
	m_vecScale = v;
}
Tile * Tile::CreateReference(Screen *pScreen)
{
    Tile *pNew = new Tile();
	pNew->m_pParentScreen = pScreen;
	pNew->m_layer = m_layer;
	pNew->m_pFatherTile = this; //let it know who it is referencing
	pNew->m_type = C_TILE_TYPE_REFERENCE;

	//let our daddy know we exist and to kill us when he dies
	if (!m_pEdgeCaseList)
	{
		m_pEdgeCaseList = new std::list<Tile*>;
	}

	m_pEdgeCaseList->push_back(pNew);
	return pNew;
}


void Tile::GetAlignment(CL_Origin &origin, int &x, int &y)
{
	origin = CL_Origin(m_origin);
	x = m_alignX; 
	y= m_alignY;
}
void Tile::SetAlignment(CL_Origin origin, int &x, int &y)
{
	m_origin = origin;
	m_alignX = x; 
	m_alignY = y;
}

void Tile::GetBaseAlignment( CL_Origin &origin, int &x, int &y )
{
	origin = CL_Origin(m_origin);
	x = m_alignX; 
	y= m_alignY;
}

void RenderTilePic(TilePic *pTile, CL_GraphicContext *pGC)
{

	static CL_OpenGLSurface *pSurf;
	static CL_Vector2 vecScale;
	
	vecScale = GetCamera->GetScale();
	
	pSurf = (CL_OpenGLSurface*) GetHashedResourceManager->GetResourceByHashedID(pTile->m_resourceID);
	if (!pSurf) return; //error

	static CL_Vector2 vecPos;

	static bool bUseParallax;
	static Layer *pLayer;
	static Map *pWorld;
	pWorld = pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
	static EntMapCache *pWorldCache;
	
	pWorldCache = pWorld->GetMyMapCache();
	
	assert(pWorldCache && "Why render nothing?");
	
	if (GetGameLogic()->GetParallaxActive())
	{
		bUseParallax = true;
		pLayer = &pWorld->GetLayerManager().GetLayerInfo(pTile->GetLayer());
		
		if (pLayer->RequiresParallax())
		{
		if (GetGameLogic()->GetMakingThumbnail())
		{
			if (!pLayer->GetUseParallaxInThumbnail())
			{
				bUseParallax = false;
			}
		}
		} else
		{
			bUseParallax = false;
		}
	} else
	{
		bUseParallax = false;
	}
	
	CL_Rectf worldRect = pTile->GetWorldRect();
	vecPos = CL_Vector2(worldRect.left, worldRect.top);

	if (bUseParallax)
	{
		vecPos = pWorldCache->WorldToScreen(pWorldCache->WorldToModifiedWorld(vecPos, 
		pLayer->GetScrollMod()));
	} else
	{
		vecPos = pWorldCache->WorldToScreen(vecPos);
	}
	
	static CL_Rectf rectDest;
	rectDest.left = vecPos.x;
	rectDest.top = vecPos.y;
	rectDest.right = vecPos.x+ ( float(pTile->m_rectSrc.get_width()) * vecScale.x * pTile->GetScale().x);
	rectDest.bottom = vecPos.y+ float(pTile->m_rectSrc.get_height()) * vecScale.y* pTile->GetScale().y;
	
	if (pTile->GetBit(Tile::e_flippedX))
	{
		pSurf->set_angle_yaw(-180);
	} else
	{
		pSurf->set_angle_yaw(0);
	}

	if (pTile->GetBit(Tile::e_flippedY))
	{
		pSurf->set_angle_pitch(-180);
	} else
	{
		pSurf->set_angle_pitch(0);
	}

	//fix holes that can appear when zoomed way in.  I suspect this is due to a pixel rounding error when
	//doing the blit? 
   
/*
	rectDest.bottom = int(rectDest.bottom);
	rectDest.right = int(rectDest.right);
	rectDest.top = int(rectDest.top);
	rectDest.left = int(rectDest.left);
*/

	//this fixes glitches with tiling when the scale isn't exactly 1.0
	
	rectDest.bottom = RoundNearest(rectDest.bottom, 1.0f);
	rectDest.right = RoundNearest(rectDest.right, 1.0f);
	rectDest.top = RoundNearest(rectDest.top, 1.0f);
	rectDest.left = RoundNearest(rectDest.left, 1.0f);
	
	CL_OpenGLState state(pGC);
	state.set_active();
	clBindTexture(CL_TEXTURE_2D, pSurf->get_handle());

	// Check out whether the Use Linear Filter flag is set.
	if (pTile->GetBit(Tile::e_linearFilter))
	{
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_LINEAR);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_LINEAR);
	}
	else
	{
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);
	}

	pSurf->set_color(pTile->GetColor());
	pSurf->draw_subpixel(pTile->m_rectSrc, rectDest, pGC);
}


void RenderTilePicShadow(TilePic *pTile, CL_GraphicContext *pGC)
{

	static CL_OpenGLSurface *pSurf;
	static CL_Vector2 vecScale;

	vecScale = GetCamera->GetScale();

	pSurf = (CL_OpenGLSurface*) GetHashedResourceManager->GetResourceByHashedID(pTile->m_resourceID);
	if (!pSurf) return; //error

	static CL_Vector2 vecPos;

	static Layer *pLayer;
	
	/*
	static Map *pWorld;
	
	pWorld = pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
	static EntMapCache *pWorldCache;
	pWorldCache = pWorld->GetMyMapCache();
	*/
	static CL_Rectf rectDest;
	rectDest =   ConvertWorldRectToScreenRect(pTile->GetWorldRect());
	
	/*
	vecPos = CL_Vector2(worldRect.left, worldRect.top);

	vecPos = pWorldCache->WorldToScreen(vecPos);
	
	static CL_Rectf rectDest;
	rectDest.left = vecPos.x;
	rectDest.top = vecPos.y;
	rectDest.right = vecPos.x+ ( float(pTile->m_rectSrc.get_width()) * vecScale.x * pTile->GetScale().x);
	rectDest.bottom = vecPos.y+ float(pTile->m_rectSrc.get_height()) * vecScale.y* pTile->GetScale().y;
*/
	if (pTile->GetBit(Tile::e_flippedX))
	{
		pSurf->set_angle_yaw(-180);
	} else
	{
		pSurf->set_angle_yaw(0);
	}

	if (pTile->GetBit(Tile::e_flippedY))
	{
		pSurf->set_angle_pitch(-180);
	} else
	{
		pSurf->set_angle_pitch(0);
	}

	//this fixes glitches with tiling when the scale isn't exactly 1.0

	rectDest.bottom = RoundNearest(rectDest.bottom, 1.0f);
	rectDest.right = RoundNearest(rectDest.right, 1.0f);
	rectDest.top = RoundNearest(rectDest.top, 1.0f);
	rectDest.left = RoundNearest(rectDest.left, 1.0f);

//	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
//	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

	//draw shadow
		static CL_Surface_DrawParams1 params1;
		pSurf->set_color(CL_Color(0,0,0,50));
		
		pSurf->setup_params(pTile->m_rectSrc, rectDest, params1, true);
		AddShadowToParam1(params1, pTile);
		pSurf->draw(params1, pGC);
}

TilePic::~TilePic()
{
	KillBody();
}

void TilePic::KillBody()
{
	if (m_pBody && m_pParentScreen)
	{
		m_pParentScreen->GetParentMapChunk()->GetParentMap()->GetPhysicsManager()->GetBox2D()->DestroyBody(m_pBody);
		m_pBody = NULL;
	}

}



void TilePic::RenderShadow(CL_GraphicContext *pGC)
{
		RenderTilePicShadow(this, pGC);
}
void TilePic::Render(CL_GraphicContext *pGC)
{
	RenderTilePic(this, pGC);
}

void TilePic::SetScale(const CL_Vector2 &v)
{
	if (v != m_vecScale)
	{
		if (m_bUsingCustomCollision)
		{
			m_bUsingCustomCollision = false;
			SAFE_DELETE(m_pCollisionData);
		} else
		{
			m_pCollisionData = false;
		}
	}
	m_vecScale = v;
}

void TilePic::ForceUsingCustomCollisionData()
{
	if (!m_bUsingCustomCollision)	
	{
		m_pCollisionData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(m_resourceID, m_rectSrc);

		//well, this has real data most likely, so let's create our own copy and apply scale to it
		CollisionData *pNewCol = new CollisionData(*m_pCollisionData);
		m_pCollisionData = pNewCol;
		m_bUsingCustomCollision = true;
		m_pCollisionData->SetScale(m_vecScale);
	}

}

void TilePic::SaveToMasterCollision()
{
	if (m_bUsingCustomCollision)
	{
		CollisionData *pMaster = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(m_resourceID, m_rectSrc);

		*pMaster = *m_pCollisionData;
		pMaster->SetScale(CL_Vector2(1,1));
		pMaster->SetDataChanged(true);
	}
}

void TilePic::ReinitCollision()
{
	if (m_bUsingCustomCollision)
	{
		delete m_pCollisionData;
		m_bUsingCustomCollision = false;
	}

	m_pCollisionData = NULL;
}

CollisionData * TilePic::GetCollisionData()
{
	if (m_pCollisionData) return m_pCollisionData;

	m_pCollisionData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(m_resourceID, m_rectSrc);

	if (!m_pCollisionData) return NULL;
	if (GetParentScreen() && m_pCollisionData->GetLineList()->size() > 0)
	{

		if (m_vecScale != CL_Vector2(1,1))
		{
			//we may have to do something custom here
			//well, this has real data most likely, so let's create our own copy and apply scale to it
			CollisionData *pNewCol = new CollisionData(*m_pCollisionData);
				m_pCollisionData = pNewCol;
			m_bUsingCustomCollision = true;
			m_pCollisionData->SetScale(m_vecScale);

		}

		//MAKE BODY
		KillBody();

		b2BodyDef bd;
		bd.position = ToPhysicsSpace(GetPos());
		m_pBody = GetParentScreen()->GetParentMapChunk()->GetParentMap()->GetPhysicsManager()->GetBox2D()->CreateBody(&bd);
		if (m_pBody->GetWorld()->GetProxyCount() > b2_maxProxies-10)
		{
			LogError("Box2d collision system cannot handle any more objects on this map. (Max is %d)  Ask Seth to fix it.",
				b2_maxProxies);

		} else
		{
	
			b2PolygonDef shapeDef;

			CollisionData *pColTemp = m_pCollisionData;
			CollisionData col;
			if (UsesTileProperties())
			{
				CreateCollisionDataWithTileProperties(this, col);
				pColTemp = &col;
			}
			line_list::iterator itor = pColTemp->GetLineList()->begin();

			for (; itor != pColTemp->GetLineList()->end(); itor++)
			{
				PointList * pPointList = &(*itor);
				CMaterial *pMat = g_materialManager.GetMaterial(pPointList->GetType());
				if (pMat->GetType() == CMaterial::C_MATERIAL_TYPE_DUMMY )		
				{
					continue;
				}

				pPointList->GetAsPolygonDef(&shapeDef);

				ShapeUserData *pShapeUserData;
				pShapeUserData = new ShapeUserData();
				pShapeUserData->pOwnerTile = this;
				pShapeUserData->pOwnerEnt = NULL;
				pShapeUserData->materialID = pPointList->GetType();
				shapeDef.userData = pShapeUserData;
				shapeDef.isSensor = false;
				shapeDef.filter.categoryBits = 0x0002; //tilepic category


				if (pMat->GetType() == CMaterial::C_MATERIAL_TYPE_NORMAL)
				{
					m_pBody->CreateShape(&shapeDef);
				} else
				{
					shapeDef.isSensor = true;
					m_pBody->CreateShape(&shapeDef);

				}

			}
		}

	}

	return m_pCollisionData;
}

const CL_Rect & TilePic::GetBoundsRect()
{
	/*
	static CL_Rect r(0,0,0,0);
	r.right = m_rectSrc.get_width()*m_vecScale.x;
	r.bottom = m_rectSrc.get_height()*m_vecScale.y;
	return r;
	*/

	CL_Rectf r(0, 0, m_rectSrc.get_width()*m_vecScale.x, m_rectSrc.get_height()*m_vecScale.y);

	r.apply_alignment(CL_Origin(m_origin), m_alignX, m_alignY);

	static CL_Pointf offset;
	offset = calc_origin(CL_Origin(m_origin), r.get_size());
	r -= offset;
	r -= offset;
	static CL_Rect ri;
	ri = CL_Rect(r);
	return ri;
}

Tile * TilePic::CreateClone()
{
	//required
	TilePic *pNew = new TilePic;
	CopyFromBaseToClone(pNew);

	//custom things to copy
	pNew->m_resourceID = m_resourceID;
	pNew->m_rectSrc = m_rectSrc;
	pNew->m_rot = m_rot;
	pNew->m_color = m_color;
	//LogMsg("Copying resource %u", m_resourceID);
	return pNew;
}

void TilePic::Serialize(CL_FileHelper &helper)
{
	//base class saves
	SerializeBase(helper);

	//save our own data
	helper.process(m_rectSrc);
	helper.process(m_resourceID);
	helper.process(m_rot);
}

CL_Rectf TilePic::GetWorldRect()
{
	CL_Rectf r(m_vecPos.x, m_vecPos.y, 
		m_vecPos.x + m_rectSrc.get_width()*m_vecScale.x, m_vecPos.y + m_rectSrc.get_height()*m_vecScale.y);

	r.apply_alignment(CL_Origin(m_origin), m_alignX, m_alignY);

	static CL_Pointf offset;
	offset = calc_origin(CL_Origin(m_origin), r.get_size());
	r -= offset;
	r -= offset;

	return r;
	

}



/*
Object: TilePic
TilePics are the simplest form of visual.

Low overheard but can only display a single image and must share collision data with other tiles of the same image and dimensions.

Not publicly accessible at this time.

*/
