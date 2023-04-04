#include "AppPrecomp.h"
#include "GameLogic.h"
#include "EntEditMode.h"
#include "EntEditor.h"
#include "EntMapCache.h"
#include "EntCollisionEditor.h"
#include "HashedResource.h"
#include "TileEntity.h"
#include "MovingEntity.h"
#include "MessageManager.h"
#include "VisualProfileManager.h"
#include "EntVisualProfileEditor.h"
#include "MiscClassBindings.h"
#include "GlobalScriptFunctionBindings.h"
#include "Console.h"
#include "DataEditor.h" //we use a few of its helper functions
#include "EntCreationUtils.h"

#define C_SCALE_MOD_AMOUNT 0.01f

TileEditOperation g_EntEditModeCopyBuffer; //ctrl-c puts it here

EntEditMode::EntEditMode(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pMenu = NULL;
	m_pOriginalEditEnt = NULL;
	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntEditMode::onButtonDown);
	m_slots.connect (CL_Mouse::sig_key_up(), this, &EntEditMode::onButtonUp);
	m_slots.connect (CL_Keyboard::sig_key_down(), this, &EntEditMode::onButtonDown);
	m_slots.connect (CL_Keyboard::sig_key_up(), this, &EntEditMode::onButtonUp);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntEditMode::OnMouseMove);

	m_slots.connect(CL_Mouse::sig_key_dblclk(), this, &EntEditMode::OnMouseDoubleClick);

	m_slots.connect(GetGameLogic()->GetMyWorldManager()->sig_map_changed, this, &EntEditMode::OnMapChange);

	m_pContextMenu = NULL;

	m_pTileWeAreEdittingCollisionOn = NULL;
	m_vecDragSnap = CL_Vector2(1.0f, 1.0f);
	m_pWindow = NULL;
	m_pLabelMain = NULL;
	m_pLabelSelection = NULL;
	m_pCheckBoxSnap = NULL;
	m_pInputBoxSnapSizeX = NULL;
	m_pInputBoxSnapSizeY = NULL;
	m_pTilePicColList = NULL;
	
	m_pWindowBaseTile = NULL;
	m_pListBaseTile = NULL;

	m_dragInProgress = false;
	m_vecDragStart = m_vecScreenDragStop = CL_Vector2(0,0);
	m_pEntCollisionEditor = NULL;
	m_bDialogIsOpen = false;
	m_pPropertiesInputScript= NULL;
	m_pPropertiesListData = NULL;
	m_operation = C_OP_NOTHING;
	m_bHideSelection = false;
	m_bTilePaintModeActive = false;

	SetName("edit mode");
	Init();
}

EntEditMode::~EntEditMode()
{
	Kill();
}

void EntEditMode::RefreshActiveBrush(CL_Point mousePos)
{
	if (!m_pContextMenu || !m_pContextMenu->is_open())
	{
		//mouse is being dragged?  well, just in case.
		m_vecScreenDragStop = CL_Vector2(mousePos.x,mousePos.y);
	}
}

void EntEditMode::OnMouseMove(const CL_InputEvent &key)
{
	CL_Vector2 mouseWorldPos =g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(key.mouse_pos.x,key.mouse_pos.y));
	char stTemp[256];
	
	if (m_pCheckBoxSnap->is_checked())
	{
		CL_Vector2 vTemp = g_pMapManager->GetActiveMap()->SnapWorldCoords(mouseWorldPos, m_vecSnapSize);
		sprintf(stTemp, "Mouse Pos: X: %.1f Y: %.1f", vTemp.x, vTemp.y);

	} else
	{
		sprintf(stTemp, "Mouse Pos: X: %.1f Y: %.1f", mouseWorldPos.x, mouseWorldPos.y);

	}
	
	m_pLabelMain->set_text(stTemp);

	RefreshActiveBrush(key.mouse_pos);


	/*
	if (!CL_Display::get_current_window()->get_position()->is_inside(key.mouse_pos))
	{
		LogMsg("Lost mouse");
	}
*/

	if (m_dragInProgress)
	{
	
		
		if (CL_Keyboard::get_keycode(CL_KEY_SPACE))
		{
			
			
			CL_Vector2 oldWorldPos = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(m_vecLastMousePos.x,m_vecLastMousePos.y));
			//let's move our start by the amount they dragged with space bar down, old photoshop trick
			
			if (m_pCheckBoxSnap->is_checked())
			{
				CL_Vector2 vOffset = oldWorldPos-g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop);
				 //move by snap
				
				if (abs(vOffset.x) > 3)
				{
					if (vOffset.x > 0)
					m_vecDragStart.x -= m_vecSnapSize.x; else
					m_vecDragStart.x += m_vecSnapSize.x;
				}

				if (abs(vOffset.y) > 3)
				{
					if (vOffset.y > 0)
						m_vecDragStart.y -= m_vecSnapSize.y; else
						m_vecDragStart.y += m_vecSnapSize.y;
				}


			} else
			{
				//freeform moving
				m_vecDragStart -= (oldWorldPos-g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop));
			}
		}
	}

	m_vecLastMousePos = key.mouse_pos;
}

void EntEditMode::OnSelectSimilar()
{
	if (m_selectedTileList.IsEmpty())
	{
		LogMsg("To select similar, you must first select one or more tiles to work from.");
		return;
	}
	
	if (m_selectedTileList.m_selectedTileList.size() > 25)
	{
		if (!ConfirmMessage("Warning: Slow!","Are you sure you meant to find similar?  With this many source tiles it might be very slow." ))
		{
			return;
		}
	}
	
	TileEditOperation tempList = m_selectedTileList;
	
	//go through an select similar of each of these

	selectedTile_list::iterator itor = tempList.m_selectedTileList.begin();

	CL_Rectf r = GetCamera->GetViewRectWorld();
	
	while (itor != tempList.m_selectedTileList.end())
	{

		m_bHideSelection = false;
		int operation = TileEditOperation::C_OPERATION_ADD;
		m_selectedTileList.AddTilesByWorldRectIfSimilar(CL_Vector2(r.left,r.top), CL_Vector2(r.right,r.bottom), operation, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList(), (*itor)->m_pTile);

		itor++;
	}
	
}

void EntEditMode::OnDeleteBadTiles()
{
	tile_list tlist;

	BlitMessage("Checking for tiles with resource errors...");
	
	CL_Rectf r = GetCamera->GetViewRectWorld();
	g_pMapManager->GetActiveMapCache()->AddTilesByRect(CL_Rect(r.left, r.top, r.right, r.bottom), &tlist, g_pMapManager->GetActiveMap()->GetLayerManager().GetAllList());

	LogMsg("Found %d tiles to look through.", tlist.size());

	//delete any that have missing art

	TilePic *pTilePic;

	int tilesRemoved = 0;
	tile_list::iterator itor;
	for (itor = tlist.begin(); itor != tlist.end(); itor++)
	{
		
		if ((*itor)->GetType() == C_TILE_TYPE_PIC)
		{
			//here is a tile pic.  but does it have a problem?

			pTilePic = (TilePic*) (*itor);

			if (!GetHashedResourceManager->GetResourceByHashedID(pTilePic->m_resourceID))
			{
				//error here.  Remove this tile
				pTilePic->GetParentScreen()->RemoveTileByPointer(pTilePic);
				tilesRemoved++;
			}

		}
	}

	CL_MessageBox::info("Bad tile search results", "Removed " + CL_String::from_int(tilesRemoved) + " tiles.", GetApp()->GetGUI());
}



void EntEditMode::OnRoundOffTiles()
{
	tile_list tlist;

	BlitMessage("Rounding off tiles...");

	CL_Rectf r = GetCamera->GetViewRectWorld();
	g_pMapManager->GetActiveMapCache()->AddTilesByRect(CL_Rect(r.left, r.top, r.right, r.bottom), &tlist, g_pMapManager->GetActiveMap()->GetLayerManager().GetAllList());

	LogMsg("Found %d tiles to look through.", tlist.size());

	//delete any that have missing art

	int tilesRemoved = 0;
	tile_list::iterator itor;
	for (itor = tlist.begin(); itor != tlist.end(); itor++)
	{
		Tile *pTile = (*itor);

		CL_Vector2 vPos = pTile->GetPos();
		vPos.x =  RoundNearest(vPos.x, 1.0f);
		vPos.y =  RoundNearest(vPos.y, 1.0f);

		
		(*itor)->SetPos(vPos);

	}

}


void EntEditMode::OnReplace()
{
	if (m_selectedTileList.IsEmpty())
	{
		CL_MessageBox::info("Can't replace: Nothing selected, so nothing to replace.", GetApp()->GetGUI());
		return;
	}

	if (g_EntEditModeCopyBuffer.IsEmpty())
	{
		CL_MessageBox::info("Can't replace:  Nothing is in the copy buffer. Select the new tile, Ctrl-C, then select what you want to replace.", GetApp()->GetGUI());
		return;
	}

	//make a copy of what is selected
	TileEditOperation tempList = m_selectedTileList;

	//delete selection
	Tile blankTile; 
	AddToUndo(&m_selectedTileList);
	m_selectedTileList.FillSelection(&blankTile);

	//now create a new tile operation that will do a massive paste

	m_selectedTileList.ClearSelection();
	//look at each tile that we delete individually

	selectedTile_list::iterator itor;
	
	MovingEntity *pEnt = NULL;
	Tile *pTile = NULL;
	CL_Vector2 destPos;

	
	//check out the first tile available for pasting to get some info about it that may help us line it up better later
	Tile *pPastTile = (*g_EntEditModeCopyBuffer.m_selectedTileList.begin())->m_pTile;
	
	CL_Vector2 vPasteOffset = CL_Vector2::ZERO;

	MovingEntity *pPasteEnt = NULL;

	if (pPastTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		//entities are always centered, not upper left like tilepics.
		pPasteEnt = ((TileEntity*)pPastTile)->GetEntity();
		vPasteOffset = CL_Vector2(pPasteEnt->GetWorldRect().left, pPasteEnt->GetWorldRect().top) - pPasteEnt->GetPos();
	}
	for (itor = tempList.m_selectedTileList.begin(); itor != tempList.m_selectedTileList.end(); itor++)
	{
		g_EntEditModeCopyBuffer.SetIgnoreParallaxOnNextPaste();
		pTile = (*itor)->m_pTile;

		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			pEnt = ((TileEntity*)pTile)->GetEntity();
		} else
		{
			pEnt = NULL;
		}

		destPos = pTile->GetPos();
		OnPaste(g_EntEditModeCopyBuffer, destPos+vPasteOffset);
	}

	PushUndosIntoUndoOperation();
}

void EntEditMode::ScaleSelection(CL_Vector2 vMod, bool bBigMovement)
{
	if (m_selectedTileList.IsEmpty())
	{
		LogMsg("Can't scale, no selection.");
		return;
	}

	if (bBigMovement)
	{
		vMod.x *= vMod.x; //strengthen the effect
		vMod.y *= vMod.y; //strengthen the effect

		vMod.x *= vMod.x; //strengthen the effect
		vMod.y *= vMod.y; //strengthen the effect

		vMod.x *= vMod.x; //strengthen the effect
		vMod.y *= vMod.y; //strengthen the effect
	}
	
	TileEditOperation temp = m_selectedTileList; //copy whatever is highlighted
	OnDelete(&m_selectedTileList);
		temp.ApplyScaleMod(vMod);
		temp.SetIgnoreParallaxOnNextPaste();
	OnPaste(temp, temp.GetUpperLeftPos());
	m_selectedTileList = temp;

	PushUndosIntoUndoOperation();

}

void EntEditMode::ScaleUpSelected()
{
	ScaleSelection(CL_Vector2(1 + C_SCALE_MOD_AMOUNT, 1 + C_SCALE_MOD_AMOUNT), CL_Keyboard::get_keycode(CL_KEY_SHIFT));
}

void EntEditMode::ScaleDownSelected()
{
	ScaleSelection(CL_Vector2(1 - C_SCALE_MOD_AMOUNT, 1 -C_SCALE_MOD_AMOUNT), CL_Keyboard::get_keycode(CL_KEY_SHIFT));
}

void EntEditMode::OnHideModeChanged(bool bHide)
{
	m_pWindow->show(!bHide);
}

