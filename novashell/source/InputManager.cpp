#include "AppPrecomp.h"
#include "InputManager.h"
#include "GameLogic.h"
#include "ScriptManager.h"
#include "Console.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#define C_KEY_UNASSIGNED -1
#define C_JOYSTICK_ID_MULT 1000 //so we can use the same map for joystick interactions too
#define C_JOYSTICK_DIR_MOD 90 //add to direction ids so we don't conflict with buttons/axis ids
#define C_JOYSTICK_AXIS_MOD 50 //add to axis ids so we don't conflict with buttons
#define C_JOYSTICK_ID_ANY_MOD 20000 //reserved area to mean "any joystick"

StickData::StickData()
{
	m_vec = CL_Vector2(0,0);
	
	//360 controller is 5,6 btw
	m_axis[0] = 2;
	m_axis[1] = 7;
}

void InputManager::on_joystick_down(const CL_InputEvent &key, int joyID)
{
	//LogMsg("Joy %d: Button %d pressed", joyID, key.id);
	CL_InputEvent k;
	k.id = (joyID*C_JOYSTICK_ID_MULT) +key.id;
	g_inputManager.HandleEvent(k, true);

	//any joystick version
	k.id = C_JOYSTICK_ID_ANY_MOD +key.id;
	g_inputManager.HandleEvent(k, true);
}

void InputManager::on_joystick_up(const CL_InputEvent &key, int joyID)
{
	//LogMsg("Joy %d: Button %d released", joyID,key.id);

	CL_InputEvent k;
	k.id = (joyID*C_JOYSTICK_ID_MULT) +key.id;
	g_inputManager.HandleEvent(k, false);

	//any joystick version
	k.id = C_JOYSTICK_ID_ANY_MOD +key.id;
	g_inputManager.HandleEvent(k, false);
}

void InputManager::on_joystick_move(const CL_InputEvent &key, int joyID)
{
	//LogMsg("Joy %d: Axis %d now at %.4f",joyID, key.id, key.axis_pos);

	//first, we keep track of our own button states
	if (key.id == 1)
	{
		//up and down
		m_joyInfo[joyID].UpdateAxis(JoystickInfo::DOWN, key.axis_pos);
		m_joyInfo[joyID].UpdateAxis(JoystickInfo::UP, -key.axis_pos);

		m_joyInfo[joyID].m_stick[0].m_vec.y = key.axis_pos;
	} else if (key.id == 0)
	{
		m_joyInfo[joyID].UpdateAxis(JoystickInfo::RIGHT, key.axis_pos);
		m_joyInfo[joyID].UpdateAxis(JoystickInfo::LEFT, -key.axis_pos);
		m_joyInfo[joyID].m_stick[0].m_vec.x = key.axis_pos;
	} else if (key.id == m_joyInfo[joyID].m_stick[1].m_axis[0])
	{
		m_joyInfo[joyID].m_stick[1].m_vec.x = key.axis_pos;

	} else if (key.id == m_joyInfo[joyID].m_stick[1].m_axis[1])
	{
		m_joyInfo[joyID].m_stick[1].m_vec.y = key.axis_pos;

	} else if (key.id == 32)
	{
		//hack for xbox secondary control pad
			CL_InputEvent k;
			JoystickInfo::eDirs dir = JoystickInfo::UP;
		
			switch (int(key.axis_pos))
			{
			case 0: dir = JoystickInfo::UP; break;
			case 90: dir = JoystickInfo::RIGHT; break;
			case 180: dir = JoystickInfo::DOWN; break;
			case 270: dir = JoystickInfo::LEFT; break;
			case -1000:
				//just shut them all off?
				
				m_joyInfo[joyID].UpdateAxis(JoystickInfo::UP, 0);
				m_joyInfo[joyID].UpdateAxis(JoystickInfo::RIGHT, 0);
				m_joyInfo[joyID].UpdateAxis(JoystickInfo::DOWN, 0);
				m_joyInfo[joyID].UpdateAxis(JoystickInfo::LEFT, 0);
				

				return;
				break;
			default:
				//probably a diagonal or -1000 to signal they stopped touching it
				return;
			}
			
			m_joyInfo[joyID].UpdateAxis(dir, 1);
			m_joyInfo[joyID].UpdateAxis(dir, 0);

	
	}
}

