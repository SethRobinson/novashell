#include "AppPrecomp.h"
#include "BrainScale.h"
#include "MovingEntity.h"

BrainScale registryInstanceBrainScale(NULL); //self register ourselves i nthe brain registry

BrainScale::BrainScale(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_scaleSpeedMS = 1000;
	m_vScaleTarget = CL_Vector2(0.1f,0.1f);
	m_vStartScale = pParent->GetScale();
	m_timeCreated = GetApp()->GetGameTick();
	m_deleteEntity = false;
}

BrainScale::~BrainScale()
{
}

void BrainScale::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "scale_speed_ms")
		{
			m_scaleSpeedMS = CL_String::to_int(words[1]);
			
		} else
		if (words[0] == "scalex")
		{
			m_vScaleTarget.x = CL_String::to_float(words[1]);
		} else
		if (words[0] == "scaley")
		{
			m_vScaleTarget.y = CL_String::to_float(words[1]);
		} else
		if (words[0] == "delete_entity")
		{
			m_deleteEntity = CL_String::to_bool(words[1]);
		} 
		else
		{
			LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
		}
	}

}

void BrainScale::Update(float step)
{
	//set alpha for fading
	int timeLeft = (m_timeCreated+ m_scaleSpeedMS) - GetApp()->GetGameTick();

	if (timeLeft > 0)
	{
		float pos = 1-( float(timeLeft) / float(m_scaleSpeedMS));
		m_pParent->SetScale(Lerp(m_vStartScale, m_vScaleTarget, pos));

	} else
	{
		//we're done
		m_pParent->SetScale(m_vScaleTarget); //make sure it's exactly right
		this->SetDeleteFlag(true);
		if (m_deleteEntity) m_pParent->SetDeleteFlag(true);
	}
}


/*
Object: Scale
A brain for use with the <BrainManager> that causes its owner <Entity> to scale to a specific size then optionally delete the <Entity> completely.

Note:

This automatically deletes its own brain when the scale is complete and optionally deletes the entity as well.

Parameters it understands:

scale_speed_ms - How many milliseconds the scale should take.  Default is 1000. (one second)
scalex - the desired final x scale.
scaley - the desired final y scale.
delete_entity - if true, the entire entity will be deleted when the scale is complete.

Usage:
(code)
//scale down to nothing and then delete itself
this:GetBrainManager():Add("Scale", "scalex=0.1;scaley=0.1;scale_speed_ms=500;delete_entity=true");
(end)

*/