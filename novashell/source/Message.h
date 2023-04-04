
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 9:3:2006   12:58
*/


#ifndef Message_HEADER_INCLUDED // include guard
#define Message_HEADER_INCLUDED  // include guard

#include <API/signals.h>
#include "MovingEntity.h"

class Message
{
public:

    Message();
	Message(int msgType);
	Message(int msgType, int parm1, int parm2);
    virtual ~Message();


	enum TimingType
	{
		GAME_TIME,
		SYSTEM_TIME
	};

	void SetDefaults();

	unsigned int m_deliveryTime;
	string m_text;
	void SetTarget(unsigned int entID);
	void EntDeleted(int ID);
	void Deliver();
	void SetTimingType(TimingType type) {m_timingType = type;}
	TimingType GetTimingType(){return m_timingType;}
	void SetMsgType(int msgType) {m_msgType = msgType;}
	void SetParm1(int parm1) {m_parm1 = parm1;}
	void SetParm2(int parm2) {m_parm2 = parm2;}

	int GetMsgType() const {return m_msgType;}
	int GetParm1() const {return m_parm1;}
	int GetParm2() const {return m_parm2;}

private:
	int m_targetID; //the entity we will deliver too
	CL_SlotContainer m_slots;
	BaseGameEntity *m_pEnt;
	TimingType m_timingType;
	int m_msgType, m_parm1, m_parm2; //for misc data storage
};

#endif                  // include guard
