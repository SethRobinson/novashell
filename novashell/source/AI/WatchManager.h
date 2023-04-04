//  ***************************************************************
//  WatchManager - Creation date: 09/06/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//watch entities and run their logic no matter where they are in the world
//Also independently can notify objects when they have become onscreen/offscreen visually

#ifndef WatchManager_h__
#define WatchManager_h__

class MovingEntity;

class WatchObject
{
public:

	enum eMode
	{
		MODE_NORMAL,
		MODE_SILENT //no notifications
	};
	unsigned int m_watchTimer;
	unsigned int m_entID;
	eMode m_watchMode;
};

typedef std::list<WatchObject> watch_list;

class WatchManager
{
public:
	WatchManager();
	virtual ~WatchManager();

	void Add(MovingEntity *pEnt, int timeToWatchMS);
	void Update(float step, unsigned int drawID);
	void PostUpdate(float step);
	void ProcessPendingEntityMovementAndDeletions();
	void OnEntityDeleted(MovingEntity *pEnt);
	void AddEntityToVisibilityList(MovingEntity *pEnt);
	void Clear();
	int GetWatchCount() {return m_watchList.size();}
	void RemoveFromVisibilityList(MovingEntity *pEnt);
	void Remove(MovingEntity *pEnt);
	void OnMapChange(const string &mapName);
	void AddEntityToPostUpdateList(MovingEntity *pEnt); //usually you wouldn't use this..!
	unsigned int GetVisibilityID() {return m_visibilityID;}
	bool IsEntityOnWatchList(int entID);
	void AddSilently(MovingEntity *pEnt, int timeToWatchMS);
	void Render();
	void AddWatched();
protected:

	watch_list m_watchList;
	vector<MovingEntity*> m_postUpdateList;
	vector<MovingEntity*> m_visibilityNotificationList; //entities we need to check to see if they weren't rendered the next frame
	unsigned int m_visibilityID; //changes every update cycle

private:
};

extern WatchManager g_watchManager;
#endif // WatchManager_h__
