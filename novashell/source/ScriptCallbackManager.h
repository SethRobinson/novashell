//  ***************************************************************
//  ScriptCallbackManager - Creation date: 03/08/2008
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2008 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ScriptCallbackManager_h__
#define ScriptCallbackManager_h__

class MovingEntity;

class ScriptCallbackItem
{
public:

	string m_callback;
	int m_entityID;

};


class ScriptCallbackManager
{
public:
	ScriptCallbackManager();
	virtual ~ScriptCallbackManager();

	bool Add(const string &callbackFunction, int entityID);
	void OnActivate();
	void OnActivate(float fNum);
	void Reset();
	void OnEntDeleted(int entID);

protected:
	

private:

	CL_SlotContainer m_slots;
	list<ScriptCallbackItem> m_items;
};

#endif // ScriptCallbackManager_h__