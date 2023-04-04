#ifndef GOAL_Approach_H
#define GOAL_Approach_H
#pragma warning (disable:4786)

#include "Goal_Composite.h"
#include "Goal_Types.h"
#include "WorldNavManager.h"

class Map;

class Goal_Approach : public Goal_Composite<MovingEntity>
{

public:

	Goal_Approach(MovingEntity* pBot, int targetID, int distanceRequired):
	  Goal_Composite<MovingEntity>(pBot, goal_move_to_position),
		  m_targetID(targetID), m_distanceRequired(distanceRequired)
	  {
		  SetName("Approach");
		  
		  ReinitializeCompletely();
		  
	  }

	  //the usual suspects
	  void Activate();
	  int  Process();
	  virtual void Terminate();
	  virtual void LostFocus();
	
	  //this goal is able to accept messages
	  bool HandleMessage(const Message& msg);
	  void Render();

private:

	
	bool UpdatePositionFromEntityID();
	void ProcessNextMapChunk();
	bool CloseEnoughAndFacingTheRightWay();
	void ReinitializeCompletely();
	//the position the bot wants to reach
	CL_Vector2 m_vDestination, m_vLookPosition;
	Map * m_pDestMap;
	bool m_bTriedSimpleWay;
	MacroPathInfo m_macroPath;
	bool m_bRequestNextChunk;
	unsigned int m_targetID;
	int m_distanceRequired; //how close we have to be to our target
	bool m_bWaitingForTurn;
	unsigned int m_locationUpdateTimer;
	bool m_bTriedComplexWay;
	int m_targetNotFoundCount; //how many times we've been notified that there is an invalid path.  It's to cope with partial paths, so not every part of a level has to be pathed out.
};

#endif