void EntEditMode::Init()
{
	Kill();
	
	//setup GUI
	m_pWindow = new CL_Window(CL_Rect(0, 0, 200, 150), "Tile Edit Floating Palette", CL_Window::close_button, GetApp()->GetGUI());
	
	m_pWindow->set_position(0, C_EDITOR_MAIN_MENU_BAR_HEIGHT);
	m_slotClose = m_pWindow->sig_close_button_clicked().connect_virtual(this, &EntEditMode::OnClose);

	//setup our own tip display
	m_pLabelMain = new CL_Label(CL_Point(2,52),"", m_pWindow->get_client_area());
	
	m_pLabelSelection = new CL_Label(CL_Point(2,66),"", m_pWindow->get_client_area());

	//set main dialog tips
	EntEditor *pEditor = (EntEditor*) EntityMgr->GetEntityByName("editor");
	if (!pEditor) throw CL_Error("Unable to locate editor entity");
	pEditor->SetTipLabel(
		"Tile Edit Mode - Use the mouse wheel or +/- to zoom in/out.  Drag the middle mouse button or Space+Left mouse button to pan.  Drag or click with the left mouse button to make a selection.\n" \
		"\n"\
		"Hold Shift,Alt, or Ctrl while clicking to add/remove/toggle from selection.  Use arrow keys to nudge selection one pixel.  (think photoshop controls)\n"\
		"");

	m_pWindow->set_event_passing(false);
	m_slots.connect(pEditor->sig_hide_mode_changed, this, &EntEditMode::OnHideModeChanged);

	m_pMenu = new CL_Menu(m_pWindow->get_client_area());
	CL_MenuNode *pItem;
	m_pMenu->set_event_passing(false);

	pItem = m_pMenu->create_item("Edit/Undo (Ctrl+Z)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnUndo);
	m_pMenuUndo = pItem; //remember this for later

	pItem = m_pMenu->create_item("Edit/Cut (Ctrl+X)");
	m_slots.connect(pItem->sig_clicked(),this, &EntEditMode::OnCut);

	pItem = m_pMenu->create_item("Edit/Copy (Ctrl+C)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnCopy);

	pItem = m_pMenu->create_item("Edit/Delete (DELETE)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDeleteSimple);

	pItem = m_pMenu->create_item("Edit/Deselect (Ctrl+D)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDeselect);

	pItem = m_pMenu->create_item("Edit/Hide Selection Box (Ctrl+H)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnHideSelection);


	pItem = m_pMenu->create_item("Edit/ ");
	pItem = m_pMenu->create_item("Edit/(Ctrl-V to paste selection, hold V for paint mode)");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Utilities/Import Image(s)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::BuildBaseTilePageBox);

	pItem = m_pMenu->create_item("Utilities/Import Blank Entity (puts in copy buffer)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::BuildDefaultEntity);

	
	pItem = m_pMenu->create_item("Utilities/Select similar to current selection (limited by view and active edit layers) (Ctrl-R)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnSelectSimilar);

	pItem = m_pMenu->create_item("Utilities/Replace each selected with what is in the copy buffer");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnReplace);

	pItem = m_pMenu->create_item("Utilities/Remove tiles with missing graphics (limited by view)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDeleteBadTiles);

	pItem = m_pMenu->create_item("Utilities/Round off all tile and entity positions to 1 unit (limited by view)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnRoundOffTiles);


	pItem = m_pMenu->create_item("Utilities/Create tile from tile (Ctrl-C while dragging a selection rectangle (Hold space to adjust drag position)");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Utilities/Tip: You can create a tile from the active selection size (without dragging) with Shift-Ctrl-C");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Utilities/Note: Tile from tile only works when everything is on the same bitmap");
	pItem->enable(false);

	pItem = m_pMenu->create_item("Modify Selected/Edit properties (Ctrl-E)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnProperties);

	pItem = m_pMenu->create_item("Modify Selected/Edit collision data (Toggle) (Ctrl-W)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnDefaultTileHardness);

	pItem = m_pMenu->create_item("Modify Selected/Edit Image Alignment (Ctrl-Shift-E)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnEditVisualProfile);

	pItem = m_pMenu->create_item("Modify Selected/Scale Down ([)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::ScaleDownSelected);

	pItem = m_pMenu->create_item("Modify Selected/Scale Up (])");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::ScaleUpSelected);

	pItem = m_pMenu->create_item("Modify Selected/ ");
	pItem = m_pMenu->create_item("Modify Selected/(Tip: Hold shift to scale by a larger amount)");
	pItem->enable(false);

	CL_Point offset = CL_Point(2,30);
	m_pCheckBoxSnap = new CL_CheckBox(offset, "Grid Snap - X", m_pWindow->get_client_area());
	m_slots.connect(m_pCheckBoxSnap->sig_clicked(), this, &EntEditMode::SnapCheckBoxedClicked);

	m_pCheckBoxSnap->set_focusable(false);
	//make the coordinate input boxes
	
	CL_Point tmp = offset+CL_Point(m_pCheckBoxSnap->get_width(), -2);
	CL_Rect recSize(tmp, CL_Size(40,16));
	
	if (g_pMapManager->GetActiveMap())
	{
		m_vecSnapSize.x = g_pMapManager->GetActiveMap()->GetDefaultTileSizeX();
		m_vecSnapSize.y = g_pMapManager->GetActiveMap()->GetDefaultTileSizeY();
	}

	
	m_pInputBoxSnapSizeX = new CL_InputBox(CL_String::from_int(int(m_vecSnapSize.x)), m_pWindow->get_client_area());
    m_pInputBoxSnapSizeX->set_position(recSize);
	
	//m_slots.connect(m_pInputBoxSnapSize->sig_lost_focus(), this, &EntEditMode::SnapSizeChanged);
	m_slots.connect(m_pInputBoxSnapSizeX->sig_return_pressed(), this, &EntEditMode::SnapSizeChanged);
	m_slots.connect(m_pInputBoxSnapSizeX->sig_changed(), this, &EntEditMode::OnSnapSizeChanged);

	//need another label for the Y box
	CL_Label *pLabel = new CL_Label(tmp + CL_Point(43,2), "Y", m_pWindow->get_client_area());


	m_pInputBoxSnapSizeY = new CL_InputBox(CL_String::from_int(int(m_vecSnapSize.y)), m_pWindow->get_client_area());
	recSize.apply_alignment(origin_top_left, -54, 0);
	m_pInputBoxSnapSizeY->set_position(recSize);

	m_slots.connect(m_pInputBoxSnapSizeY->sig_return_pressed(), this, &EntEditMode::SnapSizeChanged);
	m_slots.connect(m_pInputBoxSnapSizeY->sig_changed(), this, &EntEditMode::OnSnapSizeChanged);

	//m_pInputBoxSnapSize->set_position()
	offset.y += m_pCheckBoxSnap->get_height();
	
	//also add the new tile paint mode

	
	
	
	
	GetSettingsFromWorld();
	UpdateMenuStatus();
	SnapSizeChanged(); //handle graying it out if needed
}

void EntEditMode::OnSnapSizeChanged(const string &st)
{
	SnapSizeChanged();
}

void EntEditMode::BuildDefaultEntity()
{
	MovingEntity *pEnt = new MovingEntity;
	TileEntity *pTile = new TileEntity;
	pTile->SetEntity(pEnt);
	pEnt->Init();

	g_EntEditModeCopyBuffer.ClearSelection();
	g_EntEditModeCopyBuffer.AddTileToSelection(TileEditOperation::C_OPERATION_ADD,
		false, pTile);
}

void EntEditMode::OnExternalDialogClose(int entID)
{
	m_bDialogIsOpen = false;
	LogMsg("Closing external dialog");

	//fix any bad selections..
	
	m_selectedTileList.UpdateSelectionFromWorld();
}
void EntEditMode::OnEditVisualProfile()
{
	
	if (m_selectedTileList.IsEmpty())
	{
		CL_MessageBox::info("Select an entity first.", GetApp()->GetGUI());
		return;
	}

	if (m_selectedTileList.m_selectedTileList.size() > 1)
	{
		CL_MessageBox::info("Choose only ONE entity to edit its visual profile.", GetApp()->GetGUI());
		return;
	}


	Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;

	//sure, we have the copy in the selection buffer, but this is only a copy.  Let's go grab a pointer
	//to the real instance to use directly.
	
	pTile = g_pMapManager->GetActiveMap()->GetScreen(pTile->GetPos())->GetTileByPosition(pTile->GetPos(), pTile->GetLayer());

	if (!pTile)
	{
		CL_MessageBox::info("Error finding entity.", GetApp()->GetGUI());
		return;
	}
	
	/*
	if (pTile->GetType() != C_TILE_TYPE_ENTITY)
	{
		CL_MessageBox::info("This option can only be used on entities that have a visual profile attached by script.", GetApp()->GetGUI());
		return;
	}

	MovingEntity *pEnt = ((TileEntity*)pTile)->GetEntity();

	if (!pEnt->GetVisualProfile())
	{
		CL_MessageBox::info("No visual profile has been assigned to this entity yet.  This must be done in its script.", GetApp()->GetGUI());
		return;
	}

*/


	//LogMsg("Editing visual profile %s", pEnt->GetVisualProfile()->GetName().c_str());
	EntVisualProfileEditor *pEditor = (EntVisualProfileEditor*) GetMyEntityMgr->Add(new EntVisualProfileEditor);

	m_bDialogIsOpen = true; //this cripples the EntEditMode functionality so we don't
	//interfere with the new dialog we just opened

	if (!pEditor || !pEditor->Init(pTile))
	{
		LogError("Error initializing visual profile editor");
		return;
	}

	
	//However, we do want to know when it closes so we can change it back

	m_slots.connect(pEditor->sig_delete, this, &EntEditMode::OnExternalDialogClose);
}



void EntEditMode::SnapSizeChanged()
{
	EntEditor * pEnt = (EntEditor *)EntityMgr->GetEntityByName("editor");
	
	if (m_pCheckBoxSnap->is_checked())
	{
		if (pEnt && !GetGameLogic()->GetShowGrid()) pEnt->OnToggleGrid();

		m_pInputBoxSnapSizeX->enable(true);
		m_pInputBoxSnapSizeY->enable(true);
		int snapX = CL_String::to_int(m_pInputBoxSnapSizeX->get_text());
		int snapY = CL_String::to_int(m_pInputBoxSnapSizeY->get_text());

		if (snapX > 0)
		{
			snapX = max(1, snapX);
			snapX = min(snapX, 4048);
			g_pMapManager->GetActiveMap()->SetDefaultTileSizeX(snapX);
			m_pInputBoxSnapSizeX->set_text(CL_String::from_int(snapX));
		}

		if (snapY > 0)
		{
			snapY = max(1, snapY);
			snapY = min(snapY, 4048);
			g_pMapManager->GetActiveMap()->SetDefaultTileSizeY(snapY);
			m_pInputBoxSnapSizeY->set_text(CL_String::from_int(snapY));
		}

		g_pMapManager->GetActiveMap()->SetSnapEnabled(true);
	} else
	{
	
		if (pEnt && GetGameLogic()->GetShowGrid()) pEnt->OnToggleGrid();
		
		m_pInputBoxSnapSizeX->enable(false);
		m_pInputBoxSnapSizeY->enable(false);
		g_pMapManager->GetActiveMap()->SetSnapEnabled(false);
	}

	m_vecSnapSize.x = g_pMapManager->GetActiveMap()->GetDefaultTileSizeX();
	m_vecSnapSize.y = g_pMapManager->GetActiveMap()->GetDefaultTileSizeY();

	if (m_pWindowBaseTile && m_pWindowBaseTile->is_enabled(false))
	{
		OnSelectBaseTilePage();
	}
}

void EntEditMode::SnapCheckBoxedClicked()
{
	SnapSizeChanged();
}

void EntEditMode::Kill()
{
	

	SetOperation(C_OP_NOTHING);

	if (m_pEntCollisionEditor)
	{
		//close this thing too
		m_pEntCollisionEditor->SetDeleteFlag(true);
		m_pEntCollisionEditor = NULL;
	}
	
	SAFE_DELETE(m_pContextMenu);
	SAFE_DELETE(m_pCheckBoxSnap);
	SAFE_DELETE(m_pInputBoxSnapSizeX);
	SAFE_DELETE(m_pInputBoxSnapSizeY);
	SAFE_DELETE(m_pListBaseTile);
	SAFE_DELETE(m_pWindowBaseTile);
	SAFE_DELETE(m_pMenu);
	SAFE_DELETE(m_pWindow);
}

void EntEditMode::onButtonDownTilePicInfo(const CL_InputEvent &key)
{
	switch (key.id)
	{
		case CL_KEY_DELETE:
			
			int item = m_pTilePicColList->get_current_item();

			if (item == -1)
			{
				LogMsg("Select a collision data piece first.");
			} else
			{
				CL_ListItem *pItem = m_pTilePicColList->get_item(item);
				CL_Rect r = StringToRect(pItem->str);
				
				m_pResInfo->DeleteCollisionDataByRect(r);
				m_pTilePicColList->remove_item(item);
			}
			
			break;
	}
}


void EntEditMode::PopUpImagePropertiesDialog(const string &fileName, unsigned int resourceID)
{
	HashedResource *pRes = GetHashedResourceManager->GetResourceClassByHashedID(resourceID);
	CL_Color col = pRes->GetColorKey();
	m_pResInfo = pRes;
	m_bDialogIsOpen = true;
	// Creating InputDialog
	string stTitle(CL_String::format("%1 (hashID: " + CL_String::from_int(resourceID) +") Properties", fileName));

	CL_Rect dialogRect = CL_Rect(0,0,300,340);
	CL_Point dialogPos = CL_Point((GetScreenX/2)-dialogRect.get_width()/2, 310);
	CL_Window *pWindow = new CL_Window(dialogRect+dialogPos, stTitle, GetApp()->GetGUI());
	pWindow->set_event_passing(false);

	CL_Point pos = CL_Point(5, 5);
	CL_CheckBox &cbColorKey = *new CL_CheckBox(pos, "Uses colorkey (transparent color) ", pWindow->get_client_area());
	cbColorKey.set_checked(pRes->HasColorKey());
	CL_SlotContainer slots;

	pos.y +=20;
	CL_Label &colorLabel = *new CL_Label(pos+ CL_Point(30,0), "RGB value of colorkey: ",pWindow->get_client_area());

	CL_Rect inputRect(colorLabel.get_position().right+6, colorLabel.get_position().top,0,0);
	//now figure the height and depth
	inputRect.right = inputRect.left + 130;
	inputRect.bottom = inputRect.top + 16;

	CL_InputBox *pColorInput = new CL_InputBox(inputRect, ColorToString(pRes->GetColorKey()), pWindow->get_client_area());

	pos.y +=25;

	//label

	CL_Label &listLabel = *new CL_Label(pos+ CL_Point(0,0), "Shared collision data in image's .dat file (Del to delete)",pWindow->get_client_area());

	pos.y += 25;
	int listHeight = 200;
	
	CL_Rect r(5, pos.y, dialogRect.right-10, pos.y+listHeight);

	m_pTilePicColList = new CL_ListBox (r, pWindow->get_client_area());

	pos.y += listHeight+10; //spacer too

	pos.x += 120;
	CL_Button *pButtonOk = new CL_Button (CL_Rect(pos.x, pos.y, pos.x + 50, pos.y + 16), "Ok", pWindow->get_client_area());

	slots.connect(pButtonOk->sig_clicked(), (CL_Component*)pWindow, &CL_Component::quit);

	slots.connect (CL_Keyboard::sig_key_down(), this, &EntEditMode::onButtonDownTilePicInfo);

	// Connecting signals, to allow only numbers
	//slots.connect(pScrollX->sig_validate_character(), this, &EntEditor::validator_numbers);
	
	pRes->PopulateListBoxWithCollisionData(m_pTilePicColList);

	// Run dialog
	pWindow->run();
	m_bDialogIsOpen = false;
	pRes->SetHasColorKey(cbColorKey.is_checked(), StringToColor(pColorInput->get_text()));

	delete pButtonOk;
	delete pColorInput;
	SAFE_DELETE(m_pTilePicColList);
	delete pWindow;

}

void EntEditMode::OnSelectBaseTileDoubleClick(const CL_InputEvent &input)
{
	int index = m_pListBaseTile->get_item(input.mouse_pos);
	if (index == -1) return;

	PopUpImagePropertiesDialog(m_pListBaseTile->get_current_text(), FileNameToID(m_pListBaseTile->get_current_text().c_str()));
}

void EntEditMode::OnSelectBaseTilePage()
{
	int resourceID = FileNameToID(m_pListBaseTile->get_current_text().c_str());

	//break apart this pic into chunks
	CL_Vector2 vecSnap = CL_Vector2(0,0);
	if (m_pCheckBoxSnap->is_checked()) vecSnap = m_vecSnapSize;
	GetHashedResourceManager->PutGraphicIntoTileBuffer(resourceID, g_EntEditModeCopyBuffer, vecSnap);
}

void EntEditMode::OnCloseBaseTileDialog()
{
	if (m_pWindowBaseTile)
	{
		m_pWindowBaseTile->enable(false);
		//m_pWindowBaseTile->se
	}
}
void EntEditMode::BuildBaseTilePageBox()
{
	SAFE_DELETE(m_pListBaseTile);
	SAFE_DELETE(m_pWindowBaseTile);
	int textAreaHeight = 130;


	int width = 230;
	int height =  (GetHashedResourceManager->GetHashedResourceMap()->size()*12) +40 + textAreaHeight;
	height = min(height, GetScreenY-100);

	CL_Rect rectSize = CL_Rect((GetScreenX/2) -20, C_EDITOR_MAIN_MENU_BAR_HEIGHT, 0, 0);
	rectSize.set_size(CL_Size(width,height));

	m_pWindowBaseTile = new CL_Window(rectSize, "Import Image" , CL_Window::close_button, GetApp()->GetGUI()->get_client_area());
	m_pWindowBaseTile->set_event_passing(false);

	CL_Label *pLabel = new CL_Label(CL_Point(5,5), 
"Left click on a filename to put it in the\n" \
"copy buffer.  Then right click on the map\n"\
"to paste it."\
"\n\nIf GRID-SNAP is on, the image will be\n"\
"be split up when pasted.\n\n"\
"Double-click an image to change its global\n"\
"properties.",
		m_pWindowBaseTile->get_client_area());

	CL_Rect r = m_pWindowBaseTile->get_children_rect();
	r.set_size(r.get_size() - CL_Size(5,28));
	r.top = textAreaHeight;
	m_pListBaseTile = new CL_ListBox(r, m_pWindowBaseTile->get_client_area());

	//add all maps

	HashedResourceMap * pMap = GetHashedResourceManager->GetHashedResourceMap();
	HashedResourceMap::iterator itor = pMap->begin();

	while(itor != pMap->end())
	{
		m_pListBaseTile->insert_item(GetStrippedFilename((*itor).second->m_strFilename));
		itor++;
	}
	m_pListBaseTile->sort();

	m_slotSelectedBaseTilePage = m_pListBaseTile->sig_selection_changed().connect(this, &EntEditMode::OnSelectBaseTilePage);
	m_slots.connect( m_pListBaseTile->sig_mouse_dblclk(), this, &EntEditMode::OnSelectBaseTileDoubleClick);
	m_slots.connect( m_pWindowBaseTile->sig_close(), this, &EntEditMode::OnCloseBaseTileDialog);

}

//helps us know when it's a bad time to process hotkeys
bool EntEditMode::IsDialogOpen(bool bCheckModelessDialogToo, bool bEvasive)
{
	EntEditor *pEditor = (EntEditor*) EntityMgr->GetEntityByName("editor");
	if (pEditor)
	{
		if (pEditor->IsDialogOpen()) return true;
	}
	
	if (bCheckModelessDialogToo)
	{
		if (m_pInputBoxSnapSizeX->has_focus()) return true;
		if (m_pInputBoxSnapSizeY->has_focus()) return true;
	}

	
	EntVisualProfileEditor *pVisEditor = (EntVisualProfileEditor*) EntityMgr->GetEntityByName("visualeditor");
	if (pVisEditor)
	{
		if (bEvasive) return true; //allow this to happen most of the time
	
		return pVisEditor->IsDialogOpen();
	}

	//the secondary check is done for script based hot keys and mouse wheels.. but in general, even though a dialog
	//is open, we still want to be able to zoom, so ..
	
	if (m_bDialogIsOpen) return true;
	return false;
}

void EntEditMode::ClearSelection()
{
	m_selectedTileList.ClearSelection();
}

void EntEditMode::OnClose(CL_SlotParent_v0 &parent_handler)
{
	SetDeleteFlag(true);
}

void EntEditMode::onButtonUp(const CL_InputEvent &key)
{
	if (m_pEntCollisionEditor) 
	{
	  switch (key.id)
		{
		case CL_KEY_W:
			m_pEntCollisionEditor->OnOk();
			break;
		}

		return; //now is a bad time for most stuff
	}

	if (IsDialogOpen(false, true)) 
	{
		//we must be doing something in a dialog, don't want to delete entities accidentally	
		return;
	}

	switch(key.id)
	{
		
	case CL_KEY_W:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnDefaultTileHardness();
		}
		break;

	case CL_KEY_V:

			m_bTilePaintModeActive = false;
			PushUndosIntoUndoOperation();

		break;
	
	case CL_MOUSE_LEFT:
		{
			KillContextMenu();
	
			if (m_operation == C_OP_MOVE)
			{

				
				if ( (g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop)- m_vecDragStart).length() < 1)
				{
					
					m_operation = C_OP_NOTHING; //don't want the undo to cache a meaningless copy
					//they clicked one thing, without really dragging.  Treat it special
					ClearSelection();
					m_bHideSelection = false;			
					m_selectedTileList.AddTileByPoint(m_vecDragStart, TileEditOperation::C_OPERATION_ADD, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList());
				} else
				{
					SetOperation(C_OP_NOTHING); //this handles the paste and undo buffer for us

				}
			}
			
			//highlight selected tile(s)
			
			if (!m_dragInProgress) break; //not a real drag was happening

			//scan every tile within this range
			m_vecScreenDragStop =  g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
			
			if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL) &&
				!CL_Keyboard::get_keycode(CL_KEY_SHIFT) &&
#ifdef __APPLE__
				!CL_Keyboard::get_keycode(CL_KEY_COMMAND) &&
#endif

				!CL_Keyboard::get_keycode(CL_KEY_MENU)
				)
			{
				m_bHideSelection = false;			

				ClearSelection();
			}
			m_dragInProgress = false;
			
			int operation = TileEditOperation::C_OPERATION_TOGGLE;
			if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
			{
				operation = TileEditOperation::C_OPERATION_ADD;
			}
			if (CL_Keyboard::get_keycode(CL_KEY_MENU))
			{
				operation = TileEditOperation::C_OPERATION_SUBTRACT;
			}

			if ( (m_vecScreenDragStop- m_vecDragStart).length() < 2)
			{
				m_bHideSelection = false;			
				//they clicked one thing, without really dragging.  Treat it special
				m_selectedTileList.AddTileByPoint(m_vecDragStart, operation, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList());
			} else
			{
				
				if (m_pCheckBoxSnap->is_checked())
				{
					
					//these tweaks allow the snapped grid to be inclusive of the tile we're half on
					if (m_vecScreenDragStop.x > m_vecDragStart.x) m_vecScreenDragStop.x += m_vecSnapSize.x; else m_vecDragStart.x += m_vecSnapSize.x;
					if (m_vecScreenDragStop.y > m_vecDragStart.y) m_vecScreenDragStop.y += m_vecSnapSize.y; else m_vecDragStart.y += m_vecSnapSize.y;

					
					m_vecDragStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(m_vecDragStart, m_vecSnapSize);
					m_vecScreenDragStop = g_pMapManager->GetActiveMap()->SnapWorldCoords(m_vecScreenDragStop, m_vecSnapSize);
				}

				m_bHideSelection = false;			
				m_selectedTileList.AddTilesByWorldRect(m_vecDragStart, m_vecScreenDragStop, operation, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList());

		
			}
			RefreshActiveBrush(key.mouse_pos);

		}

		break;
	}
}

void EntEditMode::SelectByLayer(const vector<unsigned int> &layerVec)
{
	m_selectedTileList.ClearSelection();
	int operation = TileEditOperation::C_OPERATION_ADD;

	CL_Rectf r = GetCamera->GetViewRectWorld();
	m_bHideSelection = false;
	m_selectedTileList.AddTilesByWorldRect(CL_Vector2(r.left,r.top), CL_Vector2(r.right,r.bottom), operation, layerVec);

}
void EntEditMode::OnCopy()
{
	//copy whatever is highlighted
	g_EntEditModeCopyBuffer = m_selectedTileList;
}

void EntEditMode::OnPaste(TileEditOperation &editOperation, CL_Vector2 vecWorld, bool bSelectPasted )
{
   //OPTIMIZE can avoid a copy operation of the undo buffer
	
	if (editOperation.IsEmpty()) return;
	TileEditOperation undo;
	editOperation.PasteToWorld(vecWorld, TileEditOperation::PASTE_UPPER_LEFT, &undo);
	
	//change selection to what we just pasted
	
	if (bSelectPasted )
	{
		m_selectedTileList = editOperation;
		m_selectedTileList.ApplyOffset(vecWorld-m_selectedTileList.GetUpperLeftPos());
	}
	
	AddToUndo(&undo);

}

void EntEditMode::UpdateMenuStatus()
{
	//grey out undo button if it has no data
	if (g_pMapManager->GetActiveMapCache())
	m_pMenuUndo->enable(g_pMapManager->GetActiveMapCache()->GetUndoOpsAvailableCount() != 0);
}
void EntEditMode::AddToUndo( TileEditOperation *pTileOperation)
{
	if (pTileOperation->IsEmpty()) return;

	TileEditOperation to;
	m_undo.push_front(to);
	m_undo.front() = *pTileOperation;
	UpdateMenuStatus();
}

void EntEditMode::PushUndosIntoUndoOperation()
{
	if (m_undo.empty()) return;

	g_pMapManager->GetActiveMapCache()->PushUndoOperation(m_undo);
	m_undo.clear();

	UpdateMenuStatus();
}

void EntEditMode::OnUndo()
{
	KillContextMenu();

	//LogMsg("Undo size is %u", m_undo.size());
	g_pMapManager->GetActiveMapCache()->PopUndoOperation();
	UpdateMenuStatus();
	ClearSelection();
}

void EntEditMode::OnDelete(TileEditOperation *pTileOperation)
{
	
	KillContextMenu();

	if (pTileOperation->IsEmpty()) return;
	//delete selection
	Tile blankTile; 
	AddToUndo(pTileOperation);
	pTileOperation->FillSelection(&blankTile);
	ClearSelection();
}

void EntEditMode::OnCut()
{
	KillContextMenu();

	OnCopy();
	OnDeleteSimple();
	PushUndosIntoUndoOperation();

}

void EntEditMode::CutSubTile(CL_Rect recCut)
{
	LogMsg("Cutting %d, %d, %d, %d (%d, %d)", recCut.left, recCut.top, recCut.right, recCut.bottom,
		recCut.get_width(), recCut.get_height());

	//which bitmap are we trying to cut from?
	tile_list tList;
	g_pMapManager->GetActiveMapCache()->AddTilesByRect(recCut, &tList, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList());
	if (tList.size() == 0)
	{
		CL_MessageBox::info("SubTile Create failed, you need to do it over an existing tile on this layer.", GetApp()->GetGUI());
		return;
	}

	//guess at the most suitable one
	tile_list::iterator itor = tList.begin();
	Tile *pTile = *tList.begin(); //default
	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();

	while (itor != tList.end())
	{
		
		if (pTile->GetType() != C_TILE_TYPE_PIC)
		{
			//well, this can't be right
			pTile = (*itor);
		}

		int newTileSort = layerMan.GetLayerInfo( (*itor)->GetLayer()).GetSort();
		int chosenTileSort = layerMan.GetLayerInfo( pTile->GetLayer()).GetSort();
		if ( chosenTileSort < newTileSort)
		{
			//this might be a better candidate
			pTile = (*itor);
		} else
		{
			if (newTileSort == chosenTileSort)
			{
				//anybodys guess

				if ( (*itor)->GetBoundsSize().length() > pTile->GetBoundsSize().length())
				{
					//this is bigger, it's probably what they wanted
					pTile = (*itor);
				}
				

			}
		}
			
			
		itor++;
	}

	if (pTile->GetType() != C_TILE_TYPE_PIC)
	{
		CL_MessageBox::info("Can't make subtiles out of entities, only with normal tile pics.", GetApp()->GetGUI());
		return;
	}
	
	if (pTile->UsesTileProperties())
	{
		CL_MessageBox::info("Can only make subtiles out of tile pics that are not scaled or flipped.  Set scale to 1 1 first.", GetApp()->GetGUI());
		return;
	}
	
	//now that we have it, we need to create a new subtile from it, if possible
	if (
		(pTile->GetWorldRect().calc_union(recCut).get_width() != recCut.get_width())
		|| (pTile->GetWorldRect().calc_union(recCut).get_height() != recCut.get_height())

		)
	{
		LogMsg("Warning: Subtile selection too big for tile it looks like...");
	}

	CL_Rect localRect = recCut;
	localRect.apply_alignment(origin_top_left, pTile->GetPos().x, pTile->GetPos().y);

	LogMsg("We're going to cut the new tile at %d,%d,%d,%d", localRect.left, localRect.top, localRect.right, localRect.bottom);

	GetHashedResourceManager->PutSubGraphicIntoTileBuffer((TilePic*)pTile, g_EntEditModeCopyBuffer, localRect);
	m_dragInProgress = false;
}

void EntEditMode::OffsetSelectedItems(CL_Vector2 vOffset, bool bBigMovement)
{
	if (m_selectedTileList.IsEmpty())
	{
		LogMsg("Can't nudge, no selection.");
		return;
	}

	if (bBigMovement) vOffset *= 10;
	TileEditOperation temp = m_selectedTileList; //copy whatever is highlighted
	OnDeleteSimple();
	temp.ApplyOffset(vOffset);
	temp.SetIgnoreParallaxOnNextPaste();
	OnPaste(temp, temp.GetUpperLeftPos());
	m_selectedTileList = temp;

	PushUndosIntoUndoOperation();
}

std::string EntEditMode::HandleMessageString( const string &msg )
{
	vector<string> words = CL_String::tokenize(msg, "|");
	
	if (words[0] == "offset_selected")
	{
		assert(words.size() == 4);
		OffsetSelectedItems(CL_Vector2(CL_String::to_float(words[1]),CL_String::to_float(words[2])), 
			CL_String::to_bool(words[3]) );

	} else
		if (words[0] == "open_properties_on_selected")
		{
			BuildTilePropertiesMenu(&m_selectedTileList);

		} else
	{
		LogMsg("Don't know how to handle message %s", msg.c_str());
	}

		return "";
}


bool EntEditMode::MouseIsOverSelection(CL_Point ptMouse)
{
	if (m_selectedTileList.m_selectedTileList.empty()) return false; //can't be, nothing is there

	//get the mouse coords in world coordindates
	CL_Vector2 mousePos = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(ptMouse.x, ptMouse.y));

	Tile *pSelTile;
	if (pSelTile = m_selectedTileList.GetTileAtPos(mousePos))
	{
		//the mouse cursor is over a selected tile	
		Tile *pWorldSelTile = GetTileByWorldPos(g_pMapManager->GetActiveMap(),mousePos, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList(), false);

	
		if (pWorldSelTile)
		{
			return TilesAreSimilar(pWorldSelTile, pSelTile, true);
		}

	}

	return false;

}

void EntEditMode::SetOperation(int op)
{

	if (op == C_OP_NOTHING && m_operation == C_OP_MOVE && g_pMapManager->GetActiveMapCache())
	{
		//Delete the old ones
		Tile blankTile; 
		AddToUndo(&m_selectedTileList);
		m_selectedTileList.FillSelection(&blankTile);

		//paste the new ones
		CL_Vector2 worldDragStop = g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop);

		CL_Vector2 vecStart(worldDragStop.x -m_vecDragStart.x, worldDragStop.y - m_vecDragStart.y);
		vecStart += m_selectedTileList.GetUpperLeftPos();

		if (g_pMapManager->GetActiveMap()->GetSnapEnabled())
		{
			
			if (m_selectedTileList.m_selectedTileList.size() == 1)
				vecStart = worldDragStop; //special case when in snap mode
			
			vecStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecStart, m_vecSnapSize);

		} else
		{
			vecStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecStart, m_vecDragSnap);
		}

		TileEditOperation undo;
		m_selectedTileList.PasteToWorld(vecStart, TileEditOperation::PASTE_UPPER_LEFT, &undo);

		//change selection to what we just pasted

		m_selectedTileList.ApplyOffset(vecStart-m_selectedTileList.GetUpperLeftPos());
	
		AddToUndo(&undo);
		PushUndosIntoUndoOperation();

	}

	m_operation = op;
	//LogMsg("Setting op to %d", op);
	
	switch(op)
	{
		case C_OP_MOVE:
		//we'll want to draw our selection as we move it, without really pasting it

		break;
	}
}

void EntEditMode::OnMouseDoubleClick(const CL_InputEvent &key)
{
	switch(key.id)
	{
	
	case CL_MOUSE_LEFT:
			
			m_operation = C_OP_NOTHING;
			//SetOperation(C_OP_NOTHING);
			
			//they double clicked?
			if (!CL_Keyboard::get_keycode(CL_KEY_SHIFT) && !CL_Keyboard::get_keycode(CL_KEY_MENU))
			{
				if (!m_selectedTileList.IsEmpty())
				{
					if (MouseIsOverSelection(key.mouse_pos))
					{
						if (m_pEntCollisionEditor)
						{
							//now is a bad time..
							LogMsg("Close collision editor first.");
							return;
						}
						BuildTilePropertiesMenu(&m_selectedTileList);
						return;
					}
				}
			}
			break;
	}
}

void EntEditMode::OnPasteContext()
{
	CL_Vector2 vecWorld = ConvertMouseToCenteredSelectionUpLeft(m_vecScreenDragStop);
	OnPaste(g_EntEditModeCopyBuffer, vecWorld, true);
	PushUndosIntoUndoOperation();
	KillContextMenu();
}

void EntEditMode::KillContextMenu()
{
	if (m_pContextMenu)
	m_pContextMenu->collapse();
}

void EntEditMode::OpenContextMenu(CL_Vector2 clickPos)
{
	CL_Size rsize = CL_Size(50,20);
	CL_Rect r = CL_Rect(clickPos.x, clickPos.y, clickPos.x + rsize.width, clickPos.y + rsize.height);

	//we might need to adjust this so it's actually on the screen

	CL_Rect screenRect = CL_Rect(0,0,GetScreenX, GetScreenY);

	SAFE_DELETE(m_pContextMenu);
	m_lastContextWorldPos = g_pMapManager->GetActiveMapCache()->ScreenToWorld(clickPos);

	//check to see if we can "grab" the current selection and move it
	if (m_selectedTileList.IsEmpty())
	{
		//why not just select whatever is here?
		m_selectedTileList.AddTileByPoint(m_lastContextWorldPos, TileEditOperation::C_OPERATION_ADD, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList());
	}

	m_pContextMenu = new CL_Menu(r, GetApp()->GetGUI());
	CL_MenuNode *pItem;
	m_pContextMenu->set_event_passing(false);
	m_pContextMenu->set_vertical(true);
	m_pContextMenu->set_auto_resize(true);
	
	pItem = m_pContextMenu->create_item("Cut (Ctrl+X)");
	m_slots.connect(pItem->sig_clicked(),this, &EntEditMode::OnCut);
	pItem->set_close_on_click(true);
	pItem->enable(!m_selectedTileList.IsEmpty());

	pItem = m_pContextMenu->create_item("Copy (Ctrl+C)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnCopy);
	pItem->set_close_on_click(true);
	pItem->enable(!m_selectedTileList.IsEmpty());

	pItem = m_pContextMenu->create_item("Paste (Ctrl-V)");
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnPasteContext);
	pItem->set_close_on_click(true);

	pItem->enable(!g_EntEditModeCopyBuffer.IsEmpty());
	pItem = m_pContextMenu->create_item("Properties (Ctrl-E)");
	pItem->enable(!m_selectedTileList.IsEmpty());
	m_slots.connect(pItem->sig_clicked(), this, &EntEditMode::OnProperties);
	
	m_pContextMenu->set_click_to_open(false);
	//m_pContextMenu->set_root_collapsing(true);
	
	m_pContextMenu->open();
	m_pContextMenu->paint();

	r = m_pContextMenu->get_position_with_children();
	
	CL_Rect rectUnion = screenRect.calc_union(r);

	if (rectUnion != r)
	{
		//it needs to be adjusted to not hang off the screen

		int offsetx = r.get_width() - rectUnion.get_width();
		int offsety = r.get_height() - rectUnion.get_height();
		r.apply_alignment(origin_top_left, offsetx, 0);

		if (offsety != 0)
		{
			//we're low on the screen, let's move the whole menu above us
			r.top = clickPos.y - (r.get_height() + 2);
		}

		m_pContextMenu->set_position(r.left, r.top);
	}
}

void EntEditMode::OnDeselect()
{
	m_selectedTileList.ClearSelection();
}

void EntEditMode::OnHideSelection()
{
	m_bHideSelection = !m_bHideSelection;

	if (m_bHideSelection)
	{
		LogMsg("Selection box hidden.");
	} else
	{
		LogMsg("Selection box un-hidden.");
	}
}

void EntEditMode::FloodFill(CL_Rect r)
{
	
	if (g_EntEditModeCopyBuffer.IsEmpty())
	{
		CL_MessageBox::info("Can't flood fill, you need to select something and hit Ctrl-C to put it into the buffer first.", GetApp()->GetGUI());
		return;
	}

	CL_Vector2 vOffset;

	if (m_pCheckBoxSnap->is_checked()) 
	{
		vOffset = m_vecSnapSize;
	} else
	{
		//use the whole size..
		vOffset = g_EntEditModeCopyBuffer.GetSelectionSizeInWorldUnits();
	}

	if (vOffset.x == 0 || vOffset.y == 0)
	{
		CL_MessageBox::info("Can't flood fill - invalid spacing.  If grid snap is on, make sure one isn't set to 0.", GetApp()->GetGUI());
		return;
	}

	if (g_EntEditModeCopyBuffer.m_selectedTileList.size() > 10)
	{
		if (!ConfirmMessage("Flood Fill With Multiple Tiles", "This may take a long time, are you sure?")) return;
	}

	LogMsg("Flood filling %s using spacing of %s.",PrintRectInt(r).c_str(), PrintVector(vOffset).c_str());

	CL_Vector2 vPos = CL_Vector2(r.left, r.top);

	for (; vPos.y < r.bottom;)
	{
		for (; vPos.x < r.right;)
		{
			//paste it here
			g_EntEditModeCopyBuffer.SetIgnoreParallaxOnNextPaste();
			OnPaste(g_EntEditModeCopyBuffer, vPos);
			vPos.x += vOffset.x;
		}

		vPos.x = r.left;
		vPos.y += vOffset.y;
	}

	PushUndosIntoUndoOperation();

	
}

void EntEditMode::TurnOffFocus()
{
	if (m_pInputBoxSnapSizeX->has_focus() || m_pInputBoxSnapSizeY->has_focus())
	{
		GetApp()->GetGUI()->set_focus(GetApp()->GetGUI());
	}
}

void EntEditMode::onButtonDown(const CL_InputEvent &key)
{
	if (m_pEntCollisionEditor) return; //now is a bad time

	if (IsDialogOpen(false, true)) 
	{
		//we must be doing something in a dialog, don't want to delete entities accidently	
		return;
	}
	
	switch(key.id)
	{

	case CL_MOUSE_LEFT:

		if (!g_pMapManager->GetActiveMap()) 
		{
			LogMsg("Error, no map is active?!");
			return;
		}

		if (CL_Keyboard::get_keycode(CL_KEY_SPACE))
		{
			//they probably want to move the map, let EntEditor handle it
			return;
		}

		if (!m_dragInProgress)
		{
			//do want to to drag out a selection box or move something?
			TurnOffFocus();


			m_vecDragStart = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
			m_vecScreenDragStop = CL_Vector2(key.mouse_pos.x, key.mouse_pos.y);

			if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL) && !CL_Keyboard::get_keycode(CL_KEY_SHIFT) && !CL_Keyboard::get_keycode(CL_KEY_MENU)

#ifdef __APPLE__
				&& !CL_Keyboard::get_keycode(CL_KEY_COMMAND)
#endif
				)
			{

				//check to see if we can "grab" the current selection and move it
				if (MouseIsOverSelection(key.mouse_pos))
				{

					//initiate a tile move operation
					SetOperation(C_OP_MOVE);
					return;
				} else
				{
					if (GetTileByWorldPos(g_pMapManager->GetActiveMap(),m_vecDragStart, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList(), true))
					{
						//well, heck, why not let them just drag whatever tile they are over?
						ClearSelection();
						m_selectedTileList.AddTileByPoint(m_vecDragStart, TileEditOperation::C_OPERATION_ADD, g_pMapManager->GetActiveMap()->GetLayerManager().GetEditActiveList());
						SetOperation(C_OP_MOVE);
						return;
					}
				}
			}

			m_dragInProgress = true;
		}
		break;

	case CL_MOUSE_RIGHT:
		{
			CL_Vector2 vecMouseClickPos = CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y());

			OpenContextMenu(vecMouseClickPos);
			/*

			//if we wanted right click to just paste..

			CL_Vector2 vecMouseClickPos = CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y());
			CL_Vector2 vecWorld = ConvertMouseToCenteredSelectionUpLeft(vecMouseClickPos);

			OnPaste(g_EntEditModeCopyBuffer, vecWorld, true);
			PushUndosIntoUndoOperation();
			*/
		}

		break;
	}


	if (g_Console.IsActive())
	{
		if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL)) return;
		if (key.id == CL_KEY_C || key.id == CL_KEY_V|| key.id == CL_KEY_X) return;
	}

	switch(key.id)
	{
	case CL_KEY_LEFT_BRACKET:
		ScaleDownSelected();
		break;
	
	case CL_KEY_RIGHT_BRACKET:
		ScaleUpSelected();
		break;

	case CL_KEY_D:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnDeselect();
		}
		break;

	case CL_KEY_H:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			OnHideSelection();
		}
		break;

	case CL_KEY_F:
		if (m_dragInProgress)
		{
			CL_Vector2 dragStart = m_vecDragStart; //remember this so we can put it back after doing weird snaps on it
			CL_Vector2 worldVecDragStop = g_pMapManager->GetActiveMap()->SnapWorldCoords(g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop), m_vecDragSnap);
			
				if (m_pCheckBoxSnap->is_checked())
				{
				
					//these tweaks allow the snapped grid to be inclusive of the tile we're half on
					if (worldVecDragStop.x > m_vecDragStart.x) worldVecDragStop.x += m_vecSnapSize.x; else m_vecDragStart.x += m_vecSnapSize.x;
					if (worldVecDragStop.y > m_vecDragStart.y) worldVecDragStop.y += m_vecSnapSize.y; else m_vecDragStart.y += m_vecSnapSize.y;
					
					worldVecDragStop = g_pMapManager->GetActiveMap()->SnapWorldCoords(worldVecDragStop, m_vecSnapSize);
					m_vecDragStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(m_vecDragStart, m_vecSnapSize);;
				}

				CL_Rect rec( int(m_vecDragStart.x), int(m_vecDragStart.y), int(worldVecDragStop.x), int(worldVecDragStop.y));
			rec.normalize();

			FloodFill(rec);
			m_vecDragStart = dragStart;
		}

		break;

	//nudging
	case CL_KEY_RIGHT:
		
		ScheduleSystem(1, ID(), ("offset_selected|1|0|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_KEY_LEFT:
		ScheduleSystem(1, ID(), ("offset_selected|-1|0|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_KEY_UP:
		ScheduleSystem(1, ID(), ("offset_selected|0|-1|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;

	case CL_KEY_DOWN:
		ScheduleSystem(1, ID(), ("offset_selected|0|1|" + CL_String::from_int(CL_Keyboard::get_keycode(CL_KEY_SHIFT))).c_str());
		break;
	
	case CL_KEY_R:

		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL) && !CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{
			OnSelectSimilar();
		}
		break;

	case CL_KEY_C:

		if (IsControlKeyDown())
		{
			//two modes of copy, one to cut out subtiles..
			if (m_dragInProgress)
			{
				m_vecDragStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(m_vecDragStart, m_vecDragSnap);
				CL_Vector2 worldVecDragStop = g_pMapManager->GetActiveMap()->SnapWorldCoords(g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop), m_vecDragSnap);
				
				if (m_pCheckBoxSnap->is_checked())
				{
					
					//these tweaks allow the snapped grid to be inclusive of the tile we're half on
					if (worldVecDragStop.x > m_vecDragStart.x) worldVecDragStop.x += m_vecSnapSize.x; else m_vecDragStart.x += m_vecSnapSize.x;
					if (worldVecDragStop.y > m_vecDragStart.y) worldVecDragStop.y += m_vecSnapSize.y; else m_vecDragStart.y += m_vecSnapSize.y;

					m_vecDragStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(m_vecDragStart, m_vecSnapSize);
					worldVecDragStop = g_pMapManager->GetActiveMap()->SnapWorldCoords(worldVecDragStop, m_vecSnapSize);
				}
		
				CL_Rect rec( int(m_vecDragStart.x), int(m_vecDragStart.y), int(worldVecDragStop.x), int(worldVecDragStop.y));
				rec.normalize();
				CutSubTile(rec);
			} else if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
			{
				
				if (m_selectedTileList.IsEmpty())
				{
					CL_MessageBox::info("Shift-Ctrl-C makes a subtile from the current selection size.  But you don't have a selection!", GetApp()->GetGUI());

					return;
				}
				//cut subtile from current selection, if possible..
				 CL_Rect rec(int(m_selectedTileList.GetUpperLeftPos().x), int(m_selectedTileList.GetUpperLeftPos().y),
					 int(m_selectedTileList.GetLowerRightPos().x), int(m_selectedTileList.GetLowerRightPos().y));
				rec.normalize();
				CutSubTile(rec);
			} else
			{
				OnCopy();
			}
		}

		break;

	case CL_KEY_X:
		if (!IsControlKeyDown()) return;
		OnCut();
		break;

    case CL_KEY_DELETE:
			OnDeleteSimple();
			PushUndosIntoUndoOperation();

		break;

	case CL_KEY_E:
		if (!CL_Keyboard::get_keycode(CL_KEY_CONTROL)) return;
		
		if (!CL_Keyboard::get_keycode(CL_KEY_SHIFT))
		{
			OnProperties();
		} else
		{
			OnEditVisualProfile();
		}

		break;

	case CL_KEY_Z:
		if (!IsControlKeyDown()) return;
		OnUndo();
		break;

	case CL_KEY_V:
			
	if (IsControlKeyDown())
	{
		CL_Vector2 vecMouseClickPos = CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y());
		CL_Vector2 vecWorld = ConvertMouseToCenteredSelectionUpLeft(vecMouseClickPos);
		OnPaste(g_EntEditModeCopyBuffer, vecWorld, true);
		PushUndosIntoUndoOperation();
		SAFE_DELETE(m_pContextMenu);
	} else
	{
			m_bTilePaintModeActive = true;
	}

	break;
	default: ;
	}
}

void EntEditMode::TilePaint()
{
	if (!g_pMapManager->GetActiveMap()->GetSnapEnabled()) return;
	if (g_EntEditModeCopyBuffer.m_selectedTileList.size() != 1) return;

	CL_Vector2 vecMouseClickPos = CL_Vector2(CL_Mouse::get_x(),CL_Mouse::get_y());
	CL_Vector2 vecWorld = ConvertMouseToCenteredSelectionUpLeft(vecMouseClickPos);
	vecWorld = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecWorld, m_vecSnapSize);
	
	Screen *pScreen = g_pMapManager->GetActiveMap()->GetScreen(vecWorld);
	Tile *pReplaceTile = g_EntEditModeCopyBuffer.m_selectedTileList.front()->m_pTile;

	TileEditOperation edit;

	Tile *pTile = pScreen->GetTileByPosition(vecWorld, pReplaceTile->GetLayer());

	//TileEditOperation temp = m_selectedTileList; //copy whatever is highlighted

	if (pTile)
	{
		//if this tile is the same as the one we're going to paste, don't do it
		if (TilesAreSimilar(pTile, pReplaceTile, true)) return;
	}

	//now add the one

	g_EntEditModeCopyBuffer.SetIgnoreParallaxOnNextPaste();
	OnPaste(g_EntEditModeCopyBuffer, vecWorld);
	
}

