/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SETH_H
#define SETH_H

#include "../Framework/Render.h"

class MyFilter: public b2ContactFilter
{

	bool ShouldCollide(b2Shape* shape1, b2Shape* shape2)
	{

		const b2FilterData& filter1 = shape1->GetFilterData();
		const b2FilterData& filter2 = shape2->GetFilterData();
		printf("Checking col..");
		if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
		{
			return filter1.groupIndex > 0;
		}

		bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;
		return collide;
	}

};

class Seth : public Test
{
public:
	Seth()
	{
		
		// Ground body
		m_world->SetContactFilter(&m_contactFilter);
			
		{

		
		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(0, -10);
		b2Body * groundBody = m_world->CreateBody(&groundBodyDef);
		b2PolygonDef groundShapeDef;
		groundShapeDef.SetAsBox(50,10);
		groundBody->CreateShape(&groundShapeDef);

		}
	

		{
				b2BodyDef groundBodyDef;
				
			groundBodyDef.position.Set(0.0, 4);
			m_playerBody = m_world->CreateBody(&groundBodyDef);
			b2PolygonDef shapeDef;

			shapeDef.SetAsBox(1,1);
			shapeDef.density = 1;
			shapeDef.friction = 0.3;
			shapeDef.restitution = 0.3;
			//shapeDef.isSensor = true;
			m_playerBody ->CreateShape(&shapeDef);
			m_playerBody ->SetMassFromShapes();
		}


			/*		
			b2PolygonDef sd;
			sd.SetAsBox(50.0f, 10.0f);
			sd.friction = 0.3f;
			sd.filter.categoryBits = 0x0001;
			
			b2BodyDef bd;
			bd.position.Set(0.0f, -10.0f);
			b2Body* ground = m_world->CreateBody(&bd);
			ground->CreateShape(&sd);
			*/
		
	}

	
	void DestroyBody()
	{
		
	}
	void MovePlayer(b2Vec2 v)
	{
		//m_playerBody->SetXForm(m_playerBody->GetPosition()+v, m_playerBody->GetAngle());

		m_playerBody->ApplyForce( (200*v),m_playerBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));

	}
	void Keyboard(unsigned char key)
	{
		switch (key)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			
			break;

	
		case 'w':
				MovePlayer(b2Vec2(0,1));
				break;

				case 's':
					MovePlayer(b2Vec2(0,-1));
					break;
					case 'a':
						MovePlayer(b2Vec2(-1,0));
						break;
						case 'd':
							MovePlayer(b2Vec2(1,0));
							break;

		}
	}

	void Step(Settings* settings)
	{
		Test::Step(settings);
		DrawString(5, m_textLine, "Press 1-5 to drop stuff");
		m_textLine += 15;
	}

	static Test* Create()
	{
		return new Seth;
	}

	b2Body * m_playerBody;
	MyFilter m_contactFilter;
};

#endif
