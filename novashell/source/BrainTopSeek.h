//  ***************************************************************
//  BrainTopSeek - Creation date: 08/29/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainTopSeek_h__
#define BrainTopSeek_h__

#include "Brain.h"

class BrainTopSeek: public Brain
{
public:
	BrainTopSeek(MovingEntity *pParent);
	virtual ~BrainTopSeek();
	virtual void Update(float step);
	virtual const char * GetName() {return "TopSeek";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainTopSeek(pParent);}
	virtual void HandleMsg(const string &msg);
	virtual void OnRemove();

protected:

	CL_Vector2 m_vecTarget;
	bool m_bUsingTarget;
	float m_padding;

private:
};

#endif // BrainTopSeek_h__
