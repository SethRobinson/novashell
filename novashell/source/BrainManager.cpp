#include "AppPrecomp.h"
#include "BrainManager.h"
#include "Brain.h"
#include "State.h"

BrainRegistry::BrainRegistry()
{
	
}

void BrainRegistry::ListAllBrains()
{
	LogMsg("Showing all %d valid brain types.", m_brainMap.size());

	brainMap::iterator itor = m_brainMap.begin();
	while (itor != m_brainMap.end())
	{
		LogMsg(itor->second->GetName());
		itor++;
	}
}

Brain * BrainRegistry::CreateBrainByName(const string &brainName, MovingEntity *pParent)
{
	brainMap::iterator itor = m_brainMap.find(brainName);

	if (itor == m_brainMap.end())
	{
		return NULL; //failed, it will handle  the error message later
	}

	return itor->second->CreateInstance(pParent);
}

BrainRegistry * BrainRegistry::GetInstance()
{
	static BrainRegistry theInstance;
	return &theInstance;
}

void BrainRegistry::Add(Brain *pBrain)
{
	brainMap::iterator itor = m_brainMap.find(pBrain->GetName());

	if (itor != m_brainMap.end())
	{
		LogError("Brain %s was already registered.  Only do this once.", pBrain->GetName());
		return;
	}

	m_brainMap[pBrain->GetName()] = pBrain;

}



StateRegistry::StateRegistry()
{

}

void StateRegistry::ListAllStates()
{
	LogMsg("Showing all %d valid State types.", m_stateMap.size());

	stateMap::iterator itor = m_stateMap.begin();
	while (itor != m_stateMap.end())
	{
		LogMsg(itor->second->GetName());
		itor++;
	}
}

State * StateRegistry::CreateStateByName(const string &stateName, MovingEntity *pParent)
{
	stateMap::iterator itor = m_stateMap.find(stateName);

	if (itor == m_stateMap.end())
	{
		LogError("State %s is unknown.  It's case sensitive.", stateName.c_str());
		ListAllStates();
		return NULL; //failed, it will handle  the error message later
	}

	return itor->second->CreateInstance(pParent);
}

StateRegistry * StateRegistry::GetInstance()
{
	static StateRegistry theInstance;
	return &theInstance;
}

void StateRegistry::Add(State *pState)
{
	stateMap::iterator itor = m_stateMap.find(pState->GetName());

	if (itor != m_stateMap.end())
	{
		LogError("State %s was already registered.  Only do this once.", pState->GetName());
		return;
	}

	m_stateMap[pState->GetName()] = pState;

}


BrainManager::BrainManager()
{
	m_pParent = NULL;
	m_pActiveState = NULL;
	m_pLastState = NULL;
	m_pBrainBase = NULL;
}

BrainManager::~BrainManager()
{
	
	//hmm, do we want to run its OnRemove() here?  Maybe not.
	
	Kill();	
}

void BrainManager::Kill()
{
	
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{

		delete *itor;
		itor++;
	}

	m_brainVec.clear();

	SAFE_DELETE(m_pActiveState);
	SAFE_DELETE(m_pLastState);
	m_pBrainBase = NULL;

}


bool compareBrainBySort(const Brain *pA, const Brain *pB) 
{
	return pA->GetSort() < pB->GetSort();
}

void BrainManager::Sort()
{
	m_brainVec.sort(compareBrainBySort);
}

Brain * BrainManager::Add(const string &brainName, const string &initMsg)
{

	Brain *pNewBrain = BrainRegistry::GetInstance()->CreateBrainByName(brainName, m_pParent);

	if (!pNewBrain)
	{
		LogError("Brain name %s was not found in the brain registry.  It's case sensitive.  Valid brains are:", brainName.c_str());
		BrainRegistry::GetInstance()->ListAllBrains();
		return NULL;
	}
	
     m_brainVec.push_back(pNewBrain);
	 Sort();

	 pNewBrain->OnAdd();
	 if (initMsg.size() > 0)
	 {
		 pNewBrain->HandleMsg(initMsg);
	 }

	 return pNewBrain; //just in case we want to fool with it
}

