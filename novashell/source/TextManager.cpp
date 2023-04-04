#include "AppPrecomp.h"
#include "main.h"
#include "TextManager.h"
#include "MovingEntity.h"

#define C_TEXT_FADE_IN_SPEED_MS 200
#define C_TEXT_FADE_OUT_SPEED_MS 400

TextManager g_textManager;

TextObject::TextObject(TextManager *pTextManager)
{
	m_pTextManager = pTextManager;
	m_pEntity = NULL;
	m_vecDisplacement = CL_Vector2::ZERO;
}

void TextObject::InitCustom(const string &text, MovingEntity * pEnt, const CL_Vector2 &vecPos,
							const CL_Vector2 &vecMovement, const CL_Color &col, int timetoShowMS,
							int fontID)
{
	m_text = text;
	m_timeCreated = GetApp()->GetGameTick();
	m_timeToShowMS = timetoShowMS;
	m_pEntity = pEnt;
	m_worldPos = vecPos;
	m_vecMovement = vecMovement;
	m_color = col;
	
	SetMode(CUSTOM);

	//looks like we're going to draw, let's setup where in advance
	m_boundingRect = CL_Rect(0,0,1024,1024);
	m_fontID = fontID;

	CL_Font *pFont = GetApp()->GetFont(m_fontID);
	//get the real size of what we have to draw
	m_boundingRect = pFont->bounding_rect(m_boundingRect, m_text);

	if (pEnt)
	{
		//kill the text when this ent dies or is off the screen
		m_slots.connect(pEnt->sig_delete, this, &TextObject::EntDeleted);
	}
}

int TextObject::Init(const string &text, MovingEntity * pEnt, int fontID)
{
	assert(pEnt);
	m_text = text;
	m_timeCreated = GetApp()->GetGameTick();
	m_timeToShowMS = text.size()* 80;
	SetMode(DIALOG);

   //TODO: Figure out where the max macro is?
  if (m_timeToShowMS < 2900) m_timeToShowMS = 2900;

	m_pEntity = pEnt;

	TextObject *pLastText = m_pTextManager->GetLastTextForEntity(pEnt);

	if (pLastText)
	{
		m_bColorOdd = !pLastText->GetColorOdd();
	} else
	{
		m_bColorOdd = false;
	}

	m_color = m_pEntity->GetDefaultTextColor();
	
	if (m_bColorOdd)
	{
		//odd number, let's color it slightly differently
		short r,g,b;

		r = m_color.get_red() - 30;
		g = m_color.get_green() - 30;
		b = m_color.get_blue() - 30;

		//force them to be within range
		r = cl_min(r, 255); r = cl_max(0, r);
		g = cl_min(g, 255); g = cl_max(0, g);
		b = cl_min(b, 255); b = cl_max(0, b);

		m_color = CL_Color(r,g,b);
	} 

	//looks like we're going to draw, let's setup where in advance
	m_boundingRect = CL_Rect(0,0,400,400);
	m_fontID = fontID;

	CL_Font *pFont = GetApp()->GetFont(fontID);

	//get the real size of what we have to draw
	m_boundingRect = pFont->bounding_rect(m_boundingRect, m_text);

	//now we'll do a very slow way to determine if it's actually on the screen or not.  If not, assume they couldn't hear it. 

	UpdateDialog(false);

	//let's extend the range vertically a bit
	m_rect.bottom += 300;
	if (!GetCamera->GetScreenRect().is_overlapped(m_rect))
	{
		//out of view
		m_timeToShowMS = 0;
		m_bVisible = false;
		return 0;
	}

	m_slots.connect(pEnt->sig_delete, this, &TextObject::EntDeleted);

	return m_timeToShowMS - C_TEXT_FADE_OUT_SPEED_MS; //don't include the fading in the time we return
}

void TextObject::EntDeleted(int ID)
{
	m_pEntity = NULL;
}


CL_Point TextObject::CalculateTextAvoidenceOffset()
{
	//do a slow scan through everybody to figure out where text already is from this guy

	CL_Point ptOffset = CL_Point(0,0);
	
	textObject_list::iterator itor;
	for (itor = m_pTextManager->GetTextList().begin(); itor != m_pTextManager->GetTextList().end();)
	{
		if (itor->GetEntity() == m_pEntity && itor->GetMode() == DIALOG)
		{
			//it's our same guy!  Wait, is it actually this instance?
			if ( &(*itor) == this)
			{
				//yep, we're done here.
				break;
			} else
			{
				//apply what we can learn
				ptOffset.y -= itor->GetTextRect().get_height(); //extra spacer
			}
		}
		itor++;
	}

	return ptOffset;
}

