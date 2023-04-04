//  ***************************************************************
//  Console - Creation date: 05/09/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef Console_h__
#define Console_h__

class MovingEntity;

class ConsoleItem
{
public:
	
	ConsoleItem()
	{
		m_color = CL_Color(255,255,255,255);
	}
	string m_text;
	CL_Color m_color;
};

typedef deque <ConsoleItem> console_cont;

class Console
{
public:
	Console();
	virtual ~Console();

	void Add(const string line, CL_Color c = CL_Color(255,255,255,255));
	void AddError(const string line);

	bool IsActive() {return m_bOnScreen;}
	void SetOnScreen(bool bNew);
	void Render();
	void OnKeyDown(const CL_InputEvent &key);
	void Init();
	void KillGUI(); //should be called before the main GUI is shutdown for good

	void RenderPostGUI();
	void OnScreenChanged(); //screensize changed

protected:

	MovingEntity * GetSelectedEntityFromEditor();
	void AddGeneric(ConsoleItem &item);
	void CopyToTextBuffer();
	void RenderGUIOverlay();
	void PressedEnter();
	string GetBackBufferLine(int idx);
	void InputValidator(char &character, bool &accept);
	void ProcessString(string s);

	console_cont m_lineVec;
	bool m_bOnScreen; //if true, we're displaying it

	CL_SlotContainer m_slots;
	int m_curViewIndexOffset;

	CL_InputBox *m_pInputBox; //console input

	deque< string > m_backBuffer; //so up arrow will show previously entered commands
	int m_curBackBufferIndex;
	bool m_bRequestGUIToggle, m_bRequestClipboardCopy;

};

extern Console g_Console;

#endif // Console_h__

