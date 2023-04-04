//  ***************************************************************
//  StateJump - Creation date: 04/01/2008
//  -------------------------------------------------------------
//  Copyright 2008: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateJump_h__
#define StateJump_h__

#include "State.h"

class StateJump: public State
{
public:
	StateJump(MovingEntity *pParent);
	virtual ~StateJump();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Jump";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateJump(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:


private:
		bool m_bCallbackActive;
};

#endif // StateJump_h__
