#include "AppPrecomp.h"
#include "GameLogic.h"
#include "EntVisualProfileEditor.h"
#include "VisualProfileManager.h"
#include "EntCreationUtils.h"

EntVisualProfileEditor::EntVisualProfileEditor(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pWindow = NULL;
	m_pListAnims = NULL;
	m_pListAlignment = NULL;
	m_pEnt = NULL;

	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntVisualProfileEditor::OnButtonDown);
	m_slots.connect (CL_Mouse::sig_key_up(), this, &EntVisualProfileEditor::OnButtonUp);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntVisualProfileEditor::OnMouseMove);
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &EntVisualProfileEditor::OnButtonDown);

	SetName("visualeditor");
	LogMsg("Creating visual profile editor");
}

EntVisualProfileEditor::~EntVisualProfileEditor()
{
	SAFE_DELETE(m_pWindow);
}

bool EntVisualProfileEditor::IsDialogOpen()
{
	//if yes, keyboard and mouse editing controls will be killed so we don't zoom or do things while editing text
	return false;
}

void EntVisualProfileEditor::OnEditorClosed(int entID)
{
	OnClose();
}

void EntVisualProfileEditor::OnEntityDeleted(int entID)
{
	//well, the entity we were editing was just deleted, this could happen when the game is shutdown.
	//Don't bother saving..
	SetDeleteFlag(true);

}

void EntVisualProfileEditor::OnClose()
{
	//save and close

	if (GetDeleteFlag()) return; //this was called more than once?

	if (m_pEnt && m_pEnt->GetVisualProfile())
	{
		m_pEnt->GetVisualProfile()->GetParentVisualResource()->Save();
	
		//reinit all possible sprites that could have been affected.
		tile_list t;
		GetPointersToSimilarTilesOnMap(g_pMapManager->GetActiveMap(), t, m_pEnt->GetTile());
		ReInitTileList(t);

	} else
	{
		g_pMapManager->GetActiveMap()->ReInitCollisionOnTilePics();
	}

	SetDeleteFlag(true);
}

int SetListBoxByString(CL_ListBox &lbox, const string &st)
{

	vector<CL_ListItem*> &items = lbox.get_items();

	for (unsigned int i=0; i < items.size(); i++)
	{
		if (items[i]->str == st)
		{
			//found match
			lbox.set_current_item(i);
			lbox.set_top_item(i); //so we can see it
			return i;
		}
	}
	return -1; //error
}

void EntVisualProfileEditor::OnChangeAnim()
{
	if (m_pEnt && m_pEnt->GetVisualProfile())
	{
		m_pEnt->SetAnimByName(m_pListAnims->get_current_text());
		
		CL_Sprite *pSprite = m_pEnt->GetVisualProfile()->GetSpriteByAnimID(m_pEnt->GetAnimID());

	if (!pSprite)
	{
		LogError("No active sprite right now?  Can't adjust it");
		return;
	} 
		
		CL_Origin o;
		int x,y;

		pSprite->get_alignment(o, x,y);
		m_pListAlignment->set_current_item(o);
		m_pAlignX->set_text(CL_String::from_int(x));
		m_pAlignY->set_text(CL_String::from_int(y));
	}
}

