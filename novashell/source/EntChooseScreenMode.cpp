#include "AppPrecomp.h"
#include "EntChooseScreenMode.h"
#include "GameLogic.h"
//#include <strstream>
#include "EntEditor.h"
#include "EntMapCache.h"
#include "MovingEntity.h"

#define C_OFFSET_FROM_TOP_OF_SCREEN C_EDITOR_MAIN_MENU_BAR_HEIGHT

EntChooseScreenMode::EntChooseScreenMode() : BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pMenu = NULL;
	m_pThumb = NULL;
	m_pUsedBlock = NULL;
	m_pUnusedBlock = NULL;
	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntChooseScreenMode::onButtonDown);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntChooseScreenMode::OnMouseMove);
	m_vecLastMousePos = CL_Point(0,0);

	m_slots.connect(GetGameLogic()->GetMyWorldManager()->sig_map_changed, this, &EntChooseScreenMode::Init);
	//GUI
	m_pWindow = NULL;
	m_pLabel = NULL;
	m_lastInfoScreenID = C_INVALID_SCREEN;
	SetName("choose screen mode");
	Init();
}


EntChooseScreenMode::~EntChooseScreenMode()
{
    Kill();
}


void EntChooseScreenMode::OnMouseMove(const CL_InputEvent &key)
{
	m_vecLastMousePos = key.mouse_pos;
}


void EntChooseScreenMode::Kill()
{
	SAFE_DELETE(m_pUnusedBlock);
    SAFE_DELETE(m_pUsedBlock);
	SAFE_DELETE(m_pMenu);
    SAFE_DELETE(m_pLabel);
	SAFE_DELETE(m_pWindow);

	SAFE_DELETE(m_pThumb);
}

void EntChooseScreenMode::onButtonDown(const CL_InputEvent &key)
{

	switch(key.id)
	{
	case CL_MOUSE_LEFT:
		{

			int screenID = GetScreenClicked(key.mouse_pos.x, key.mouse_pos.y);
			if (screenID != C_INVALID_SCREEN)
			{
				CL_Vector2 vecPos = g_pMapManager->GetActiveMap()->ScreenIDToWorldPos(screenID);
				//offset it to the middle of the screen
				vecPos.x += g_pMapManager->GetActiveMap()->GetMapChunkPixelSize()/2;
				vecPos.y += g_pMapManager->GetActiveMap()->GetMapChunkPixelSize()/2;
				GetCamera->SetTargetPosCentered(vecPos);
				GetCamera->InstantUpdate();
			}  else
			{
				//clicked on an invalid part of the screen
			}
		}
		break;

	default: ;
	}
}

void EntChooseScreenMode::SetStatus(const string &strMsg)
{
	m_pLabelStatus->set_text(strMsg);
}


void EntChooseScreenMode::Init()
{
    Kill();
    
    m_pUsedBlock = new CL_Surface("usedBlock", GetApp()->GetResourceManager());
    m_pUnusedBlock = new CL_Surface("unusedBlock", GetApp()->GetResourceManager());
 
    cl_assert(m_pUsedBlock->get_width() == m_pUnusedBlock->get_width() && 
        m_pUsedBlock->get_height() == m_pUnusedBlock->get_height() &&
        "The unused and used block graphics should be the same size")
    
	//setup GUI
	m_pWindow = new CL_Window(CL_Rect(0, 0, 300, 105), "Map Info Floating Palette", CL_Window::close_button, GetApp()->GetGUI());
	m_pWindow->set_position(0, C_EDITOR_MAIN_MENU_BAR_HEIGHT);
	m_pWindow->set_event_passing(false);

	//setup our own tip display
	m_pLabelStatus = new CL_Label(CL_Point(2,25),"", m_pWindow->get_client_area());
	m_pLabel = new CL_Label(CL_Point(2,45),"", m_pWindow->get_client_area());

	SetGenericStatus();
	//set main dialog tips
	EntEditor *pEditor = (EntEditor*) EntityMgr->GetEntityByName("editor");
	if (!pEditor) throw CL_Error("Unable to locate editor entity");
	pEditor->SetTipLabel("Map Info Mode - Click the map to move the camera there.\n"\
		"\nMove the mouse over a square to see info about it.");
	
	if (g_pMapManager->GetActiveMap())
	{

	
		if (g_pMapManager->GetActiveMap()->GetThumbnailWidth() != 0)
		{
			CL_PixelBuffer pb(g_pMapManager->GetActiveMap()->GetThumbnailWidth(), g_pMapManager->GetActiveMap()->GetThumbnailHeight(), g_pMapManager->GetActiveMap()->GetThumbnailWidth()*3, C_THUMBNAIL_FORMAT);
			m_pThumb = new CL_Surface(pb);
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);

		}
	}
	m_slotClose = m_pWindow->sig_close().connect_virtual(this, &EntChooseScreenMode::OnClose);

