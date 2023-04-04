#include "AppPrecomp.h"
#include "BrainBob.h"
#include "MovingEntity.h"

BrainBob registryInstanceBrainBob(NULL); //self register ourselves i nthe brain registry

BrainBob::BrainBob(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_lastDisplacement = CL_Vector2(0,0);

	m_bobDistance = 5;
	m_bobSpeed = 1;
	m_timer.SetInterval(100);
}

BrainBob::~BrainBob()
{
}


void BrainBob::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "distance")
		{
			m_bobDistance = CL_String::to_float(words[1]);
		} 
		else
			if (words[0] == "speed")
			{
				m_bobSpeed = CL_String::to_float(words[1]);
			} 
			else
				if (words[0] == "visual_interval_ms")
				{
					m_timer.SetInterval(CL_String::to_int(words[1]));
				} 
				else
				{
			LogMsg("Brain %s doesn't understand keyword '%s'", GetName(), words[0].c_str());
		}
	}

}


void BrainBob::Update(float step)
{
	if (m_timer.IntervalReached())
	{

		
		CL_Vector2 pos;
		
		if (m_pParent->GetAttachEntityID() != C_ENTITY_NONE)
		{
			pos = m_pParent->GetAttachOffset();
		} else
		{
			pos = m_pParent->GetPos();
		}

		pos -= m_lastDisplacement; //undo the last displacement
		float bobAmount = sin( (double(GetApp()->GetGameTick()) /100) * m_bobSpeed);
		bobAmount *= m_bobDistance;
		m_lastDisplacement = CL_Vector2(0, bobAmount);
		pos += m_lastDisplacement;

		if (m_pParent->GetAttachEntityID() != C_ENTITY_NONE)
		{
			m_pParent->SetAttachOffset(pos);
		} else
		{
			m_pParent->SetPos(pos);
		}
	}
}


/*
Object: Bob
A brain for use with the <BrainManager> that causes its <Entity>'s visuals to "bob" up and down, without really moving the <Entity>.

Parameters it understands:

distance - The total offset of the bobbing movement.
speed - How fast the bobbing is.
visual_interval_ms - How jerky/smooth the bobbing movement is.

Usage:
(code)
this:GetBrainManager():Add("Bob","distance=5;speed=2;visual_interval_ms=10");

//if later, we wanted to change functionality as it's running, we could do...
this:GetBrainManager():SendToBrainByName("Bob", "speed=2.5");
(end)

*/