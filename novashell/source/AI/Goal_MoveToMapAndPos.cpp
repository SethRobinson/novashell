#include "AppPrecomp.h"
#include "Goal_MoveToMapAndPos.h"
#include "Goal_SeekToPosition.h"
#include "PathPlanner.h"
#include "Goal_FollowPath.h"
#include "Map.h"
#include "Goal_MoveToPosition.h"
#include "message_types.h"

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_MoveToMapAndPos::Activate()
{
	m_bRequestNextChunk = false;
	m_macroPath.m_path.clear();
	m_iStatus = active;
	//make sure the subgoal list is clear.
	RemoveAllSubgoals();

	if (!m_bTriedSimpleWay && m_pDestMap == m_pOwner->GetMap())
	{
		m_bTriedSimpleWay = true;
	
		//we don't really need this fancy system?
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		return;
		//however, if this fails, we'll need it.
	}

if (GetGameLogic()->GetShowPathfinding())
	LogMsg("Ent %d (%s) is calculating a MACRO path.  Cur facing is %s, facing target is %s.", m_pOwner->ID(), m_pOwner->GetName().c_str(),
		PrintVector(m_pOwner->GetVectorFacing()).c_str(), PrintVector(m_pOwner->GetVectorFacingTarget()).c_str());


	//the destination is on a different map or requires a complicated intra-map route.  We need to sketch out a path to get to that map.
	m_macroPath = g_worldNavManager.FindPathToMapAndPos(m_pOwner, m_pDestMap, m_vDestination);
	
	if (m_macroPath.m_status == MacroPathInfo::NO_NODE_CLOSE)
	{

		//dumb seek method
		AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_vDestination));
		return;
	}


	if (!m_macroPath.IsValid())
	{
		m_iStatus = failed;
		LogMsg("Ent %d (%s) failed to find valid path", m_pOwner->ID(), m_pOwner->GetName().c_str());
		return;
	}
	
	if (m_macroPath.m_path.size() == 1)
	{
if (GetGameLogic()->GetShowPathfinding())
		LogMsg("Entity %d (%s) can get there without warping actually", m_pOwner->ID(), m_pOwner->GetName().c_str());
			
	//if this happens, it means we CAN actually go directly there without warping.  Possibly the 
		//m_bTriedSimpleWay thing failed.

		assert(m_pDestMap == m_pOwner->GetMap());
		m_macroPath.m_path.clear();
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		m_bTriedSimpleWay = false;
		return;
	}
	
	//we have data on which way to go
	ProcessNextMapChunk();
}


void Goal_MoveToMapAndPos::ProcessNextMapChunk()
{
	m_bRequestNextChunk = false;

	if (!m_macroPath.IsValid())
	{
if (GetGameLogic()->GetShowPathfinding())
		LogMsg("Entity %d (%s) is done with micro path, go straight to the right spot",
		m_pOwner->ID(), m_pOwner->GetName().c_str());


		//no more macro path, just move to the right spot now and we're done
		assert(m_pDestMap == m_pOwner->GetMap() && "How this happen?!");
		m_bTriedSimpleWay = false; //if we have to recompute, we can try this way again
		AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vDestination));
		return;
	}
	
	int nextNodeID = m_macroPath.m_path.front();
	
