#ifndef GOAL_SEEK_TO_POSITION_H
#define GOAL_SEEK_TO_POSITION_H
#pragma warning (disable:4786)

#include "Goal.h"
#include "Goal_Types.h"


class Goal_SeekToPosition : public Goal<MovingEntity>
{
public:

  Goal_SeekToPosition(MovingEntity* pBot, CL_Vector2 target);

  //the usual suspects
  void Activate();
  int  Process();
  void Terminate();
  void Render();
  virtual bool HandleMessage(const Message& msg);
  virtual const char * GetName() const {return "Seek";}
	
private:

	//the position the bot is moving to
	CL_Vector2  m_vPosition;

	//returns true if a bot gets stuck
	bool      isStuck()const;
	
	unsigned int m_startTime;
	unsigned int m_timeToReachPos;

};




#endif

