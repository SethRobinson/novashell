// ***************************************************************
//  TagManager - date created: 04/22/2006
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com) 
//  Copyright (C) 2006 Robinson Technologies - All Rights Reserved

#ifndef TagManager_HEADER_INCLUDED // include guard
#define TagManager_HEADER_INCLUDED  // include guard

#include "AI/NodeTypeEnumerations.h"

class Map;
class MovingEntity;

const int C_TAG_DATA_VERSION  = 1;
#define C_TAGCACHE_FILENAME "tagcache.dat"

class TagObject
{
public:

	TagObject()
	{
		m_graphNodeID = invalid_node_index;
	}
	CL_Vector2 & GetPos() {return m_pos;}
	const string &GetMapName();
	int GetID() {return m_entID;} //will be 0 unless already loaded, this is normal

	CL_Vector2 m_pos;
	Map *m_pWorld;
	int m_entID; //so we can keep track of the owner of a tag, even with multiple tags of the same name
	
	string m_warpTarget; //if not empty, this is the tagname of where this can warp to.  It's used in the
	//inter-town navigation only.
	int m_graphNodeID; //used for inter-town navigation only, -1 if not used
	unsigned int m_hashID; //sometimes I need it, trust me
	string m_tagName; //purely for debug info, not really needed
};

typedef std::map<unsigned int, std::list<TagObject> > TagResourceMap;

class TagManager
{
public:

    TagManager();
    ~TagManager();

	void Kill();
	void Update(Map *pWorld, MovingEntity *pEnt);
	void Remove(MovingEntity *pEntity);
	void RemoveByHashID(unsigned int hashID, int entID); //should never be used except to erase a bad tag, just use 0 for the entity
	TagObject * GetFromString(const string &name);
	TagObject * GetFromHash(unsigned int hashID);
	void Save(Map *pWorld);
	void Load(Map *pWorld);
	void PrintStatistics();
	CL_Vector2 GetPosFromName(const string &name);

	void RegisterAsWarp(MovingEntity *pEnt, const string &targetName);
	int GetCount() {return m_tagMap.size();}

protected:

private:

	enum
	{
		E_TAG_RECORD,
		E_TAG_DONE
	};

	void AddCachedNameTag(unsigned int hashID, const TagObject &o);

	TagResourceMap m_tagMap;
};

/*
Object: Tag
Contains information about a named <Entity>.

About:

The function <TagManager::GetTagFromName> returns this.

Group: Member Functions

func: GetMapName
(code)
string GetMapName()
(end)
Returns:

The <Map> name that this <Entity> exists in.  The <Map> may or may not be loaded.

func: GetPos
(code)
Vector2 GetPos()
(end)
Returns:

A <Vector2> object containing the <Entity>'s position.

func: GetID
(code)
number GetID()
(end)
Returns:

The <Entity>'s ID or <C_ENTITY_NONE> if the <Entity> has not been loaded yet.
*/

#endif