#define C_JOY_DIR_SENSITIVITY 0.5f

bool JoyDirection::Update(float pos )
{
	if (pos > C_JOY_DIR_SENSITIVITY)
	{
		if (!IsPressed())
		{
			m_bDown = true;
			return true; //changed it
		}
	} else
	{
		if (IsPressed())
		{
			m_bDown = false;
			return true; //change was made
		}
	}

	//no change was made
	return false;
}


void JoystickInfo::UpdateAxis(eDirs dir, float pos)
{
	if (m_Dir[dir].Update(pos))
	{
		CL_InputEvent k;
		k.id = (m_joyID*C_JOYSTICK_ID_MULT) +C_JOYSTICK_DIR_MOD+dir;
		g_inputManager.HandleEvent(k, m_Dir[dir].IsPressed());

		//a second call for when someone maps to "all joysticks"
		k.id = C_JOYSTICK_ID_ANY_MOD+C_JOYSTICK_DIR_MOD+dir;
		g_inputManager.HandleEvent(k, m_Dir[dir].IsPressed());
	}
}

std::string JoystickInfo::GetName()
{
	return m_name;
}

int JoystickInfo::GetButtonCount()
{
	return CL_Display::get_current_window()->get_ic()->get_joystick(m_joyID).get_button_count();
}

void JoystickInfo::SetRightStickAxis(int axis1, int axis2)
{
	m_stick[1].m_axis[0] = axis1;
	m_stick[1].m_axis[1] = axis2;

}
InputManager::InputManager()
{
	Init();
}

InputManager::~InputManager()
{
}

void InputManager::OneTimeInit()
{
	
	//we have to wait for CL to initialize before calling this one
	
	//setup the CL callback for joystick stuff
#ifdef __APPLE__
	LogMsg("Joysticks are currently not supported in Clanlib on OSX.  Make Seth add them?");
#endif
	for (int i=0; i < GetJoystickCount(); i++)
	{
		CL_InputDevice joystick =CL_Display::get_current_window()->get_ic()->get_joystick(i);

		LogMsg("Found \"%s\", %d buttons.", joystick.get_name().c_str(), joystick.get_button_count());
		//std::cout << "Axis: " << joystick.get_axis_count() << std::endl;

		m_slots.connect(joystick.sig_axis_move(), this, &InputManager::on_joystick_move, i);
		m_slots.connect(joystick.sig_key_up(), this, &InputManager::on_joystick_up, i);
		m_slots.connect(joystick.sig_key_down(), this, &InputManager::on_joystick_down, i);
		m_joyInfo.push_back(JoystickInfo(i, joystick.get_name()));
		
	}
}

void InputManager::PrintStatistics()
{
	LogMsg("");
	LogMsg("  ** InputManager Statistics **");

	ScriptKeyMap::iterator itor = m_map.begin();
	int totalBinds = 0;

	for (;itor != m_map.end(); itor++)
	{
		//LogMsg("Hash %d", itor->first);
		totalBinds += itor->second.size();
	}

	LogMsg("    %d input bindings active.", totalBinds);
}

void InputManager::ShowJoystickErrorMessage(string keyName)
{
	LogError("Error converting %s to an input ID. (Joystick mapping should look like: joy_0_right or joy_any_right, or joy_0_axis_0 or joystick_1_button_2)", keyName.c_str());
}