void EntEditMode::Update(float step)
{
	if (m_bTilePaintModeActive)
	{
		TilePaint();
	}
}

CL_Vector2 EntEditMode::ConvertMouseToCenteredSelectionUpLeft(CL_Vector2 vecMouse)
{
	vecMouse = g_pMapManager->GetActiveMapCache()->ScreenToWorld(vecMouse);
	
	if (g_EntEditModeCopyBuffer.IsEmpty())
	{
		//this is no selection, so let's just convert to what they are highlighting
		return g_pMapManager->GetActiveMap()->SnapWorldCoords(vecMouse, CL_Vector2(1,1));
		return vecMouse;
	}

	if (m_pCheckBoxSnap->is_checked())
	{
		
		if (g_EntEditModeCopyBuffer.m_selectedTileList.size() > 1)
		{
			//center the selection over the mouse cursor..
			vecMouse -= g_EntEditModeCopyBuffer.GetSelectionSizeInWorldUnits()/2;

			//and a little less to feel right
			vecMouse += m_vecSnapSize/2;
		}

		vecMouse = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecMouse, m_vecSnapSize);
	} else
	{
		vecMouse = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecMouse, CL_Vector2(1,1));
		vecMouse -= g_EntEditModeCopyBuffer.GetSelectionSizeInWorldUnits()/2;
	}
	return vecMouse;
}

