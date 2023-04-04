//  ***************************************************************
//  StateSpecial1 - Creation date: 04/01/2008
//  -------------------------------------------------------------
//  Copyright 2008: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateSpecial1_h__
#define StateSpecial1_h__

#include "State.h"

class StateSpecial1: public State
{
public:
	StateSpecial1(MovingEntity *pParent);
	virtual ~StateSpecial1();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Special1";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateSpecial1(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:


private:
};

#endif // StateSpecial1_h__