void BrainManager::SendToBrainByName(const string &brainName, const string &msg)
{
	Brain *pBrain = GetBrainByName(brainName);
	if (!pBrain)
	{
		LogMsg("Can't locate brain %s to send %s to it.", brainName.c_str(), msg.c_str());
		return;
	}

	pBrain->HandleMsg(msg);
}

void BrainManager::SendToBrainBase(const string &msg)
{
	if (!GetBrainBase())
	{
		LogError("No base brain assigned yet.");
		return;
	}

	GetBrainBase()->HandleMsg(msg);
}

void BrainManager::SendToBrainBaseSimple(int message, int user1, int user2)
{
	if (!GetBrainBase())
	{
		LogError("No base brain assigned yet.");
		return;
	}

	GetBrainBase()->HandleSimpleMessage(message, user1, user2);
}

string BrainManager::AskBrainByName(const string &brainName, const string &msg)
{
	Brain *pBrain = GetBrainByName(brainName);
	if (!pBrain)
	{
		LogMsg("Can't locate brain %s to ask it %s.", brainName.c_str(), msg.c_str());
		return "";
	}

	return pBrain->HandleAskMsg(msg);
}

Brain * BrainManager::GetBrainByName(const string &brainName)
{
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		if ( !(*itor)->GetDeleteFlag())
		if ( (*itor)->GetName() == brainName)
		{
			//found it
			return (*itor);
		}
		itor++;
	}

	return NULL; //couldn't find this brain
}

int BrainManager::Remove(const string &brainName)
{
	int count = 0;
	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		if ( !(*itor)->GetDeleteFlag())
			if ( (*itor)->GetName() == brainName)
			{
				//found it
				count++;
				(*itor)->SetDeleteFlag(true);
			}
			itor++;
	}

	return count;
}


void BrainManager::SetBrainBase(Brain *pBrain)
{
	if (m_pBrainBase)
	{
		LogError("Uh, shouldn't you add code to remove brain %s first?", m_pBrainBase->GetName());
	}
	m_pBrainBase = pBrain;

	//notify everywhere the base brain changed here?
}

State * BrainManager::SetState(State *pState)
{
	if (!m_pBrainBase)
	{
		LogError("Must add a base brain before state %s can be set.", pState->GetName());
		return NULL;
	}

	if (!pState)
	{
		return NULL;
	}
	if (m_pActiveState)
	{
		m_pActiveState->OnRemove();
	}

	SAFE_DELETE(m_pLastState);
	m_pLastState = m_pActiveState; //save this for later queries, might be useful
	m_pActiveState = pState;
	m_pActiveState->OnAdd();
	return m_pActiveState;
}

State * BrainManager::SetStateByName(const string &stateName)
{
	if (InState(stateName)) return m_pActiveState;
	return SetState(StateRegistry::GetInstance()->CreateStateByName(stateName, m_pParent));
}

bool BrainManager::SetStateByNameNoReturn(const string &stateName)
{
	if (InState(stateName)) return false;
	SetState(StateRegistry::GetInstance()->CreateStateByName(stateName, m_pParent));
	return true; //state was changed
}

const char * BrainManager::GetStateByName()
{
	if (!m_pActiveState)
	{
		return "";
	}
	return m_pActiveState->GetName();
}


bool BrainManager::InState(const string &stateName)
{
	 return (m_pActiveState && stateName == m_pActiveState->GetName());
}

bool BrainManager::LastStateWas(const string &stateName)
{
	return (m_pLastState && stateName == m_pLastState->GetName());
}

void BrainManager::Update(float step)
{

	if (m_pActiveState) m_pActiveState->Update(step);

	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		if ( !(*itor)->GetDeleteFlag())
		{
			(*itor)->Update(step);
		}

		itor++;
	}


}


