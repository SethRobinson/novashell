#include "AppPrecomp.h"
#include "Tile.h"
#include "Screen.h"
#include "CollisionData.h"
#include "MovingEntity.h"
#include "LayerManager.h"
#include "TileEntity.h"

TileEntity::TileEntity()
{
	m_type = C_TILE_TYPE_ENTITY;
	m_pEntity = NULL;
	//SetBit(Tile::e_notPersistent, true); //don't save this, we don't know how yet
	SetLayer(C_LAYER_ENTITY);
}

TileEntity::~TileEntity()
{
	SAFE_DELETE(m_pEntity);
}

void TileEntity::Serialize(CL_FileHelper &helper)
{
	if (!helper.IsWriting())
	{
		//we're loading so we have to do some extra init stuff
		this->SetEntity(new MovingEntity);
	}

	//base class saves
	SerializeBase(helper);

	assert(m_pEntity);
	//save our own data
	m_pEntity->Serialize(helper);
}

void TileEntity::Render(CL_GraphicContext *pGC)
{
	m_pEntity->Render(pGC);
}

void TileEntity::RenderShadow(CL_GraphicContext *pGC)
{
	m_pEntity->RenderShadow(pGC);
}

void TileEntity::SetScale(const CL_Vector2 &v)
{
	m_vecScale = v;
	m_pEntity->SetCollisionScale(v);
}

void TileEntity::Update(float step)
{
	m_pEntity->Update(step);
}

Tile * TileEntity::CreateClone()
{
	//required
	TileEntity *pNew = new TileEntity;
	CopyFromBaseToClone(pNew);

	//custom things to copy
	if (m_pEntity)
	{
		assert(m_pEntity);
		pNew->m_pEntity = (MovingEntity*) m_pEntity->CreateClone(pNew);
		assert(pNew->m_pEntity && "No cloning function created for this entity type yet");
	}
	return pNew;
}

CL_Vector2 TileEntity::GetBoundsSize()
{
	return CL_Vector2(m_pEntity->GetSizeX(),m_pEntity->GetSizeY());
}

const CL_Rect & TileEntity::GetBoundsRect()
{
	return m_pEntity->GetBoundsRect();
}

CL_Rectf TileEntity::GetWorldRect()
{
	return m_pEntity->GetWorldRect();
}

CL_Vector2 TileEntity::GetPos()
{
	assert(m_pEntity);
	return m_pEntity->GetPos();
}

void TileEntity::SetPos(const CL_Vector2 &vecPos)
{
	assert(m_pEntity);
	m_pEntity->SetPos(vecPos);
}

CollisionData * TileEntity::GetCollisionData()
{
	return m_pEntity->GetCollisionData();
}
void TileEntity::SetEntity(MovingEntity *pEnt)
{
	m_pEntity = pEnt;
	if (m_pEntity)	
	{
		m_pEntity->SetTile(this);
	}
}

b2Body * TileEntity::GetCustomBody()
{
	assert(m_pEntity);
	return m_pEntity->GetBody();
}

void TileEntity::GetAlignment( CL_Origin &origin, int &x, int &y )
{
	assert(m_pEntity);
	m_pEntity->GetAlignment(origin, x, y);
}

bool PixelAccurateHitDetection(CL_Vector2 vWorldPos, Tile *pTile)
{

	bool flippedx = false;
	bool flippedy = false;

	//get the real pixels, this is very slow because we're probably downloading from the video card
	//OPTIMIZE:  We could set most sprites to cache the pixelbuffer...
	
   CL_PixelBuffer pBuf;

   switch (pTile->GetType())
	{
	case C_TILE_TYPE_ENTITY:
		{
			MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();
			const CL_Sprite *pSprite = pEnt->GetSprite();
			
			if (pEnt->GetSizeOverride()) return true; //we've played with its size, would not be wise to fool with its internal bitmap data...

			if (!pSprite || pSprite->get_frame_count() == 0)
			{
				assert(!"Huh?");
				return false;
			}

			if (pSprite->get_base_angle() != 0)
			{
				//we don't know how to do this with rotation..
				return true;
			}

			if (pSprite->get_angle_pitch() != 0) flippedy = true;
			if (pSprite->get_angle_yaw() != 0) flippedx = true;
		
			
			pBuf = pSprite->get_frame_pixeldata(pSprite->get_current_frame());
			
		}
		break;

	default:
	//	assert(!"Unsupported tile type");
		return true; //always report a hit
	}

	
   //we now have the pixels and can do a hit test.  But we need to figure out where to check

   CL_Rectf r = pTile->GetWorldRect(); 

   vWorldPos.x -= r.left;
   vWorldPos.y -= r.top;

   //apply scale

   vWorldPos.x /= pTile->GetScale().x;
   vWorldPos.y /= pTile->GetScale().y;


   //LogMsg("FlippedX: %d, flippedY: %d", int(flippedx), int(flippedy));
   
   //if the image is flipped, we need to account for it

   if (flippedx)
   {
	   vWorldPos.x = pBuf.get_width()-vWorldPos.x;
   }
 
   if (flippedy)
   {
	   vWorldPos.y = pBuf.get_height()-vWorldPos.y;
   }

  //do the actual check

   CL_Surface c(pBuf);

   /*
   //visuals for debugging:
   c.draw(0,0);
   CL_Display::flip(); //show it now
   Sleep(200);
	LogMsg("Checking %s, Color is %s", PrintVector(vWorldPos).c_str(), ColorToString(pBuf.get_pixel(vWorldPos.x, vWorldPos.y)).c_str());
	*/

	pBuf.lock();
   if (pBuf.get_pixel(vWorldPos.x, vWorldPos.y).get_alpha() > 0)
   {
	   //LogMsg("Hit at %.2f, %.2f", vWorldPos.x, vWorldPos.y);
	   return true;
   } else
   {
	   //LogMsg("Miss");
   }
	pBuf.unlock();
  	return false;
}