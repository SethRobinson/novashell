#include "AppPrecomp.h"
#include "Goal_RunScript.h"
#include "MovingEntity.h"
#include "Goal_Types.h"
#include "message_types.h"

//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_RunScript::Goal_RunScript(MovingEntity* pBot,	const string &scriptCommand):

Goal<MovingEntity>(pBot, goal_run_script), m_scriptCommand(scriptCommand)
{

}


//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_RunScript::Activate()
{
	m_iStatus = active;
}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_RunScript::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	//there are a lot of ugly issues to worry about if we actually let scripts run
	//from within the goal system, because scripts can add/delete goals.

	//So instead, we run it outside, and set this up to continue on the next update tick.
	if (!m_scriptCommand.empty())
	{
		m_pOwner->SetRunStringASAP(m_scriptCommand.c_str());
		m_scriptCommand.clear(); //so we know not to run it again
	} else
	{
		//we already ran it I guess
		m_iStatus = completed;
	}

	return m_iStatus;
}

//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_RunScript::Terminate()
{
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_RunScript::Render()
{
}

bool Goal_RunScript::HandleMessage(const Message& msg)
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