bool EntVisualProfileEditor::Init(Tile *pTile)
{
	assert(!m_pWindow && "We don't support initting things twice");

	m_pBaseTile = pTile;

	if (pTile->GetType() == C_TILE_TYPE_ENTITY)
	{
		m_pTilePic = NULL;
		m_pEnt = ((TileEntity*)pTile)->GetEntity();
	} else
	{
		m_pEnt = NULL;
		assert(pTile->GetType() == C_TILE_TYPE_PIC);
		m_pTilePic = (TilePic*)pTile;
	}

	//build the GUI window

	CL_Point ptSize(450,220);
	m_pWindow = new CL_Window(CL_Rect(0, 0, ptSize.x, ptSize.y), "Image Alignment Editor Palette", 0, GetApp()->GetGUI());
	m_pWindow->set_position(200, GetScreenY-ptSize.y);
	m_pWindow->set_event_passing(false);

	int buttonOffsetX = 10;
	int curY = 10;
	CL_Button *pButton = new CL_Button(CL_Point(buttonOffsetX,ptSize.y-50), "Close & Save", m_pWindow->get_client_area());

	new CL_Label(CL_Point(buttonOffsetX, curY), "Use the arrow keys to adjust the offset of the selected animation.  Hold shift to", m_pWindow->get_client_area()); curY += 16;
	//new CL_Label(CL_Point(buttonOffsetX, curY), "shift to move in larger increments.", m_pWindow->get_client_area()); curY += 16;
	//new CL_Label(CL_Point(buttonOffsetX, curY), "", m_pWindow->get_client_area()); curY += 16;
	new CL_Label(CL_Point(buttonOffsetX, curY), "move in larger increments.  Use Control+Up/Down to quickly change animations.", m_pWindow->get_client_area()); curY += 16;

	m_slots.connect(pButton->sig_clicked(), this, &EntVisualProfileEditor::OnClose);

	//while we're at it, if they close the edit mode window, let's also close ourself
	m_slots.connect(EntityMgr->GetEntityByName("edit mode")->sig_delete, this, &EntVisualProfileEditor::OnEditorClosed);

	vector<string> vecAnims;

	if (m_pEnt)
	{
		//what if the entity itself is destroy for some reason?  handle that too
		m_slots.connect(m_pEnt->sig_delete, this, &EntVisualProfileEditor::OnEntityDeleted);

		if (m_pEnt->GetVisualProfile())
		{
			vecAnims = m_pEnt->GetVisualProfile()->GetListOfActiveAnims();
		}
	}

		//set the list of anims that can be clicked?

	curY += 12;

	new CL_Label(CL_Point(buttonOffsetX, curY), "Choose Anim:", m_pWindow->get_client_area());
	
	int animPropertiesStartX = 170;
	new CL_Label(CL_Point(animPropertiesStartX, curY), "Anim/Image Alignment:", m_pWindow->get_client_area());
	

	
	curY += 14;

	CL_Rect r = CL_Rect(0,0, 150, 95);
	r.apply_alignment(origin_top_left, -buttonOffsetX, -curY);
	m_pListAnims = new CL_ListBox(r, m_pWindow->get_client_area());
	m_pListAnims->set_focusable(false);
	for (unsigned int i=0; i < vecAnims.size(); i++)
	{
		m_pListAnims->insert_item(vecAnims[i].c_str());
	}
	if (m_pListAnims->get_count() > 0)
	{
		SetListBoxByString(*m_pListAnims, m_pEnt->GetVisualProfile()->AnimIDToText(m_pEnt->GetAnimID()));

	} else
	{
		//empty, we're not editing a visual profile I guess
		m_pListAnims->insert_item("NA - Not using a visual");
		m_pListAnims->insert_item("profile.");
		m_pListAnims->enable(false);
	}

	m_slots.connect( m_pListAnims->sig_selection_changed(), this, &EntVisualProfileEditor::OnChangeAnim);



	r = CL_Rect(0,0, 150, 95);
	r.apply_alignment(origin_top_left, -animPropertiesStartX, -curY);
	m_pListAlignment = new CL_ListBox(r, m_pWindow->get_client_area());
	m_pListAlignment->set_focusable(false);
	CL_Origin o;
	int x,y;

	m_pBaseTile->GetAlignment(o, x,y);

	for (int i=0; i < 9; i++)
	{

		m_pListAlignment->insert_item(OriginToString(CL_Origin(i)));
		if (o == i)
		{
			m_pListAlignment->set_selected(i, true);
		}
	}

	m_slots.connect( m_pListAlignment->sig_selection_changed(), this, &EntVisualProfileEditor::OnChangeAlignment);


	//add the XY input boxes

	curY += 100;
	new CL_Label(CL_Point(animPropertiesStartX, curY), "Offset: X", m_pWindow->get_client_area());
	new CL_Label(CL_Point(animPropertiesStartX, curY)+CL_Point(80,0), "Y", m_pWindow->get_client_area());


	CL_Rect recSize(0,0,26,14);
	recSize.apply_alignment(origin_top_left, - (animPropertiesStartX+50), -curY);
	m_pAlignX = new CL_InputBox(CL_String::from_int(x), m_pWindow->get_client_area());
	m_pAlignX->set_position(recSize);

	recSize = CL_Rect(0,0,26,14);
	recSize.apply_alignment(origin_top_left, - (animPropertiesStartX+50+50), -curY);
	m_pAlignY = new CL_InputBox(CL_String::from_int(y), m_pWindow->get_client_area());
	m_pAlignY->set_position(recSize);


	m_slots.connect(m_pAlignY->sig_return_pressed(), this, &EntVisualProfileEditor::OffsetChanged);
	m_slots.connect(m_pAlignY->sig_changed(), this, &EntVisualProfileEditor::OnOffsetChanged);

	m_slots.connect(m_pAlignX->sig_return_pressed(), this, &EntVisualProfileEditor::OffsetChanged);
	m_slots.connect(m_pAlignX->sig_changed(), this, &EntVisualProfileEditor::OnOffsetChanged);

	//m_slots.connect(m_pInputBoxSnapSize->sig_lost_focus(), this, &EntEditMode::SnapSizeChanged);
	//m_slots.connect(m_pAlignX->sig_return_pressed(), this, &EntEditMode::AlignmentChanged);


	if (m_pTilePic)
	{
		//m_pListAlignment->enable(false);
	}

	return true; //success
}

