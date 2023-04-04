//  ***************************************************************
//  BrainShake - Creation date: 06/07/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainShake_h__
#define BrainShake_h__

#include "Brain.h"

class BrainShake: public Brain
{
public:
	BrainShake(MovingEntity * pParent);
	virtual ~BrainShake();

	virtual void Update(float step);
	virtual const char * GetName(){return "Shake";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainShake(pParent);}

protected:

private:

	CL_Vector2 m_lastDisplacement;

};

#endif // BrainShake_h__
