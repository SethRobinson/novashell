#include "AppPrecomp.h"
#include "PhysicsManager.h"
#include "Map.h"
#include "GameLogic.h"
#include "MovingEntity.h"

PhysicsManager::PhysicsManager()
{
	m_pWorld = NULL;
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::Kill()
{
	SAFE_DELETE(m_pWorld);
}

bool PhysicsManager::Init( Map *pMap )
{
	Kill();
	
	m_pParent = pMap;
	m_worldAABB.lowerBound.Set(-500.0f, -500.0f);
	m_worldAABB.upperBound.Set(500.0f, 500.0f);

	m_pWorld = new b2World(m_worldAABB, b2Vec2(0, pMap->GetGravity()), true);

	m_pWorld->SetDebugDraw(&m_debugDraw);

	m_pWorld->SetContactListener(&m_contactListener);
	m_pWorld->SetContactFilter(&m_contactFilter);
	m_pWorld->SetDestructionListener(&m_destructionLister);


	return true;
}


void PhysicsManager::DrawShape(b2Shape* shape, const b2XForm& xf, const b2Color& color, bool core)
{
	b2Color coreColor(0.9f, 0.6f, 0.6f);

	switch (shape->GetType())
	{
	case e_circleShape:
		{
			b2CircleShape* circle = (b2CircleShape*)shape;

			b2Vec2 center = b2Mul(xf, circle->GetLocalPosition());
			float32 radius = circle->GetRadius();
			b2Vec2 axis = xf.R.col1;

			m_debugDraw.DrawSolidCircle(center, radius, axis, color);

			if (core)
			{
				m_debugDraw.DrawCircle(center, radius - b2_toiSlop, coreColor);
			}
		}
		break;

	case e_polygonShape:
		{
			b2PolygonShape* poly = (b2PolygonShape*)shape;
			int32 vertexCount = poly->GetVertexCount();
			const b2Vec2* localVertices = poly->GetVertices();

			b2Assert(vertexCount <= b2_maxPolygonVertices);
			b2Vec2 vertices[b2_maxPolygonVertices];

			for (int32 i = 0; i < vertexCount; ++i)
			{
				vertices[i] = b2Mul(xf, localVertices[i]);
			}

			m_debugDraw.DrawSolidPolygon(vertices, vertexCount, color);

			if (core)
			{
				const b2Vec2* localCoreVertices = poly->GetCoreVertices();
				for (int32 i = 0; i < vertexCount; ++i)
				{
					vertices[i] = b2Mul(xf, localCoreVertices[i]);
				}
				m_debugDraw.DrawPolygon(vertices, vertexCount, coreColor);
			}
		}
		break;
	}
}

void PhysicsManager::SetDrawDebug( bool bNew )
{
	return;
	uint32 flags = 0;

	if (bNew)
	{
		flags += b2DebugDraw::e_shapeBit;
		
		//flags += b2DebugDraw::e_jointBit;
		//flags += b2DebugDraw::e_coreShapeBit;
		//flags +=  b2DebugDraw::e_aabbBit;
		//flags +=  b2DebugDraw::e_obbBit;
		//flags +=  b2DebugDraw::e_pairBit;
		//flags +=  b2DebugDraw::e_centerOfMassBit;
		
	}

	m_debugDraw.SetFlags(flags);
}


void PhysicsManager::SetGravity( CL_Vector2 vGrav )
{
	if (m_pWorld)
	{
		m_pWorld->SetGravity(b2Vec2(vGrav.x, vGrav.y));
	}
}

b2Vec2 ToPhysicsSpace( CL_Vector2 v )
{
	return  *(b2Vec2*) &(v / C_PHYSICS_PIXEL_SIZE);
}

CL_Vector2 FromPhysicsSpace( b2Vec2 v )
{
return  *(CL_Vector2*) &(C_PHYSICS_PIXEL_SIZE * v);
}


float ToPhysicsSpace( float f )
{
	return  f / C_PHYSICS_PIXEL_SIZE;
}

float FromPhysicsSpace( float f )
{
	return  C_PHYSICS_PIXEL_SIZE * f;
}



void SetCollisionCategoryBit(b2FilterData &filterData, int bitNum, bool bOn)
{
	assert(bitNum >= 0 && bitNum < 16);
		if (bOn)
			{
				//set it
				filterData.categoryBits  |= SetBitSimple(bitNum);

			} else
			{
				//clear it
				filterData.categoryBits &= ~SetBitSimple(bitNum);
			}
}

void UpdateBodyFilterData(b2Body *pBody, const b2FilterData &filterData)
{
	b2Shape *pShape = pBody->GetShapeList();
		for (;pShape; pShape = pShape->GetNext())
	{
		pShape->SetFilterData(filterData);
		pBody->GetWorld()->Refilter(pShape);
	}
}



void SetCollisionMaskBit(b2FilterData &filterData, int bitNum, bool bOn)
{
	
	assert(bitNum >= 0 && bitNum < 16);
	if (bOn)
	{
		//set it
		filterData.maskBits |= SetBitSimple(bitNum);

	} else
	{
		//clear it
		filterData.maskBits &= ~SetBitSimple(bitNum);
	}
}

void ContactListener::SetupAndSendContactInfo(ContactState contactState, ContactPoint &cp, const b2ContactPoint *point)
{
	cp.state = contactState;
	cp.position = point->position;
	cp.normal = point->normal;
	cp.id = point->id;
	cp.separation = -point->separation;
	cp.velocity  = point->velocity;
	ShapeUserData *pShape1UserData = (ShapeUserData*)point->shape1->GetUserData();
	ShapeUserData *pShape2UserData = (ShapeUserData*)point->shape2->GetUserData();


	const b2FilterData& filter1 = point->shape1->GetFilterData();
	const b2FilterData& filter2 = point->shape2->GetFilterData();
	
	if (contactState == e_contactRemoved)
	{
		//well, the entity inside is invalid, need a way to deal with that before we try to handle these messages.
		return;
	}
	

	if (!point->shape1->IsSensor()  && !point->shape2->IsSensor())
	{
		//we can use this to tell an entity if he's on the ground or not
		if (pShape1UserData->pOwnerEnt)
		{
			//first this guy
			if ( (-1*cp.normal).y < -0.7f) //higher # here means allows a stronger angled surface to count as "ground"
			{
				//LogMsg("%s Touched ground of some sort", GetName().c_str());
				pShape1UserData->pOwnerEnt->SetIsOnGround(true);
			}
		}

		if (pShape2UserData->pOwnerEnt)
		{

			//first this guy
			if ( cp.normal.y < -0.7f) //higher # here means allows a stronger angled surface to count as "ground"
			{
				//LogMsg("%s Touched ground of some sort", GetName().c_str());
				pShape2UserData->pOwnerEnt->SetIsOnGround(true);
			}
		}

	}

	
	//LogMsg("Checking col..");
	/*
	if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
	{
		return filter1.groupIndex > 0;
	}
	*/

	//bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;

	if (pShape2UserData->pOwnerEnt)
	{
		if (pShape2UserData->pOwnerEnt->GetCollisionListenBits() & filter1.categoryBits)
		{
		cp.shape1 = point->shape2;
		cp.shape2 = point->shape1;
		cp.pOwnerTile = pShape1UserData->pOwnerTile;
		cp.pOwnerEnt = pShape1UserData->pOwnerEnt;
		cp.materialID = pShape1UserData->materialID;
		
		//if it passed our criteria, let's send the contact

		pShape2UserData->pOwnerEnt->AddContact(cp);
		}
	}

	if (pShape1UserData->pOwnerEnt)
	{
		if (pShape1UserData->pOwnerEnt->GetCollisionListenBits() & filter2.categoryBits)
		{
		cp.shape1 = point->shape1;
		cp.shape2 = point->shape2;
		cp.normal = -1*cp.normal;
		cp.velocity = -1*cp.velocity;
		//cp.separation = -1*cp.separation;
		cp.pOwnerTile = pShape2UserData->pOwnerTile;
		cp.pOwnerEnt = pShape2UserData->pOwnerEnt;
		cp.materialID = pShape2UserData->materialID;

		pShape1UserData->pOwnerEnt->AddContact(cp);
		}
	}

}

void ContactListener::Add(const b2ContactPoint* point)
{
	SetupAndSendContactInfo(e_contactAdded, m_cp, point);	
}

void ContactListener::Persist(const b2ContactPoint* point)
{
	SetupAndSendContactInfo(e_contactPersisted, m_cp, point);
}

void ContactListener::Remove(const b2ContactPoint* point)
{
	SetupAndSendContactInfo(e_contactRemoved, m_cp, point);	
}

bool ContactFilter::ShouldCollide(b2Shape* shape1, b2Shape* shape2)
{

	if (shape1->IsSensor() || shape2->IsSensor())
	{
		//sensors are never aborted, because they aren't real collisions, just for information
		return true;
	}

	const b2FilterData& filter1 = shape1->GetFilterData();
	const b2FilterData& filter2 = shape2->GetFilterData();
	//LogMsg("Checking col..");
	if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
	{
		return filter1.groupIndex > 0;
	}

	bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;
	
	return collide;
}


void DestructionListener::SayGoodbye( b2Joint* joint ) 
{
	
}

void DestructionListener::SayGoodbye( b2Shape* shape )
{
	delete shape->GetUserData();
}

void PhysicsManager::Update( float step )
{
		if (!GetGameLogic()->GetGamePaused())
	{
		//LogMsg("Running physics round..");
		float stepSize = GetApp()->GetGameLogicSpeed()/1000;
		//LogMsg("Step is %.2f", stepSize);
		m_pWorld->Step(stepSize, 10);

#ifdef _DEBUG
	m_pWorld->Validate();
#endif

	for (b2Body* pBody = m_pWorld->GetBodyList(); pBody; pBody = pBody->GetNext())
	{
		if (pBody->GetUserData())
			if (!pBody->IsSleeping())
			{
				g_pMapManager->AddToEntityUpdateList((MovingEntity*)pBody->GetUserData());
			}
	}


		//if (m_debugDraw.GetFlags() != 0)
		//CL_Display::flip(-1); //show it now
	} else
	{

		//if (m_debugDraw.GetFlags() != 0)
		{
			//run simulation just to draw the debug stuff
			//m_pWorld->Step(0, 0);

		}
	
	}

}