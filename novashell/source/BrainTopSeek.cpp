#include "AppPrecomp.h"
#include "BrainTopSeek.h"
#include "MovingEntity.h"
#include "State.h"
#include "AI/message_types.h"

BrainTopSeek registryInstanceBrainTopSeek(NULL); //self register ourselves in the brain registry

#define C_DEFAULT_ARRIVE_PADDING 10; //in pixels

BrainTopSeek::BrainTopSeek(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_vecTarget = CL_Vector2::ZERO;
	m_bUsingTarget = false;
	m_padding = C_DEFAULT_ARRIVE_PADDING;
	//LogMsg("Top seek added");
}

BrainTopSeek::~BrainTopSeek()
{
}

void BrainTopSeek::OnRemove()
{
	//LogMsg("TopSeek removed");
	if (m_pParent->GetVisualState() == VisualProfile::VISUAL_STATE_WALK || m_pParent->GetVisualState() == VisualProfile::VISUAL_STATE_IDLE)
	{
		m_pParent->GetBrainManager()->SetStateByName("Idle");
	}
	
}

void BrainTopSeek::Update(float step)
{
	
	if (m_bUsingTarget)
	{
		CL_Vector2 v(m_vecTarget-m_pParent->GetPos());

		float dist = v.length();
		v.unitize();

		m_pParent->SetVectorFacingTarget(v);

		if (dist < m_padding)
		{
			//close enough
			m_pParent->HandleMessage(Message(C_MSG_SEEK_ARRIVED));
		}

	}

	if (!m_pParent->GetBrainManager()->GetState() || m_pParent->GetBrainManager()->GetState()->GetName() != "Walk")
	{
		m_pParent->GetBrainManager()->SetStateByName("Walk");
	}

}

void BrainTopSeek::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "target")
		{
			m_vecTarget = StringToVector(words[1]);
			m_bUsingTarget = true;
		} else
			if (words[0] == "padding")
			{
				m_padding = CL_String::to_float(words[1]);
			} else
			{
			LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
		}
	}

}