int InputManager::StringToInputID(vector<string> & word, const string & keyName)
{
	
	word = CL_String::tokenize(keyName, ",");

	int keyId = C_KEY_UNASSIGNED;

	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	if (word.size() < 1)
	{
		LogError("Bad input manager string: Didn't specify anything?");
		return keyId;
	}
	
	//special checks for joystick stuff..

	if (word[0][0] == 'j' && word[0][1] == 'o' && word[0][2] == 'y')
	{
		//detected a joystick mapping
		
		vector<string> joyWords = CL_String::tokenize(word[0], "_");

		if (joyWords.size() < 3)
		{
			ShowJoystickErrorMessage(keyName);
			return keyId;
		}

		int joyNum = 0;
		
		if (joyWords[1] == "any")
		{
			joyNum = -1;
		} else
		{
			joyNum = CL_String::to_int(joyWords[1]);
		}

		//does joystick exist?
		if (joyNum >= GetJoystickCount())
		{
			LogMsg("Warning: Mapping to a  non-existing joystick...");
		}
		if (joyNum == -1)
		{
			keyId = C_JOYSTICK_ID_ANY_MOD; //we'll add more to this in a bit
		} else
		{
			keyId = joyNum * C_JOYSTICK_ID_MULT; //we'll add more to this in a bit
		}

		//now check the rest of the string to see what specifically we need to map to this joystick

		if (joyWords[2] == "right") keyId += C_JOYSTICK_DIR_MOD+JoystickInfo::RIGHT;
		else if (joyWords[2] == "left") keyId += C_JOYSTICK_DIR_MOD+JoystickInfo::LEFT; 
		else if (joyWords[2] == "up") keyId += C_JOYSTICK_DIR_MOD+JoystickInfo::UP; 
		else if (joyWords[2] == "down") keyId += C_JOYSTICK_DIR_MOD+JoystickInfo::DOWN; 
		else if (joyWords[2] == "down") keyId += C_JOYSTICK_DIR_MOD+JoystickInfo::DOWN; 
		else if (joyWords[2] == "button")
		{
			//button mappings...		
			if (joyWords.size() < 4)
			{
				ShowJoystickErrorMessage(keyName);
			} else
			{
				keyId += CL_String::to_int(joyWords[3]);
			}
		}

	} else
	{

		//parse key string into virtual id
		for (unsigned int i=0; i < word.size(); i++)
		{
			int id = pKeyboard->string_to_keyid(word[i]);

			if ( word.size() == 1 || (id != CL_KEY_CONTROL && id != CL_KEY_SHIFT) )
			{
				if (id != 0)
				{
					keyId = id;
					break;
				}
			}

			//check for mouse stuff
			if (word[0] == "mouse_left")
			{
				keyId = CL_MOUSE_LEFT;
			} else
				if (word[0] == "mouse_right")
				{
					keyId = CL_MOUSE_RIGHT;
				} else
					if (word[0] == "mouse_middle")
					{
						keyId = CL_MOUSE_MIDDLE;
					} else
						if (word[0] == "mouse_wheel_down")
						{
							keyId = CL_MOUSE_WHEEL_DOWN;
						} else
							if (word[0] == "mouse_wheel_up")
							{
								keyId = CL_MOUSE_WHEEL_UP;
							} else
								if (word[0] == "mouse_xbutton1")
								{
									keyId = CL_MOUSE_XBUTTON1;
								} else
									if (word[0] == "mouse_xbutton2")
									{
										keyId = CL_MOUSE_XBUTTON2;
									} 

									if (keyId != C_KEY_UNASSIGNED)
									{
										break;
									}
		}
	}

	if (keyId == C_KEY_UNASSIGNED)
	{
		LogError("Error converting %s to an input ID. (must be lower case, also, don't use 'always' with only the control key)", keyName.c_str());
	}

	return keyId;
}

void InputManager::RemoveBindingsByEntity(MovingEntity *pEnt)
{

	ScriptKeyMap::iterator itor = m_map.begin();
	ScriptKeyMap::iterator itorTemp;

	for (;itor != m_map.end();)
	{

		vector<KeyInfo> &ki = itor->second;

		for (vector<KeyInfo>::iterator kitor = ki.begin(); kitor != ki.end(); )
		{
			if ( (*kitor).m_entityID == pEnt->ID())
			{
				//remove this
				kitor = ki.erase(kitor);
				continue;
			}

			kitor++;
		}

		if (ki.empty())
		{
			itorTemp = itor;
			itor++;
			m_map.erase(itorTemp);
			continue;
		}


		itor++;
	}
}