void EntEditMode::DrawActiveBrush(CL_GraphicContext *pGC)
{
	if (g_EntEditModeCopyBuffer.IsEmpty()) return;
	CL_Vector2 vecMouse = ConvertMouseToCenteredSelectionUpLeft(m_vecScreenDragStop);
	CL_Vector2 vecBottomRight = (vecMouse + (g_EntEditModeCopyBuffer.GetSelectionSizeInWorldUnits()));
	SelectedTile *pSelTile = (*g_EntEditModeCopyBuffer.m_selectedTileList.begin());

	DrawRectFromWorldCoordinates(vecMouse, vecBottomRight, CL_Color(255,0,0,150), pGC);
}


void EntEditMode::DrawSelection(CL_GraphicContext *pGC)
{
	if (m_selectedTileList.IsEmpty()) return;
	
	CL_Vector2 worldDragStop = g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop);

	CL_Vector2 vecStart( worldDragStop.x -m_vecDragStart.x, worldDragStop.y - m_vecDragStart.y);
	
	vecStart += m_selectedTileList.GetUpperLeftPos();
	
	if (g_pMapManager->GetActiveMap()->GetSnapEnabled())
	{
		if (m_selectedTileList.m_selectedTileList.size() == 1)
		vecStart = g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop);
		vecStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecStart, m_vecSnapSize);
	} else
	{
		vecStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(vecStart, m_vecDragSnap);
	}
	
	CL_Vector2 vecStop(vecStart.x + m_selectedTileList.GetSelectionSizeInWorldUnits().x, vecStart.y + m_selectedTileList.GetSelectionSizeInWorldUnits().y);

	vecStart = g_pMapManager->GetActiveMapCache()->WorldToScreen(vecStart);
	vecStop = g_pMapManager->GetActiveMapCache()->WorldToScreen(vecStop);

	pGC->draw_rect(CL_Rectf(vecStart.x, vecStart.y, vecStop.x, vecStop.y), CL_Color(50,255,50,180));
}

