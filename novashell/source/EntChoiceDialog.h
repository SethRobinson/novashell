//  ***************************************************************
//  EntChoiceDialog - Creation date: 09/25/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntChoiceDialog_h__
#define EntChoiceDialog_h__

#include "BaseGameEntity.h"
#include "AppUtils.h"

class DialogChoice
{
public:
	string m_text; //the options text the player sees
	string m_result; //the key result code we return when its selected
};

class EntChoiceDialog: public BaseGameEntity
{
public:
	EntChoiceDialog(const string &initMessage="");
	virtual ~EntChoiceDialog();
	virtual string HandleMessageString(const std::string &msg);

protected:


	enum
	{
		nothing,
		adding,
		waiting_for_selection,
		removing
	};

	void BuildDialog();
	void ChangeSelection(int offset);
	void OnSelection(int selItem);
	void ProcessInitString(const string &msg);
	void CalculateSize();
	virtual void Update(float step);
	void AddItems();
	void FinalSelectionProcessing();
	void RemoveItems();
	void RenderOverlay();
	void CalculateCenteredWindowPosition();
	void OnHighlighted(int index);

	CL_Window *m_pWindow;
	CL_ListBox *m_pListWorld; //control which worlds are drawn
	CL_SlotContainer m_slots; //generic one, easier
	string m_title;
	int m_entityID; //0 if not used
	vector<DialogChoice> m_choices;
	GameTimer m_timer;
	unsigned int m_curElementToPlace;
	int m_mode;
	int m_chosenID;
	string m_callbackFunctionName;
	CL_Font *m_pFont, *m_pTitleFont;
	CL_Rect m_textRect;
	int m_titleOffsetY;
	int m_textBorder;
	vector<string> m_msg;
	vector<string> m_msgToShow;
	string m_formattedMsg;
	string m_startingSelection;

};

#endif // EntChoiceDialog_h__