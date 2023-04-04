#include "AppPrecomp.h"
#include "DrawManager.h"
#include "MapManager.h"
#include "AppUtils.h"

DrawManager g_drawManager;

#define C_VERY_LOW_NUMBER -999999;

DrawManager::DrawManager()
{
	m_lastRenderedLayerID = C_VERY_LOW_NUMBER;
}

DrawManager::~DrawManager()
{
}


DrawEventList * DrawManager::GetDrawEventList(int layerID)
{
	list<DrawEventList>::iterator itor = m_eventList.begin();

	while (itor != m_eventList.end())
	{
		if ( itor->m_layerID == layerID)
		{
			//this is it
			return &(*itor);
		} else
		{
			if (itor->m_layerID > layerID)
			{
				//insert a new one before this
				
				return & (*m_eventList.insert(itor,DrawEventList(layerID)));
			}
		}
		itor++;
	}

	//add it and return it
	m_eventList.push_back(DrawEventList(layerID));
	return &m_eventList.back();
}

void DrawManager::DrawLine( CL_Vector2 a, CL_Vector2 b, CL_Color col, int layerID, int drawStyle )
{
	DrawEvent *pEvent = GetDrawEventList(layerID)->GetNewEvent();

	pEvent->m_type = DrawEvent::TYPE_DRAW_LINE;
	
	if (drawStyle == C_DRAW_WORLD_COORDS)
	{
		pEvent->m_a = g_pMapManager->GetActiveMap()->GetMyMapCache()->WorldToScreen(a);
		pEvent->m_b = g_pMapManager->GetActiveMap()->GetMyMapCache()->WorldToScreen(b);

	} else
	{
		pEvent->m_a = a;
		pEvent->m_b = b;
	}
	
	pEvent->m_color = col;
	pEvent->m_drawStyle = drawStyle;
}


void DrawManager::DrawPixel( CL_Vector2 vPos, CL_Color col, int layerID, int drawStyle )
{
	DrawEvent *pEvent = GetDrawEventList(layerID)->GetNewEvent();

	pEvent->m_type = DrawEvent::TYPE_DRAW_PIXEL;

	if (drawStyle == C_DRAW_WORLD_COORDS)
	{
		pEvent->m_a = g_pMapManager->GetActiveMap()->GetMyMapCache()->WorldToScreen(vPos);

	} else
	{
		pEvent->m_a = vPos;
	}

	pEvent->m_color = col;
	pEvent->m_drawStyle = drawStyle;
}

void DrawManager::DrawFilledRect( CL_Rectf rArea, CL_Color col, int layerID, int drawStyle )
{
	DrawEvent *pEvent = GetDrawEventList(layerID)->GetNewEvent();

	pEvent->m_type = DrawEvent::TYPE_DRAW_FILLED_RECT;

	if (drawStyle == C_DRAW_WORLD_COORDS)
	{
		rArea = ConvertWorldRectToScreenRect(rArea);
	} 
	pEvent->m_rect = rArea;

	pEvent->m_color = col;
	pEvent->m_drawStyle = drawStyle;
}

void DrawManager::DrawRect( CL_Rectf rArea, CL_Color col, int layerID, int drawStyle )
{
	DrawEvent *pEvent = GetDrawEventList(layerID)->GetNewEvent();

	pEvent->m_type = DrawEvent::TYPE_DRAW_RECT;

	if (drawStyle == C_DRAW_WORLD_COORDS)
	{
		rArea = ConvertWorldRectToScreenRect(rArea);
	} 
	pEvent->m_rect = rArea;

	pEvent->m_color = col;
	pEvent->m_drawStyle = drawStyle;
}

void DrawManager::Update(float delta)
{
	m_eventList.clear();
	m_lastRenderedLayerID = C_VERY_LOW_NUMBER;
}

void DrawManager::RenderUpToLayerID(int layerID, CL_GraphicContext *pGC)
{
	if (layerID < m_lastRenderedLayerID)
	{
		//reset it
		m_lastRenderedLayerID = C_VERY_LOW_NUMBER; //some very low number
	}
	
	list<DrawEventList>::iterator itor = m_eventList.begin();

	for (; itor != m_eventList.end(); )
	{
		if (itor->m_layerID > m_lastRenderedLayerID && itor->m_layerID <= layerID)
		{
			itor->Render(pGC);
			m_lastRenderedLayerID = itor->m_layerID;
			itor++;
		} else
		{
			//all done for now I guess
			return;
		}
	}
}

