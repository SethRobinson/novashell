
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 3/19/2005 9:08:16 PM
*/


#pragma once

#include "BaseGameEntity.h"
#define C_EDITOR_MAIN_MENU_BAR_HEIGHT 95

class GeneratorInterface;

class EntEditor: public BaseGameEntity
{
public:
    
    EntEditor();
    virtual ~EntEditor();
   
	//signal slot functions
	void OnModeEnded(int id);
    void Render(void *pTarget);
    void Update(float step);
    void Write(std::ostream&  os)const{/*not implemented*/}
    void Read (std::ifstream& is){/*not implemented*/}
	void SetTipLabel(std::string tip);  //so different modes can overwrite this with useful info
	void OnGenerateDink();
	void OnToggleEditMode();
	void BuildWorldListBox();
	void BuildLayerControlBox();
	bool IsDialogOpen() {return m_bDialogOpen;}
	virtual string HandleMessageString(const string &msg);
	void SetHideMode(bool bHide);
	bool GetHideMode();
	void OnToggleGrid();
	void OnScreenChanged();

	CL_Signal_v1<bool> sig_hide_mode_changed;
	void Kill();

private:
    
	void OnRevertChanges();
	void DisableAllModes();
	void OnMouseMove(const CL_InputEvent &key);
	void SetDefaultTipLabel();
    void UpdateAll();
    bool Init();
    void OnClose();
    void OnChooseScreenMode();
    void ForceChooseScreenMode(bool bOn);
    void OnGenerateSmall();
    void OnGenerateLarge();
    void SimpleGenerateInit();
    bool SimpleGenerateStep();
	void OnWorldPackage();

	void OnClearMap();
	void OnExit();
	void OnSaveMap();
	void OnToggleFullScreen();
	void OnCloseButton(CL_SlotParent_v0 &parent_handler);
	void OnSelectMap();
	void OnListLoseFocus(const CL_InputEvent &key);
	void validator_numbers(char &character, bool &accept);
	void MapOptionsDialog();
	void OnWorldProperties();
	void OnPreferences();
	void onButtonDown(const CL_InputEvent &key);
	void OnMouseDown(const CL_InputEvent &key);
	void OnMouseUp(const CL_InputEvent &key);
	void OnShowHidePalettes();
	void OnToggleWorldChunkGrid();
	void OnToggleShowCollision();
	void OnToggleShowPathfinding();
	void OnToggleShowAI();
	void OnToggleShowEntityCollision();
	void OnToggleParallax();
	void OnMapChange();
	int GetWorldInfoListHeight();
	void OnLayerAdd();
	void OnLayerSelect();
	void OnToggleLockAtRefresh();
	void OnToggleGamePaused();
	void OnLayerDoubleClick(const CL_InputEvent &input);
	void OnWorldDoubleClick(const CL_InputEvent &input);
	void PopUpLayerPropertiesDialog(int layerID);
	void OnResetCamera();
	void OnDumpEngineStatistics();
	void OnRestart();
	void OnOpenScript();
	void OnAddNewMap();
	void OnEditStartupLua();
	void OnEditSetupConstants();
	void OnEditGameStartLua();
	void OnDumpWorldNavStatistics();
	void OnRebuildNavigationMaps();
	void OnResetCameraScale();

    enum
	{
		e_modeTileEdit = 0,
		e_modeMap,
		e_modeCount
	};
	
	CL_Window *m_pWindow;
	CL_Window *m_pWorldListWindow;
	
	CL_Window *m_pLayerListWindow;
	CL_ListBox *m_pListLayerDisplay; //control which maps are drawn
	CL_ListBox *m_pListLayerActive; //which layer has active focus
	CL_Label *m_pLayerLabel;

	void KillLayerListStuff();
	void OnLayerChange();
	void PopulateLayerListMenu();
	void OnLayerHighlight();
	void OnLayerAll();
	void OnLayerNone();
	void SetAllLayersActive(bool bNew);
	void OnSetScreenSize();
	void PopulateResolutionList();
	void OnGameGlobalProperties();
	void OnExportXML();
	void OnMapUserProperties();
	CL_Label *m_pLabel;
    CL_Menu *m_pMenu;
    CL_Button *m_pButton;
    CL_Button *m_pButtonToggleEditMode;
	CL_ListBox *m_pListBoxWorld;
	CL_MenuItem *m_pMenuWorldChunkCheckbox;
	CL_MenuItem *m_pMenuShowCollisionCheckbox;
	CL_MenuItem *m_pMenuShowPathfindingCheckbox;
	CL_MenuItem *m_pMenuShowAICheckbox;
	CL_MenuItem *m_pMenuShowEntityCollisionCheckbox;
	CL_MenuItem *m_modeCheckBoxArray[e_modeCount];
	CL_MenuItem *m_pMenuLockAtRefreshCheckbox;
	CL_MenuItem *m_pMenuParallaxCheckbox;
	CL_MenuItem *m_pMenuShowFPSCheckbox;
	CL_MenuItem *m_pShowGridCheckBox;

	CL_ListBox *m_pResListBox;

	CL_Slot m_slotClose;
	CL_SlotContainer m_slot; //generic one, easier
	bool m_bGenerateActive;
    int m_generateStepSize;
	CL_Point m_vecLastMousePos;
	bool m_bShowWorldChunkGridLines;
	bool m_bShowCollision;
	bool m_bShowPathfinding;
	bool m_bShowAI;
	bool m_bHideMode;
	
	CL_MenuItem *m_pMenuGamePausedCheckbox;
	bool m_bDialogOpen;
	bool m_bScrollingMap;
    GeneratorInterface *m_pGenerator; //the class to create our random world, different generators can be plugged in
};


void OpenScriptForEditing(string scriptName);