void EntEditMode::DrawDragRect(CL_GraphicContext *pGC)
{
	//
	CL_Vector2 worldStop = g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop);
	CL_Vector2 worldStart = m_vecDragStart;
	if (m_pCheckBoxSnap->is_checked())
	{
#ifdef _DEBUG
		//LogMsg("DragStart is %s", PrintVector(m_vecDragStart).c_str());
#endif	

		//these tweaks allow the snapped grid to be inclusive of the tile we're half on
		if (worldStop.x > worldStart.x) worldStop.x += m_vecSnapSize.x; else worldStart.x += m_vecSnapSize.x;
		if (worldStop.y > worldStart.y) worldStop.y += m_vecSnapSize.y; else worldStart.y += m_vecSnapSize.y;
		worldStart = g_pMapManager->GetActiveMap()->SnapWorldCoords(worldStart, m_vecSnapSize);
#ifdef _DEBUG
		//LogMsg("Converted to %s", PrintVector(worldStart).c_str());
#endif
		worldStop = g_pMapManager->GetActiveMap()->SnapWorldCoords(worldStop, m_vecSnapSize);
	}

	m_pLabelSelection->set_text(CL_String::format("Size: %1, %2", int(g_pMapManager->GetActiveMap()->SnapWorldCoords(worldStop, m_vecDragSnap).x - g_pMapManager->GetActiveMap()->SnapWorldCoords(worldStart, m_vecDragSnap).x),
		int(g_pMapManager->GetActiveMap()->SnapWorldCoords(worldStop, m_vecDragSnap).y - g_pMapManager->GetActiveMap()->SnapWorldCoords(worldStart, m_vecDragSnap).y)));
	//draw a rect on the screen
	
	CL_Rectf rec;

	worldStart = g_pMapManager->GetActiveMapCache()->WorldToScreen(worldStart);
	worldStop = g_pMapManager->GetActiveMapCache()->WorldToScreen(worldStop);

	rec.left = worldStart.x;
	rec.top = worldStart.y;
	rec.right = worldStop.x;
	rec.bottom = worldStop.y;
	pGC->draw_rect(rec, CL_Color(255,255,0,200));
	
	worldStart -= CL_Vector2(1,1);
	worldStop -= CL_Vector2(1,1);
	pGC->draw_rect(rec, CL_Color(140,140,0,200));
	
	//draw a little help bar text thing
	CL_Rect r(0,GetScreenY-15,GetScreenX, GetScreenY);
	CL_Display::fill_rect(r, CL_Color(0,0,0,80));

	//draw the text over it
	ResetFont(GetApp()->GetFont(C_FONT_GRAY));
	GetApp()->GetFont(C_FONT_GRAY)->set_alignment(origin_center);
	GetApp()->GetFont(C_FONT_GRAY)->draw(GetScreenX/2,GetScreenY-7,"Selection Drag - Press F to flood fill with current tile, Ctrl-C to cut tile from a tile");
}

