#include "AppPrecomp.h"
#include "main.h"
#include "Console.h"
#include "AppUtils.h"
#include "API/Core/System/clipboard.h"
#include "ScriptManager.h"
#include "EntEditMode.h"
#include "BaseGameEntity.h"
#include "MovingEntity.h"

Console g_Console;
#define C_MAX_LOG_LINES 500
#define C_PAGE_SCROLL_LINES 50

#define C_CONSOLE_ENTER_HEIGHT 20

#define C_CONSOLE_BACKBUFFER_SIZE 20
#define C_CONSOLE_ACTIVE_LINE -1

#define C_CONSOLE_INPUT_LABEL_SIZE 140

Console::Console()
{ 
	m_bOnScreen = false; 
	m_curViewIndexOffset = 0;
	m_pInputBox = NULL;
	m_bRequestGUIToggle = false;
	m_bRequestClipboardCopy = false;
}


void Console::Init()
{
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &Console::OnKeyDown);
}

void Console::KillGUI()
{
	//When main gui is killed, it will kill this for us
	SAFE_DELETE(m_pInputBox);
}

Console::~Console()
{
	KillGUI();
}

void Console::CopyToTextBuffer()
{

	string final;

	console_cont::iterator itor;

	for (itor = m_lineVec.begin(); itor != m_lineVec.end(); itor++)
	{
		
		vector<string> lines = CL_String::tokenize(itor->m_text, "\n", false);
		for (unsigned int i=0; i < lines.size(); i++)
		{
			final += lines[i]+"\r\n";
		}
	}

	CL_Clipboard::set_text(final);
}

string Console::GetBackBufferLine(int idx)
{
	assert(idx >= 0 && idx < int(m_backBuffer.size()) && "Idiot!");
	return m_backBuffer[idx];
}


void Console::OnKeyDown(const CL_InputEvent &key)
{
	
	if ( (!IsActive() && key.id != CL_KEY_TILDE) || CL_Keyboard::get_keycode(CL_KEY_CONTROL) && key.id != CL_KEY_C )
	{
		return;
	}
	
	if (!IsActive() && key.id == CL_KEY_TILDE)
	{
		if (GetApp()->GetMyScriptManager())
		{
			if (GetGameLogic()->IsRetail())
			{
				//in retail mode, don't allow them to use this console thing
				return;
			}

		}

	}

	switch (key.id)
	{
		case CL_KEY_TILDE: //aka GRAVE or backtick
		case CL_KEY_ESCAPE:
			m_bRequestGUIToggle = true;
		break;

		case CL_KEY_UP:

			if (m_curBackBufferIndex+1 < int(m_backBuffer.size()))
			{
				m_curBackBufferIndex++;
				m_pInputBox->set_text(GetBackBufferLine(m_curBackBufferIndex));
			}

			break;

		case CL_KEY_DOWN:

			if (m_curBackBufferIndex-1 > C_CONSOLE_ACTIVE_LINE)
			{
				m_curBackBufferIndex--;
				m_pInputBox->set_text(GetBackBufferLine(m_curBackBufferIndex));
			}

			break;


		case CL_KEY_C:
			if (m_bOnScreen && CL_Keyboard::get_keycode(CL_KEY_CONTROL))

			{
				//let's put this in the text buffer
				 m_bRequestClipboardCopy = true;
			}
			break;

#ifndef __APPLE__ //we need to fix clanlib to not have different mappings for these...
		case CL_KEY_PRIOR:
#else
	case CL_KEY_PAGE_UP:
#endif
			m_curViewIndexOffset += C_PAGE_SCROLL_LINES;
			break;

	
#ifndef __APPLE__ //we need to fix clanlib to not have different mappings for these...
	case CL_KEY_NEXT:
#else
	case CL_KEY_PAGE_DOWN:
		LogMsg("Page down");
#endif
		
		m_curViewIndexOffset -= C_PAGE_SCROLL_LINES;
		
		if (m_curViewIndexOffset < 0) m_curViewIndexOffset = 0;
		break;
	}

}

