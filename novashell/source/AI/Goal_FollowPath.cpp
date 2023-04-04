#include "AppPrecomp.h"
#include "Goal_FollowPath.h"
#include "../MovingEntity.h"
#include "Goal_TraverseEdge.h"
#include "message_types.h"

//#include "Goal_NegotiateDoor.h"


//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------
Goal_FollowPath::
Goal_FollowPath(MovingEntity*  pBot,
                std::list<PathEdge> path):Goal_Composite<MovingEntity>(pBot, goal_follow_path),
                                                  m_Path(path)
{
	SetName("FollowPath");
}


//------------------------------ Activate -------------------------------------
//-----------------------------------------------------------------------------
void Goal_FollowPath::Activate()
{
  m_iStatus = active;
  
  //get a reference to the next edge
  PathEdge edge = m_Path.front();

//  LogMsg("Popping front");
  //remove the edge from the path
  m_Path.pop_front(); 

  //some edges specify that the bot should use a specific behavior when
  //following them. This switch statement queries the edge behavior flag and
  //adds the appropriate goals/s to the subgoal list.
  switch(edge.Behavior())
  {
  case NavGraphEdge::normal:
    {
      AddSubgoal(new Goal_TraverseEdge(m_pOwner, edge, m_Path.empty()));
	}

    break;

  case NavGraphEdge::goes_through_door:
    {

		assert(0);
	  //also add a goal that is able to handle opening the door
      //AddSubgoal(new Goal_NegotiateDoor(m_pOwner, edge, m_Path.empty()));
    }

    break;

  case NavGraphEdge::jump:
    {
      //add subgoal to jump along the edge
    }

    break;

  case NavGraphEdge::grapple:
    {
      //add subgoal to grapple along the edge
    }

    break;

  default:

    LogError("<Goal_FollowPath::Activate>: Unrecognized edge type");
  }
}



bool Goal_FollowPath::HandleMessage(const Message& msg)
{
	//first, pass the message down the goal hierarchy
	bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

	//if the msg was not handled, test to see if this goal can handle it
	if (bHandled == false)
	{


		switch(msg.GetMsgType())
		{

		/*
		case C_MSG_SEEK_ARRIVED:

			if (!m_Path.empty())
			{
				Activate(); 
			} else
			{
				m_iStatus = completed;
			}
			return true;
			break;

	*/
		default:
			return false;
		}
	

	}
	

	//not handled
	return true;
}

//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------

int Goal_FollowPath::Process()
{
  //if status is inactive, call Activate()
  ActivateIfInactive();

  m_iStatus = ProcessSubgoals();

  
  //if there are no subgoals present check to see if the path still has edges.
  //remaining. If it does then call activate to grab the next edge.
  if (m_iStatus == completed && !m_Path.empty())
  {
    Activate(); 
  }

  return m_iStatus;
}
 


//---------------------------- Render -----------------------------------------
//-----------------------------------------------------------------------------
void Goal_FollowPath::Render()
{ 
	//render all the path waypoints remaining on the path list
  std::list<PathEdge>::iterator it;
  for (it = m_Path.begin(); it != m_Path.end(); ++it)
  {  
	  CL_Color col(255,0,0);
	  
	  DrawLineWithArrowWorld(it->Source(),  it->Destination(), 15, col, CL_Display::get_current_window()->get_gc());
	  DrawCenteredBoxWorld(it->Destination(), 3, col, CL_Display::get_current_window()->get_gc());
  }

  //forward the request to the subgoals
  Goal_Composite<MovingEntity>::Render();
}
  