void EntEditMode::OnProperties()
{
	KillContextMenu();
	BuildTilePropertiesMenu(&m_selectedTileList);
}

void EntEditMode::OnCollisionDataEditEnd(int id)
{
	
	if (!m_pEntCollisionEditor) return;
	
	m_pMenu->enable();

	bool bDataChanged = m_pEntCollisionEditor->GetDataChanged();
	
	//last minute thing to update the tiles that might be modified
	m_pEntCollisionEditor = NULL; //back to normal mode

	//need to reinit all script based items possibly
	
	if (m_pOriginalEditEnt)
	{
		m_pTileWeAreEdittingCollisionOn = m_pOriginalEditEnt->GetTile(); //fix a probably bad pointer, from moving it
		
		//also fix our selection box

		m_selectedTileList.ClearSelection();
		m_selectedTileList.AddTileToSelection(TileEditOperation::C_OPERATION_ADD,
			false, m_pTileWeAreEdittingCollisionOn);
		//before
	}

	if (bDataChanged)
	{
		if (m_pTileWeAreEdittingCollisionOn->GetType() == C_TILE_TYPE_ENTITY)
		{
			TileEntity *pEnt = (TileEntity*)m_pTileWeAreEdittingCollisionOn;
			if (pEnt->GetEntity())
			{


				tile_list t;
			
				GetPointersToSimilarTilesOnMap(g_pMapManager->GetActiveMap(), t, pEnt);

				
				pEnt->GetEntity()->Kill(); //force it to save out its collision info to disk, so when
				
				//we reinit all similar tiles, they will use the changed version
				ReInitTileList(t);
				ReInitEntity(pEnt->GetEntity());

			} else
			{
				assert(!"Huh?");
			}
		}

		if (m_pTileWeAreEdittingCollisionOn->GetType() == C_TILE_TYPE_PIC)
		{
			TilePic *pTilePic = (TilePic*)m_pTileWeAreEdittingCollisionOn;

			pTilePic->SaveToMasterCollision();
			g_pMapManager->GetActiveMap()->ReInitCollisionOnTilePics();
		}
	}
}

void EntEditMode::OnDefaultTileHardness()
{
	
	if (m_pEntCollisionEditor)
	{
		m_pEntCollisionEditor->OnOk();
		return;
	}

	if (m_selectedTileList.IsEmpty())
	{
		CL_MessageBox::info("No tile selected.  Left click on a tile first.", GetApp()->GetGUI());
		return;
	}

	if (m_selectedTileList.m_selectedTileList.size() > 1)
	{
		CL_MessageBox::info("Collision data editing can only be done on single tiles, just select one.", GetApp()->GetGUI());
		return;
	}

	Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;
	pTile  =  g_pMapManager->GetActiveMap()->GetScreen(pTile->GetPos())->GetTileByPosition(pTile->GetPos(), pTile->GetLayer());
	
	if (!pTile)
	{
		CL_MessageBox::info("Highlighted entity not found, don't try this on something that is moving.", GetApp()->GetGUI());
		return;
	}

	if (pTile->GetBit(Tile::e_flippedX) || pTile->GetBit(Tile::e_flippedY))
	{
		CL_MessageBox::info("Can't edit tiles that are X or Y flipped currently.  Yes, Seth sucks.", GetApp()->GetGUI());
		return;

	}
 
	m_pOriginalEditEnt = 	NULL;

	bool bIsEnt = pTile->GetType()==C_TILE_TYPE_ENTITY;
	if (bIsEnt)
	{
		m_pOriginalEditEnt = ((TileEntity*)pTile)->GetEntity();
	}
	
	bool bUsesCustomCollisionData = bIsEnt &&  ((TileEntity*)pTile)->GetEntity()->UsingCustomCollisionData();

	//well, now we have the tile that was in our selection buffer.  But that isn't really good enough
	
	if (!pTile->GetCollisionData())
	{
		//it's an entity but no collision data has been assigned
		CL_MessageBox::info("This tile/entity doesn't currently have collision data enabled.\n(Add this:LoadCollisionInfo(\"~/name.col\"); to its script?", GetApp()->GetGUI());
		return;
	}

	m_pEntCollisionEditor = (EntCollisionEditor*) GetMyEntityMgr->Add(new EntCollisionEditor);
	m_slots.connect(m_pEntCollisionEditor->sig_delete, this, &EntEditMode::OnCollisionDataEditEnd);


	CL_Vector2 vEditPos = pTile->GetPos();
	CL_Rect rec(pTile->GetWorldRect());
	rec.apply_alignment(origin_top_left, vEditPos.x, vEditPos.y);


	CL_Vector2 vOriginalPos = vEditPos;
	if (bIsEnt)
	{
		m_collisionEditOldTilePos = m_pOriginalEditEnt->GetPos();
	
	} else
	{

		//if it's a tile and scaled, let's make sure it's using custom collision data
		assert(pTile->GetType() == C_TILE_TYPE_PIC);

		((TilePic*)pTile)->ForceUsingCustomCollisionData();
	}

	m_pTileWeAreEdittingCollisionOn = pTile; //messy way of remembering this in the
	//callback that is hit when editing is done

	m_pMenu->enable(false);

	m_pEntCollisionEditor->Init(vEditPos, rec, pTile->GetCollisionData(), false);
	m_pEntCollisionEditor->SetBullsEye(vOriginalPos);
	m_pEntCollisionEditor->SetClip(!bIsEnt);
}

Tile * EntEditMode::GetSingleSelectedTile()
{
	if (m_selectedTileList.m_selectedTileList.size() == 1)
	{
		Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;
		return g_pMapManager->GetActiveMap()->GetScreen(pTile->GetPos())->GetTileByPosition(pTile->GetPos(), pTile->GetLayer());
	} else
	{
		//well, one thing isn't selected so this won't work, return NULL so they know
	}
	
	return NULL;
}
void EntEditMode::Render(void *pTarget)
{
	//uh... there's gotta be a better way, but I don't want to pollute the EntBase class
	//with Clanlib specific things
	CL_GraphicContext *pGC = (CL_GraphicContext*)pTarget;

	selectedTile_list::iterator itor = m_selectedTileList.m_selectedTileList.begin();

	//draw the drag rect
	m_pLabelSelection->set_text("");

	if (!m_selectedTileList.IsEmpty())
	{
		string s;
		char stTemp[256];
		sprintf(stTemp, "Sel Pos: X: %.1f Y: %.1f\n(%.1f, %.1f)", m_selectedTileList.GetUpperLeftPos().x, m_selectedTileList.GetUpperLeftPos().y,
			m_selectedTileList.GetSelectionSizeInWorldUnits().x, m_selectedTileList.GetSelectionSizeInWorldUnits().y);
	
		s = stTemp;

		if (m_selectedTileList.m_selectedTileList.size() == 1)
		{

			Tile *pTile = (*m_selectedTileList.m_selectedTileList.begin())->m_pTile;
			unsigned int tileLayer = m_selectedTileList.m_selectedTileList.back()->m_pTile->GetLayer();
			LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();

			if (tileLayer >= layerMan.GetLayerCount())
			{
				s += ( "\nLayer: Invalid (ID "+CL_String::from_int(tileLayer)+")");
			} else
			{
				s += ( "\nLayer: "+ layerMan.GetLayerInfo(tileLayer).GetName());
			}

			if (pTile->GetType() == C_TILE_TYPE_ENTITY)
			{
				//it's pointless to show the entity ID unless we are working from the real source tile instead of the copy
				Tile *pSrcTile  =  GetSingleSelectedTile(); 

				if (pSrcTile && pSrcTile->GetType() == C_TILE_TYPE_ENTITY)
				{
					//located the real version underneath on the map
					MovingEntity *pEnt = ((TileEntity*)pSrcTile)->GetEntity();
					s += "\nID: " + CL_String::from_int(pEnt->ID());
					if (!pEnt->GetName().empty())
					{
						s += " Tag: "+pEnt->GetName();
					}
				}
			}
			
		} else
		{
			//multiple tiles selected
			s += "\n" + CL_String::from_int(m_selectedTileList.m_selectedTileList.size()) +" selected.";
		}
		m_pLabelSelection->set_text(s);
	}
	
	if (!m_bHideSelection && !IsDialogOpen(false, true))
	{
		while (itor != m_selectedTileList.m_selectedTileList.end())
		{
			DrawSelectedTileBorder((*itor)->m_pTile, pGC);
			itor++;
		}
	}

	if (m_pEntCollisionEditor) return; //don't draw this extra GUI stuff right now
	
	if (m_operation == C_OP_MOVE)
	{
		//LogMsg("Rendering..");
		DrawSelection(pGC);
	}
	
	if (m_dragInProgress && m_vecDragStart != g_pMapManager->GetActiveMapCache()->ScreenToWorld(m_vecScreenDragStop))
	{
		DrawDragRect(pGC);
	} else
	{
		//draw the brush, if available
		if (m_operation == C_OP_NOTHING)
		DrawActiveBrush(pGC);
	}

	if (m_bTilePaintModeActive)
	{
		if (!g_pMapManager->GetActiveMap()->GetSnapEnabled() || g_EntEditModeCopyBuffer.m_selectedTileList.size() != 1) 
		{
			//show message so they know how to use the tile paint function
			DrawTextBar(GetScreenY-20, CL_Color(0,0,0,120), "(To use TilePaint, you must have grid snap on and have one tile/entity in the copy buffer)"); 
		}
	}


	//for testing
}

