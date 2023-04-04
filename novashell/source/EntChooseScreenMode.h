/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 3/20/2005 7:31:35 PM
*/

#pragma once
#include "Map.h"
#include "BaseGameEntity.h"

class EntChooseScreenMode: public BaseGameEntity
{
public:
    

	EntChooseScreenMode();
    virtual ~EntChooseScreenMode();

    virtual void Render(void *pTarget);
    virtual void Update(float step);
	void GenerateThumbnailsIfNeeded();
	void QuickInit();
	bool IsGeneratingThumbnails() {return m_thumbList.size() != 0;};

private:
   
	void OnClose(CL_SlotParent_v0 &parent_handler);
	void OnMouseMove(const CL_InputEvent &key);

	void Init();
    void Kill();
    void onButtonDown(const CL_InputEvent &key);
    void DrawBlock(int x, int y);
    int GetScreenClicked(int x, int y);
	void CalculateMapSizes();
	void SetupInfoPanel(ScreenID screenID);
	void DrawPlayerPositionOverlay();
	CL_Vector2 WorldToScreen(CL_Vector2 vecWorldPos);
	void SetStatus(const string &strMsg);
	void SetGenericStatus();
	void DrawViewScreenOverlay();
	void ProcessNextThumb(); //returns true if one was actually generated
	void OnAutoResize();
	void OnClearThumbnails();

	CL_Slot m_slotClose;

	CL_SlotContainer m_slots;

	CL_Surface *m_pUnusedBlock, *m_pUsedBlock; 
	CL_Rect m_blockSize;

	CL_Point m_drawOffset;
	CL_Point m_vecLastMousePos;

	//for the GUI parts
	CL_Window *m_pWindow;
	CL_Label *m_pLabel, *m_pLabelStatus;
	CL_Menu *m_pMenu;

	ScreenID m_lastInfoScreenID; //C_INVALID_SCREEN if none shown so far
	CL_Surface *m_pThumb;
	deque<int> m_thumbList; //stores a list of thumnails we'll need to generate
};