bool InputManager::RemoveBinding(const string &keyName, const string &callbackFunction, int entityID)
{
	vector<string> word;
	
	int keyID = StringToInputID(word, keyName);
	if (keyID == C_KEY_UNASSIGNED) return false;

	ScriptKeyMap::iterator itor = m_map.find(keyID);

	if (m_map.end() == itor)
	{
		//LogMsg("GetInputManager:RemovingBinding failed to find binding %s, %s.", keyName.c_str(), callbackFunction.c_str());
		return false;
	}

	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	bool bCtrl = false;
	bool bShifted = false;
	bool bAlways = false;

	if (keyID == CL_KEY_SHIFT || keyID == CL_KEY_CONTROL || keyID == CL_KEY_MENU)
	{
		bAlways = true;
	}

	int inputMode = C_INPUT_GAME_ONLY;

	//check for special keys
	for (unsigned int i=0; i < word.size(); i++)
	{
	
		if (word[i] == "always")
		{
			bAlways = true; //always reacts, regardless of which modifiers are down
			continue;
		} 

		if (word[i] == "editor_only")
		{
			inputMode = C_INPUT_EDITOR_ONLY;
			continue;
		}

		if (word[i] == "game_and_editor")
		{
			inputMode = C_INPUT_GAME_AND_EDITOR;
			continue;
		}
	
		
	int specialID = pKeyboard->string_to_keyid(word[i]);

	if (specialID == keyID) continue; //we already handled this one..

		switch (specialID)
		{
		case CL_KEY_CONTROL:
			bCtrl = true;
			break;

		case CL_KEY_SHIFT:
			bShifted = true;
			break;
		}
	}

	vector<KeyInfo> &ki = itor->second;
	vector<KeyInfo>::reverse_iterator ritor;
	
	ritor = ki.rbegin();

	for (;ritor != ki.rend(); ritor++)
	{

		if ((*ritor).m_bShifted == bShifted && (*ritor).m_bCtrl == bCtrl && entityID == (*ritor).m_entityID
			&& (*ritor).m_bAlways == bAlways && (*ritor).m_inputMode == inputMode)
		{
			//we found it!
			ki.erase( (++ritor).base()); //weirdness to convert it to a normal iterator
			
			if (ki.empty())
			{
				//we might as well kill this old thing, nothing left in the array
				m_map.erase(itor);
			}

			return true;
		}

	}

	return false;
}

void InputManager::AddBinding(const string &keyName, const string &callbackFunction, int entityID)
{
	vector<string> word;
	
	int keyID = StringToInputID(word, keyName);
	
	if (keyID == C_KEY_UNASSIGNED) return;

	if (callbackFunction.empty())
	{
		//remove it completely
		m_map.erase(m_map.find(keyID));
		return;
	}

	KeyInfo k;
	k.m_callback = callbackFunction;

	if (keyID == CL_KEY_SHIFT || keyID == CL_KEY_CONTROL || keyID == CL_KEY_MENU)
	{
		k.m_bAlways = true;
	}

	CL_InputDevice *pKeyboard = &CL_Display::get_current_window()->get_ic()->get_keyboard();

	//check for special keys
	for (unsigned int i=0; i < word.size(); i++)
	{
		if (word[i] == "always")
		{
			k.m_bAlways = true; //always reacts, regardless of which modifiers are down
			continue;
		} 
		
		if (word[i] == "editor_only")
		{
			k.m_inputMode = C_INPUT_EDITOR_ONLY;
			continue;
		}

		if (word[i] == "game_and_editor")
		{
			k.m_inputMode = C_INPUT_GAME_AND_EDITOR;
			continue;
		}
	
		int specialID = pKeyboard->string_to_keyid(word[i]);

		if (specialID == keyID) continue; //we already handled this one..

		switch (specialID)
		{
		case CL_KEY_CONTROL:
		//case CL_KEY_LCONTROL:
		//case CL_KEY_RCONTROL:
			k.m_bCtrl = true;
			break;

		case CL_KEY_SHIFT:
		//case CL_KEY_LSHIFT:
		//case CL_KEY_RSHIFT:
			k.m_bShifted = true;
			break;
		}
	}
	
	k.m_entityID = entityID;

	vector<KeyInfo> &ki = m_map[keyID];
	ki.push_back(k);
}

CL_Vector2 InputManager::GetMousePos()
{
	return CL_Vector2(CL_Mouse::get_x(), CL_Mouse::get_y());
}

void InputManager::SetMousePos(const CL_Vector2 &pos)
{
	CL_Mouse::set_position(pos.x, pos.y);
}


