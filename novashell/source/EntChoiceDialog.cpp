#include "AppPrecomp.h"
#include "EntChoiceDialog.h"
#include "GameLogic.h"
#include "GUIStyleBitmap/listbox_Bitmap.h"

#define C_MAX_DIALOG_TEXT_WIDTH 1024 //will be truncated to the screen size if required as well


EntChoiceDialog::EntChoiceDialog(const string &initMessage): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pFont  = NULL;
	m_pListWorld = NULL;
	m_pWindow = NULL;
	m_entityID = C_ENTITY_NONE;
	m_chosenID = -1;
	m_mode = nothing;
	m_pTitleFont = NULL;
	m_callbackFunctionName = "OnDialogResponse";
	m_textRect = CL_Rect(0,0,0,0);
	m_startingSelection = "_first_";

	m_timer.SetUseSystemTime(true);

	SetName("ChoiceDialog");
	ProcessInitString(initMessage);
	BuildDialog();

	GetApp()->GetMyGameLogic()->SetGamePaused(true);
}

EntChoiceDialog::~EntChoiceDialog()
{
	if (GetApp() && GetApp()->GetMyScriptManager())	
	{
		GetApp()->GetMyScriptManager()->RunFunction("OnDialogClose", (BaseGameEntity*)this);
	}

	SAFE_DELETE(m_pListWorld);
	SAFE_DELETE(m_pWindow);
	GetApp()->GetMyGameLogic()->SetGamePaused(false);
	SAFE_DELETE(m_pFont);
	SAFE_DELETE(m_pTitleFont);
}

void EntChoiceDialog::ProcessInitString(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "entity_id")
		{
			m_entityID = CL_String::to_int(words[1]);
		}  else
		
		if (words[0] == "title")
		{
			m_title = words[1];
		} else
			if (words[0] == "selection")
			{
				m_startingSelection = words[1];
			} else
				if (words[0] == "callback")
			{
				m_callbackFunctionName = words[1];
			} else
		
		if (words[0] == "msg")
		{
			if (words.size() == 1)
			{
				m_msg.clear();
			} else
			{
				m_msg = CL_String::tokenize(words[1], "\n", false);
			}
		}else
		
		{
			LogMsg("EntChoiceDialog doesn't understand keyword %s", words[0].c_str());
		}
	}
}

string EntChoiceDialog::HandleMessageString(const std::string &msg)
{
	vector<string> words = CL_String::tokenize(msg, "|");

	if (words[0] == "mod_selection")
	{
		if (words.size() < 2)
		{
			LogError("Mod selection should look like:  mod_selection|1  (or -1)");
				return "";
		}

		ChangeSelection(CL_String::to_int(words[1]));

	} else
	if (words[0] == "add_choice")
	{
		DialogChoice d;
		if (words.size() > 2)
		{
			d.m_result = words[2];
		}
		d.m_text = words[1];
		m_choices.push_back(d);
	} else
		if (words[0] == "get_callback")
		{
			return m_callbackFunctionName;
		} else
			if (words[0] == "get_entity_id")
		{
			return CL_String::from_int(m_entityID);
		} else
			if (words[0] == "get_selection")
			{
				return m_startingSelection;
			} else
				if (words[0] == "select_active")
				{
					OnSelection(-1);
				} else
		
		if (words[0] == "activate")
		{
			m_mode = adding;	
	
			//adjust timing for how many things we have to show for the crappy roll-up/down effect
			int speedMS = 50; //max time per item
			int maxTimeInMS = 200;
			int totalUpdates = m_choices.size() + m_msg.size();
			
			m_timer.SetInterval(min(speedMS, maxTimeInMS/totalUpdates));
			m_timer.SetIntervalReached();
			m_curElementToPlace = 0;
			
			if (m_startingSelection.empty() || m_startingSelection == "_first_") 
			{
				m_startingSelection = m_choices[0].m_result; //default to first option
			}
			AddItems(); //add the first one
			CalculateCenteredWindowPosition();
			GetApp()->GetMyScriptManager()->RunFunction("OnDialogOpen", (BaseGameEntity*)this);
		} else
	{
		LogMsg("EntChoiceDialog: Don't know how to handle message %s", msg.c_str());
	}

	return "";
}

