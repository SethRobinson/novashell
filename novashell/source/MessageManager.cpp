#include "AppPrecomp.h"
#include "main.h"
#include "MessageManager.h"


MessageManager::MessageManager()
{
}

MessageManager::~MessageManager()
{
}
void MessageManager::Reset()
{
	m_messageList.clear();
}

void MessageManager::Schedule(unsigned int deliveryMS, unsigned int targetID, const char * pMsg)
{
	//LogMsg("scheduling message to %u at %d", targetID, GetApp()->GetGameTick());

	Message m;
	m.m_deliveryTime = GetApp()->GetGameTick()+deliveryMS;
	m.m_text = pMsg;
	
	m_messageList.push_back(m);
	m_messageList.rbegin()->SetTarget(targetID);

}

//like above, but uses system time instead of game time.
void MessageManager::ScheduleSystem(unsigned int deliveryMS, unsigned int targetID, const char * pMsg)
{
	//LogMsg("scheduling message to %u", targetID);

	Message m;
	m.SetTimingType(Message::SYSTEM_TIME);

	m.m_deliveryTime = GetApp()->GetTick()+deliveryMS;
	m.m_text = pMsg;

	m_messageList.push_back(m);
	m_messageList.rbegin()->SetTarget(targetID);

}

void MessageManager::Update()
{
	message_list::iterator itor = m_messageList.begin();

	unsigned int systemTimeNow = GetApp()->GetTick();
	
	while (itor != m_messageList.end())
	{
		switch (itor->GetTimingType())	
		{
		case Message::GAME_TIME:
			//note, GetGameTick() *CAN* change from a script command, so don't
			//try to put that in a temp to save time
			if ( itor->m_deliveryTime < GetApp()->GetGameTick())
			{
				itor->Deliver();
				//delete it
				itor = m_messageList.erase(itor);
				continue;
			}
			break;
		
		case Message::SYSTEM_TIME:

			if ( itor->m_deliveryTime < systemTimeNow)
			{
				itor->Deliver();
				//delete it
				itor = m_messageList.erase(itor);
				continue;
			}
			break;
		default:
			throw CL_Error("Unknown message timing type");
		}
		

		itor++;
	}

}