void EntVisualProfileEditor::OnMouseMove(const CL_InputEvent &key)
{
}

void EntVisualProfileEditor::OnButtonUp(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_MOUSE_LEFT:
		break;
	}
}

void EntVisualProfileEditor::OffsetChanged()
{
	CL_Origin o;
	int x,y;
	CL_Sprite *pSprite = NULL;


	if (m_pEnt)
	{
		if (m_pEnt->GetVisualProfile())
		{
			pSprite = m_pEnt->GetVisualProfile()->GetSpriteByAnimID(m_pEnt->GetAnimID());
		} else
		{
			pSprite = m_pEnt->GetSprite();
		}

		if (!pSprite)
		{
			LogError("No active sprite right now?  Can't adjust it");
			return;
		} 
		pSprite->get_alignment(o, x,y);
	} else
	{
		m_pTilePic->GetAlignment(o, x, y);

	}

	//set it

	x = CL_String::to_int(m_pAlignX->get_text());
	y = CL_String::to_int(m_pAlignY->get_text());

	if (m_pEnt)
	{
		//put it back in
		pSprite->set_alignment(o,x,y);
		//update the visuals so we can see the changes
		m_pEnt->ForceSpriteUpdate(); //otherwise it would ignore the change, thinking it already has it active
		m_pEnt->SetSpriteData(pSprite);
	} else
	{

		m_pTilePic->SetAlignment(o, x, y);
	}

}

void EntVisualProfileEditor::ModifyActiveAnim(CL_Point pt)
{

	CL_Sprite *pSprite = NULL;


	CL_Origin o;
	int x,y;

	int mult = 1;

	if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
	{
		mult = 5; //move much more if shift is pressed
	}

	if (m_pEnt)
	{
	if (m_pEnt->GetVisualProfile())
	{
		pSprite = m_pEnt->GetVisualProfile()->GetSpriteByAnimID(m_pEnt->GetAnimID());
	} else
	{
		pSprite = m_pEnt->GetSprite();
	}

	if (!pSprite)
	{
		LogError("No active sprite right now?  Can't adjust it");
		return;
	} 
	pSprite->get_alignment(o, x,y);
	} else
	{
		m_pTilePic->GetAlignment(o, x, y);

	}
	
		//apply modification
		x -= pt.x * mult;
		y += pt.y * mult;


		if (m_pEnt)
		{
			//put it back in
			pSprite->set_alignment(o,x,y);
			//update the visuals so we can see the changes
			m_pEnt->ForceSpriteUpdate(); //otherwise it would ignore the change, thinking it already has it active
			m_pEnt->SetSpriteData(pSprite);
		} else
		{

			m_pTilePic->SetAlignment(o, x, y);
		}
	
	m_pAlignX->set_text(CL_String::from_int(x));
	m_pAlignY->set_text(CL_String::from_int(y));


}