void EntChoiceDialog::CalculateCenteredWindowPosition()
{
  //calculate a rect that will holds the whole dialog

	CL_Rect r = CL_Rect(0,0,0,0);

	int choiceMaxLength = 0;

	for (unsigned int i=0; i < m_choices.size(); i++)
	{
		int width = m_pFont->get_width(m_choices[i].m_text);
		choiceMaxLength = max(choiceMaxLength, width);
	}

	r.right = max(m_textRect.get_width(), choiceMaxLength);
	r.bottom = m_textRect.get_height() + (m_choices.size() * m_pFont->get_height());

	//center this rect on screen
	m_pWindow->set_position( (GetScreenX/2) - r.right/2, GetScreenY/2 - r.bottom/2);
}


void EntChoiceDialog::AddItems()
{
	if (m_timer.IntervalReached())
	{
		if (m_msgToShow.size() < m_msg.size())
		{
			m_msgToShow.push_back(m_msg[m_msgToShow.size()]);
			CalculateSize();
			return;
		}

		CL_ListItem *pItem = new CL_ListItem;
		pItem->str = m_choices[m_curElementToPlace].m_text;
		pItem->user_data = m_pListWorld->get_count();
		int index = m_pListWorld->insert_item(pItem,-1,true);

		CalculateSize();

		if (m_startingSelection != "")
		{
			m_pListWorld->clear_selection();
		} 

			if (m_choices[m_curElementToPlace].m_result == m_startingSelection || m_startingSelection == "_first_")
			{
				m_pListWorld->set_current_item(index);
				m_startingSelection = "";
				m_slots.connect( m_pListWorld->sig_highlighted(), this, &EntChoiceDialog::OnHighlighted);
			} 


		m_curElementToPlace++;
		if (m_curElementToPlace >= m_choices.size())
		{
			m_mode = waiting_for_selection;
		}
	}
}

void EntChoiceDialog::RemoveItems()
{
	if (m_timer.IntervalReached())
	{
		if (m_curElementToPlace == -1)
		{
			if (m_msgToShow.size() > 0)
			{
				m_msgToShow.pop_back();
				CalculateSize();
				return;
			}
						
			//all done
			m_mode = nothing;
			FinalSelectionProcessing();
		} else
		{
			m_pListWorld->remove_item(m_curElementToPlace);
			m_curElementToPlace--;
			CalculateSize();
		}
		
	}
}

void EntChoiceDialog::Update(float step)
{
	switch (m_mode)
	{
	case adding:
		AddItems();
		break;
	case removing:
		RemoveItems();
		break;
	}
}

void EntChoiceDialog::CalculateSize()
{
	if (m_pWindow)
	{
		//figure out much room this text is going to take
		if (!m_msg.empty())
		{
			m_formattedMsg.clear();
			for (unsigned int i=0; i < m_msgToShow.size(); i++) 
			{
				m_formattedMsg += m_msgToShow[i]; 
				if (i+1 != m_msgToShow.size()) 
				{
					m_formattedMsg += "\n";
				}
			}

			m_textRect = m_pFont->bounding_rect(  CL_Rect(0,0,  min(C_MAX_DIALOG_TEXT_WIDTH, GetScreenX-50), GetScreenY), m_formattedMsg);
		}
		
		m_titleOffsetY = m_pTitleFont->get_height(m_title) + 8;
		
		CL_Size size;
		size.height = m_titleOffsetY + m_textRect.get_height();
		size.width = m_textRect.get_width();
		
		int addedSpacer = 0;
		m_textBorder = 0;

		if (!m_msg.empty())
		{
			m_textBorder = 6;
			addedSpacer += (size.height - m_titleOffsetY) + m_textBorder*2;
		}
		
		m_pWindow->set_size(size.width+m_textBorder*2, size.height+m_textBorder*2);
		m_pListWorld->set_position(0, addedSpacer);
	}
}

void EntChoiceDialog::ChangeSelection(int offset)
{
	if (m_pListWorld->get_count() == 0) return; //not ready for this yet
	int selected = m_pListWorld->get_current_item();
	selected = altmod(selected + offset, m_pListWorld->get_count());
	m_pListWorld->set_current_item(selected);
}


CL_DomElement SendOptionElement(const string &name, const string &value)
{
	CL_DomDocument doc;
	CL_DomElement element = doc.create_element("");
	element.set_attribute(name, value);
	return element;
}

