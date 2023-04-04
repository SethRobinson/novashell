//  ***************************************************************
//  BrainForceVisual - Creation date: 01/29/2007
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2007 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainForceVisual_h__
#define BrainForceVisual_h__

#include "Brain.h"

class BrainForceVisual: public Brain
{
public:
	BrainForceVisual(MovingEntity *pParent);
	virtual ~BrainForceVisual();
	virtual void Update(float step);
	virtual const char * GetName() {return "ForceVisual";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainForceVisual(pParent);}

	virtual void HandleMsg(const string &msg);

protected:
	

	string m_forceAnim;
	int m_forceSetID;
private:
};

#endif // BrainForceVisual_h__