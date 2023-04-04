#ifndef GOAL_Say_H
#define GOAL_Say_H
#pragma warning (disable:4786)

#include "Goal.h"

class Goal_Say : public Goal<MovingEntity>
{
public:

	Goal_Say(MovingEntity* pBot, const string &msg, int entID, int entToFaceID, int delayMS);

	//the usual suspects
	void Activate();
	int  Process();
	void Terminate();
	virtual void LostFocus();

	void Render();
	virtual bool HandleMessage(const Message& msg);
	virtual const char * GetName() const {return "Say";}

private:

	string m_text;
	int m_entID;
	unsigned int m_delayTimer;
	int m_entToFaceID;
	int m_waitOverrideMS; //if not -1, it means we'll use it instead of the normal text delay
	

};

#endif

