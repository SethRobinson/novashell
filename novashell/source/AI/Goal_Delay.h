#ifndef GOAL_DELAY_H
#define GOAL_DELAY_H
#pragma warning (disable:4786)

#include "Goal.h"

class Goal_Delay : public Goal<MovingEntity>
{
public:

	Goal_Delay(MovingEntity* pBot, int time);

	//the usual suspects
	void Activate();
	int  Process();
	void Terminate();
	virtual void LostFocus();
	void Render();
	virtual bool HandleMessage(const Message& msg);
	virtual const char * GetName() const {return "Delay";}

private:

	unsigned int m_delayTimer;
	int m_timeToWait;

};

#endif

