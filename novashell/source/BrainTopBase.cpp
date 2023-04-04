#include "AppPrecomp.h"
#include "BrainTopBase.h"
#include "MovingEntity.h"

BrainTopBase registryInstanceBrainTopBase(NULL); //self register ourselves in the brain registry

BrainTopBase::BrainTopBase(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	SetSort(100); //always run last

	m_controlFilter = CL_Vector2(1,1); //full control
	m_airControlFilter = CL_Vector2(-1,-1);
}

BrainTopBase::~BrainTopBase()
{
}

void BrainTopBase::ResetForNextFrame()
{
	m_maxForce = m_pParent->GetMaxWalkSpeed();
	m_force = CL_Vector2::ZERO;
	m_DontMessWithVisual = false;
}

int BrainTopBase::HandleSimpleMessage(int message, int user1, int user2)
{
	if (message == C_BRAIN_MESSAGE_DONT_SET_VISUAL)
	{
		m_DontMessWithVisual = true;
	}

	return 0;
}

void BrainTopBase::AddWeightedForce(const CL_Vector2 & force)
{
	float magnitudeSoFar = m_force.length();
	float magnitudeRemaining = m_maxForce - magnitudeSoFar;

	if (magnitudeRemaining <= 0) return;

	float magnitudeToAdd = force.length();

	if (magnitudeToAdd < magnitudeRemaining)
	{
		m_force += force;
	} else
	{
		//only add part
		CL_Vector2 unitForce = force;
		unitForce.unitize();
		m_force += (unitForce * magnitudeRemaining);
	}

}

void BrainTopBase::OnAdd()
{
	m_pParent->GetBrainManager()->SetBrainBase(this);
	ResetForNextFrame();
}

void BrainTopBase::Update(float step)
{
	if (!m_pParent->GetBody())
	{
		LogMsg("StandardBase of %d (%s): Entity needs collision information to use StandardBase.", m_pParent->ID(), m_pParent->GetName().c_str());
		return;
	}
	CL_Vector2 curForce = m_pParent->GetLinearVelocity(); //figure out what needs to change to get our desired total force
	//LogMsg("Cur force is %s.  accel is %.2f", PrintVector(curForce).c_str(), m_pParent->GetAccel());

	m_force = m_force-curForce;

	float accel = m_pParent->GetAccel();
	Clamp(m_force.x, -accel, accel); //limit force to accel power
	Clamp(m_force.y, -accel, accel); //limit force to accel power

	if (!m_pParent->IsOnGround() && m_airControlFilter.x != -1)
	{
		m_force.x *= m_airControlFilter.x;
	
	} else
	{
		m_force.x *= m_controlFilter.x;
	}
	
	if (!m_pParent->IsOnGround() && m_airControlFilter.y != -1)
	{
		m_force.y *= m_airControlFilter.y;

	} else
	{
		m_force.y *= m_controlFilter.y;
	}

	m_pParent->RotateTowardsVectorDirection(m_pParent->GetVectorFacingTarget(), m_pParent->GetTurnSpeed() *step);
	
	//LogMsg("Adding force of %s", PrintVector(m_force* m_pParent->GetBody()->GetMass()).c_str());
	m_pParent->AddForceBurst(m_force);
	//m_pParent->GetBody()->SetLinearVelocity()

	if (!m_DontMessWithVisual)
	{
		m_pParent->SetSpriteByVisualStateAndFacing();		
	}
	
}


void BrainTopBase::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "control_filterx")
		{
			m_controlFilter.x = CL_String::to_float(words[1]);
		} 
		else if (words[0] == "control_filtery")
		{
			m_controlFilter.y = CL_String::to_float(words[1]);
		}  
		else if (words[0] == "air_control_filterx")
		{
			m_airControlFilter.x = CL_String::to_float(words[1]);
		} 
		else if (words[0] == "air_control_filtery")
		{
			m_airControlFilter.y = CL_String::to_float(words[1]);
		} 

				else
				{
					LogMsg("Brain %s doesn't understand keyword '%s'", GetName(), words[0].c_str());
				}
	}

}


void BrainTopBase::PostUpdate(float step)
{
	ResetForNextFrame();
}

/*
Object: StandardBase
A brain for use with the <BrainManager> that performs many generic functions such as movement and path-finding.

About:

This brain installs itself as a "base brain".  An <Entity> can only have one base brain active.

Parameters:

control_filterx - a number from 0 to 1 showing how much influence it should have for movement on this axis.  Default is 1, full influence.
control_filtery - a number from 0 to 1 showing how much influence it should have for movement on this axis.  Default is 1, full influence.
air_control_filterx - Like above, but only applicable to when the entity is not on the ground.  Ignored if not specifically set.
air_control_filtery - Like above, but only applicable to when the entity is not on the ground.  Ignored if not specifically set.

Usage:
(code)
this:GetBrainManager():Add("StandardBase", "");
(end)

*/