void BrainManager::Render(void *pTarget)
{

	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		if ( !(*itor)->GetDeleteFlag())
		{
			(*itor)->Render(pTarget);
		}

		itor++;
	}


}

void BrainManager::PostUpdate(float step)
{

	if (m_pActiveState) m_pActiveState->PostUpdate(step);

	brain_vector::iterator itor = m_brainVec.begin();
	while (itor != m_brainVec.end())
	{
		
		if ((*itor)->GetDeleteFlag())
		{
			(*itor)->OnRemove();
			delete *itor;

			itor = m_brainVec.erase(itor);
			continue;
		} else
		{
			(*itor)->PostUpdate(step);

		}

		itor++;
	}

}

/*
Object: BrainManager
All <Entity> objects can use their <BrainManager> to add brains to control behavior and functions.

Group: Brain Related

func: Add
(code)
nil Add(string brainName, string initMsg)
(end)

Adds a new brain.

An entity can use many brains at once, most can play nicely and intelligently with eachother.

Some brains perform complex duties, like path-finding and movement, others very simple things, like shaking, bobbing, or fading out and deleting themselves.

Parameters:

brainName - A string containing the name of a valid brain that can be constructed, for example, <Bob>.
initMsg - A string with any special information you want to send this brain. (can be a blank string)


func: Remove
(code)
number Remove(string brainName)
(end)

Parameters:

brainName - The name of the brain you'd like to remove.

Returns:

How many brains were actually removed.

func: GetBrainByName
(code)
Brain GetBrainByName(string brainName)
(end)

This lets you see if an entity has a specific brain and returns it.

Note:

<Brain> Is undocumented right now and only has a GetName() function, so this function is really only for tested to see if a certain brain exists or not at this time.

Parameters:

brainName - The name of the brain you'd like returned.

Returns:  A <Brain> object or nil if no brain by this name exists in this entities <BrainManager>.

func: SendToBrainByName
(code)
nil SendToBrainByName(string brainName, string msg)
(end)

Allows you to send a command to a brain.

Parameters:

brainName - The name of the brain you'd like to communicate with.
msg - The string of text you'd like to send.  Different brains respond to different commands.


func: SendToBrainBase
(code)
nil SendToBrainBase(string msg)
(end)

Allows you to send a command to the "base" brain if one has been added.

Certain complex brains may register themselves as the "base brain".  Right now only one, the <StandardBase> does this.

By registering as the "Base", it's letting the world and other brain's know it can handle certain commands and services.

Parameters it understands:

msg - The string of text you'd like to send.

func: AskBrainByName
(code)
string AskBrainByName(string brainName, string msg)
(end)

Allows you to send a message to a brain *and* receive its reply.

Parameters:

brainName - The name of the brain you'd like to communicate with.
msg - The string of text you'd like to send.  Different brains respond to different commands.

Returns:

The brain returns a string of data in response to the request made.

Group: State Related

func: SetStateByName
(code)
nil SetStateByName(string stateName)
(end)

Unlike brains, there can only be *one* state active at any time.

States such as walking, attacking, and idling allow visuals and AI related functions to all stay on the same page and an easy way to control entities.

Example of setting the state to <Idle> to stop an entity from moving, and to play the idle animation if it exists in his visual profile:

(code)
this:GetBrainManager():SetStateByName("Idle");
(end)

Note:

If the <Entity> is already in the desired state, the command is ignored.

Parameters:

stateName - A string containing the name of a valid state that can be activated, for example, <Idle>.

func: GetActiveStateName
(code)
string GetActiveStateName()
(end)

Returns:

The name of the active state.

func: InState
(code)
boolean InState(string stateName)
(end)

Parameters:

stateName - The name of the state you'd like to see if the entity is currently in.

Returns:

True if active state matches the state name sent.

func: LastStateWas
(code)
boolean LastStateWas(string stateName)
(end)

Parameters:

stateName - The name of the state you'd like to see if the entity was last in, before the currently active state was set.

Returns:

True if the last state that was active (before the current one was activated) matches the name sent.


*/


