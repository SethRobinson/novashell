
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 3/18/2005 12:58:47 AM
*/


#pragma once

#pragma warning (disable:4786)

class BaseGameEntity;

class MyEntityManager
{
public:

    MyEntityManager();
    ~MyEntityManager();
    
    bool Init();
    void Kill();

    void Render();
    void Update(float step);
    BaseGameEntity * Add(BaseGameEntity *pEnt);
    bool TagEntityForDeletionByName (std::string name);

private:

    std::list<BaseGameEntity*>  m_Entities;
    void RemoveEntitiesTaggedForDeletion();

};

#define GetMyEntityMgr GetApp()->GetMyGameLogic()->GetMyEntityManager()