void Console::AddGeneric(ConsoleItem &item)
{
	m_lineVec.push_back(item);

	while (m_lineVec.size() > C_MAX_LOG_LINES)
	{
		m_lineVec.pop_front();
	}
}

void Console::Add(const string line, CL_Color c /*= CL_Color(255,255,255,255)*/)
{
	ConsoleItem item;
	item.m_text = line;
	item.m_color = c;

	AddGeneric(item);
}

void Console::AddError(const string line)
{
	Add(line, CL_Color(255,150,150,255));
	SetOnScreen(true);
}

void Console::ProcessString(string s)
{
	if (s == "?" || s == "help")
	{
		CL_Color c = CL_Color(255,255,30,255);
		Add("");
		Add("Console Help",c);
		Add("------------",c);
		Add("Type in a lua command and it will be executed in the global lua namespace.  Or, select a single Entity in the editor to use its namespace.  (it must have a valid script)",c);
		Add("For example:",c);
		Add("Add(\"Hi!\");",c);
		return;
	}

	GetScriptManager->RunString(s.c_str());
}

void Console::PressedEnter()
{
	ConsoleItem item;
	item.m_text = ">>" + m_pInputBox->get_text();
	item.m_color = CL_Color(20,200,20,255);
	AddGeneric(item);

	if (!m_pInputBox->get_text().empty())
	{
		m_backBuffer.push_front(m_pInputBox->get_text());
		
		if (m_backBuffer.size() > C_CONSOLE_BACKBUFFER_SIZE)
		{
			m_backBuffer.pop_back(); //truncate it so it doesn't get too big
		}

		MovingEntity *pEnt = GetSelectedEntityFromEditor();

		if (pEnt)
		{
			//run in entitys namespace
			pEnt->GetScriptObject()->RunString(m_pInputBox->get_text().c_str());
		} else
		{
			//run as global	
			ProcessString(m_pInputBox->get_text());
		}
		
		m_curBackBufferIndex = C_CONSOLE_ACTIVE_LINE;
		m_pInputBox->clear();
	}
}

void Console::SetOnScreen(bool bNew)
{
	if (bNew != m_bOnScreen)
	{
		//a change was actually made
		
		KillGUI();

		if (bNew && GetApp()->GetMainWindow() && GetApp()->GetGUI())
		{
			//turn on the GUI stuff
			CL_Rect inputRect(C_CONSOLE_INPUT_LABEL_SIZE+2,GetScreenY-C_CONSOLE_ENTER_HEIGHT, GetScreenX-3, GetScreenY-7);
			m_pInputBox = new CL_InputBox(inputRect, "", GetApp()->GetGUI()->get_client_area());
			m_curViewIndexOffset = 0; //reset this

			m_pInputBox->set_focus();
			m_curBackBufferIndex = C_CONSOLE_ACTIVE_LINE;

			m_slots.connect(m_pInputBox->sig_return_pressed(), this, &Console::PressedEnter);
			m_slots.connect(m_pInputBox->sig_validate_character(), this, &Console::InputValidator);
			
			
			if (GetScriptManager)
			GetScriptManager->SetGlobalBool("g_consoleActive", true);

			if (m_pInputBox)
			{
				m_pInputBox->set_focus();
			}

		} else
		{
			//turn it off
			if (GetScriptManager)
			GetScriptManager->SetGlobalBool("g_consoleActive", false);
		}
	}
	m_bOnScreen = bNew;
}

void Console::InputValidator(char &character, bool &accept)
{
	// my VC6 don't like std::isdigit with locale :(

	if (character == '`') accept = false;
}

MovingEntity * Console::GetSelectedEntityFromEditor()
{
	EntEditMode *pEntEdit = (EntEditMode*)EntityMgr->GetEntityByName("edit mode");
	if (pEntEdit)
	{
		//is a single entity selected?
		Tile * pTile = pEntEdit->GetSingleSelectedTile();

		if (pTile && pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();

			if (pEnt->GetScriptObject() != 0)
			{
				return pEnt;
			}
		}
	} 
	return NULL;
}

