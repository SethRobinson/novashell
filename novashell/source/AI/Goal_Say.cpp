#include "AppPrecomp.h"
#include "Goal_Say.h"
#include "MovingEntity.h"
#include "Goal_Types.h"
#include "message_types.h"
#include "TextManager.h"
#include "VisualProfileManager.h"



//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_Say::Goal_Say(MovingEntity* pBot,	const string &msg, int entID, int entToFaceID, int delayMS):

Goal<MovingEntity>(pBot, goal_say), m_text(msg), m_entID(entID), m_entToFaceID(entToFaceID),m_waitOverrideMS(delayMS)
{
	m_delayTimer = 0;
}

//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_Say::Activate()
{
	m_iStatus = active;
	

	if (m_delayTimer != 0)
	{
		//this was previously activated.  We were probably talking and got interrupted by
		//something else.  Instead of repeating what we said, let's just keep the old
		//timer..
		return;
	}
	//say it
	MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_entID);

	if (!pEnt)
	{

		LogMsg("Can't Say() from entity %d, it no longer exists", m_entID);
		m_iStatus = failed;
		return;
	}

	if (!m_text.empty())
	{
		
		m_delayTimer = GetApp()->GetGameTick()+g_textManager.Add(m_text, pEnt);
	} else
	{
		m_delayTimer = GetApp()->GetGameTick()+100;
	}

	if (m_delayTimer != 0 && m_entToFaceID != VisualProfile::FACING_NONE)
	{
	
		if (m_entToFaceID < VisualProfile::FACING_COUNT)
		{
			//they want to turn towards a specific direction
			pEnt->SetFacingTarget(m_entToFaceID);
		} else
		{
			//they want to look at something
			if (pEnt->IsOnSameMapAsEntityByID(m_entToFaceID))
			{
				pEnt->SetFacingTarget(   VectorToFacing(pEnt->GetVectorToEntityByID(m_entToFaceID)));
			}
		}
	}

	if (m_waitOverrideMS != -1)
	{
		m_delayTimer = GetApp()->GetGameTick()+m_waitOverrideMS;
	}

}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_Say::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	if (m_delayTimer < GetApp()->GetGameTick())
	{
		//done waiting
		m_iStatus = completed;
	}
	return m_iStatus;
}

//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_Say::Terminate()
{

	//LogMsg("Say got terminated (status: %d)", m_iStatus );
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_Say::Render()
{
}

void Goal_Say::LostFocus()
{
	m_iStatus = inactive;
	//instead of having this Say command repeated, let's just say it will be
	//skipped

}
bool Goal_Say::HandleMessage(const Message& msg)
{
	/*
	switch(msg.GetMsgType())
	{

	case C_MSG_SEEK_ARRIVED:

	m_iStatus = completed;
	return true;
	break;
	}
	*/
	//not handled
	return false;
}


