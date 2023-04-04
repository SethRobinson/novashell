#include "AppPrecomp.h"
#include "BrainShake.h"
#include "MovingEntity.h"

BrainShake registryInstanceShake(NULL); //self register ourselves in the brain registry

BrainShake::BrainShake(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_lastDisplacement = CL_Vector2(0,0);
}

BrainShake::~BrainShake()
{
}

void BrainShake::Update(float step)
{
	CL_Vector2 pos = m_pParent->GetPos();

	pos -= m_lastDisplacement; //undo the last displacement
	m_lastDisplacement = CL_Vector2(random_range(-10,11),random_range(-10,11));
	pos += m_lastDisplacement;

	//move it somewhere crazy
	m_pParent->SetPos(pos);
}

/*
Object: Shake
A brain for use with the <BrainManager> that causes its owner <Entity> to rapidly change position randomly.

It's not that useful right now, more a test of concept.

Parameters it understands:

None

Usage:
(code)
//let's set this entity to invisible, and then have it fade in over one second.
this:GetBrainManager():Add("Shake","");

//let's stop shaking in two seconds

Schedule(2000, this:GetID(), "this:GetBrainManager():Remove("Shake")");

(end)

*/

