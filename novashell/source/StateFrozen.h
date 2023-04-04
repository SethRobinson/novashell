//  ***************************************************************
//  StateFrozen - Creation date: 06/30/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateFrozen_h__
#define StateFrozen_h__

#include "State.h"

class StateFrozen: public State
{
public:
	StateFrozen(MovingEntity *pParent);
	virtual ~StateFrozen();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Frozen";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateFrozen(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:
};

#endif // StateFrozen_h__
