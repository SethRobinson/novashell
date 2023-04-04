#include "AppPrecomp.h"
#include "BaseGameEntity.h"
#include "EntityManager.h"
#include "TileEntity.h"
#include "GameLogic.h"

int BaseGameEntity::m_iNextValidID = 101; //we reserve ID's less than this for special things

//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------
BaseGameEntity::BaseGameEntity(int ID):m_iType(C_ENTITY_TYPE_BASE),
                                       m_bTag(false)
{
    m_priorityLevel = C_PRIORITY_LEVEL_NORMAL;
	m_deleteFlag = false;
	m_pTile = NULL;
    SetID(ID);
	EntityMgr->RegisterEntity(this);
}

BaseGameEntity::~BaseGameEntity()
{
	if (!m_deleteFlag)
	{
		//special case for when we're abruptly deleted without using SetDeleteFlag
		sig_delete(m_ID);
	}
	
	//remove from the index map
	if (m_pTile)
	{
		m_pTile->SetEntity(NULL); //so it won't try to delete us again
	}

	EntityMgr->RemoveEntity(this);
}

bool BaseGameEntity::UnderPriorityLevel()
{
	//this is a hack for now, the plan is to add a GetGameLogic()->SetCurrentPriorityLevel() to test this against at some point

	if (!GetGameLogic()->GetGamePaused())
		return false;

	if (GetPriorityLevel() <= 0) return true; else return false;
}
void BaseGameEntity::SetDeleteFlag(bool bNew)
{

	if (bNew && bNew != m_deleteFlag)
	{
		sig_delete(m_ID);
	}
	
	m_deleteFlag = bNew;	
}


//----------------------------- SetID -----------------------------------------
//
//  this must be called within each constructor to make sure the ID is set
//  correctly. It verifies that the value passed to the method is greater
//  or equal to the next valid ID, before setting the ID and incrementing
//  the next valid ID
//-----------------------------------------------------------------------------
void BaseGameEntity::SetID(int val)
{
  //make sure the val is equal to or greater than the next available ID
  assert ( (val >= m_iNextValidID) && "<BaseGameEntity::SetID>: invalid ID");
  m_ID = val;
  m_iNextValidID = m_ID + 1;
}

/*
Object: SpecialEntity
Special entities are not associated with a tile and have a limited API that is much simpler than a normal <Entity>.  These are used for things like dialogs or miscellaneous utilities that don't really exist inside a map and don't need to be saved/loaded with it.

Group: Member Functions

func: PlaceHolder
(code)
nil PlaceHolder()
(end)
Stuff coming later.

*/
