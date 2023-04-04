#include "AppPrecomp.h"
#include "GameLogic.h"
#include "ScriptManager.h"
#include "ScriptCallbackManager.h"

ScriptCallbackManager::ScriptCallbackManager()
{
}

ScriptCallbackManager::~ScriptCallbackManager()
{
}

bool ScriptCallbackManager::Add( const string &callbackFunction, int entityID )
{
	ScriptCallbackItem item;
	item.m_callback = callbackFunction;
	item.m_entityID = entityID;

	if (item.m_entityID != C_ENTITY_NONE)
	{
		BaseGameEntity * m_pEnt = EntityMgr->GetEntityFromID(item.m_entityID);

		if (!m_pEnt || m_pEnt->GetDeleteFlag()) 
		{
			LogMsg("Can't add script callback to %s, entID %d doesn't exist", callbackFunction.c_str(), item.m_entityID );
			return false;
		}

		m_slots.connect(m_pEnt->sig_delete, this, &ScriptCallbackManager::OnEntDeleted);
	}
	m_items.push_back(item);
	return true;
}

void ScriptCallbackManager::OnActivate()
{

	//run through and call them
	list<ScriptCallbackItem>::iterator listItor = m_items.begin();

	while (listItor != m_items.end())
	{
		//LogMsg("Calling %s", listItor->m_callback.c_str());
		
		if (listItor->m_entityID == C_ENTITY_NONE)
		{
			//global kind of thing
			if (!GetScriptManager->FunctionExists(listItor->m_callback))
			GetScriptManager->SetStrict(false);

			try {
					luabind::call_function<void>(GetScriptManager->GetMainState(), 
					listItor->m_callback.c_str());
				}  LUABIND_CATCH(listItor->m_callback);

			GetScriptManager->SetStrict(true);

		} else
		{
			MovingEntity *m_pParent = (MovingEntity*) EntityMgr->GetEntityFromID(listItor->m_entityID);
			if (!m_pParent)
			{
				LogMsg("Script callback - Can't locate entity %d to run %s", listItor->m_entityID, listItor->m_callback.c_str());
			} else
			{
				//let's actually run this function in this entities namespace
				if (!m_pParent->GetScriptObject()->FunctionExists(listItor->m_callback))
				GetScriptManager->SetStrict(false);

				try {luabind::call_function<void>(m_pParent->GetScriptObject()->GetState(),listItor->m_callback.c_str());
				} LUABIND_ENT_BRAIN_CATCH(listItor->m_callback.c_str());
				GetScriptManager->SetStrict(true);
			}
		}
		listItor++;
	}

}


void ScriptCallbackManager::OnActivate(float fNum)
{

	//run through and call them
	list<ScriptCallbackItem>::iterator listItor = m_items.begin();

	while (listItor != m_items.end())
	{
		//LogMsg("Calling %s", listItor->m_callback.c_str());

		if (listItor->m_entityID == C_ENTITY_NONE)
		{
			//global kind of thing
			if (!GetScriptManager->FunctionExists(listItor->m_callback))
				GetScriptManager->SetStrict(false);

			try {
				luabind::call_function<void>(GetScriptManager->GetMainState(), 
					listItor->m_callback.c_str(), fNum);
			}  LUABIND_CATCH(listItor->m_callback);

			GetScriptManager->SetStrict(true);

		} else
		{
			MovingEntity *m_pParent = (MovingEntity*) EntityMgr->GetEntityFromID(listItor->m_entityID);
			if (!m_pParent)
			{
				LogMsg("Script callback - Can't locate entity %d to run %s", listItor->m_entityID, listItor->m_callback.c_str());
			} else
			{
				//let's actually run this function in this entities namespace
				if (!m_pParent->GetScriptObject()->FunctionExists(listItor->m_callback))
					GetScriptManager->SetStrict(false);

				try {luabind::call_function<void>(m_pParent->GetScriptObject()->GetState(),listItor->m_callback.c_str(), fNum);
				} LUABIND_ENT_BRAIN_CATCH(listItor->m_callback.c_str());
				GetScriptManager->SetStrict(true);
			}
		}
		listItor++;
	}

}

void ScriptCallbackManager::Reset()
{
	m_items.clear();
}

void ScriptCallbackManager::OnEntDeleted( int entID )
{
	//kill any callbacks that were going to send to this entity

	//run through and call them
	list<ScriptCallbackItem>::iterator listItor = m_items.begin();

	while (listItor != m_items.end())
	{
		if (listItor->m_entityID == entID)
		{
			listItor = m_items.erase(listItor);
			continue;
		}
		listItor++;
	}
}