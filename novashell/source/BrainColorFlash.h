//  ***************************************************************
//  BrainColorFlash - Creation date: 06/28/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainColorFlash_h__
#define BrainColorFlash_h__

#include "Brain.h"
#include "AppUtils.h"

class BrainColorFlash: public Brain
{
public:
	BrainColorFlash(MovingEntity *pParent);
	virtual ~BrainColorFlash();
	virtual void Update(float step);
	virtual const char * GetName() {return "ColorFlash";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainColorFlash(pParent);}
	void HandleMsg(const string &msg);
	
protected:
	
private:

	void SetPulseRate(int pulseRate);

	GameTimer m_flashTimer;
	bool m_bOn;
	short m_r, m_g, m_b, m_a;
	int m_pulseRate;
	int m_remove_brain_by_pulses;

};

#endif // BrainColorFlash_h__
