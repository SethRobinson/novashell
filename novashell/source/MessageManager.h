
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 9:3:2006   12:55
*/


#ifndef MessageManager_HEADER_INCLUDED // include guard
#define MessageManager_HEADER_INCLUDED  // include guard

#include "Message.h"

class Message;
typedef std::list<Message> message_list;

class MessageManager
{
public:

    MessageManager();
    virtual ~MessageManager();

	//game time
	void Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
	
	//system time (game being paused/slowed down won't stop delivery or timing of these)
	void ScheduleSystem(unsigned int deliveryMS, unsigned int targetID, const char * pMsg);
	void Update(); //called once a tick or whenever
	void Reset();

protected:

private:

	message_list m_messageList;
};



#endif                  // include guard

