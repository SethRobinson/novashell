//  ***************************************************************
//  BrainScale - Creation date: 03/02/2007
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2007 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainScale_h__
#define BrainScale_h__

#include "Brain.h"

class BrainScale: public Brain
{
public:
	BrainScale(MovingEntity *pParent);
	virtual ~BrainScale();
	virtual void Update(float step);
	virtual const char * GetName() {return "Scale";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainScale(pParent);}
	virtual void HandleMsg(const string &msg);

protected:

	int m_scaleSpeedMS;
	int m_timeCreated;
	CL_Vector2 m_vStartScale, m_vScaleTarget;
	bool m_deleteEntity;

private:
};

#endif // BrainScale_h__