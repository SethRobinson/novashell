#ifndef GOAL_MOVE_POS_H
#define GOAL_MOVE_POS_H
#pragma warning (disable:4786)

#include "Goal_Composite.h"
#include "Goal_Types.h"

class Goal_MoveToPosition : public Goal_Composite<MovingEntity>
{
private:

  //the position the bot wants to reach
  CL_Vector2 m_vDestination;

public:

  Goal_MoveToPosition(MovingEntity* pBot,
                      CL_Vector2   pos):
  
            Goal_Composite<MovingEntity>(pBot, goal_move_to_position),
            m_vDestination(pos)
  {

		SetName("Move");
  }

 //the usual suspects
  void Activate();
  int  Process();
  void Terminate(){}

  //this goal is able to accept messages
  bool HandleMessage(const Message& msg);

  void Render();
};





#endif