//create the menu

	m_pMenu = new CL_Menu(m_pWindow->get_client_area());

	CL_MenuNode  *pItem;
	pItem = m_pMenu->create_item("Utilities/Auto Size Map (removes dead space and saves)");
	m_slots.connect(pItem->sig_clicked(), this, &EntChooseScreenMode::OnAutoResize);

	pItem = m_pMenu->create_item("Utilities/Refresh All Thumbnails");
	m_slots.connect(pItem->sig_clicked(), this, &EntChooseScreenMode::OnClearThumbnails);

	QuickInit();
}

void EntChooseScreenMode::QuickInit()
{
	CalculateMapSizes();
	GenerateThumbnailsIfNeeded();
	SetupInfoPanel(C_INVALID_SCREEN); //default info
}

void EntChooseScreenMode::OnClearThumbnails()
{
	g_pMapManager->GetActiveMap()->InvalidateAllThumbnails();
	GenerateThumbnailsIfNeeded();
}

void EntChooseScreenMode::OnClose(CL_SlotParent_v0 &parent_handler)
{
	SetDeleteFlag(true);
}

void EntChooseScreenMode::OnAutoResize()
{
	BlitMessage("Removing blank space and saving map...");
	
	g_pMapManager->GetActiveMap()->ForceSaveNow();
	g_pMapManager->GetActiveMap()->Load();
	g_pMapManager->GetActiveMap()->RemoveUnusedFileChunks();
	QuickInit();

}
void EntChooseScreenMode::SetGenericStatus()
{
	SetStatus("");
}
void EntChooseScreenMode::GenerateThumbnailsIfNeeded()
{
	//make the list

	m_thumbList.clear();

    if (!g_pMapManager->GetActiveMap())
	{
		//no active world
		return;
	}
	map_chunk_map *pWorldMap = g_pMapManager->GetActiveMap()->GetChunkMap();
	map_chunk_map::const_iterator itor = pWorldMap->begin();

	while (itor != pWorldMap->end())
	{
		if (itor->second->GetNeedsThumbnailRefresh())
		{
			m_thumbList.push_back(itor->second->GetScreenID());
		}
		itor++;
	}
}

void EntChooseScreenMode::CalculateMapSizes()
{
	m_blockSize = CL_Rect(0,0,128, 128);

	if (g_pMapManager->GetActiveMap())
	{

	
		//adjust the size if needed to fit more blocks on the screen

		if ( m_blockSize.right*g_pMapManager->GetActiveMap()->GetSizeX() > GetScreenX)
		{
			m_blockSize.right = GetScreenX/g_pMapManager->GetActiveMap()->GetSizeX();
		}

		if (m_blockSize.bottom*g_pMapManager->GetActiveMap()->GetSizeY() > (GetScreenY - C_OFFSET_FROM_TOP_OF_SCREEN))
		{
			m_blockSize.bottom = (GetScreenY-C_OFFSET_FROM_TOP_OF_SCREEN) /g_pMapManager->GetActiveMap()->GetSizeY();
		}
	}

	if (g_pMapManager->GetActiveMap())
	{

	
	//make it square
	m_blockSize.bottom = min(m_blockSize.bottom, m_blockSize.right);
	m_blockSize.right = m_blockSize.bottom;


	m_drawOffset.x = m_blockSize.right * g_pMapManager->GetActiveMap()->GetSizeX();
	m_drawOffset.x = (GetScreenX-m_drawOffset.x)/2;

	m_drawOffset.y = GetScreenY - (m_blockSize.bottom* g_pMapManager->GetActiveMap()->GetSizeY());
	} else
	{

		m_drawOffset = CL_Point(0,0);
	}
}


