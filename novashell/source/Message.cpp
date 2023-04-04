#include "AppPrecomp.h"
#include "Message.h"
#include "GameLogic.h"

Message::Message()
{
	SetDefaults();
}

Message::Message(int msgType)
{
	SetDefaults();
	SetMsgType(msgType);
}

Message::Message(int msgType, int parm1, int parm2)
{
	SetDefaults();
	SetMsgType(msgType);
	SetParm1(parm1)	;
	SetParm2(parm2);
	
}

void Message::SetDefaults()
{
	m_pEnt = NULL;
	m_timingType = GAME_TIME;
	m_msgType = 0; 
	m_parm1 = m_parm2 = 0;
}


Message::~Message()
{
}

void Message::EntDeleted(int ID)
{
  m_pEnt = NULL;
}

void Message::Deliver()
{
	
	if (m_targetID == 0)
	{
		//special system message
		GetGameLogic()->HandleMessageString(m_text);
		return;
	}
	if (!m_pEnt) 
	{
		//the ent must have been deleted
		return;
	}

	if (m_pEnt->GetType() < 0 || m_pEnt->GetType() > 20000)
	{
		//this must be an invalid entity
		LogError("Entity ID %d has a strange SetType set. %s.  Avoiding crash.", m_targetID, m_text.c_str());
		return;
	}
	m_pEnt->HandleMessageString(m_text);
}

void Message::SetTarget(unsigned int entID)
{
	assert(!m_pEnt && "messages can't be reassigned in mid transit!");
	
	if (entID == 0)
	{
		//special kind of message that we delivery directly to the system
		m_targetID = 0;
	}  else
	{
		m_targetID = entID;
		m_pEnt = EntityMgr->GetEntityFromID(m_targetID);

		if (!m_pEnt) 
		{
			LogMsg("Unable to schedule message, entID %d doesn't exist", entID);
			assert(m_pEnt);
			return;
		}

		if (m_pEnt->GetDeleteFlag())
		{
			LogMsg("Ignoring Schedule() command for EntID %d (%s), it's marked for deletion. (\"%s\" would never have been delivered)", m_pEnt->ID(), m_pEnt->GetName().c_str(),m_text.c_str());
			m_pEnt = NULL;
			return;
		}

		m_slots.connect(m_pEnt->sig_delete, this, &Message::EntDeleted);
	}
	
	
}