void Console::RenderGUIOverlay()
{

	CL_Rect r;

	//also, label the input bar..
	r = CL_Rect(0, GetScreenY-C_CONSOLE_ENTER_HEIGHT, C_CONSOLE_INPUT_LABEL_SIZE, GetScreenY-3);

	CL_Color labelBgColor = CL_Color(20,170,20,255)	;
	string text = "Global";


	//wait, what if they are targeting a single entity in the editor?  Uh, if the editor is open that is.

	MovingEntity *pEnt = GetSelectedEntityFromEditor();

	if (pEnt)
	{
		labelBgColor = CL_Color::darkred;

		if (!pEnt->GetName().empty())
		{
			text = pEnt->GetName() + "(" + CL_String::from_int(pEnt->ID()) + ")";
		} else
		{
			text = "Entity " + CL_String::from_int(pEnt->ID());

		}
	}
	
	CL_Display::fill_rect(r, labelBgColor);
	CL_Display::draw_rect(r, CL_Color::black);



	GetApp()->GetConsoleFont()->set_alignment(origin_top_right);
	GetApp()->GetConsoleFont()->draw(C_CONSOLE_INPUT_LABEL_SIZE-2, (GetScreenY-C_CONSOLE_ENTER_HEIGHT)+2, text);

}

void Console::OnScreenChanged()
{
	if (m_bOnScreen)
	{
		SetOnScreen(false);
	m_bRequestGUIToggle = true;
	}

}

void Console::RenderPostGUI()
{
	if (!m_bOnScreen) return;

	//So we can draw the info bar across the top OVER the main editor menu if need be, while still using the editor GUI below..
	DrawTextBar(0, CL_Color(200,0,0,255), "System Console - Press ` (backtick) to close, Ctrl-C to copy into system clipboard, Page-Up/Page-Down to scroll"); 
}
void Console::Render()
{
	if (m_bRequestGUIToggle)
	{
		m_bRequestGUIToggle = false;
		SetOnScreen(!m_bOnScreen);
		//LogMsg("Toggling console display");
	}
	
	if (!m_bOnScreen) return;
	
	if (m_bRequestClipboardCopy)
	{
		m_bRequestClipboardCopy = false;
		CopyToTextBuffer();
		LogMsg("Log copied to system text buffer.");
	}
	
	//figure out overall size
	CL_Rect r(0,15, GetScreenX, GetScreenY);
	CL_Display::fill_rect(r, CL_Color(0,0,0,140));

	CL_Font *pFont = GetApp()->GetConsoleFont();
	
	if (!pFont)
	{
		//serious error, was unable to init a base font
		throw CL_Error("Serious error: Console font not initialized, check the log text file.");
	}
	
	ResetFont(pFont);

//cycle through them
	console_cont::reverse_iterator itor;

	int posy = (r.bottom-15)-C_CONSOLE_ENTER_HEIGHT;
	int posx = 10;
	
	int linesToSkip = m_curViewIndexOffset;

	for (itor = m_lineVec.rbegin(); itor != m_lineVec.rend(); itor++)
	{
		pFont->set_color(itor->m_color);

		vector<string> lines = CL_String::tokenize(itor->m_text, "\n", false);
		for (unsigned int i=0; i < lines.size(); i++)
		{
			//DrawWithShadow(posx, posy, lines[i], itor->m_color);
	
			if (linesToSkip > 0)
			{
				//this is bad way to implement a scrollback buffer because it's going to get slower the farther you scroll back.  Uh..
				linesToSkip--;
			} else
			{
				pFont->draw(posx, posy, lines[i]);
				posy -= pFont->get_height();

			}
	
		}

		if (posy < 15) 
		{
			//can't fit anymore on the screen
			break;
		}
	}

	//pFont->set_alpha(1);
	
	RenderGUIOverlay();

}
