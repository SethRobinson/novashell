//  ***************************************************************
//  StateTurn - Creation date: 04/01/2008
//  -------------------------------------------------------------
//  Copyright 2008: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTurn_h__
#define StateTurn_h__

#include "State.h"

class StateTurn: public State
{
public:
	StateTurn(MovingEntity *pParent);
	virtual ~StateTurn();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Turn";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTurn(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:


private:
};

#endif // StateTurn_h__
