//  ***************************************************************
//  BrainTopBase - Creation date: 07/28/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainTopBase_h__
#define BrainTopBase_h__

#include "Brain.h"

class BrainTopBase: public Brain
{
public:
	BrainTopBase(MovingEntity *pParent);
	virtual ~BrainTopBase();
	virtual void Update(float step);
	virtual void PostUpdate(float step);

	virtual const char * GetName() {return "StandardBase";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainTopBase(pParent);}
	virtual void OnAdd();

	//for use by other brains directly
	virtual void AddWeightedForce(const CL_Vector2 & force);
	virtual void HandleMsg(const string &msg);

	virtual int HandleSimpleMessage(int message, int user1, int user2);

protected:

	void ResetForNextFrame();

private:

	CL_Vector2 m_force;
	float m_maxForce;
	CL_Vector2 m_controlFilter, m_airControlFilter; //how much force actually gets applied
	bool m_DontMessWithVisual;
};

#endif // BrainTopBase_h__
