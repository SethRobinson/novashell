#include "AppPrecomp.h"
#include "Goal_SeekToPosition.h"
#include "../Brain.h"
#include "../BrainManager.h"
#include "message_types.h"
//#include "SteeringBehaviors.h"
//#include "PathPlanner.h"


//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_SeekToPosition::Goal_SeekToPosition(MovingEntity* pBot,
                                         CL_Vector2   target):

                                Goal<MovingEntity>(pBot,
                                                goal_seek_to_position),
                                 m_vPosition(target)
{

}

                                             
//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_SeekToPosition::Activate()
{
  m_iStatus = active;
  
 
  //record the time the bot starts this goal
  m_startTime = GetApp()->GetGameTick();
  
  //This value is used to determine if the bot becomes stuck 
  m_timeToReachPos = m_pOwner->CalculateTimeToReachPosition(m_vPosition);
  
  //factor in a margin of error for any reactive behavior
  const unsigned int MarginOfError = 500; //in gameticks, which is about MS

  m_timeToReachPos += MarginOfError;
  
  m_pOwner->GetBrainManager()->Add("TopSeek", "target=" + VectorToString(&m_vPosition));

 // m_pOwner->GetSteering()->SeekOn();
}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_SeekToPosition::Process()
{
  //if status is inactive, call Activate()
  ActivateIfInactive();
    
  //test to see if the bot has become stuck
  if (isStuck())
  {
	  m_pOwner->HandleMessage(Message(C_MSG_GOT_STUCK));

	  m_iStatus = failed;
  }
 
  return m_iStatus;
}

//--------------------------- isBotStuck --------------------------------------
//
//  returns true if the bot has taken longer than expected to reach the 
//  currently active waypoint
//-----------------------------------------------------------------------------
bool Goal_SeekToPosition::isStuck()const
{  
	double TimeTaken = GetApp()->GetGameTick() - m_startTime;

	if (TimeTaken > m_timeToReachPos)
	{
		if (GetGameLogic()->GetShowPathfinding())
			LogMsg("Seek: Entity %d (%s) appears to be stuck (%.0f MS timeout reached)",m_pOwner->ID(), 
			m_pOwner->GetName().c_str(), TimeTaken);
	
		return true;
	}

	return false;
}



//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_SeekToPosition::Terminate()
{
 // m_pOwner->GetSteering()->SeekOff();
//  m_pOwner->GetSteering()->ArriveOff();

	Brain *pBrain = m_pOwner->GetBrainManager()->GetBrainByName("TopSeek");

	if (pBrain)
	{
		pBrain->SetDeleteFlag(true);
	} else
	{
		LogError("Goal_SeekToPosition: Unable to delete TopSeek");
	}

  m_iStatus = completed;
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_SeekToPosition::Render()
{
  
	/*
	if (m_iStatus == active)
  {
    gdi->GreenBrush();
    gdi->BlackPen();
    gdi->Circle(m_vPosition, 3);
  }

  else if (m_iStatus == inactive)
  {

   gdi->RedBrush();
   gdi->BlackPen();
   gdi->Circle(m_vPosition, 3);
  }
  */
}

//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
bool Goal_SeekToPosition::HandleMessage(const Message& msg)
{

	switch(msg.GetMsgType())
	{

	case C_MSG_SEEK_ARRIVED:

		m_iStatus = completed;
		return true;
		break;
	}

	//not handled
	return false;
}


