//  ***************************************************************
//  PhysicsManager - Creation date: 06/02/2008
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2008 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef PhysicsManager_h__
#define PhysicsManager_h__

#include "Box2D/Include/Box2D.h"
#include "DebugDraw.h"
#define SetBitSimple(n) (0x01<<(n))

class Tile;
class MovingEntity;


enum ContactState
{
	e_contactAdded,
	e_contactPersisted,
	e_contactRemoved,
};


struct ContactPoint
{
	b2Shape* shape1;
	b2Shape* shape2;
	b2Vec2 normal;
	b2Vec2 position;
	b2Vec2 velocity;
	b2ContactID id;
	float separation;
	ContactState state;

	//information about shape 2, which is assumed to be the 'other' guy
	int materialID;
	Tile * pOwnerTile; //null if not a tile
	MovingEntity *pOwnerEnt; //null if shape2 didn't come from an ent

};


class ContactListener : public b2ContactListener
{
public:
	void Add(const b2ContactPoint* point);
	void Persist(const b2ContactPoint* point);
	void Remove(const b2ContactPoint* point);

private:
	void SetupAndSendContactInfo(ContactState contactState, ContactPoint &cp, const b2ContactPoint *point);
	
	ContactPoint m_cp;

};

//yes, changing this will break all game worlds.
const float C_PHYSICS_PIXEL_SIZE = 32; //1 box2d unit = C_PHYSICS_PIXEL_SIZE pixels (at 1 1 zoom)

class Map;

class ContactFilter: public b2ContactFilter
{
public:
	virtual bool ShouldCollide(b2Shape* shape1, b2Shape* shape2);

};


class DestructionListener: public b2DestructionListener
{

	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	virtual void SayGoodbye(b2Joint* joint);

	/// Called when any shape is about to be destroyed due
	/// to the destruction of its parent body.
	virtual void SayGoodbye(b2Shape* shape);

};
class PhysicsManager
{
public:
	PhysicsManager();
	virtual ~PhysicsManager();

	void Kill();
	bool Init(Map *pMap);
	b2World * GetBox2D() {return m_pWorld;}
	void SetDrawDebug(bool bNew);
	void Update(float step);
	void SetGravity(CL_Vector2 vGrav);
	void DrawShape(b2Shape* shape, const b2XForm& xf, const b2Color& color, bool core);
protected:
	

private:

	Map *m_pParent;
	b2AABB m_worldAABB;
	b2World * m_pWorld;
	DebugDraw m_debugDraw;
	ContactListener m_contactListener;
	ContactFilter m_contactFilter;
	DestructionListener m_destructionLister;

};

b2Vec2 ToPhysicsSpace(CL_Vector2 v);
CL_Vector2 FromPhysicsSpace(b2Vec2 v);
float ToPhysicsSpace( float f );
float FromPhysicsSpace( float f );
void UpdateBodyFilterData(b2Body *pBody, const b2FilterData &filterData);
void SetCollisionMaskBit(b2FilterData &filterData, int bitNum, bool bOn);
void SetCollisionCategoryBit(b2FilterData &filterData, int bitNum, bool bOn);


#endif // PhysicsManager_h__