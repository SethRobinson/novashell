
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 28:12:2005   11:45
*/


#ifndef Tile_HEADER_INCLUDED // include guard
#define Tile_HEADER_INCLUDED  // include guard

#include "misc/CBit8.h"
#include "AI/NodeTypeEnumerations.h"

class Screen;
class MovingEntity;
class CollisionData;
class CL_Surface;
class b2Body;

enum
{
	//only add to the BOTTOM!!
	C_TILE_TYPE_BLANK = 0, //used as a place holder in some edit operations
	C_TILE_TYPE_PIC,
	C_TILE_TYPE_REFERENCE, //not a real tile, but a ghost pointing to the real one.
	C_TILE_TYPE_ENTITY,
	C_TILE_TYPE_COUNT

};

class Tile
{
public:

    Tile();
	virtual ~Tile();
	cl_uint8 GetType() {return m_type;}
	const cl_uint8 GetLayer() const {return m_layer;}
	void SetLayer(cl_uint8 layer) {m_layer = layer;}
	bool GetBit(cl_uint32 bit) {return m_bitField.get_bit(bit);}
	void SetBit(cl_uint32 bit, bool bNew) {m_bitField.set_bit(bit, bNew);}
	const CL_Vector2 & GetScale() {return m_vecScale;}
	virtual void SetScale(const CL_Vector2 &v);
	virtual CL_Vector2 GetPos() {return m_vecPos;}
	CL_Vector2 GetPosSafe() {return GetPos();}
	virtual void SetPos(const CL_Vector2 &vecPos) {m_vecPos = vecPos;}
	bool IsEdgeCase() {return m_pEdgeCaseList != NULL;}
	virtual Tile * CreateClone(); //must be handled in derived class
	Tile * CopyFromBaseToClone(Tile *pNew);
	virtual const CL_Rect & GetBoundsRect();

	virtual void Serialize(CL_FileHelper &helper){}; //must be handled in derived class
	virtual void SerializeBase(CL_FileHelper &helper);
	virtual CL_Vector2 GetBoundsSize() {return CL_Vector2(64,64);}
	CL_Rect GetWorldRectInt();
	virtual CL_Rectf GetWorldRect() {return CL_Rectf(m_vecPos.x, m_vecPos.y, m_vecPos.x+64,m_vecPos.y+64);}
	const CL_Rectf & GetWorldColRect();
	CL_Rectf GetWorldCombinedRect(); //returns the collision and image rect combined
	CL_Rect GetWorldCombinedRectInt(); //returns the collision and image rect combined

	Tile * CreateReference(Screen *pScreen);
	void RemoveReference(Tile *pTileRef);
	Tile * GetTileWereAReferenceFrom(){return m_pFatherTile;}
	unsigned int GetLastScanID() {return m_lastScanID;};
	void SetLastScanID(unsigned int scanID) {m_lastScanID = scanID;}
	virtual CollisionData * GetCollisionData() {return NULL; } //by default tiles don't have collision data
	bool UsesTileProperties();
	virtual void Update(float step) {return;}
	virtual void Render(CL_GraphicContext *pGC) {return;}
	virtual void RenderShadow(CL_GraphicContext *pGC) {return;}
	Screen * GetParentScreen();
	void SetParentScreen(Screen *pScreen) {m_pParentScreen = pScreen;}
	virtual b2Body * GetCustomBody() {return NULL;}
	void SetColor(const CL_Color color) {m_color = color;}
	CL_Color GetColor() {return m_color;}
	int GetGraphNodeID() {return m_graphNodeID;}
	void SetGraphNodeID(int id) { /*assert(m_graphNodeID == invalid_node_index);*/ m_graphNodeID = id;}
	CBit32 GetBitField() {return m_bitField;}
	virtual void GetAlignment(CL_Origin &origin, int &x, int &y);
	virtual void SetAlignment(CL_Origin origin, int &x, int &y);
	void GetBaseAlignment(CL_Origin &origin, int &x, int &y);

	enum
{
 e_flippedX = D_BIT_0,
 e_flippedY = D_BIT_1,
 e_customCollision = D_BIT_2,
 e_needsUpdate = D_BIT_3, //not used?
 e_notPersistent = D_BIT_4, //if true, won't be saved to disk
 e_castShadow = D_BIT_5,
 e_sortShadow = D_BIT_6,
 e_pathNode = D_BIT_7,
 e_linearFilter = D_BIT_8 // if true, will use linear filter when scaling

 //can add more bits here, up to D_BIT_31
};

protected:
	
	cl_uint8 m_type;

	//this data is around for every tile type whether you need it or not, but actually gets saved and
	//loaded in subclasses
	CL_Vector2 m_vecPos; //where we're located if applicable
	CL_Vector2 m_vecScale;
	
	unsigned int m_lastScanID; //helps to avoid duplicates when doing certain kinds of scans
	std::list<Tile*> *m_pEdgeCaseList; //list of references a tile has
	Tile *m_pFatherTile; //if a reference, this holds its father
	Screen *m_pParentScreen; //only valid sometimes, like for reference tiles, not kept when
	//making clones and things
	cl_uint8 m_layer; //higher means on top
	CBit32 m_bitField;
	CollisionData * m_pCollisionData; //null if it hasn't been researched yet
	CL_Color m_color;
	bool m_bUsingCustomCollision; //if a tilepic is scaled weird, it needs to create it's own custom collision
	int m_graphNodeID; //only used if the path_node flag is set.  Not saved.
	int m_alignX,m_alignY;
	cl_uint8 m_origin;
};

class TilePic: public Tile
{
public:

	TilePic()
	{
		m_type = C_TILE_TYPE_PIC;
		m_rot = 0;
		m_pBody = NULL;
		m_origin = origin_top_left;
	}
	virtual ~TilePic();
	
	virtual Tile * CreateClone();

	virtual void Serialize(CL_FileHelper &helper);
	void SaveToMasterCollision(); 
	void ForceUsingCustomCollisionData();
	virtual b2Body * GetCustomBody() {return m_pBody;}

	void ReinitCollision();
	virtual const CL_Rect & GetBoundsRect();
	//TODO optimize these to be precached?

	virtual CL_Vector2 GetBoundsSize() {return CL_Vector2(m_rectSrc.get_width()*m_vecScale.x, m_rectSrc.get_height()*m_vecScale.y);}
	virtual CL_Rectf GetWorldRect();
	virtual void SetScale(const CL_Vector2 &v);
	virtual CollisionData * GetCollisionData();
	virtual void Render(CL_GraphicContext *pGC);
	virtual void RenderShadow(CL_GraphicContext *pGC);
	void KillBody();
	unsigned int m_resourceID;
	CL_Rect m_rectSrc;
	float m_rot; //rotation
	b2Body *m_pBody; //NULL if not using collision


};

bool PixelAccurateHitDetection(CL_Vector2 vWorldPos, Tile *pTile);

#endif                  // include guard
