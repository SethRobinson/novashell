//  ***************************************************************
//  BrainBob - Creation date: 11/09/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainBob_h__
#define BrainBob_h__

#include "Brain.h"
#include "AppUtils.h"

class BrainBob: public Brain
{
public:
	BrainBob(MovingEntity *pParent);
	virtual ~BrainBob();
	virtual void Update(float step);
	virtual const char * GetName() {return "Bob";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainBob(pParent);}
	virtual void HandleMsg(const string &msg);

protected:
	

private:

	CL_Vector2 m_lastDisplacement;
	float m_bobSpeed, m_bobDistance;
	GameTimer m_timer;
};

#endif // BrainBob_h__