//  ***************************************************************
//  ScriptKeyManager - Creation date: 05/10/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ScriptKeyManager_h__
#define ScriptKeyManager_h__

enum
{
	C_INPUT_GAME_ONLY = 0,
	C_INPUT_EDITOR_ONLY,
	C_INPUT_GAME_AND_EDITOR
};

class MovingEntity;

class KeyInfo
{
public:

//where do I save the actual key?  It's the index used for the map.

	KeyInfo()
	{
		m_bShifted = false;
		m_bAlt = false;
		m_bCtrl = false;
		m_bAlways = false;
		m_inputMode = C_INPUT_GAME_ONLY;
	}

	bool m_bShifted;
	bool m_bAlt;
	bool m_bCtrl;
	bool m_bAlways;
	string m_callback;
	int m_entityID;
	int m_inputMode;
};


typedef std::map<unsigned int, vector<KeyInfo> > ScriptKeyMap;


class JoyDirection
{
public:
	JoyDirection()
	{
		m_bDown = false;

	}
	bool Update(float pos); //returns true if state was changed
	bool IsPressed() {return m_bDown;}
private:

	bool m_bDown;
};

class StickData
{
public:
	StickData();
	int m_axis[2];
	CL_Vector2 m_vec;
};

class JoystickInfo
{
public:
		
	JoystickInfo(int joyID, string name) {m_joyID = joyID; m_name = name;}
	enum eDirs
	{
		LEFT,
		RIGHT,
		UP,
		DOWN,
		DIR_COUNT
	};
	
	void UpdateAxis(eDirs dir, float pos);
	int GetID() {return m_joyID;}
	string GetName();
	int GetButtonCount();
	CL_Vector2 GetLeftStickPosition() { return m_stick[0].m_vec;}
	CL_Vector2 GetRightStickPosition() {return m_stick[1].m_vec;}
	void SetRightStickAxis(int axis1, int axis2);

	string m_name; //we cache it because clanlib seems to forget??
	StickData m_stick[2];
	//JoyDirection * GetJoyDir(int dir) {return &m_Dir[DIR_COUNT];}

private:

	
	int m_joyID;
	JoyDirection m_Dir[DIR_COUNT];
	
};

class InputManager
{
public:
	InputManager();
	virtual ~InputManager();
	void Init(); //clear it

	//Send "" as the callback to remove assignment.
	//Send "g,shift" to mean shifted-g.
	void AddBinding(const string &keyName, const string &callbackFunction, int entityID);
	bool HandleEvent(const CL_InputEvent &key, bool bKeyDown);
	CL_Vector2 GetMousePos();
	void SetMousePos(const CL_Vector2 &pos);
	int StringToInputID(vector<string> & word, const string & keyName);
	bool RemoveBinding(const string &keyName, const string &callbackFunction, int entityID);
	void PrintStatistics();
	void RemoveBindingsByEntity(MovingEntity *pEnt);
	int GetJoystickCount();
	void on_joystick_down(const CL_InputEvent &key, int joyID);
	void on_joystick_up(const CL_InputEvent &key, int joyID);
	void on_joystick_move(const CL_InputEvent &key, int joyID);
	void OneTimeInit();
	JoystickInfo * GetJoystick(int joyID);
protected:

	void ShowJoystickErrorMessage(string keyName);
	CL_SlotContainer m_slots;

	ScriptKeyMap m_map;
	vector< JoystickInfo> m_joyInfo;

private:
};

#endif