void EntVisualProfileEditor::MoveAnimSelection(int offset)
{
	int selected = m_pListAnims->get_current_item();
	selected = altmod(selected + offset, m_pListAnims->get_count());
	m_pListAnims->set_current_item(selected);
	m_pListAnims->set_top_item(selected);
}

void EntVisualProfileEditor::OnButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_RIGHT:
		ModifyActiveAnim(CL_Point(-1,0));
		break;

	case CL_KEY_LEFT:
		ModifyActiveAnim(CL_Point(1,0));
		break;

	case CL_KEY_UP:
		
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			MoveAnimSelection(-1);
			return;
		}
		
		ModifyActiveAnim(CL_Point(0,1));
		break;

	case CL_KEY_DOWN:
		if (CL_Keyboard::get_keycode(CL_KEY_CONTROL))
		{
			MoveAnimSelection(1);
			return;
		}
		ModifyActiveAnim(CL_Point(0,-1));
		break;

	case CL_KEY_DELETE:
		break;

	case CL_MOUSE_LEFT:
		break;

	case CL_MOUSE_RIGHT:
		break;
	}
}


void EntVisualProfileEditor::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext*) pTarget;

	CL_Vector2 vPos;
	CL_Rectf worldRect;

	if (m_pEnt)
	{
		vPos = m_pEnt->GetPos();
		worldRect = m_pEnt->GetWorldRect();
	}  else
	{
		vPos = m_pTilePic->GetPos();
		worldRect = m_pTilePic->GetWorldRect();
	}
	DrawBullsEyeWorld(vPos, CL_Color(255,255,0,200), 10, pGC); //draw his center point

	
	//let's draw a border around this tile
	CL_Vector2 vecStart(worldRect.left, worldRect.top);
	CL_Vector2 vecStop(worldRect.right, worldRect.bottom);

	vecStart = g_pMapManager->GetActiveMapCache()->WorldToScreen(vecStart);
	vecStop = g_pMapManager->GetActiveMapCache()->WorldToScreen(vecStop);

	pGC->draw_rect(CL_Rectf(vecStart.x, vecStart.y, vecStop.x, vecStop.y), CL_Color(255,50,50,100));

}

void EntVisualProfileEditor::OnChangeAlignment()
{

	CL_Origin o;
	int x,y;
	if (m_pEnt)
	{
		
/*
		if (m_pEnt->GetVisualProfile())
		{
			//CL_MessageBox::info("Can't edit this here yet, you'll have to open the visual profile .xml directly.", GetApp()->GetGUI());
			return;
		}
*/		

		CL_Sprite *pSprite = NULL;
		if (m_pEnt->GetVisualProfile())
		{
			pSprite = m_pEnt->GetVisualProfile()->GetSpriteByAnimID(m_pEnt->GetAnimID());
		} 
		m_pEnt->GetAlignment(o,x,y);

		if (pSprite)
		{

			//put it back in
			pSprite->set_alignment(CL_Origin(m_pListAlignment->get_current_item()),x,y);
			//update the visuals so we can see the changes
			m_pEnt->ForceSpriteUpdate(); //otherwise it would ignore the change, thinking it already has it active
			m_pEnt->SetSpriteData(pSprite);


		} else
		{
			m_pEnt->SetAlignment(m_pListAlignment->get_current_item(),CL_Vector2(x,y));
		}

		

		
	} else
	{
		m_pTilePic->GetAlignment(o,x,y);
		m_pTilePic->SetAlignment(CL_Origin(m_pListAlignment->get_current_item()), x,y);
	}


	m_pAlignX->set_text(CL_String::from_int(x));
	m_pAlignY->set_text(CL_String::from_int(y));
}