

/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 23:1:2006   14:47
*/


#ifndef EntCollisionEditor_HEADER_INCLUDED // include guard
#define EntCollisionEditor_HEADER_INCLUDED  // include guard

class Tile;
#include "CollisionData.h"
#include "BaseGameEntity.h"

#define C_INVALID_VERT INT_MAX
class EntCollisionEditor: public BaseGameEntity
{
public:

	EntCollisionEditor();
	virtual ~EntCollisionEditor();
	void Init(CL_Vector2 vPos, CL_Rect vEditBounds, CollisionData *pCollisionData, bool calcOffsets);
	void Render(void *pTarget);
	void Update(float step);
	void OnOk();
	bool GetDataChanged() {return m_bDataChanged;}
	void SetSnap(bool bOn);
	void SetClip(bool bOn);
	void SetBullsEye(CL_Vector2 v)
	{
		m_trueEntityPosition = v;
	}

private:
	
	void OnHideModeChanged(bool bHide);

enum
{
 e_modeAddCreate,
 e_modeAdjust,
 //e_modeDelete,
 e_modeCount
};

	void OnButtonUp(const CL_InputEvent &key);
	void OnButtonDown(const CL_InputEvent &key);
	void OnMouseMove(const CL_InputEvent &key);
	void Kill();
	void OnRadioButtonChange(CL_RadioButton *pSelectedButton);
	void AddCreateVert(CL_Vector2 vecPos);
	void DeleteVert(CL_Vector2 vecPos);
	void AdjustVert(CL_Vector2 vecPos);
	void GetIndexOfLineAtThisPos(point_list &pointList, CL_Vector2 &localPos, CL_Vector2 **ppVertAOut, CL_Vector2 **ppVertBOut, unsigned int &lineOutA, unsigned int &lineOutB);
	void OnCancel();
	void ClearSelection();
	bool GetIndexOfVertAtThisPos(point_list &pList, CL_Vector2 &vLocalPos, CL_Vector2 ** ppVertOut, unsigned int &vertIDOut, float fPadding);
	void OnChangeLineType();
	void ApplySnap(CL_Vector2 &vec);
	void OnClear();
	void SetCollisionLineLabel();
	void OnNextLine();
	void OnPrevLine();
	void SetupLineForEdit(int line);
	void SetClippedDefaultBasedOnLine();
	void ModifyShapePosition(CL_Point pt);

	//GUI
	CL_SlotContainer m_slots;
	CL_Window * m_pWindow;
	CL_Button *m_pButtonOK, *m_pButtonCancel;
	CL_Label *m_pLabel, *m_pLabelCurLine;
	CL_Rectf m_rectArea; //size we can draw
	CL_Vector2 m_pos;
	CollisionData *m_pCollisionData; //the master collision data we should copy to when we're done to save it
	CollisionData m_col; //collision data we work with
	PointList *m_pActiveLine; //null if none
	CL_Point m_vecLastMousePos;
	int m_mode;
	CL_RadioButton *m_pRadioArray[e_modeCount];
	CL_RadioGroup * m_pRadioGroup;
	unsigned int m_dragVertID; //C_INVALID_VERT if no  vert selected
	CL_ListBox * m_pListType;
	CL_Vector2 *m_pSelectedVert;
	CL_CheckBox *m_pCheckBoxSnap;
	CL_CheckBox *m_pCheckBoxClipToImage;
	CL_Vector2 m_dragStartPos ;
	bool m_bDataChanged;
	int m_curLine;

	CL_Vector2 m_trueEntityPosition;

};

void RenderVectorPointList(const CL_Vector2 &vecPos, PointList &pl, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride);
void RenderVectorCollisionData(const CL_Vector2 &vecPos, CollisionData &col, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride);
void RenderTileListCollisionData(tile_list &tileList, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride);


#endif                  // include guard
