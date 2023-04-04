#include "EntityManager.h"
#include "BaseGameEntity.h"


//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
EntityManager* EntityManager::Instance()
{
  static EntityManager instance;

  return &instance;
}

//------------------------- GetEntityFromID -----------------------------------
//-----------------------------------------------------------------------------
BaseGameEntity* EntityManager::GetEntityFromID(int id)const
{
  //find the entity
  EntityMap::const_iterator ent = m_EntityMap.find(id);

  //assert that the entity is a member of the map
  
  if (ent == m_EntityMap.end()) return NULL;
  //assert ( (ent !=  m_EntityMap.end()) && "<EntityManager::GetEntityFromID>: invalid ID");

  return ent->second;
}
                             

//this is a slow look up, later we could easily add another map organized for fast string searches, hmm.
BaseGameEntity* EntityManager::GetEntityByName(std::string name)
{
    
    EntityMap::const_iterator ent = m_EntityMap.begin();
    for (; ent != m_EntityMap.end(); ++ent)
    {
        {
            if ( (*ent).second->GetName() == name)
			{
			
				if ( !(*ent).second->GetDeleteFlag())
				{
					return (*ent).second;
				}
			}
        }
    }
    
    //couldn't find it
    return NULL;

}



//--------------------------- RemoveEntity ------------------------------------
//-----------------------------------------------------------------------------
void EntityManager::RemoveEntity(BaseGameEntity* pEntity)
{    
  
	EntityMap::iterator ent = m_EntityMap.find(pEntity->ID());
	
	if (ent != m_EntityMap.end())
	{
		m_EntityMap.erase(ent);
	}
} 

//---------------------------- RegisterEntity ---------------------------------
//-----------------------------------------------------------------------------
void EntityManager::RegisterEntity(BaseGameEntity* NewEntity)
{
  m_EntityMap.insert(std::make_pair(NewEntity->ID(), NewEntity));
}
