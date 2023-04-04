#include "AppPrecomp.h"
#include "Goal_Delay.h"
#include "MovingEntity.h"
#include "Goal_Types.h"
#include "message_types.h"
#include "Goal_Composite.h"

//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_Delay::Goal_Delay(MovingEntity* pBot,	int time):

Goal<MovingEntity>(pBot, goal_delay), m_timeToWait(time)
{
	m_delayTimer = 0;

}


//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_Delay::Activate()
{
	m_iStatus = active;
	m_delayTimer = GetApp()->GetGameTick()+m_timeToWait;
}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_Delay::Process()
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
void Goal_Delay::Terminate()
{
}

void Goal_Delay::LostFocus()
{
	m_iStatus = inactive;	
	//adjust time to wait by how long we've already waited
	m_timeToWait -= ( int(GetApp()->GetGameTick()+m_timeToWait)) - int(m_delayTimer);
}
//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_Delay::Render()
{
}

bool Goal_Delay::HandleMessage(const Message& msg)
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


