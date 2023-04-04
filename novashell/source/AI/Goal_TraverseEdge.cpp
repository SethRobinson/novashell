#include "AppPrecomp.h"
#include "Goal_TraverseEdge.h"
#include "MovingEntity.h"
#include "Goal_Types.h"
#include "PathPlanner.h"
#include "Brain.h"
#include "message_types.h"

//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_TraverseEdge::Goal_TraverseEdge(MovingEntity* pBot,
                                     PathEdge   edge,
                                     bool       LastEdge):

                                Goal<MovingEntity>(pBot, goal_traverse_edge),
                                m_Edge(edge),
                                m_timeExpected(0),
                                m_bLastEdgeInPath(LastEdge)
                                
{

}


//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_TraverseEdge::Activate()
{
  m_iStatus = active;
  
  //the edge behavior flag may specify a type of movement that necessitates a 
  //change in the bot's max possible speed as it follows this edge
  
  /*
  switch(m_Edge.Behavior())
  {
    
  case NavGraphEdge::swim:
    {
      m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSwimmingSpeed"));
    }
   
    break;
   
    case NavGraphEdge::crawl:
    {
       m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxCrawlingSpeed"));
    }
   
    break;
  }
  */


  //record the time the bot starts this goal
  m_startTime = GetApp()->GetGameTick();
  
  //calculate the expected time required to reach the this waypoint. This value
  //is used to determine if the bot becomes stuck 
  m_timeExpected = m_pOwner->CalculateTimeToReachPosition(m_Edge.Destination());
  
  //LogMsg("Should take: %u", m_timeExpected);
  //factor in a margin of error for any reactive behavior
  static const double MarginOfError = 1000; //one second

  m_timeExpected += MarginOfError;

  //set the steering target
  m_pOwner->GetBrainManager()->Add("TopSeek", "target=" + VectorToString(&m_Edge.Destination()));

  //Set the appropriate steering behavior. If this is the last edge in the path
  //the bot should arrive at the position it points to, else it should seek
  if (m_bLastEdgeInPath)
  {
    // m_pOwner->GetSteering()->ArriveOn();
  }
}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_TraverseEdge::Process()
{
  //if status is inactive, call Activate()
  ActivateIfInactive();
  
  //if the bot has become stuck return failure
  if (isStuck())
  {
	  m_pOwner->HandleMessage(Message(C_MSG_GOT_STUCK));

	  //LogMsg("Retry");
	  m_iStatus = failed;
  }
 
  return m_iStatus;
}

//--------------------------- isBotStuck --------------------------------------
//
//  returns true if the bot has taken longer than expected to reach the 
//  currently active waypoint
//-----------------------------------------------------------------------------
bool Goal_TraverseEdge::isStuck()const
{  
  double TimeTaken = GetApp()->GetGameTick() - m_startTime;

  if (TimeTaken > m_timeExpected)
  {
	  if (GetGameLogic()->GetShowPathfinding())
		  LogMsg("TraverseEdge: Entity %d (%s) appears to be stuck (%.0f MS timeout reached)",m_pOwner->ID(), 
		  m_pOwner->GetName().c_str(), TimeTaken);

    return true;
  }

  return false;
}


//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_TraverseEdge::Terminate()
{
  //turn off steering behaviors.
//  m_pOwner->GetSteering()->SeekOff();
//  m_pOwner->GetSteering()->ArriveOff();

  Brain *pBrain = m_pOwner->GetBrainManager()->GetBrainByName("TopSeek");

  if (pBrain)
  {
	  pBrain->SetDeleteFlag(true);
  } else
  {
	  LogMsg("Goal_TraverseEdge: Unable to delete TopSeek");
  }
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_TraverseEdge::Render()
{

	if (m_iStatus == active)
  {
/*
	  gdi->BluePen();
    gdi->Line(m_pOwner->Pos(), m_Edge.Destination());
    gdi->GreenBrush();
    gdi->BlackPen();
    gdi->Circle(m_Edge.Destination(), 3);
	*/
  }
}

bool Goal_TraverseEdge::HandleMessage(const Message& msg)
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


