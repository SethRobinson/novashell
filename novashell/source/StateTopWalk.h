//  ***************************************************************
//  StateTopWalk - Creation date: 07/29/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StateTopWalk_h__
#define StateTopWalk_h__

#include "State.h"

class StateTopWalk: public State
{
public:
	StateTopWalk(MovingEntity *pParent);
	virtual ~StateTopWalk();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Walk";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StateTopWalk(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	

private:

	bool m_bCallbackActive;
};

#endif // StateTopWalk_h__