//returns true if handled, false if not
bool InputManager::HandleEvent(const CL_InputEvent &key, bool bKeyDown)
{
	if (GetGameLogic()->GetShowMessageActive()) return false;

	
	ScriptKeyMap::iterator itor = m_map.find(key.id);
	if (itor == m_map.end()) return false; //we don't have it

	//otherwise, send all the callbacks
	vector<KeyInfo> v = itor->second; //yes, we'll make a copy.  This is so it's ok to delete things inside the
	//callback without screwing anything up.

	vector<KeyInfo>::reverse_iterator ritor;

	ritor = v.rbegin();

	bool bEditorOpen = GetGameLogic()->GetEditorActive();

	for (;ritor != v.rend(); ritor++)
	{
		KeyInfo *pKeyInfo = &(*ritor);

		switch (pKeyInfo->m_inputMode)
		{
		case C_INPUT_GAME_ONLY:
			if (bEditorOpen || (g_Console.IsActive() && bKeyDown) && !CL_Keyboard::get_keycode(CL_KEY_CONTROL) )  continue;
			break;
		case C_INPUT_EDITOR_ONLY:
			if (!bEditorOpen) continue;
			
			
			if (GetGameLogic()->IsEditorDialogOpen())
			{
					//with a dialog open we don't want to run crazy code right now..
					continue;
			}
			
			break;
		}

		if (pKeyInfo->m_bAlways || (pKeyInfo->m_bCtrl == CL_Keyboard::get_keycode(CL_KEY_CONTROL)
			&& pKeyInfo->m_bShifted == CL_Keyboard::get_keycode(CL_KEY_SHIFT)) 
			)
		{
			bool bKeepPassingItOn = true;
			
			if (pKeyInfo->m_entityID == C_ENTITY_NONE)
			{
				//global kind of thing
				
				if (!GetScriptManager->FunctionExists(pKeyInfo->m_callback))
				GetScriptManager->SetStrict(false);

				try {bKeepPassingItOn = luabind::call_function<bool>(GetScriptManager->GetMainState(), 
					pKeyInfo->m_callback.c_str(), bKeyDown);
				}  LUABIND_CATCH(pKeyInfo->m_callback);
			
				GetScriptManager->SetStrict(true);

			} else
			{
				MovingEntity *m_pParent = (MovingEntity*) EntityMgr->GetEntityFromID(pKeyInfo->m_entityID);
				if (!m_pParent)
				{
					LogMsg("InputManager - Can't locate entity %d to run %s", pKeyInfo->m_entityID, pKeyInfo->m_callback.c_str());
				} else
				{
					//let's actually run this function in this entities namespace
					if (!m_pParent->GetScriptObject()->FunctionExists(pKeyInfo->m_callback))
					{
						GetScriptManager->SetStrict(false);
					}
					try {bKeepPassingItOn = luabind::call_function<bool>(m_pParent->GetScriptObject()->GetState(), pKeyInfo->m_callback.c_str(),bKeyDown);
					} LUABIND_ENT_BRAIN_CATCH(pKeyInfo->m_callback.c_str());
					GetScriptManager->SetStrict(true);
				}
			}

			if (!bKeepPassingItOn) return true; //true as in we handled it and don't want anyone else to
		}
	}

	return false; //didn't handle event
}


void InputManager::Init()
{
	m_map.clear();
}

int InputManager::GetJoystickCount()
{
	return CL_Display::get_current_window()->get_ic()->get_joystick_count();
}