void DrawEventList::Render( CL_GraphicContext *pGC )
{
	list <DrawEvent>::iterator itor = m_events.begin();
	while (itor != m_events.end())
	{
		itor->Render(pGC);
		itor++;
	}
}

DrawEvent * DrawEventList::GetNewEvent()
{
	m_events.push_back(DrawEvent());
	return &m_events.back();
}

void DrawEvent::Render( CL_GraphicContext *pGC )
{
	switch (m_type)
	{
	case TYPE_DRAW_LINE:
		CL_Display::draw_line(m_a.x, m_a.y,  m_b.x, m_b.y, m_color);
		break;
	case TYPE_DRAW_PIXEL:
		CL_Display::draw_pixel(m_a.x, m_a.y, m_color);
		break;
	case TYPE_DRAW_FILLED_RECT:
		CL_Display::fill_rect(m_rect, m_color);
		break;
	case TYPE_DRAW_RECT:
		CL_Display::draw_rect(m_rect, m_color);
		break;


	default:
		LogError("DrawManager error with type %d", m_type);
	}
}


/*
Object: DrawManager
For drawing primitives like rectangles and lines.

About:

Being able to draw a rectangle, line, or single pixel from any place is useful for debugging or things like fading the screen out.

You can specify which layer to draw over and whether or not to use screen or world coordinates.


Usage:

(code)
//draw a black line on the screen, on top of everything else
GetDrawManager:DrawLine(Vector2(0,0), Vector2(100,100), Color(0,0,0,255), C_LAYER_TOP, C_DRAW_SCREEN_COORDS);
(end)


Group: Member Functions
*/

/*
func: DrawLine
(code)
nil DrawLine(Vector2 a, Vector2 b, Color col, number layerID, number drawType) 
(end)
Draws a line of the specified color at the specified layerID.

Parameters:

a - A <Vector2> object containing the line's starting position
b - A <Vector2> object containing the line's ending position
col - A <Color> object that specifies the line's color and opacity
layerID - Onto which layer we should draw to, use C_LAYER_TOP for on top of everything
drawType - One of the <C_DRAW_TYPE_CONSTANTS>.


func: DrawPixel
(code)
nil DrawPixel(Vector2 vPos, Color col, number layerID, number drawType) 
(end)
Draws a pixel of the specified color at the specified layerID.

Parameters:

vPos - A <Vector2> object containing the location of the pixel
col - A <Color> object that specifies the line's color and opacity
layerID - Onto which layer we should draw to, use C_LAYER_TOP for on top of everything
drawType - One of the <C_DRAW_TYPE_CONSTANTS>.

func: DrawRect
(code)
nil DrawRect(Rectf area, Color col, number layerID, number drawType) 
(end)
Draws a rectangle outline of the specified color at the specified layerID.

Parameters:

area - A <Rectf> object containing the coordinates of the rect
col - A <Color> object that specifies the line's color and opacity
layerID - Onto which layer we should draw to, use C_LAYER_TOP for on top of everything
drawType - One of the <C_DRAW_TYPE_CONSTANTS>.

func: DrawFilledRect
(code)
nil DrawFilledRect(Rectf area, Color col, number layerID, number drawType) 
(end)
Draws a filled rectangle of the specified color at the specified layerID.

Parameters:

area - A <Rectf> object containing the coordinates of the rect
col - A <Color> object that specifies the line's color and opacity
layerID - Onto which layer we should draw to, use C_LAYER_TOP for on top of everything
drawType - One of the <C_DRAW_TYPE_CONSTANTS>.


Section: Related Constants

Group: C_DRAW_TYPE_CONSTANTS
Used with the <DrawManager>

constant: C_DRAW_SCREEN_COORDS
The numbers passed in are in screen coordinates, so 0,0 would always mean the upper-left most dot on the screen.

constant: C_DRAW_WORLD_COORDS
The numbers passed in are in world coordinates, drawing a dot at an entities position would draw it over wherever the entity is, possibly offscreen.

*/
