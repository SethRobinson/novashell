#ifndef GOAL_MoveToMapAndPos_H
#define GOAL_MoveToMapAndPos_H
#pragma warning (disable:4786)

#include "Goal_Composite.h"
#include "Goal_Types.h"
#include "WorldNavManager.h"

class Map;

class Goal_MoveToMapAndPos : public Goal_Composite<MovingEntity>
{

public:

	Goal_MoveToMapAndPos(MovingEntity* pBot, CL_Vector2 pos, Map *pMap):
	Goal_Composite<MovingEntity>(pBot, goal_move_to_position),
		m_vDestination(pos), m_pDestMap(pMap)
	{
		SetName("MapAndPosMove");
		m_bTriedSimpleWay = false;
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

	void ProcessNextMapChunk();
	//the position the bot wants to reach
	CL_Vector2 m_vDestination;
	Map * m_pDestMap;
	bool m_bTriedSimpleWay;
	MacroPathInfo m_macroPath;
	bool m_bRequestNextChunk;
};

#endif
