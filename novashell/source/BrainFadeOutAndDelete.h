//  ***************************************************************
//  BrainFadeOutAndDelete - Creation date: 06/27/2006
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2006 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BrainFadeOutAndDelete_h__
#define BrainFadeOutAndDelete_h__

#include "Brain.h"

class BrainFadeOutAndDelete: public Brain
{
public:
	BrainFadeOutAndDelete(MovingEntity *pParent);
	virtual ~BrainFadeOutAndDelete();
	virtual void Update(float step);
	virtual const char * GetName() {return "FadeOutAndDelete";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainFadeOutAndDelete(pParent);}
	virtual void HandleMsg(const string &msg);

protected:
	

private:

  int m_fadeOutTimeMS;
  int m_timeCreated;
 
};

#endif // BrainFadeOutAndDelete_h__