#include "AppPrecomp.h"
#include "Goal_MoveToPosition.h"
#include "Goal_SeekToPosition.h"
#include "PathPlanner.h"
#include "Goal_FollowPath.h"
#include "message_types.h"

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_MoveToPosition::Activate()
{
  m_iStatus = active;
  
  //make sure the subgoal list is clear.
  RemoveAllSubgoals();


  bool bUsePath = m_pOwner->GetPathPlanner()->IsPathNeededToGetToTarget(m_vDestination);

  if (bUsePath)
  {
	if (GetGameLogic()->GetShowPathfinding())
		LogMsg("Entity %d (%s) is calculating a micro path", m_pOwner->ID(), m_pOwner->GetName().c_str());

	  //compute path
	  if (m_pOwner->GetPathPlanner()->RequestPathToPosition(m_vDestination))
	  {
		  //we can do it?
		  int result;

		  //this is setup to do searches over time, but for now, let's just do the whole search now
		  //and not mess with getting messages later about it
	  
		  do 
		  {
			  result = m_pOwner->GetPathPlanner()->CycleOnce();
		  } while( result == search_incomplete );

		  //examine results

		  switch (result)
		  {

		  case target_not_found:
		  
	if (GetGameLogic()->GetShowPathfinding())
		LogMsg("Entity %d (%s): Goal_MoveToPosition: Couldn't locate target.", m_pOwner->ID(), m_pOwner->GetName().c_str());

			  if (m_pOwner->HandleMessage(Message(C_MSG_TARGET_NOT_FOUND)))
			  {
				  //parent signaled that they want to handle it
				  m_iStatus = failed;
				  return;
			  }
			  
			  bUsePath = false;
			  break;

			  
		  case target_found:
#ifdef C_SHOW_PATHFINDING_DEBUG_INFO
			  LogMsg("Found it");
#endif
			  break;

		  default:
			  assert(!"Unknown search result");
		  }

	  } else
	  {
		  //unable to make a path?
		  //let our parent goal know the situation ...
		  if (m_pOwner->HandleMessage(Message(C_MSG_PATH_NODES_MISSING)))
		  {
			  //parent signaled that they want to handle it
			  //m_iStatus = failed;
			  //return;
		  }
		  

		  //LogMsg("Ent %d (%s) was unable to compute a path to destination.", m_pOwner->ID(), m_pOwner->GetName().c_str());
		  bUsePath = false;
	  }

  }


  //actually do something

  if (bUsePath)
  {
	  AddSubgoal(new Goal_FollowPath(m_pOwner,
		  m_pOwner->GetPathPlanner()->GetPath()));
  } else
  {
	  //dumb seek method
	  AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_vDestination));
  }
}

  //------------------------------ Process --------------------------------------
  //-----------------------------------------------------------------------------
  int Goal_MoveToPosition::Process()
  {
	  //if status is inactive, call Activate()
	  ActivateIfInactive();
	  if (m_iStatus == failed)
	  {
		  //special case for instant failure so it will kick back nw
		  return m_iStatus;
	  }
	  //process the subgoals
	  m_iStatus = ProcessSubgoals();
	
	  //if any of the subgoals have failed then this goal re-plans
	  ReactivateIfFailed();

	  return m_iStatus;
  }

  //---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
bool Goal_MoveToPosition::HandleMessage(const Message& msg)
{
  
  //first, pass the message down the goal hierarchy
  bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

  //if the msg was not handled, test to see if this goal can handle it
  if (bHandled == false)
  {
	/*
	  switch(msg.GetMsgType())
    {
    

	case Msg_PathReady:

      //clear any existing goals
      RemoveAllSubgoals();

	  LogMsg("Adding subgoal: Followpath");
  
	  AddSubgoal(new Goal_FollowPath(m_pOwner,
                                     m_pOwner->GetPathPlanner()->GetPath()));

      return true; //msg handled

	  case Msg_NoPathAvailable:
      m_iStatus = failed;
      return true; //msg handled
    }
	
	  default: return false;
	 }
 */
  
	  return false;
  }

  //handled by subgoals
  return true;
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Goal_MoveToPosition::Render()
{
 
	//forward the request to the subgoals
  Goal_Composite<MovingEntity>::Render();

}

