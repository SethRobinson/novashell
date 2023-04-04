#include "AppPrecomp.h"
#include "TagManager.h"
#include "MapManager.h"
#include "MovingEntity.h"
#include "AI/WorldNavManager.h"

const string &TagObject::GetMapName()
{
	return m_pWorld->GetName();
}

TagManager::TagManager()
{
}

TagManager::~TagManager()
{
}

void TagManager::Kill()
{
	m_tagMap.clear();
}

void TagManager::RegisterAsWarp(MovingEntity *pEnt, const string &targetName)
{

	TagObject * pTag = GetFromHash(pEnt->GetNameHash());
	if (!pTag)
	{
		LogError("Unable to locate entity %s to register as a warp object. It must be a named entity to use RegisterAsWarp.",
			pEnt->GetName().c_str());
		return;
	}

	if (pTag->m_graphNodeID != invalid_node_index) return;

	pTag->m_warpTarget = targetName;
	g_worldNavManager.AddNode(pTag);

	//in this case, we'd also want to connect it now
	g_worldNavManager.LinkNode(pTag);
}

void TagManager::Update(Map *pWorld, MovingEntity *pEnt)
{
	if (pEnt->GetName().empty()) return; //don't hash this

	unsigned int hashID = pEnt->GetNameHash();
	TagObject *pObject = NULL;

	assert(hashID != 0);

	TagResourceMap::iterator itor = m_tagMap.find(hashID);

	if (itor == m_tagMap.end()) 
	{
		//couldn't find it, add it
		list<TagObject> *pList = &m_tagMap[hashID];
		TagObject o;
		pList->push_back(o);
		pObject = &(*pList->begin());
		//LogMsg("Creating hash for id %d", pEnt->ID());
		pObject->m_tagName = pEnt->GetName();

	} else
	{
		//an entry exists.  but is it the right one?
		
		list<TagObject> *pTagList = &m_tagMap[hashID];
		list<TagObject>::iterator itorO = pTagList->begin();

		//go through each item and see if it's supposed to be us or not
		while (itorO != pTagList->end())
		{
			
			if ( itorO->m_entID == pEnt->ID()
				||
				(
				    (itorO->m_entID == 0)
				     && (itorO->m_pos == pEnt->GetPos())
				&&  (itorO->m_pWorld == pWorld)
				)
				)
			{
				//note, if itorO->m_entID == 0, it means we're overwriting temp cache data that we loaded
				//as a way to know about entities that don't really exist yet

				//this is it
				pObject = &(*itorO);

				//not only that, but let's move it up on  the list
				//LogMsg("updating existing (id %d)", pEnt->ID());
				break;
			}
			
			//we weren't the first item.   Bad.

			/*
			if (m_pEnt->GetNavNodeType() != C_NODE_TYPE_NORMAL)
			{
				//don't allow dupes of this type

			}
			*/

			//let's just not allow dupes at all
			
			string newName = pEnt->GetName() + char('A' + random_range(0,25));

			//LogMsg("Conflict, tagname %s already in use.  Making unique name %s.", pEnt->GetName().c_str(),
			//	newName.c_str());
			pEnt->GetTile()->GetParentScreen()->GetParentMapChunk()->SetDataChanged(true);
			pEnt->SetNameEx(newName, false);
			return;

			itorO++;
		}

	   if (!pObject)
	   {
			//this must be a new entry, add it
		   TagObject o;
		   pTagList->push_front(o);
		   pObject = &(*pTagList->begin());
		   pObject->m_tagName = pEnt->GetName();
	  // LogMsg("Added new (%d)",pEnt->ID());
	   }
	}

	pObject->m_pos = pEnt->GetPos();
	pObject->m_pWorld = pWorld;
	pObject->m_entID = pEnt->ID();
	pObject->m_hashID = hashID;
	
}

TagObject * TagManager::GetFromHash(unsigned int hashID)
{
	TagResourceMap::iterator itor = m_tagMap.find(hashID);
	
	if (itor == m_tagMap.end()) 
	{
		return NULL;
	}

	//there may be more than one, for now return the "top one"
	return & (*itor->second.rbegin());
}

TagObject * TagManager::GetFromString(const string &name)
{
	return GetFromHash(HashString(name.c_str()) );
}