void EntChoiceDialog::RenderOverlay()
{
	if (m_pFont) 
	{
		m_pFont->draw( m_textRect + CL_Point(m_pWindow->get_screen_x()+m_textBorder,
			m_pWindow->get_screen_y() + m_titleOffsetY+m_textBorder+4),m_formattedMsg);
	}
}

void EntChoiceDialog::BuildDialog()
{
	assert(!m_pWindow);

	m_pWindow = new CL_Window(m_title, CL_Window::no_buttons, GetGameLogic()->GetGameGUI()->get_client_area());
	m_pTitleFont = new CL_Font("Window/font", GetGameLogic()->GetGUIStyle()->get_resources());
	//we're also going to need to show some text in addition to the options
	m_pFont = new CL_Font("ListBox/font", GetGameLogic()->GetGUIStyle()->get_resources());
	
	if (m_pWindow)
	{
		//turn the windows BG off
	
		m_pListWorld = new CL_ListBox(m_pWindow->get_client_area());
		m_pListWorld->clear_selection();
		
		if (!m_msg.empty())
		{

			MovingEntity *pEnt = ((MovingEntity*)EntityMgr->GetEntityFromID(m_entityID));

			if (pEnt)
			{
				m_pFont->set_color(pEnt->GetDefaultTextColor());
			}
			
			
		} else
		{
			m_pWindow->sig_set_options() (SendOptionElement("draw_bg", "false"));
		}
	
	} else
	{
		m_pListWorld = new CL_ListBox(GetGameLogic()->GetGameGUI()->get_client_area());
	}
	
	m_slots.connect( m_pListWorld->sig_activated(), this, &EntChoiceDialog::OnSelection);
	m_slots.connect(m_pWindow->sig_end_paint(), this, &EntChoiceDialog::RenderOverlay);
}

void EntChoiceDialog::OnSelection(int selItem)
{
	if (!m_pListWorld->is_enabled()) return; //already did this
	
	
	if (m_pListWorld->get_count() < 1) return; //not ready for prime time

	if (m_startingSelection != "") return; //not ready

	m_pListWorld->enable(false);
	if (m_pWindow)
	{
		m_pWindow->enable(false);
	}

	m_chosenID =  m_pListWorld->get_item(m_pListWorld->get_current_item())->user_data;
	GetApp()->GetMyScriptManager()->RunFunction("OnDialogSelection", (BaseGameEntity*)this);
	m_mode = removing;	
	m_timer.SetIntervalReached();
	m_curElementToPlace = m_pListWorld->get_count()-1;
	RemoveItems();
}

void EntChoiceDialog::OnHighlighted(int index)
{
	GetApp()->GetMyScriptManager()->RunFunction("OnDialogChangeSelection",(BaseGameEntity*) this);
}

void EntChoiceDialog::FinalSelectionProcessing()
{
	SetDeleteFlag(true);

	if (m_entityID != C_ENTITY_NONE)
	{
		MovingEntity *m_pParent = ((MovingEntity*)EntityMgr->GetEntityFromID(m_entityID));

		if (m_pParent)
		{
			if (!m_pParent->GetScriptObject()->FunctionExists(m_callbackFunctionName))
			GetScriptManager->SetStrict(false);

			try {luabind::call_function<void>(m_pParent->GetScriptObject()->GetState(), 
				m_callbackFunctionName.c_str(), m_choices[m_chosenID].m_text, m_choices[m_chosenID].m_result, (BaseGameEntity*)this);
			} LUABIND_ENT_BRAIN_CATCH( ("Error while calling " + m_callbackFunctionName).c_str());
			GetScriptManager->SetStrict(true);

		} else
		{
			LogMsg("ChoiceDialog: Can't send result %s, entity no longer exists", m_choices[m_chosenID].m_result.c_str());
		}
	} else
	{
		if (!GetScriptManager->FunctionExists(m_callbackFunctionName))
			GetScriptManager->SetStrict(false);
		try {luabind::call_function<void>(GetScriptManager->GetMainState(), 
			m_callbackFunctionName.c_str(), m_choices[m_chosenID].m_text, m_choices[m_chosenID].m_result, (BaseGameEntity*)this);
		} LUABIND_CATCH( ("Error while calling " + m_callbackFunctionName));
		GetScriptManager->SetStrict(true);
	}


}