void DrawSelectedTileBorder(Tile *pTile, CL_GraphicContext *pGC)
{
	//let's draw a border around this tile
	CL_Rectf worldRect(pTile->GetWorldRect());
	CL_Vector2 vecStart(worldRect.left, worldRect.top);
	CL_Vector2 vecStop(worldRect.right, worldRect.bottom);
	
	vecStart = g_pMapManager->GetActiveMapCache()->WorldToScreen(vecStart);
	vecStop = g_pMapManager->GetActiveMapCache()->WorldToScreen(vecStop);

	pGC->draw_rect(CL_Rectf(vecStart.x, vecStart.y, vecStop.x, vecStop.y), CL_Color(50,50,50,180));
	
	vecStop.x++;
	vecStop.y++;
	vecStart.x--;
	vecStart.y--;

	pGC->draw_rect(CL_Rectf(vecStart.x, vecStart.y, vecStop.x, vecStop.y), CL_Color(255,255,255,180));

}

void EntEditMode::OnPropertiesOK()
{
	m_guiResponse = C_GUI_OK;
}

void EntEditMode::OnPropertiesEditScript()
{
	string file = GetGameLogic()->GetScriptRootDir()+"/"+m_pPropertiesInputScript->get_text();
	g_VFManager.LocateFile(file);
	OpenScriptForEditing(file);
}

void EntEditMode::OnPropertiesOpenScript()
{
	//this is awfully messy, because I'm calculating paths in a strange way to get relative versions
	assert(m_pPropertiesInputScript);
	
	string fname = m_pPropertiesInputScript->get_text();
	string original_dir = CL_Directory::get_current();
	string fileNameWithoutPath = CL_String::get_filename(fname);

	string dir = GetGameLogic()->GetScriptRootDir()+"/"+fname;
	if (g_VFManager.LocateFile(dir))
	{
		//was able to locate it in one of our resource paths
	}
	
	dir = CL_String::get_path(dir);

	CL_Directory::change_to(dir);
	CL_FileDialog dlg("Open LUA Script", fileNameWithoutPath, "*.lua", GetApp()->GetGUI());
	
	//dlg.set_behavior(CL_FileDialog::quit_file_selected);
	dlg.run();

	CL_Directory::change_to(original_dir);
	
	if (dlg.get_file().empty())
	{
		//guess thet didn't choose anything
		return;
	}

	//extract what they chose with the relevant path
	
	fname = dlg.get_path() +"/" + dlg.get_file();
	//let's just convert all the slashes
	StringReplace("\\", "/", fname);

	//make the path relative

	int index = fname.find(GetGameLogic()->GetScriptRootDir()+"/");
	if (index != -1)
	{
		//don't include the root dir part
		index += GetGameLogic()->GetScriptRootDir().size();
		fname = fname.substr(index, fname.size()-index);
	}
	
	//remove slash at the front if applicable
	if (fname[0] == '/')
	{
		fname = fname.substr(1, fname.size()-1);
	}

	m_pPropertiesInputScript->set_text(fname);
}

