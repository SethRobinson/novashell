//  ***************************************************************
//  StateAttack2 - Creation date: 04/01/2008
//  -------------------------------------------------------------
//  Copyright 2008: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateAttack2_h__
#define StateAttack2_h__

#include "State.h"

class StateAttack2: public State
{
public:
	StateAttack2(MovingEntity *pParent);
	virtual ~StateAttack2();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Attack2";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateAttack2(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:


private:
};

#endif // StateAttack2_h__