if (GetGameLogic()->GetShowPathfinding())
	LogMsg("%s is going to node %d", m_pOwner->GetName().c_str(), nextNodeID);


	MovingEntity *pNodeEnt = g_worldNavManager.ConvertWorldNodeToOwnerEntity(nextNodeID, true);
	
	if (!pNodeEnt)
	{
		LogError("Node no longer exists?");
		m_iStatus = inactive; //make it restart
	} else
	{
		assert(pNodeEnt->GetMap() == m_pOwner->GetMap() && "How this happen?!");

		AddSubgoal(new Goal_MoveToPosition(m_pOwner, pNodeEnt->GetPos()));
	}

}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_MoveToMapAndPos::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	if (m_bRequestNextChunk)
	{
		ProcessNextMapChunk();
		return m_iStatus;
	}

	//process the subgoals
	m_iStatus = ProcessSubgoals();

	if (m_iStatus == completed)
	{
		if (m_macroPath.IsValid())
		{
			//go to the next leg of the journey perhaps
			m_macroPath.m_path.pop_front();

			if (!m_macroPath.IsValid())
			{
				LogMsg("Goal_MoveToMapAndPos::Process - Path cut off early?");
				m_iStatus = inactive;
			} else
			{
				int nextNodeID = m_macroPath.m_path.front();
				MovingEntity *pNodeEnt = g_worldNavManager.ConvertWorldNodeToOwnerEntity(nextNodeID, true);

				if (!pNodeEnt)
				{
					LogMsg("Warp node was deleted?");
					//uh oh, we absolutely need to load this now
					m_iStatus = inactive; //force a restart

				} else
				{
if (GetGameLogic()->GetShowPathfinding())
					LogMsg("Warping %s to %s from %s", m_pOwner->GetName().c_str(), 
						m_pOwner->GetMap()->GetName().c_str(), pNodeEnt->GetMap()->GetName().c_str());

if (pNodeEnt->GetScriptObject())
					{
						if (pNodeEnt->GetScriptObject()->FunctionExists("OnGoalPreWarp"))
						{
							try {	luabind::call_function<void>(pNodeEnt->GetScriptObject()->GetState(), "OnGoalPreWarp", m_pOwner);
							} catch (luabind::error &e) 
							{
								ShowLUAMessagesIfNeeded(e.state(), 1); 
								LogError("Error occurred in Warp entity ID %d while entity %d (%s) was warping through it with goal AI",
									pNodeEnt->ID(), m_pOwner->ID(), m_pOwner->GetName().c_str());
							}

						}
					}
					m_pOwner->SetPosAndMap(pNodeEnt->GetPos(), pNodeEnt->GetMap()->GetName());
					m_macroPath.m_path.pop_front();
					m_iStatus = active;
					m_bRequestNextChunk = true;

					if (pNodeEnt->GetScriptObject())
					{
						if (pNodeEnt->GetScriptObject()->FunctionExists("OnGoalPostWarp"))
						{
							try {	luabind::call_function<void>(pNodeEnt->GetScriptObject()->GetState(), "OnGoalPostWarp", m_pOwner);
							} catch (luabind::error &e) 
							{
								ShowLUAMessagesIfNeeded(e.state(), 1); 
								LogError("Error occurred in Warp entity ID %d while entity %d (%s) was warping through it with goal AI",
									pNodeEnt->ID(), m_pOwner->ID(), m_pOwner->GetName().c_str());
							}

						}
					}

				
				}
			}
		}
	}
	//if any of the subgoals have failed then this goal re-plans

	ReactivateIfFailed();

	return m_iStatus;
}

//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
bool Goal_MoveToMapAndPos::HandleMessage(const Message& msg)
{

	//first, pass the message down the goal hierarchy
	bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

	//if the msg was not handled, test to see if this goal can handle it
	if (bHandled == false)
	{

		switch(msg.GetMsgType())
		{

		case C_MSG_PATH_NODES_MISSING:
			m_bTriedSimpleWay = false;
			return true; //signal that we handled it
			break;

		case C_MSG_TARGET_NOT_FOUND:
			//LogMsg("Target wasn't found, restarting");
			return true; //signal that we handled it
			break;

		case C_MSG_GOT_STUCK:
			m_bTriedSimpleWay = false;
			//LogMsg("Got stuck..");
			return true; //signal that we handled it
			break;

			
		default: return false;
		}
	}

	//handled by subgoals
	return false;
}

void Goal_MoveToMapAndPos::Terminate()
{
  
}

void Goal_MoveToMapAndPos::LostFocus()
{
	m_iStatus = inactive;
	 
if (GetGameLogic()->GetShowPathfinding())
	LogMsg("Entity %d (%s): Aborting goal_MoveToMapAndPos.. (brain lost focus)", m_pOwner->ID(), m_pOwner->GetName().c_str());

	m_bTriedSimpleWay = false;
    RemoveAllSubgoals();
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Goal_MoveToMapAndPos::Render()
{
	//forward the request to the subgoals
	Goal_Composite<MovingEntity>::Render();
}