CL_Vector2 TagManager::GetPosFromName(const string &name)
{
    TagObject *pTag = GetFromString(name);
	if (!pTag)
	{
		LogError("TagManager::GetPosFromName: Can't locate TAG %s, sending back 0.0", name.c_str());
		return CL_Vector2::ZERO;
	}
	return pTag->m_pos;
}

void TagManager::RemoveByHashID(unsigned int hashID, int entID)
{
	TagResourceMap::iterator itor = m_tagMap.find(hashID);

	if (itor == m_tagMap.end()) 
	{
		//it's not in here
		LogError("Failed to remove hash %d (ID %d)", hashID, entID);
		return;
	} else
	{
		list<TagObject> *pTagList = &m_tagMap[hashID];
		list<TagObject>::iterator itorO = pTagList->begin();

		if (itorO->m_graphNodeID != invalid_node_index)
			g_worldNavManager.RemoveNode(&(*itorO));

		m_tagMap.erase(itor);
		return;
	}

	LogError("Failed to remove hash %d (ID %d) in list search", hashID, entID);
}
void TagManager::Remove(MovingEntity *pEnt)
{
	if (!pEnt || pEnt->GetName().empty()) return;
	
	int hashID = pEnt->GetNameHash();
	
	//remove it, but make sure it's the right one

	RemoveByHashID(hashID, pEnt->ID());

}

void TagManager::PrintStatistics()
{
	LogMsg("");
	LogMsg("  ** TagManager Statistics **");

	//count instances

	string name;
	TagResourceMap::iterator itor = m_tagMap.begin();
	while (itor != m_tagMap.end()) 
	{
		//an entry exists.  but is it the right one?
		list<TagObject> *pTagList = &itor->second;
		list<TagObject>::iterator itorO = pTagList->begin();
		while (itorO != pTagList->end())
		{
			MovingEntity *pEnt = NULL;
			string extra;

			if (itorO->m_entID != 0)
			{
				pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(itorO->m_entID);
				if (pEnt)
				{
					extra = "GraphID: "+ CL_String::from_int(itorO->m_graphNodeID) + " WarpTarget: " + itorO->m_warpTarget;
					name = pEnt->GetName();
				} else
				{
					name = "Unable to locate entity! ";
				}
			} else
			{
				name = itorO->m_tagName;

				//it's not an entity, it's a tile.  But hey, can we make sure a tile actually exists at this position?
				//nah, too much work
			}


if (pEnt)
{
	assert(pEnt->GetName() == itorO->m_tagName && "Name mismatch! Out of date?");
}
			LogMsg("    Entity %s (%d) located at %s (in %s) Hash:%u %s", itorO->m_tagName.c_str(), itorO->m_entID, PrintVector(itorO->m_pos).c_str(), itorO->m_pWorld->GetName().c_str(), 
				itorO->m_hashID, extra.c_str());

			itorO++;
		}

  	itor++;	
	}

	LogMsg("    %d tag names active.\n", m_tagMap.size());
}

void TagManager::Save(Map *pWorld)
{
  //cycle through and save all tag data applicable
	
	CL_OutputSource *pFile = g_VFManager.PutFile(pWorld->GetDirPath()+C_TAGCACHE_FILENAME);
	
	
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

    helper.process_const(C_TAG_DATA_VERSION);

	int tag = E_TAG_RECORD;

	TagResourceMap::iterator itor = m_tagMap.begin();
	while (itor != m_tagMap.end()) 
	{
		//an entry exists.
		list<TagObject> *pTagList = &itor->second;
		list<TagObject>::iterator itorO = pTagList->begin();
		while (itorO != pTagList->end())
		{
			//save out our entries if this is really from our world
				if (itorO->m_pWorld == pWorld)
				{
					
					if (itorO->m_entID != 0)
					{
						//let's get data directly from the entity first to fix any sync problems if it wasn't updated
						//on the last frame
						MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(itorO->m_entID);
						if (!pEnt)
						{
							LogError("Tagcache data for %s is wrong", itorO->m_tagName.c_str());
						} else
						{
							if (!pEnt->GetPersistent())
							{
								//uh oh, the entity is not going to be saved, this tag stuff shouldn't either
								itorO++;
								continue;

							}
							itorO->m_pos = pEnt->GetPos();
						}

					}
					
					helper.process(tag);
					helper.process_const( itor->first);
					
					helper.process(itorO->m_pos);
					helper.process(itorO->m_tagName);
					helper.process(itorO->m_warpTarget);

				}
				itorO++;
		}

		itor++;	
	}

	tag = E_TAG_DONE;
	helper.process(tag);

	SAFE_DELETE(pFile);
}

