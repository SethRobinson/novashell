//  ***************************************************************
//  BrainFadeIn - Creation date: 12/21/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainFadeIn_h__
#define BrainFadeIn_h__

#include "Brain.h"

class BrainFadeIn: public Brain
{
public:
	BrainFadeIn(MovingEntity *pParent);
	virtual ~BrainFadeIn();
	virtual void Update(float step);
	virtual const char * GetName() {return "FadeIn";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainFadeIn(pParent);}
	virtual void HandleMsg(const string &msg);

protected:
	

private:

	int m_fadeOutTimeMS;
	int m_timeCreated;
};

#endif // BrainFadeIn_h__