//returns C_INVALID_SCREEN for a bad screen
int EntChooseScreenMode::GetScreenClicked(int x, int y)
{
	CL_Point pt (x, y);

	if (pt.x < m_drawOffset.x) return C_INVALID_SCREEN;
	if (pt.y < m_drawOffset.y) return C_INVALID_SCREEN;
	
    pt -= CL_Point(m_drawOffset.x, m_drawOffset.y);

    pt.x /= m_blockSize.right;
    pt.y /= m_blockSize.bottom;
	
    if (pt.x < 0 || pt.x >= g_pMapManager->GetActiveMap()->GetSizeX()) return C_INVALID_SCREEN;
    if (pt.y < 0 || pt.y >= g_pMapManager->GetActiveMap()->GetSizeY()) return C_INVALID_SCREEN;
	
	pt.x += g_pMapManager->GetActiveMap()->GetWorldRect()->left;
	pt.y += g_pMapManager->GetActiveMap()->GetWorldRect()->top;

    return g_pMapManager->GetActiveMap()->GetScreenID(pt.x, pt.y);
}


void EntChooseScreenMode::DrawBlock(int x, int y)
{
    CL_Rect drawRect(m_blockSize);
    drawRect += CL_Point( x*m_blockSize.right, y*m_blockSize.bottom);
    drawRect += m_drawOffset;
   
	x += g_pMapManager->GetActiveMap()->GetWorldRect()->left;
	y += g_pMapManager->GetActiveMap()->GetWorldRect()->top;

	ScreenID screenID = g_pMapManager->GetActiveMap()->GetScreenID(x,y);
	MapChunk *pWorldChunk;

    if (pWorldChunk = g_pMapManager->GetActiveMap()->DoesWorldChunkExist(screenID))
    {
  		if (pWorldChunk->GetThumbnail() && m_pThumb)
        {
			m_pThumb->set_pixeldata(*pWorldChunk->GetThumbnail());
			m_pThumb->set_color(CL_Color(255,255,255,255));
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
			m_pThumb->draw(drawRect);


        } else
        {
				m_pUsedBlock->set_color(CL_Color(255,100,100,255));
	

			if (pWorldChunk->IsEmpty())
			{
				m_pUnusedBlock->draw(drawRect);
			} else
			{
				//no thumbnail exists
				m_pUsedBlock->draw(drawRect);
			}
        }
    } else
    {
        m_pUnusedBlock->draw(drawRect);
    }
}


//send C_INVALID_SCREEN to clear info panel?
void EntChooseScreenMode::SetupInfoPanel(ScreenID screenID)
{
	m_lastInfoScreenID = screenID;

	string st;

	MapChunk *pChunk;

	if (screenID != C_INVALID_SCREEN)
	{
		st = "Screen ID: " + CL_String::from_int(screenID) + " (X: "
			+ CL_String::to(g_pMapManager->GetActiveMap()->GetXFromScreenID(screenID)) + " Y: "
			+ CL_String::to(g_pMapManager->GetActiveMap()->GetYFromScreenID(screenID)) + ")\n";
		
		if (! (pChunk =g_pMapManager->GetActiveMap()->DoesWorldChunkExist(screenID)))
		{
			st += "Doesn't exist\n";
		} else
		{
			st += "Exists ";
			//if (GetWorldCache->IsScreenCached(screenID)) st += "(cached) ";
			if (!pChunk->IsScreenLoaded()) st += "(on disk) "; else st += "(in mem) ";
			if (pChunk->GetScreen()->IsEmpty()) st +="(empty) "	;
			if (pChunk->GetNeedsThumbnailRefresh()) st += "(needs new thumb)";
			if (pChunk->GetDataChanged()) st += "(needs save )";

			st += "\n";
		}
	} else
	{
		if (g_pMapManager->GetActiveMap())
		{
			st = CL_String::from_int(g_pMapManager->GetActiveMap()->GetChunkMap()->size()) + " screens exist.";
		}
	}
	
	m_pLabel->set_text(st);
}


