
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 4:1:2006   13:45
*/


#ifndef EntEditMode_HEADER_INCLUDED // include guard
#define EntEditMode_HEADER_INCLUDED  // include guard

#include "main.h"
#include "Screen.h"
#include "TileEditOperation.h"
#include "BaseGameEntity.h"


class EntCollisionEditor;
class DataObject;
class DataManager;
class HashedResource;

class EntEditMode: public BaseGameEntity
{
public:

	virtual void Render(void *pTarget);
	virtual void Update(float step);
	void SelectByLayer(const vector<unsigned int> &layerVec);
	void Init();

	EntEditMode();
    virtual ~EntEditMode();
	bool IsDialogOpen(bool bCheckModelessDialogToo, bool bIsZoomOrMovement = false);
	Tile * GetSingleSelectedTile(); //returns the single tile selected, or NULL if none or multiple are selected

protected:
	
	void ClearSelection();
    void Kill();
	void onButtonDown(const CL_InputEvent &key);
	void onButtonUp(const CL_InputEvent &key);
	void DrawActiveBrush(CL_GraphicContext *pGC);
	void OnDelete(TileEditOperation *pTileOperation);
	void OnDeleteSimple();
	void OnCopy();
	void OnCut();
	void OnPaste(TileEditOperation &editOperation, CL_Vector2 vecWorld, bool bSelectPasted = false);
	void OnPasteContext();
	void OnUndo();
	void AddToUndo(TileEditOperation *pTileOperation);
	void UpdateMenuStatus();
	CL_Vector2 ConvertMouseToCenteredSelectionUpLeft(CL_Vector2 vecMouse);
	void OnClose(CL_SlotParent_v0 &parent_handler);
	void OnSelectBaseTilePage();
	void BuildBaseTilePageBox();
	void BuildTilePropertiesMenu(TileEditOperation *pTileList);
	void OnProperties();
	void SnapCheckBoxedClicked();
	void SnapSizeChanged();
	void OnMouseMove(const CL_InputEvent &key);
	void CutSubTile(CL_Rect recCut);
	void OnDefaultTileHardness();
	void OnCollisionDataEditEnd(int id);
	void OnSnapSizeChanged(const string &st);
	void BuildDefaultEntity();
	void OffsetSelectedItems(CL_Vector2 vOffset, bool bBigMovement);
	virtual string HandleMessageString(const string &msg);
	void OnMapChange();
	void GetSettingsFromWorld();
	void ScaleUpSelected();
	void ScaleDownSelected();
	void ScaleSelection(CL_Vector2 vMod, bool bBigMovement);
	void OnSelectSimilar();
	void OnReplace();
	void OnDeleteBadTiles();
	void OnEditVisualProfile();

	void PopUpImagePropertiesDialog(const string &fileName, unsigned int resourceID);
	void OnSelectBaseTileDoubleClick(const CL_InputEvent &input);
	void OnCloseBaseTileDialog();
	void KillContextMenu();
	void OnHideModeChanged(bool bHide);

	CL_Slot m_slotClose;
	
	CL_SlotContainer m_slots;

	//for the GUI parts
	CL_Window *m_pWindow;
	CL_Label *m_pLabelSelection;
	CL_Label *m_pLabelMain;
	CL_CheckBox *m_pCheckBoxSnap;
	CL_InputBox *m_pInputBoxSnapSizeX; //what tile size we should emulate
	CL_InputBox *m_pInputBoxSnapSizeY; //what tile size we should emulate

	CL_Vector2 m_vecDragStart, m_vecScreenDragStop;
	TileEditOperation m_selectedTileList;
	bool m_dragInProgress;

	operation_deque m_undo; //undo buffer
	CL_Menu *m_pMenu;
	CL_MenuNode *m_pMenuUndo;
	
	CL_Slot m_slotSelectedBaseTilePage;
	CL_Slot m_slotOpenBaseTileWindow;

	CL_ListBox *m_pListBaseTile;
	CL_Window * m_pWindowBaseTile;
	CL_Vector2 m_vecSnapSize;
	CL_Point m_vecLastMousePos;
	EntCollisionEditor * m_pEntCollisionEditor;
	Tile *m_pTileWeAreEdittingCollisionOn;
	CL_Vector2 m_collisionEditOldTilePos;
	MovingEntity *m_pOriginalEditEnt;
	bool m_bDialogIsOpen; //if true, we know not to respond to DELETE key presses to delete items and things

	CL_Vector2 m_vecDragSnap; 

	//for the properties dialog

	void OnPropertiesOK();
	void OnPropertiesOpenScript();
	void OnPropertiesEditScript();
	void OnPropertiesConvert();
	void OnEditAlignment();
	void OnExternalDialogClose(int entID);
	bool MouseIsOverSelection(CL_Point ptMouse);
	void SetOperation(int op);
	void DrawSelection(CL_GraphicContext *pGC);
	void PushUndosIntoUndoOperation();
	void OnMouseDoubleClick(const CL_InputEvent &input);
	void OpenContextMenu(CL_Vector2 clickPos);
	void OnHideSelection();
	void OnDeselect();
	void FloodFill(CL_Rect r);
	void RefreshActiveBrush(CL_Point mousePos);
	void DrawDragRect(CL_GraphicContext *pGC);
	void onButtonDownTilePicInfo(const CL_InputEvent &key);
	void TurnOffFocus();
	void TilePaint();
	void OnRoundOffTiles();
	enum
	{
		C_GUI_CANCEL,
		C_GUI_OK,
		C_GUI_CONVERT

	};

	enum
	{
		C_OP_NOTHING,
		C_OP_MOVE
	};

	int m_guiResponse;
	CL_InputBox *m_pPropertiesInputScript;
	CL_ListBox *m_pPropertiesListData;
	int m_operation; //if we're currently drag-moving or not
	CL_Vector2 m_lastContextWorldPos;
	CL_Menu *m_pContextMenu;
	bool m_bHideSelection;
	CL_ListBox * m_pTilePicColList;
	HashedResource * m_pResInfo; 
	bool m_bTilePaintModeActive;
};


void DrawSelectedTileBorder(Tile *pTile, CL_GraphicContext *pGC);

extern TileEditOperation g_EntEditModeCopyBuffer; //needed so we can clear it before shutdown, otherwise
//weird things can happen because many subsystems are gone

#endif                  // include guard
