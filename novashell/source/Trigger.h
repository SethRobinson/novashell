//  ***************************************************************
//  Trigger - Creation date: 07/05/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Trigger_h__
#define Trigger_h__

#include "AppUtils.h"

class MovingEntity;

class Trigger
{
public:
	Trigger();
	virtual ~Trigger();

	void Set(MovingEntity *pParent, int triggerType, int typeVar, int triggerBehavior, int behaviorVar);
	void Reset();
	void Update(float step);

	enum eTriggerType
	{
		TYPE_NONE,
		TYPE_REGION_IMAGE
	};

	enum eBehaviorType
	{
		BEHAVIOR_NORMAL,
		BEHAVIOR_PULSE
	};

	enum eCurrentState
	{
		STATE_INSIDE,
		STATE_OUTSIDE
	};

private:

	void SetTriggerState(bool bInsideRegion);

	MovingEntity *m_pParent;

	eTriggerType m_triggerType;
	eBehaviorType m_behaviorType;

	int m_typeVar;
	int m_behaviorVar;
	
	GameTimer m_pulseTimer;
	eCurrentState m_state;

};

#endif // Trigger_h__
