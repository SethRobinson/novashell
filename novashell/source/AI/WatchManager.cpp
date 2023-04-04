#include "AppPrecomp.h"
#include "WatchManager.h"
#include "../MovingEntity.h"

WatchManager g_watchManager;


WatchManager::WatchManager()
{
}

WatchManager::~WatchManager()
{
}

void WatchManager::Clear()
{
	m_watchList.clear();
	m_postUpdateList.clear();
	m_visibilityNotificationList.clear();
}

void WatchManager::AddSilently(MovingEntity *pEnt, int timeToWatchMS)
{
	if (!pEnt->IsPlaced())
	{
		LogError("WatchManager::Add cannot add an entity to the watch list until it's actually placed on the map.  Do it in function OnMapInsert() instead?");
		return;
	}
	if (IsEntityOnWatchList(pEnt->ID())) return;


	WatchObject w;
	w.m_watchMode = WatchObject::MODE_SILENT;
	w.m_entID = pEnt->ID();
	w.m_watchTimer = GetApp()->GetGameTick()+timeToWatchMS;
	m_watchList.push_back(w);
}


void WatchManager::Add(MovingEntity *pEnt, int timeToWatchMS)
{
	if (!pEnt->IsPlaced())
	{
		LogError("WatchManager::Add cannot add an entity to the watch list until it's actually placed on the map.  Do it in function OnMapInsert() instead?");
		return;
	}
	//is it already on the list?

	watch_list::iterator itor;
	for (itor=m_watchList.begin(); itor != m_watchList.end(); ++itor)
	{
		if (itor->m_entID == pEnt->ID())
		{
			itor->m_watchTimer = GetApp()->GetGameTick()+timeToWatchMS;
			itor->m_watchMode = WatchObject::MODE_NORMAL;
			return;
		}
	}
	
	WatchObject w;
	w.m_watchMode = WatchObject::MODE_NORMAL;
	w.m_entID = pEnt->ID();
	w.m_watchTimer = GetApp()->GetGameTick()+timeToWatchMS;
	m_watchList.push_back(w);
}

void WatchManager::Remove(MovingEntity *pEnt)
{
	watch_list::iterator itor;

	for (itor=m_watchList.begin(); itor != m_watchList.end(); ++itor)
	{
		if (itor->m_entID == pEnt->ID())
		{
			itor = m_watchList.erase(itor);
			return;
		}
	}

	
}

bool WatchManager::IsEntityOnWatchList(int entID)
{
	watch_list::iterator itor;
	for (itor=m_watchList.begin(); itor != m_watchList.end(); ++itor)
	{
		if (itor->m_entID == entID)
		{
			return true;
		}
	}
	return false;
}

void WatchManager::Render()
{
	
}

void WatchManager::AddWatched()
{
	watch_list::iterator itor;

	MovingEntity *pEnt;

	for (itor=m_watchList.begin(); itor != m_watchList.end();)
	{
		pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(itor->m_entID);

		if (pEnt)
		{
			//force update if it needs it
			g_pMapManager->AddToEntityUpdateList(pEnt);
		}
		
		itor++;
	}

}


void WatchManager::Update(float step, unsigned int drawID)
{
	//if any of these entities have not been processed yet, let's do it
	m_postUpdateList.clear();

	MovingEntity *pEnt;
	unsigned int gameTick = GetApp()->GetGameTick();
	watch_list::iterator itor;
	bool bNotify;
	for (itor=m_watchList.begin(); itor != m_watchList.end();)
	{
		pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(itor->m_entID);

		if (!pEnt)
		{
			//entity doesn't exist anymore I guess
			itor = m_watchList.erase(itor);
			continue;
		}

		if (itor->m_watchTimer < gameTick)
		{
			//LogMsg("Removing %s from watch list", pEnt->GetName().c_str());
			bNotify = false;

			if (itor->m_watchMode == WatchObject::MODE_NORMAL)
			{
				bNotify = true;
			}
			itor = m_watchList.erase(itor);

			if (bNotify)
			{
				//now, it's just going to instantly run the "OnLoseVisible" again if we don't do something..
				pEnt->OnWatchListTimeout(pEnt->IsOnScreen());
			}
			continue;
		}

		itor++;
	}

	

}