void EntChooseScreenMode::DrawPlayerPositionOverlay()
{
	if (GetPlayer == NULL)
	{
		//no player is currently in view, this doesn't mean an error necessarily
		return;
	}

	CL_Vector2 vecPos = WorldToScreen(CL_Vector2(GetPlayer->GetPos().x, GetPlayer->GetPos().y));

	CL_Rect drawRect(vecPos.x, vecPos.y, vecPos.x+6,vecPos.y+6);
	m_pUsedBlock->set_color(CL_Color(0,255,100,255));
	m_pUsedBlock->draw(drawRect);
}

void EntChooseScreenMode::DrawViewScreenOverlay()
{
		CL_Vector2 vecPosUpperLeft = WorldToScreen(GetCamera->GetPos());
		CL_Vector2 vecLowerRightPos = GetCamera->GetPos();

		vecLowerRightPos.x += (GetScreenX / GetCamera->GetScale().x);
		vecLowerRightPos.y += (GetScreenY / GetCamera->GetScale().y);
		vecLowerRightPos = WorldToScreen(vecLowerRightPos);

		CL_Rect drawRect(vecPosUpperLeft.x, vecPosUpperLeft.y, vecLowerRightPos.x, vecLowerRightPos.y);
		m_pUsedBlock->set_color(CL_Color(0,0,255,80));
		m_pUsedBlock->draw(drawRect);
}

CL_Vector2 EntChooseScreenMode::WorldToScreen(CL_Vector2 vecWorldPos)
{

	Map *pWorld = g_pMapManager->GetActiveMap();

	ScreenID screenID = pWorld->GetScreenIDFromWorld(vecWorldPos);

	CL_Vector2 vecPos;
	vecPos.x = pWorld->GetXFromScreenID(screenID);
	vecPos.y = pWorld->GetYFromScreenID(screenID);

	vecPos.x -= pWorld->GetWorldRect()->left;
	vecPos.y -= pWorld->GetWorldRect()->top;

	CL_Vector2 vecFinal(vecPos.x*m_blockSize.right, vecPos.y*m_blockSize.bottom);
	vecFinal.x += m_drawOffset.x;
	vecFinal.y += m_drawOffset.y;

	//now figure out the in-between block part, first a ratio between 0 and 1
	float offsetX = float(altmod(vecWorldPos.x, pWorld->GetMapChunkPixelSize())) / pWorld->GetMapChunkPixelSize();  
	float offsetY = float(altmod(vecWorldPos.y, pWorld->GetMapChunkPixelSize())) / pWorld->GetMapChunkPixelSize();  
	//LogMsg("OffsetX: %.2f", offsetX);

	vecFinal.x += float(m_blockSize.right)*offsetX;
	vecFinal.y += float(m_blockSize.bottom)*offsetY;

	return vecFinal;
}


void EntChooseScreenMode::ProcessNextThumb()
{
	
	bool bGeneratedThumb = false;

	while (!m_thumbList.empty() && !bGeneratedThumb)
	{
		//keep generating thumbnails little by little
		ScreenID screenID = m_thumbList.front();
		if (g_pMapManager->GetActiveMap()->DoesWorldChunkExist(screenID))
		{
			//it still exists, yay
			bGeneratedThumb = g_pMapManager->GetActiveMapCache()->GenerateThumbnail(screenID);
		}

		m_thumbList.pop_front();

	}
}

void EntChooseScreenMode::Update(float step)
{
	if (!m_thumbList.empty())
	{
		string str;
		str = "Generating thumbnails (" + CL_String::from_int(m_thumbList.size()) + " to go)";

		SetStatus(str);
		ProcessNextThumb();
		
		if (m_thumbList.empty())
		{
			SetGenericStatus();
		}

	}
}


void EntChooseScreenMode::Render(void *pTarget)
{

	CalculateMapSizes(); //should be moved later, it's so it resizes in realtime after blocks are added
    

	for (int x=0; x < g_pMapManager->GetActiveMap()->GetSizeX(); x++)
    {
        for (int y=0; y < g_pMapManager->GetActiveMap()->GetSizeY(); y++)
        {
             DrawBlock(x, y);
        }
    }
	
DrawViewScreenOverlay();
DrawPlayerPositionOverlay();

ScreenID screenID = GetScreenClicked(m_vecLastMousePos.x, m_vecLastMousePos.y);
 
 if (screenID != m_lastInfoScreenID || screenID == C_INVALID_SCREEN)
 {
	 SetupInfoPanel(screenID);
 }

}
