//  ***************************************************************
//  StateTopIdle - Creation date: 07/28/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTopIdle_h__
#define StateTopIdle_h__

#include "State.h"

class StateTopIdle: public State
{
public:
	StateTopIdle(MovingEntity *pParent);
	virtual ~StateTopIdle();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Idle";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTopIdle(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	


private:

	bool m_bCallbackActive;
};

#endif // StateTopIdle_h__
