//  ***************************************************************
//  StateRun - Creation date: 04/1/2008
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateRun_h__
#define StateRun_h__

#include "State.h"

class StateRun: public State
{
public:
	StateRun(MovingEntity *pParent);
	virtual ~StateRun();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Run";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateRun(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:


private:

	bool m_bCallbackActive;
};

#endif // StateRun_h__