void TagManager::AddCachedNameTag(unsigned int hashID, const TagObject &o)
{
	
	TagResourceMap::iterator itor = m_tagMap.find(hashID);


	list<TagObject> *pList = &m_tagMap[hashID];

	if (pList->empty()) 
	{
		//couldn't find it, add it
		pList->push_back(o);
	} else
	{
		//add it to the end of whatever is here
		pList->push_front(o);
	}

	TagObject *pTag =  &(*pList->begin());
	
	pTag->m_hashID = hashID;
	
   if (!pTag->m_warpTarget.empty())
   {
	   g_worldNavManager.AddNode(&(*pList->begin()));
   }

}

void TagManager::Load(Map *pWorld)
{
	//LogMsg("Loading tag data..");
	string fullPath = pWorld->GetDirPath()+C_MAP_DAT_FILENAME;

	//ok, let's make sure we load the tagcache from the dir that actually has a world in it
	
	CL_InputSource *pFile =  g_VFManager.GetFile(CL_String::get_path(fullPath)+"/"+C_TAGCACHE_FILENAME);

	if (!pFile) return;
	
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	//load everything we can find

	int version;
	helper.process(version);
		
	CL_Vector2 pos;
	int tag;
	TagObject o;
	o.m_entID = 0;
	o.m_pWorld = pWorld;

	
	unsigned int hashID;

	try
	{

	while(1)
	{
		helper.process(tag);

		switch (tag)
		{
		case E_TAG_DONE:
			SAFE_DELETE(pFile);
			return;

		case E_TAG_RECORD:

			helper.process(hashID);
			
			helper.process(o.m_pos);
			helper.process(o.m_tagName);
			helper.process(o.m_warpTarget);

			AddCachedNameTag(hashID, o);
			break;

		default:
			throw CL_Error("Error reading tagdata");
		}
	}
	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage(error.message.c_str(), "Error loading tagcache data.  Corrupted?");
		SAFE_DELETE(pFile);
		return;
	}
	

}


/*
Object: TagManager
When an <Entity> is "named", it's instantly locatable throughout the world at all times through the tag manager.

About:

In most cases usage of the <TagManager> happens behind the scenes, for instance, when you use <GetEntityByName>.

If you need to be able to locate entities without actually loading them or their <Map>, using the <TagManager> directly is useful.

Usage:

(code)
local tag = GetTagManager:GetFromString("Gary");
(end)

Group: Member Functions

func: GetFromString
(code)
Tag GetFromString(string name)
(end)
Returns information about a named <Entity>.  Very fast.

Parameters:

name - the name of an <Entity>.

Usage:
(code)
local tag = GetTagManager:GetFromString("Gary");

if (tag == nil) then
		LogMsg("Gary doesn't exist!");
	else
		LogMsg("Gary is located on the map called " .. tag:GetMapName());
		LogMsg("Gary's xy location is " .. tostring(tag:GetPos());

		if (tag:GetID() == C_ENTITY_NONE) then
				LogMsg("Gary exists, but he hasn't actually been loaded yet.");
			else
				LogMsg("Gary exists and has been loaded, his entityID is " .. tag:GetID());
			end
	end
	
(end)

Returns:
A <Tag> object containing inforamtion about the <Entity> or nil if the name doesn't exist.

func: GetPosFromName
(code)
Vector2 GetPosFromName(string name)
(end)
Allows you to quickly get the position of a named <Entity> that may or may not be loaded.

Returns:
A <Vector2> object containing the position.  If not found, will return 0,0 and an error will pop  up.

func: RegisterAsWarp
(code)
nil RegisterAsWarp(Entity ent, string targetName)
(end)
Connects two entities together in the nagivational graph.  This is so the path-finding engine can understand warps, even those that lead to other <Map>s.
For a two-way warp, the other side must also register as a warp.


Note:

You probably shouldn't play with this directly.  But if you do, you must put it in the function "OnMapInsert" in the script of an <Entity>.

Parameters:

ent - The source <Entity> connection.
targetName - The destination <Entity> name. (Doesn't have to actually be loaded)
*/