//return false to delete this object
bool TextObject::UpdateDialog(bool bFancyPositioning)
{
	if (!m_pEntity) return false;

	m_bVisible = true;
	m_rect = m_boundingRect;
	
	//now we need to position it

	CL_Vector2 entPos = m_pEntity->GetMap()->GetMyMapCache()->WorldToScreen(m_pEntity->GetPos());
	
	m_pos = CL_Point(entPos.x- (m_rect.right/2)  , entPos.y - m_rect.bottom );

	m_pos.y -= ( (m_pEntity->GetSizeY()*0.8) * GetCamera->GetScale().y * m_pEntity->GetScale().y);

	
	if ( bFancyPositioning)
	{
		//clip to screen
		m_pos.x = max(0, m_pos.x);
		m_pos.y = max(0, m_pos.y);
		m_pos.x = min(GetScreenX-m_rect.get_width(), m_pos.x);
		m_pos.y = min(GetScreenY-m_rect.get_height(), m_pos.y);
		m_pos += CalculateTextAvoidenceOffset();
	}

	//apply to rect

	m_rect.apply_alignment(origin_top_left, -m_pos.x, -m_pos.y);
	//LogMsg(PrintRect(m_rect).c_str());
	return true;
}

//return false to delete this object
bool TextObject::UpdateCustom()
{
	m_bVisible = true;
	m_rect = m_boundingRect;

	//apply movement
	m_vecDisplacement += m_vecMovement;

	
	CL_Vector2 entPos;
	
	if (GetMode() == CUSTOM)
	{
		entPos = g_pMapManager->GetActiveMapCache()->WorldToScreen(m_worldPos);
	} else
	{
		entPos = m_worldPos;
		//screen coordinates used
	}
	
	m_pos = CL_Point(entPos.x- (m_rect.right/2)  , entPos.y - (m_rect.bottom));
	m_pos += CL_Point(m_vecDisplacement.x, m_vecDisplacement.y);
	m_rect.apply_alignment(origin_top_left, -m_pos.x, -m_pos.y);
	
	//LogMsg(PrintRect(m_rect).c_str());
	return true;
}

bool TextObject::Update()
{

	if (m_pEntity)
	{
		if (m_pEntity->GetTile()->GetParentScreen()->GetParentMapChunk()->GetParentMap() != g_pMapManager->GetActiveMap())
		{
			m_bVisible = false;
			return true;
		}
	}
	int timeLeft = (m_timeCreated+m_timeToShowMS) - GetApp()->GetGameTick();
	
	if (timeLeft < 0)
	{
		//done showing
		return false;
	}
	
	if (timeLeft < C_TEXT_FADE_OUT_SPEED_MS)
	{
		m_alpha = ( float(timeLeft) / float(C_TEXT_FADE_OUT_SPEED_MS));
	} else
	{
		
		m_alpha = 1;

		//but wait, should we fade in now?
		
		int timeElapsed = m_timeToShowMS - timeLeft;
		
		if (timeElapsed < C_TEXT_FADE_IN_SPEED_MS)
		{
			m_alpha = float(timeElapsed)/ float(C_TEXT_FADE_IN_SPEED_MS);
		}
	}


	switch (m_mode)
	{
	case DIALOG:
		return UpdateDialog();
		break;

	case CUSTOM:
	case CUSTOM_SCREEN:
		return UpdateCustom();
		break;

	default:
		assert(0);
	}

	return true; //don't delete yet
}

void TextObject::Render()
{
	if (!m_bVisible) return;

	//draw a semi transparent box around it so we can read the text easier
	CL_Display::fill_rect(m_rect, CL_Color(0,0,0,cl_min(130, (m_alpha*180))));
	CL_Font *pFont = GetApp()->GetFont(m_fontID);
	pFont->set_color(m_color);
	pFont->set_alpha(m_alpha);
	pFont->draw(m_rect, m_text);
}

TextManager::TextManager()
{
	Reset();
}

TextManager::~TextManager()
{
}

void TextManager::Reset()
{
	m_textList.clear();
}

void TextManager::AddCustom(const string &text, const MovingEntity *pEnt, const CL_Vector2 &vecPos,
					  const CL_Vector2 &vecMovement, const CL_Color &col, int timeToShowMS, int fontID)
{
	TextObject t(this);
	m_textList.push_back(t);

	m_textList.rbegin()->InitCustom(text, const_cast<MovingEntity*>(pEnt),
		vecPos, vecMovement, col, timeToShowMS, fontID);
}

void TextManager::AddCustomScreen(const string &text, const CL_Vector2 &vecPos,
							const CL_Vector2 &vecMovement, const CL_Color &col, int timeToShowMS, int fontID)
{
	TextObject t(this);
	m_textList.push_back(t);

	m_textList.rbegin()->InitCustom(text, NULL,
		vecPos, vecMovement, col, timeToShowMS, fontID);
	m_textList.rbegin()->SetMode(TextObject::CUSTOM_SCREEN);

}

int TextManager::Add(const string &text, MovingEntity *pEnt)
{
	if (!pEnt)
	{
		LogError("NULL entity passed into TextManager::Add, ignoring it.");
		return 0;
	}

	if (!pEnt->GetTile()->GetParentScreen())
	{
		LogMsg("Warning: TextManager type things shouldn't go into the visual Init(), use GameInit()");
		return 0;
	}
	
	TextObject t(this);
	m_textList.push_back(t);
	return m_textList.rbegin()->Init(text, const_cast<MovingEntity*>(pEnt), C_FONT_NORMAL);
}

