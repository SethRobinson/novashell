//  ***************************************************************
//  StatePain - Creation date: 01/28/2007
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2007 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a state file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a state behavior.

#ifndef StatePain_h__
#define StatePain_h__

#include "State.h"

class StatePain: public State
{
public:
	StatePain(MovingEntity *pParent);
	virtual ~StatePain();
	virtual void Update(float step);
	virtual void PostUpdate(float step);
	virtual const char * GetName() {return "Pain";};
	virtual State * CreateInstance(MovingEntity *pParent) {return new StatePain(pParent);}
	virtual void OnAdd();
	virtual void OnRemove();

protected:
	bool m_bCallbackActive;

private:
};

#endif // StatePain_h__