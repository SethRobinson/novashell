
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 16:2:2006   9:19
*/


#ifndef TileEntity_HEADER_INCLUDED // include guard
#define TileEntity_HEADER_INCLUDED  // include guard

#include "Tile.h"

class TileEntity: public Tile
{
public:

	TileEntity();
	virtual ~TileEntity();

	virtual Tile * CreateClone();
	virtual void Serialize(CL_FileHelper &helper);
	virtual CL_Vector2 GetBoundsSize();
	virtual const CL_Rect & GetBoundsRect();
	virtual CL_Rectf GetWorldRect(); //like above but in floats
	virtual void SetScale(const CL_Vector2 &v);
	virtual CollisionData * GetCollisionData();
	void SetEntity(MovingEntity *pEnt);
	MovingEntity * GetEntity(){return m_pEntity;}
	virtual void Update(float step);
	virtual void Render(CL_GraphicContext *pGC);

	virtual CL_Vector2 GetPos();
	virtual void SetPos(const CL_Vector2 &vecPos);
	virtual b2Body * GetCustomBody();
	virtual void RenderShadow(CL_GraphicContext *pGC);
	virtual void GetAlignment(CL_Origin &origin, int &x, int &y);

protected:

	MovingEntity *m_pEntity;

};


#endif                  // include guard