void EntEditMode::OnPropertiesConvert()
{
	m_guiResponse = C_GUI_CONVERT;
}
void EntEditMode::BuildTilePropertiesMenu(TileEditOperation *pTileList)
{
if (m_bDialogIsOpen) return;
	//change options on one or many tiles
	if (pTileList->IsEmpty())
	{
		CL_MessageBox::info("No tiles selected.  Left click on a tile first.", GetApp()->GetGUI());
		return;
	}
	int tilesSelected = pTileList->m_selectedTileList.size();

	if ( tilesSelected == 0 || ! (*pTileList->m_selectedTileList.begin())->m_pTile)
	{
		CL_MessageBox::info("No actual tile layer data exist in the current selection.", GetApp()->GetGUI());
		return;
	}

	selectedTile_list::iterator itor = pTileList->m_selectedTileList.begin();

	if (pTileList->m_selectedTileList.size() == 0)
	{
		LogMsg("No tile selected error");
		return;
	}
	Tile *pTile = (*pTileList->m_selectedTileList.begin())->m_pTile->CreateClone();

	string st;
	st = "Tile/Entity Properties ("+ CL_String::to(tilesSelected) + " selected)";

	// Creating InputDialog

	MovingEntity *pEnt = NULL;

	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		//note, we're getting the pointer to the REAL tile, not the copy we had made.  We do our
		//final operations to the real entity and tile because we don't have a nice way to propagate
		//changes to the whole list.  we're only allowing these changes to be made to single-selections
		//as a result

		pEnt = ((TileEntity*)(*pTileList->m_selectedTileList.begin())->m_pTile)->GetEntity();
		assert(pEnt);
	}

	//make GUI here
	m_bDialogIsOpen = true; //don't let other operations happen until we're done with this
	
	CL_Point ptSize(370,472);
	CL_Window window(CL_Rect(0, 0, ptSize.x, ptSize.y), st, 0, GetApp()->GetGUI());
	window.set_event_passing(false);
	m_guiResponse = C_GUI_CANCEL; //default
	window.set_position(300, GetScreenY- (ptSize.y+100));

	CL_SlotContainer slots;

	//add some buttons
	int buttonOffsetX = 10;
	int SecondOffsetX = 100;
	int ThirdOffsetX = 190;
	int offsetY = 10;
	
	int rightSideBarStartingX = 135;
	int rightBarOffsetX = 210;

	
	CL_Button buttonCancel (CL_Point(buttonOffsetX+100,ptSize.y-45), "Cancel", window.get_client_area());
	CL_Button buttonOk (CL_Point(buttonOffsetX+200,ptSize.y-45), "Ok", window.get_client_area());
	
	CL_CheckBox flipX (CL_Point(buttonOffsetX,offsetY), "Flip X", window.get_client_area());
	flipX.set_checked(pTile->GetBit(Tile::e_flippedX));

	CL_Label labelName(CL_Point(SecondOffsetX, offsetY), "If not blank, entity is locatable by name:", window.get_client_area());
	offsetY+= 20;

	CL_Label labelName2(CL_Point(SecondOffsetX, offsetY), "Name:", window.get_client_area());
	CL_Rect rPos(0,0,120,16);
	rPos.apply_alignment(origin_top_left, - (labelName2.get_width()+labelName2.get_client_x()+5), -(offsetY));
	CL_InputBox inputName(rPos, "", window.get_client_area());

	CL_Button buttonConvert (CL_Point(inputName.get_client_x()+inputName.get_width()+5,offsetY), "Convert to entity", window.get_client_area());

	CL_CheckBox flipY (CL_Point(buttonOffsetX,offsetY),"Flip Y", window.get_client_area());
	offsetY+= 20;
	flipY.set_checked(pTile->GetBit(Tile::e_flippedY));
	
	CL_CheckBox castShadow (CL_Point(buttonOffsetX,offsetY),"Cast Shadow", window.get_client_area());
	castShadow.set_checked(pTile->GetBit(Tile::e_castShadow));

	CL_CheckBox sortShadow (CL_Point(SecondOffsetX,offsetY),"Sort Shadow", window.get_client_area());
	sortShadow.set_checked(pTile->GetBit(Tile::e_sortShadow));

	CL_CheckBox pathNode(CL_Point(ThirdOffsetX,offsetY),"Path Node", window.get_client_area());
	pathNode.set_checked(pTile->GetBit(Tile::e_pathNode));

	offsetY+= 20;
	
	CL_Label labelEntity (CL_Point(buttonOffsetX, offsetY), "LUA Script:", window.get_client_area());

	rPos = CL_Rect(0,0,200,16);
	rPos.apply_alignment(origin_top_left, - (labelEntity.get_width()+buttonOffsetX+5), -(offsetY));
	CL_InputBox inputEntity(rPos, "", window.get_client_area());

	m_pPropertiesInputScript = &inputEntity; //need this for the file open function, messy, I know

	//add a little file open button
	CL_Button buttonOpenScript (CL_Point(inputEntity.get_client_x()+inputEntity.get_width(), offsetY), "File...", window.get_client_area());

	//add edit button
	CL_Button buttonEditScript (CL_Point(buttonOpenScript.get_client_x()+buttonOpenScript.get_width(), offsetY), "Edit", window.get_client_area());
	offsetY+= 20;
	
	
	CL_Button buttonEditAlignment(CL_Point(10, offsetY), "Modify Image Alignment", window.get_client_area() );

	CL_Origin origin;
	int origin_x, origin_y;
	pTile->GetAlignment(origin, origin_x, origin_y);

	string originText = CL_String::format("Current: %1 X:%2 Y:%3",
		OriginToString(origin), origin_x, origin_y);
	CL_Label labelAlignment (CL_Point(142, offsetY+2), originText, window.get_client_area());


	offsetY+= 20;


	CL_Label labelData (CL_Point(10, offsetY), "Custom data attached to the object (dbld click to edit):", window.get_client_area());
	offsetY+= 20;

	CL_Point ptOffset = CL_Point(buttonOffsetX, offsetY);
	rPos = CL_Rect(0,0,ptSize.x - (buttonOffsetX*2),100);
	rPos.apply_alignment(origin_top_left, -ptOffset.x, -ptOffset.y);

	CL_ListBox listData(rPos, window.get_client_area());
	//listData.set_multi_selection(true);
	m_pPropertiesListData = &listData;
	offsetY+= 4 + listData.get_height();

	CL_Button buttonAddData (CL_Point(buttonOffsetX, offsetY), "Add", window.get_client_area());
	CL_Button buttonRemoveData (CL_Point(buttonOffsetX+50, offsetY), "Remove Selected", window.get_client_area());
	offsetY += 20;
	//put in a layer select list too
	CL_Label labelLayer(CL_Point(buttonOffsetX, offsetY), "Changing layer affects all selected", window.get_client_area());

	offsetY += 20;
	rPos = CL_Rect(0,0, 110, 150);
	ptOffset = CL_Point(buttonOffsetX, offsetY);
	rPos.apply_alignment(origin_top_left, -ptOffset.x, -ptOffset.y);


	CL_ListBox layerList(rPos, window.get_client_area());
	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();
	vector<unsigned int> layerVec;
	layerMan.PopulateIDVectorWithAllLayers(layerVec);
	//sort it
	std::sort(layerVec.begin(), layerVec.end(), compareLayerBySort);
	int originalLayer;

	unsigned int layers = layerMan.GetLayerCount();
	for (unsigned int i=0; i < layers; i++)
	{
		int index = layerList.insert_item(layerMan.GetLayerInfo(layerVec[i]).GetName());
		layerList.get_item(i)->user_data = layerVec[i];
		if (pTile->GetLayer() == layerVec[i])
		{
			originalLayer = layerVec[i]; //so we can tell if it changed later
			layerList.set_selected(i, true);
			layerList.set_top_item(i);
		}
	}
	
	CL_Label labelName5(CL_Point(rightSideBarStartingX, offsetY), "Pos X/Y:", window.get_client_area());
	rPos = CL_Rect(0,0,110,16);
	rPos.apply_alignment(origin_top_left, - rightBarOffsetX, -(offsetY));

	string originalPos =  VectorToString(&pTile->GetPos());
	CL_InputBox inputPos(rPos, originalPos, window.get_client_area());

	offsetY += 20;


	 CL_Label labelName3(CL_Point(rightSideBarStartingX, offsetY), "Base Color:", window.get_client_area());
	 rPos = CL_Rect(0,0,110,16);
	 rPos.apply_alignment(origin_top_left, - rightBarOffsetX, -(offsetY));
	 
	 string originalColor = ColorToString(pTile->GetColor());
	 CL_InputBox inputColor(rPos, originalColor, window.get_client_area());

	 offsetY += 20;

	 CL_Label labelName4(CL_Point(rightSideBarStartingX, offsetY), "Scale X/Y:", window.get_client_area());
	 rPos = CL_Rect(0,0,110,16);
	 rPos.apply_alignment(origin_top_left, - rightBarOffsetX, -(offsetY));

	 string originalScale = VectorToString(&pTile->GetScale());
	 CL_InputBox inputScale(rPos, originalScale, window.get_client_area());

	 offsetY += 20;

	// Marcio
	CL_CheckBox linearFilter(CL_Point(rightSideBarStartingX,offsetY),"Use smoothing", window.get_client_area());
	linearFilter.set_checked(pTile->GetBit(Tile::e_linearFilter));

	
	if (pTile->GetType() == C_TILE_TYPE_PIC && tilesSelected == 1)
	{
		buttonConvert.enable(true);
	} else
	{
		buttonConvert.enable(false);
	}

	const char C_MULTIPLE_SELECT_TEXT[] = "<multiple selected>";

	if (pEnt)
	{
		if (tilesSelected > 1)
		{
			inputName.set_text(C_MULTIPLE_SELECT_TEXT);
			inputName.enable(false);
			inputEntity.set_text(C_MULTIPLE_SELECT_TEXT);
			inputPos.set_text(C_MULTIPLE_SELECT_TEXT);
			inputPos.enable(false);
			listData.enable(false);
			buttonAddData.enable(false);
			buttonRemoveData.enable(false);
	
		} else
		{
			inputEntity.set_text(pEnt->GetMainScriptFileName());
			inputName.set_text(pEnt->GetName());
		}
	
	} else
	{
		inputEntity.set_text("<N/A, not an entity>");
		inputEntity.enable(false);
		buttonOpenScript.enable(false);
		buttonEditScript.enable(false);
		inputName.enable(false);
		inputName.set_text("<N/A, not an entity>");

		listData.enable(false);
		buttonAddData.enable(false);
		buttonRemoveData.enable(false);
	}


	slots.connect(buttonCancel.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);

	//on ok, run our ok function and then kill the window
	slots.connect(buttonOk.sig_clicked(), this, &EntEditMode::OnPropertiesOK);
	slots.connect(buttonOk.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);

	slots.connect(buttonConvert.sig_clicked(), this, &EntEditMode::OnPropertiesConvert);
	slots.connect(buttonConvert.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);


	slots.connect(buttonOpenScript.sig_clicked(), this, &EntEditMode::OnPropertiesOpenScript);
	slots.connect(buttonEditScript.sig_clicked(), this, &EntEditMode::OnPropertiesEditScript);

	slots.connect(buttonAddData.sig_clicked(), &OnPropertiesAddData, &listData);
	slots.connect(buttonRemoveData.sig_clicked(), &OnPropertiesRemoveData, &listData);

	slots.connect(listData.sig_mouse_dblclk(), &OnPropertiesEditData, &listData);

	slots.connect(buttonEditAlignment.sig_clicked(), this, &EntEditMode::OnEditAlignment);


	//populate the listbox

	if (pEnt)
	{
		dataList *dList = pEnt->GetData()->GetList();
		dataList::iterator ditor = dList->begin();

		string stTemp;

		while (ditor != dList->end())
		{
			listData.insert_item(PropertyMakeItemString(ditor->second));
			ditor++;
		}
	}

	listData.sort();
	window.run();	//loop in the menu until quit() is called by hitting a button
	


	
	//Process GUI here

	if (m_guiResponse == C_GUI_OK  )
	{
	
	
		unsigned int flags = 0;
		
		if (pTile->GetType() == C_TILE_TYPE_ENTITY)
		{
			if (inputEntity.get_text() != C_MULTIPLE_SELECT_TEXT)
			{
				((TileEntity*)pTile)->GetEntity()->SetMainScriptFileName(inputEntity.get_text());
				flags = flags | TileEditOperation::eBitScript;
			}
		}
		
		if (flipX.is_checked() != pTile->GetBit(Tile::e_flippedX))
		{
			pTile->SetBit(Tile::e_flippedX, flipX.is_checked());
			flags = flags | TileEditOperation::eBitFlipX;
		}

		if (flipY.is_checked() != pTile->GetBit(Tile::e_flippedY))
		{
			pTile->SetBit(Tile::e_flippedY, flipY.is_checked());
			flags = flags | TileEditOperation::eBitFlipY;
		}

		if (castShadow.is_checked() != pTile->GetBit(Tile::e_castShadow))
		{
			pTile->SetBit(Tile::e_castShadow, castShadow.is_checked());
			flags = flags | TileEditOperation::eBitCastShadow;
		}

		if (sortShadow.is_checked() != pTile->GetBit(Tile::e_sortShadow))
		{
			pTile->SetBit(Tile::e_sortShadow, sortShadow.is_checked());
			flags = flags | TileEditOperation::eBitSortShadow;
		}

		if (pathNode.is_checked() != pTile->GetBit(Tile::e_pathNode))
		{
			pTile->SetBit(Tile::e_pathNode, pathNode.is_checked());
			flags = flags | TileEditOperation::eBitPathNode;
		}

		if (linearFilter.is_checked() != pTile->GetBit(Tile::e_linearFilter))
		{
			pTile->SetBit(Tile::e_linearFilter, linearFilter.is_checked());
			flags = flags | TileEditOperation::eBitLinearFilter;
		}

		if (inputColor.get_text() != originalColor)
		{
			pTile->SetColor(StringToColor(inputColor.get_text()));
			flags = flags | TileEditOperation::eBitColor;
		}
	
		if (inputScale.get_text() != originalScale)
		{
			CL_Vector2 vScale = StringToVector(inputScale.get_text());
			//ensure it's not 0, otherwise it will be invisible...
			if (vScale.x == 0) 
			{
				LogError("Bad scale input, setting X to 1 instead of 0.");
				vScale.x = 1;
			}

			if (vScale.y == 0) 
			{
				LogError("Bad scale input, setting Y to 1 instead of 0.");
				vScale.y = 1;
			}
			pTile->SetScale(vScale);
			
			flags = flags | TileEditOperation::eBitScale;
		}


		if (pEnt && pTileList->m_selectedTileList.size() == 1)
		{
			Tile blankTile; 
			AddToUndo(pTileList);
			pTileList->FillSelection(&blankTile);
		}

		//run through and copy these properties to all the valid tiles in the selection
		pTileList->CopyTilePropertiesToSelection(pTile, flags);

		if (pEnt && pTileList->m_selectedTileList.size() == 1)
		{
			//it's a single ent, so extra property changes can be made too
			if (inputName.get_text() != pEnt->GetName())
			{
				pEnt->SetName(inputName.get_text());
			}
		
			CL_Vector2 vNewPos = StringToVector(inputPos.get_text());
			if (vNewPos != pEnt->GetPos())
			{
				//special handling because it changes position as well

				//delete the old stuff and put it in the undo buffer
				pEnt->SetPos(vNewPos);
			}

			PropertiesSetDataManagerFromListBox(pEnt->GetData(), listData);
		}

		int selectedLayer = layerVec[layerList.get_current_item()];
		if (selectedLayer == -1) selectedLayer = originalLayer;
	
		if ( selectedLayer != originalLayer)
		{
			pTileList->SetForceLayerOfNextPaste(selectedLayer);
			//LogMsg("Changing layer to %d", layerList.get_item(selectedLayer)->user_data);
		}
	

		pTileList->SetIgnoreParallaxOnNextPaste();
		

		//paste our selection, this helps by creating an undo for us
		OnPaste(*pTileList, pTileList->GetUpperLeftPos());
		PushUndosIntoUndoOperation();
		if (selectedLayer != originalLayer)
		{
			//couldn't change it before, because the paste needed it to delete the old tiles			
			pTileList->SetLayerOfSelection(selectedLayer);
		}

		//g_pMapManager->GetActiveMap()->ReInitEntities();
		//update our current selection
		
		pTileList->UpdateSelectionFromWorld();
		
		tile_list t;
		GetPointersToSimilarTilesOnMap(g_pMapManager->GetActiveMap(), t, pTile);
		ReInitTileList(t);

	} else if (m_guiResponse == C_GUI_CONVERT)
	{
		
		MovingEntity *pEnt = new MovingEntity;
		TileEntity *pNewTile = new TileEntity;
		pNewTile->SetEntity(pEnt);
		pEnt->SetPos(pTile->GetPos());
		pNewTile->SetBit(Tile::e_flippedX, pTile->GetBit(Tile::e_flippedX));
		pNewTile->SetBit(Tile::e_flippedY, pTile->GetBit(Tile::e_flippedY));
		pNewTile->SetBit(Tile::e_castShadow, pTile->GetBit(Tile::e_castShadow));
		pNewTile->SetBit(Tile::e_sortShadow, pTile->GetBit(Tile::e_sortShadow));
		pNewTile->SetBit(Tile::e_pathNode, pTile->GetBit(Tile::e_pathNode));
		pNewTile->SetBit(Tile::e_linearFilter, pTile->GetBit(Tile::e_linearFilter));
		pNewTile->SetScale(pTile->GetScale());
		pNewTile->SetColor(pTile->GetColor());

		pNewTile->SetLayer(pTile->GetLayer());
		pEnt->SetImageFromTilePic((TilePic*)pTile);
		pEnt->SetMainScriptFileName("");
		pEnt->Init();
		
		OnDeleteSimple(); //kill the old one
		
		m_selectedTileList.ClearSelection();
		m_selectedTileList.AddTileToSelection(TileEditOperation::C_OPERATION_ADD,
			false, pNewTile);
	
		m_selectedTileList.SetIgnoreParallaxOnNextPaste();
		//paste our selection, this helps by creating an undo for us
		OnPaste(m_selectedTileList, m_selectedTileList.GetUpperLeftPos());
		PushUndosIntoUndoOperation();

		g_pMapManager->GetActiveMap()->ReInitCollisionOnTilePics();
		//g_pMapManager->GetActiveMapCache()->Update(0); //force it to rescan nearby tiles so the visual updates now
		
		ScheduleSystem(1, ID(), "open_properties_on_selected"); //tell it to open this properties dialog again as soon
	}

	m_bDialogIsOpen = false;
	SAFE_DELETE(pTile);
	m_pPropertiesInputScript= NULL;
	m_pPropertiesListData = NULL;
}

void EntEditMode::GetSettingsFromWorld()
{
		
	if (g_pMapManager->GetActiveMap())	
	{
		m_pCheckBoxSnap->set_checked(g_pMapManager->GetActiveMap()->GetSnapEnabled());
		m_vecSnapSize.x = g_pMapManager->GetActiveMap()->GetDefaultTileSizeX();
		m_vecSnapSize.y = g_pMapManager->GetActiveMap()->GetDefaultTileSizeY();
		m_pInputBoxSnapSizeX->set_text(CL_String::from_int(m_vecSnapSize.x));
		m_pInputBoxSnapSizeY->set_text(CL_String::from_int(m_vecSnapSize.y));
	}
}

void EntEditMode::OnMapChange()
{
	GetSettingsFromWorld();
}

void EntEditMode::OnEditAlignment()
{
	CL_MessageBox::info("About editing alignment", "To interactively edit the alignment of a tilepic/entity, exit this dialog, highlight it, and hit Ctrl-Shift-E or choose Modify Selected->Edit Image Alignment.", GetApp()->GetGUI());
}

void EntEditMode::OnDeleteSimple()
{
	OnDelete(&m_selectedTileList);
}