JoystickInfo * InputManager::GetJoystick( int joyID )
{
	if (joyID >= GetJoystickCount())
	{
		return NULL;
	}

	return &m_joyInfo[joyID];
}
/*
Object: InputManager
Handles user input such as keyboard, mouse and joystick.

About:

This is a global object that can always be accessed.

By "binding" input to functions, it causes the function to be run when the key/button is pressed and again when it is released.

If the same key is bound more than one, more than one function will be called, in the opposite order that they were bound in.

Usage:
(code)
//move the mouse to the center of the screen
GetInputManager:SetMousePos(Vector2(C_SCREEN_X/2, C_SCREEN_Y/2));
(end)

Group: Binding Related

func: AddBinding
(code)
nil AddBinding(string inputName, string callbackName, number entityID)
(end)
Adding a binding causes the engine to call the specified function every time the key is pressed or released.

How to build the inputName string:

This can be a single key, joystick button/dir, or mouse button.

Valid input name examples: *a*, *b*, *c*, *3*, *4*, *f1*, *control*, *mouse_middle*, *mouse_left*, *space*, *mouse_wheel_up*, *tab*, *escape*, *subtract*, *equals*, *numpad_subtract*, *numpad_add*, *left*, *right*, *up*, *down* and so forth.

In addition, the following can be appended to the inputName (for non joystick keys:

return - The Enter key, don't ask me why it's called return in Clanlib, did they write it on a Commodore 64?
control - The control key must be held down also.  "control,r" means Ctrl-R must be pressed.
shift - The shift key must be held down also. "shift,r" means Shift-R must be pressed.
alt - The alt/option key must be held down also. "shift,r" means Shift-R must be pressed.
always - This means the key will always trigger, even if shift/alt/control is also down.  Without "left,always", you couldn't fire with control and press left at the same time.
editor_only - Means these keybindings will only trigger when the editor is open.
game_and_editor - Means these keybindings will always trigger.  Default is only while the editor is not open.

Usage:
(code)
//binding with a global callback
GetInputManager:AddBinding("f1,always", "OnHitF1Key", C_ENTITY_NONE);

//Let's also bind something to a specific entity, by providing a valid entityID
GetInputManager:AddBinding("mouse_left", "OnMouseLeft", someEntityID);
(end)

Now if you press f1 or click the left mouse button you will see an error about functions not being found.  You need to handle them, add this somewhere to a globally loaded script:
(code)
function OnHitF1Key(bKeyDown)

	if (bKeyDown) then
		LogMsg("You pressed F1 down.");
	else
		LogMsg("You released F1.");
	end

	return true; //keep processing additional callbacks for F1 if applicable
end
(end)

That's one down.  Now, we need to add a function to the entity's script (whoever someEntityID came from):
(code)
function OnMouseLeft(bKeyDown)

	if (bKeyDown) then
		LogMsg("You pressed the left mouse button down.");
	else
		LogMsg("You released the left mouse button.");
	end

	return true; //keep processing additional callbacks for the left mouse button if applicable
end
(end)

About Joystick mapping:

Valid joystick name examples: *joy_0_left*, *joy_1_up*, *joy_0_button_0*, *joy_any_down*

Instead of a # for which joystick, you can use "*any*" to map to all of them.

Parameters:

inputName - A string defining some forum of input.  Use comma to separate words.  The order doesn't matter.
callbackName - A string containing the name of a properly setup function that should be called. (it must accept one parm and return true or false)
entityID - If not <C_ENTITY_NONE> this means the function is located in this <Entity>'s namespace.

Note:

When an <Entity> is deleted, any associated bindings to it are also automatically deleted for you.


func: RemoveBinding
(code)
boolean RemoveBinding(string inputName, string callbackName, int entityID)
(end)
This allows you to remove any previously setup binding.  Because there may be many bindings to the same key, you must enter all the information so it knows which binding to remove.

Parameters:

inputName - A string defining the input, the same one used to make the binding.
string callBackName - The name of the callback function that was used to create the binding we're after.
entityID - If the original binding was to an <Entity> function, you'll need to enter its ID here.  Otherwise, <C_ENTITY_NONE>.

Returns:

True if a binding was actually removed.

func: RemoveBindingByEntity
(code)
nil RemoveBindingByEntity(Entity ent)
(end)
Allows you to easily remove all bindings from a specified <Entity>.

Note:

When <Entity>'s are deleted, they automatically call this function.

Parameters:

ent - The <Entity> we need to remove all bindings from. *Note that this wants the Entity, not the ID*.

Group: Direct Device Access

func: GetMousePos
(code)
Vector2 GetMousePos()
(end)

Returns:

A <Vector2> object containing the location of the mouse in screen coordinates.  Use <ScreenToWorld> to convert to a position on the active <Map>.

func: SetMousePos
(code)
nil SetMousePos(Vector2 vPos)
(end)

Parameters:

vPos - A <Vector2> object containing the new mouse position in screen coordinates.

func: GetJoystickCount
(code)
number GetJoystickCount()
(end)

Returns:

How many joysticks are currently available.

func: GetJoystick
(code)
Joystick GetJoystick(number joystickID)
(end)

This let's you get access to additional information about the connected joysticks.  So you can detect say, a 360 controller and properly map to its extra buttons.

Note:

You use the normal <AddBinding> to map to joysticks.

Returns:

A <Joystick> object or nil if the joystick doesn't exist.

*/