int TextManager::GetCountOfTextActiveForEntity(const MovingEntity *pEnt)
{
	int count = 0;

	textObject_list::iterator itor;
	for (itor = m_textList.begin(); itor != m_textList.end();)
	{
		if (itor->GetEntity() == pEnt) 
		{
			count++;
		}

		itor++;
	}

	return count;
}


TextObject * TextManager::GetLastTextForEntity(const MovingEntity *pEnt)
{

	textObject_list::iterator itor;
	for (itor = m_textList.begin(); itor != m_textList.end();)
	{
		if (itor->GetEntity() == pEnt) 
		{
			return &(*itor);
		}

		itor++;
	}

	return NULL;  //none existed
}

bool compareTextObjectsBackwards(const TextObject &A, const TextObject &B) 
{
	return (A.GetTimeCreated() > B.GetTimeCreated());
}


void TextManager::Update(float step)
{
	if (GetGameLogic()->GetGamePaused()) return;
	
	//sort them based on time created
	m_textList.sort(compareTextObjectsBackwards);

	textObject_list::iterator itor;
	for (itor = m_textList.begin(); itor != m_textList.end();)
	{
		if (!itor->Update())
		{
			//time to delete it
			itor = m_textList.erase(itor);
			continue;
		}

		itor++;
	}

}

void TextManager::Render()
{
	//render them out in reverse order

	textObject_list::reverse_iterator itor;
	for (itor = m_textList.rbegin(); itor != m_textList.rend();)
	{
		itor->Render();
		itor++;
	}
}


/*
Object: TextManager
Intelligently draws text and captions.

About:

This is a global object that can always be accessed.

Text is always drawn over all tiles/entities. (will not be obscured)

Usage:

(code)
GetTextManager:Add("Hmm?", this);
(end)


Group: Member Functions

func: Add
(code)
number Add(string msg, Entity ent)
(end)
This function causes text to pop up over an entity, wait an appropriate amount of time (depends on text length) and then fade out.

Text is vertically stacked (newest on the bottom) if an <Entity> has multiple lines showing.

Text is depth sorted by age. (newer text is readable over older text)

If an <Entity> is deleted, any associated text is automatically deleted as well.

The color of text can be adjusted with <Entity::SetDefaultTalkColor>.

Parameters:

msg - The line of text this <Entity> should say.  Automatically text wrapped if needed.
ent - The <Entity> object that should "say" this line of text.

Returns:

How many milliseconds the text will be displayed, sometimes useful to know when timing things.

func: AddCustom
(code)
nil AddCustom(string msg, Entity ent, Vector2 vPos, Vector2 vMovement, Color color,
		number timeToShowMS, number fontID)
(end)
This is like a turbo-powered version of <Add> that gives you many more options.

An important difference is the text doesn't follow around the <Entity> like with <Add>, it stays in the position sent and optionally applies movement to it.

If an <Entity> is deleted, any associated text is automatically deleted as well.

Usage:
(code)
//pretend we got hit for 5 damage.  Let's make a red 5 appear and float up, a common RPG damage effect.
GetTextManager:AddCustom("5", this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);
(end)

Parameters:

msg - The line of text that should be displayed.  Automatically text wrapped if needed.
ent - The <Entity> object that should "say" this line of text.
vPos - A <Vector2> object containing the world coordinates of where the text should be displayed. (will be centered around it)
vMovement - A <Vector2> describing a direction and amount of movement to apply to the text.  Send 0,0 for no movement.
Color - A <Color> object describing the color of the text.
timeToShowMS - How many milliseconds this text should remain.
fontID - A valid fontID or one of the <C_FONT_CONSTANTS>.

func: AddCustomScreen
(code)
nil AddCustomScreen(string msg, Vector2 vPos, Vector2 vMovement, Color color,
number timeToShowMS, number fontID)
(end)
This is similar to <AddCustom> with some important differences:

* vPos is set in screen coordinates. (0,0 would be the upper left)  The text stays on the screen at all times and is not affected by camera movement or changing <Map>s.
* It is not associated with an <Entity>

This function is used to show tips and help in the examples.

Parameters:

msg - The line of text that should be displayed.  Automatically text wrapped if needed.
vPos - A <Vector2> object containing the screen coordinates of where the text should be displayed. (will be centered around it)
vMovement - A <Vector2> describing a direction and amount of movement to apply to the text.  Send 0,0 for no movement.
Color - A <Color> object describing the color of the text.
timeToShowMS - How many milliseconds this text should remain.
fontID - A valid fontID or one of the <C_FONT_CONSTANTS>.

Section: Related Constants

Group: C_FONT_CONSTANTS
Can be used as fontID's with functions such as <TextManager::AddCustom>.

constant: C_FONT_SMALL
A very small font.

constant: C_FONT_DEFAULT
A medium sized font.


*/
