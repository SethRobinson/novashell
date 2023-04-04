#include "AppPrecomp.h"
#include "MyEntityManager.h"
#include "main.h"
#include "BaseGameEntity.h"

MyEntityManager::MyEntityManager()
{
}

MyEntityManager::~MyEntityManager()
{
    Kill();
}

BaseGameEntity * MyEntityManager::Add(BaseGameEntity *pEnt)
{
    m_Entities.push_back(pEnt);
 	return pEnt; //just in case they need it..
}

bool MyEntityManager::Init()
{
	Kill();
    return true;
}

bool MyEntityManager::TagEntityForDeletionByName(std::string name)
{
    BaseGameEntity *pEnt = EntityMgr->GetEntityByName(name);

    if (!pEnt) return false;

    pEnt->SetDeleteFlag(true);
    return true;
}

void MyEntityManager::Kill()
{
    std::list<BaseGameEntity*>::iterator it = m_Entities.begin();
    for (it; it != m_Entities.end(); ++it)
    {
              delete *it;
    }
    m_Entities.clear();
    EntityMgr->Reset();
}

void MyEntityManager::Render()
{
   
	CL_GraphicContext *pGC = GetApp()->GetMainWindow()->get_gc();

	std::list<BaseGameEntity*>::const_iterator ent = m_Entities.begin();
    for (ent; ent != m_Entities.end(); ++ent)
    {
        {
            (*ent)->Render(pGC);
        }
    }
    
}

void MyEntityManager::RemoveEntitiesTaggedForDeletion()
{
    //remove anything that was tagged for deletion
    
    std::list<BaseGameEntity*>::iterator ent = m_Entities.begin();
    for (ent; ent != m_Entities.end();)
    {
        {
            if ((*ent)->GetDeleteFlag()) 
            {
                //actually delete it
                delete *ent;
                ent = m_Entities.erase(ent);
				continue;
            }
        }
		++ent;
    }

}

void MyEntityManager::Update(float step)
{
    std::list<BaseGameEntity*>::const_iterator ent = m_Entities.begin();
    for (ent; ent != m_Entities.end(); ++ent)
    {
        {
            (*ent)->Update(step);
        }
    }
    RemoveEntitiesTaggedForDeletion();
}