void WatchManager::AddEntityToPostUpdateList(MovingEntity *pEnt)
{
	m_postUpdateList.push_back(pEnt);
}

void WatchManager::PostUpdate(float step)
{
	
	//at this point all entities who are going to run logic, have.  We can check our list to allow entities
	//to see if they were are no longer being updated or not

	for (unsigned int i=0; i < m_visibilityNotificationList.size(); i++)
	{
		if (m_visibilityNotificationList.at(i))
			m_visibilityNotificationList.at(i)->CheckVisibilityNotifications();
	}

	m_visibilityNotificationList.clear();

}

void WatchManager::OnMapChange(const string &mapName)
{

	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		if (m_postUpdateList.at(i))
			m_postUpdateList.at(i)->OnMapChange(mapName);
	}
}



void WatchManager::AddEntityToVisibilityList(MovingEntity *pEnt)
{
	m_visibilityNotificationList.push_back(pEnt);
}



void WatchManager::RemoveFromVisibilityList(MovingEntity *pEnt)
{
	for (unsigned int i=0; i < m_visibilityNotificationList.size(); i++)
	{
		//uh, is it possible this pointer could be bad?  Not sure
		if (pEnt == m_visibilityNotificationList.at(i))
		{
			m_visibilityNotificationList.at(i) = NULL;
		}
	}
}

void WatchManager::OnEntityDeleted(MovingEntity *pEnt)
{
	//allows us to remove someone from our list when he's deleted to avoid a crash
	for (unsigned int i=0; i < m_postUpdateList.size(); i++)
	{
		//uh, is it possible this pointer could be bad?  Not sure
		if (pEnt == m_postUpdateList.at(i))
		{
			m_postUpdateList.at(i) = NULL;
		}
	}

	if (pEnt->GetVisibilityNotifications())
	{
		RemoveFromVisibilityList(pEnt);
	}
}

/*
Object: WatchManager
Allows entities to function off-screen.

About:

This is a global object that can always be accessed.

Sometimes you want <Entities> to continue to function even when off screen or on a different <Map>.  

For instance, if an NPC needs to walk to his bed and sleep, he needs to be added to the watch manager so he can function offscreen.

When toggling the FPS display (Ctrl-F) the number after the W indicates how many entities are currently being "watched".

Usage:

(code)
//insure this entity will function for the next 5 seconds no matter where it is
GetWatchManager:Add(this, 1000*5); (5000 milliseconds)

(end)

Group: Member Functions

func: Add
(code)
nil Add(Entity ent, number timeToWatchMS)
(end)
If a function called "OnWatchTimeout" exists in the <Entity>'s namespace, it will be called when the watch times-out (finishes).
The function must accept a parameter that will be a boolean set to TRUE if the <Entity> is currently on the screen.
If an <Entity> is deleted who was on the watch list, it is automatically removed.

Usage:
(code)

//we want to know when the watch is over
OnWatchTimeout(bIsOnScreen)
	LogMsg("Watch complete, no longer letting this entity run on its own!");
	
	if (bIsOnScreen) then
		LogMsg("Entity is currently on the screen!");
	else
		LogMsg("Entity is off screen.");
	end
end
(end)

Notes:

If you would like an entity to become as its map is loaded, you can call this function in an entity's *OnMapInsert()* script function.

Parameters:

ent - The <Entity> that should remain active regardless of where it is.
timeToWatchMS - How long before this <Entity> returns to a normal state. (ie, unwatched) Use <C_TIME_FOREVER> to stay watched forever.

func: Remove
(code)
nil Remove(Entity ent)
(end)
Removes this <Entity> from the watch list.

Parameters:

The <Entity> that shouldn't be watched (run off-screen) anymore.

func: GetWatchCount
(code)
number GetWatchCount()
(end)
Returns:

How many entities are currently being watched.  (Allowed to function off-screen)

Section: Related Constants

Group: C_TIME_CONSTANTS
Used with <WatchManager::Add>.

constant: C_TIME_FOREVER
A very large number, as close to "forever" as we can get with milliseconds of time.
*/
