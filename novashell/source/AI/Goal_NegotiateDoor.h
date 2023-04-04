#ifndef GOAL_NEGOTIATE_DOOR_H
#define GOAL_NEGOTIATE_DOOR_H
#pragma warning (disable:4786)

#include "Goal_Composite.h"
#include "Goal_Types.h"
#include "../MovingEntity.h"
#include "PathEdge.h"


class Goal_NegotiateDoor : public Goal_Composite<MovingEntity>
{
private:

  PathEdge m_PathEdge;

  bool     m_bLastEdgeInPath;

public:

  Goal_NegotiateDoor(MovingEntity* pBot, PathEdge edge, bool LastEdge);

 //the usual suspects
  void Activate();
  int  Process();
  void Terminate(){}
};



#endif
