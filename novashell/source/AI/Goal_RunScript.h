#ifndef GOAL_RunScript_H
#define GOAL_RunScript_H
#pragma warning (disable:4786)

#include "Goal.h"

class Goal_RunScript : public Goal<MovingEntity>
{
public:

	Goal_RunScript(MovingEntity* pBot, const string & scriptCommand);

	//the usual suspects
	void Activate();
	int  Process();
	void Terminate();
	void Render();
	virtual bool HandleMessage(const Message& msg);
	virtual const char * GetName() const {return "RunScript";}

private:

	unsigned int m_RunScriptTimer;
	string m_scriptCommand;
};

#endif

