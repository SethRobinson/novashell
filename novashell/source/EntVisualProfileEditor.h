//  ***************************************************************
//  EntVisualProfileEditor - Creation date: 08/21/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntVisualProfileEditor_h__
#define EntVisualProfileEditor_h__

#include "CollisionData.h"
#include "BaseGameEntity.h"

class MovingEntity;

class EntVisualProfileEditor: public BaseGameEntity
{
public:
	EntVisualProfileEditor();
	virtual ~EntVisualProfileEditor();

	bool Init(Tile *pTile);
	void OnEditorClosed(int entID);
	virtual void Render(void *pTarget);
	bool IsDialogOpen();
	void OnEntityDeleted(int entID);
	void OffsetChanged();
	void OnOffsetChanged(const string &st)
	{
		OffsetChanged();
	};

protected:
	
	void OnButtonUp(const CL_InputEvent &key);
	void OnButtonDown(const CL_InputEvent &key);
	void OnMouseMove(const CL_InputEvent &key);

	void OnClose();
	
	void ModifyActiveAnim(CL_Point pt);

	void OnChangeAnim();
	void OnChangeAlignment();
	
	void MoveAnimSelection(int offset);

	//GUI
	CL_SlotContainer m_slots;
	CL_Window * m_pWindow;
	CL_ListBox * m_pListAnims, *m_pListAlignment;

	MovingEntity *m_pEnt; //the object we're working with, or null if not an entity
	TilePic *m_pTilePic; //null if not a tilepic
	Tile *m_pBaseTile; //in both cases, they do have a base tile we can access
	CL_InputBox * m_pAlignX, *m_pAlignY;
private:
};

#endif // EntVisualProfileEditor_h__
