#include "AppPrecomp.h"
#include "Screen.h"
#include "EntCollisionEditor.h"
#include "Tile.h"
#include "GameLogic.h"
#include "MaterialManager.h"
#include "MovingEntity.h"
#include "EntEditor.h"

#define C_COL_VERT_SIZE 6.0f

EntCollisionEditor::EntCollisionEditor(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_bDataChanged = false;
	m_pWindow = NULL;
	m_pLabel = NULL;
	m_pCollisionData = NULL;
	m_pRadioGroup = NULL;
	m_pListType = NULL;
	m_pCheckBoxSnap = NULL;
	m_pCheckBoxClipToImage = NULL;
	
	m_slots.connect(GetApp()->GetGUI()->sig_mouse_down(), this, &EntCollisionEditor::OnButtonDown);
	m_slots.connect (CL_Mouse::sig_key_up(), this, &EntCollisionEditor::OnButtonUp);
	m_slots.connect(CL_Mouse::sig_move(), this, &EntCollisionEditor::OnMouseMove);
	
	m_slots.connect(CL_Keyboard::sig_key_down(), this, &EntCollisionEditor::OnButtonDown);
	m_mode = e_modeAddCreate;
	m_pButtonOK = m_pButtonCancel = NULL;
	ClearSelection();
	SetName("coleditor");

	EntEditor *pEditor = (EntEditor*) EntityMgr->GetEntityByName("editor");
	if (pEditor)
	{
		m_slots.connect(pEditor->sig_hide_mode_changed, this, &EntCollisionEditor::OnHideModeChanged);
	}

}

EntCollisionEditor::~EntCollisionEditor()
{
	Kill();
}

void EntCollisionEditor::OnHideModeChanged(bool bHide)
{
	m_pWindow->show(!bHide);
}

void EntCollisionEditor::Kill()
{
	SAFE_DELETE(m_pButtonOK);
	SAFE_DELETE(m_pButtonCancel);
	SAFE_DELETE(m_pRadioGroup); 
	SAFE_DELETE(m_pLabel);
	SAFE_DELETE(m_pWindow);
}

void EntCollisionEditor::OnMouseMove(const CL_InputEvent &key)
{
	CL_Vector2 vecScreenOld = CL_Vector2(m_vecLastMousePos.x, m_vecLastMousePos.y);
	CL_Vector2 vecScreenNew = CL_Vector2(key.mouse_pos.x, key.mouse_pos.y);
	CL_Vector2 vecWorldDisp = g_pMapManager->GetActiveMapCache()->ScreenToWorld(vecScreenNew) - g_pMapManager->GetActiveMapCache()->ScreenToWorld(vecScreenOld);
	
		if (m_mode == e_modeAdjust)
	{
		if (m_dragVertID != C_INVALID_VERT)
		{
			//move it
			if (m_pActiveLine)
			{
				if (m_pSelectedVert)
				{
					//force it within range
					CL_Vector2 localPos = m_dragStartPos += vecWorldDisp;

					ApplySnap(localPos);

					if (m_pCheckBoxClipToImage->is_checked())
					{

						localPos.x = max(localPos.x, m_rectArea.left);
						localPos.x = min(localPos.x, m_rectArea.right);

						localPos.y = max(localPos.y, m_rectArea.top);
						localPos.y = min(localPos.y, m_rectArea.bottom);
					}
					
						
					*m_pSelectedVert = localPos;
				}
			}

		}
	}
	m_vecLastMousePos = key.mouse_pos;
}

void EntCollisionEditor::ApplySnap(CL_Vector2 &vec)
{
	if (m_pCheckBoxSnap->is_checked())
	{
		vec.x = int(vec.x);
		vec.y = int(vec.y);
	}
}

void EntCollisionEditor::OnButtonUp(const CL_InputEvent &key)
{
	switch(key.id)
	{
		case CL_MOUSE_LEFT:
			if (m_mode == e_modeAdjust)
			{
				//snap them
				CL_Vector2 vecPos =g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
				AdjustVert(vecPos);
			}
			break;
	}

	ClearSelection();

}

void EntCollisionEditor::AddCreateVert(CL_Vector2 vecPos)
{
	CL_Vector2 localPos = vecPos - m_pos;
	ApplySnap(localPos);

	if (m_pCheckBoxClipToImage->is_checked())
	{
		localPos.x = max(localPos.x, m_rectArea.left);
		localPos.x = min(localPos.x, m_rectArea.right);

		localPos.y = max(localPos.y, m_rectArea.top);
		localPos.y = min(localPos.y, m_rectArea.bottom);

	}

	//it's a click in a valid position
	if (m_pActiveLine)
	{
		//line is available to add this plot point to
		m_pActiveLine->GetPointList()->push_back(localPos);
	}
}

void EntCollisionEditor::ClearSelection()
{
	m_pSelectedVert = NULL;
	m_dragVertID = C_INVALID_VERT;
}

void EntCollisionEditor::DeleteVert(CL_Vector2 vecPos)
{
	CL_Vector2 localPos = vecPos - m_pos;

	CL_Vector2 *m_pSelectedVert;
	unsigned int vertID;
	
	CL_Vector2 vScale = GetCamera->GetScale();

	GetIndexOfVertAtThisPos(*m_pActiveLine->GetPointList(), localPos, &m_pSelectedVert, vertID, C_COL_VERT_SIZE/vScale.x);

	if (m_pSelectedVert)
	{
		//delete this vert they clicked
		m_pActiveLine->GetPointList()->erase(m_pActiveLine->GetPointList()->begin() + vertID);
		ClearSelection();
	}
	
}

bool EntCollisionEditor::GetIndexOfVertAtThisPos(point_list &pList, CL_Vector2 &vLocalPos, CL_Vector2 ** ppVertOut, unsigned int &vertIDOut, float fPadding)
{
	CL_Rectf r(vLocalPos.x-fPadding, vLocalPos.y-fPadding, vLocalPos.x+fPadding, vLocalPos.y+fPadding );

	vertIDOut = C_INVALID_VERT;
	*ppVertOut = NULL;
	for (unsigned int i=0; i < pList.size(); i++)
	{
		if (r.is_inside(*(CL_Pointf*)&pList[i]))
		{
			//found it
			*ppVertOut = &pList[i];
			vertIDOut = i;
			return true;
		}
	}

	return false;
}

void EntCollisionEditor::AdjustVert(CL_Vector2 vecPos)
{
	
	CL_Vector2 localPos = vecPos - m_pos;
	point_list &pointList = *m_pActiveLine->GetPointList(); 
	CL_Vector2 vScale = GetCamera->GetScale();
	
	GetIndexOfVertAtThisPos(pointList, localPos, &m_pSelectedVert, m_dragVertID, C_COL_VERT_SIZE/vScale.x);
	if (m_pSelectedVert)
	{
		m_dragStartPos = *m_pSelectedVert;
	}
}

void EntCollisionEditor::OnButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_DELETE:

		if (m_dragVertID != C_INVALID_VERT)
		{
			m_pActiveLine->GetPointList()->erase(m_pActiveLine->GetPointList()->begin() + m_dragVertID);
			m_dragVertID = C_INVALID_VERT;
		}
		break;
	
	case CL_KEY_RIGHT:
		ModifyShapePosition(CL_Point(-1,0));
		break;

	case CL_KEY_LEFT:
		ModifyShapePosition(CL_Point(1,0));
		break;
	case CL_KEY_UP:
		ModifyShapePosition(CL_Point(0,1));
		break;

	case CL_KEY_DOWN:
		ModifyShapePosition(CL_Point(0,-1));
		break;


	case CL_MOUSE_LEFT:
		{
	  	 
			CL_Vector2 vecPos =g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(key.mouse_pos.x, key.mouse_pos.y));
			switch (m_mode)
			{
			case e_modeAddCreate:
				AddCreateVert(vecPos);
				break;

			case e_modeAdjust:
				AdjustVert(vecPos);
				break;

			/*
			case e_modeDelete:
				DeleteVert(vecPos);
				break;
				*/

			default: ;

			}
		}
		break;

	case CL_MOUSE_RIGHT:
		
			//close the shape?
			if (m_pActiveLine)
			{
				int pointCount = m_pActiveLine->GetPointList()->size();
				if (pointCount > 1)
				{
					
					if (m_pActiveLine->GetPointList()->at(0) != m_pActiveLine->GetPointList()->at(pointCount-1))
					{
						//close the poly
						m_pActiveLine->GetPointList()->push_back(*m_pActiveLine->GetPointList()->begin());
						
					
					} else
					{
						//it already seems to be closed
					}
					
				}
			}
			break;
	}
}

void EntCollisionEditor::OnRadioButtonChange(CL_RadioButton *pSelectedButton)
{
	ClearSelection();

	for (int i=0; i < e_modeCount; i++)
	{
		if (pSelectedButton == m_pRadioArray[i])
		{
			m_mode = i;
		}
	}

	switch (m_mode)
	{
	case e_modeAddCreate:
		m_pLabel->set_text("Left click to add vertex points.");
		break;
	case e_modeAdjust:
		m_pLabel->set_text("Click and drag verts to move, tap DELETE while dragging\nto remove a vertex.  Use the arrow\nkeys to move everything.");
		break;
	}
}

void EntCollisionEditor::OnOk()
{
	//accept changes
	
	m_bDataChanged = true;
	*m_pCollisionData = m_col; //copy it back
	
	m_pCollisionData->SetDataChanged(true);

	LogMsg("Final collision edit data:");
	LogMsg( (string("Offset is ") + PrintVector(m_pCollisionData->GetLineList()->begin()->GetOffset())).c_str());

	line_list::iterator itor = m_pCollisionData->GetLineList()->begin();
	while (itor != m_pCollisionData->GetLineList()->end())
	{
		itor->ComputeConvexHull();
		itor->PrintPoints();
		itor++;
	}

	m_pCollisionData->CheckForErrors("", true);
	//OTE:  Check OnCollisionDataEditEnd in EntEditMode to see what else we do after ending an edit
	SetDeleteFlag(true);

}

void EntCollisionEditor::OnCancel()
{
	SetDeleteFlag(true);
}

void EntCollisionEditor::OnChangeLineType()
{

	if (m_pActiveLine && m_pListType->get_current_item() != -1)
	{
		m_pActiveLine->SetType(m_pListType->get_current_item());
	}
}

void EntCollisionEditor::OnClear()
{
	if (ConfirmMessage("Clear active shape", "Are you sure you want to clear the active shape?"))
	{
		if (m_pActiveLine)
		{
			m_pActiveLine->GetPointList()->clear();
			m_mode = e_modeAddCreate;
			m_pRadioGroup->set_checked(m_pRadioArray[m_mode]);
		}
	}
}

void EntCollisionEditor::SetCollisionLineLabel()
{
	m_pLabelCurLine->set_text("(collision shape "+ CL_String::from_int(m_curLine) + " of " + CL_String::from_int(
		m_col.GetLineList()->size())+")");

}


void EntCollisionEditor::Init(CL_Vector2 vPos, CL_Rect vEditBounds, CollisionData *pCollisionData, bool calcOffsets)
{
	m_trueEntityPosition = vPos;

	m_pCollisionData =  pCollisionData; //remember location of the original and hope it doesn't change
	m_col = *pCollisionData; //make a local copy

	m_rectArea = vEditBounds;
	
	m_pos = vPos;

	//create our GUI window at the bottom of the screen
	CL_Point ptSize(440,150);
	m_pWindow = new CL_Window(CL_Rect(0, 0, ptSize.x, ptSize.y), "Collision Edit Palette", 0, GetApp()->GetGUI());
	m_pWindow->set_position(200, GetScreenY-ptSize.y);

	int buttonOffsetX = 10;
	m_pButtonOK = new CL_Button(CL_Point(buttonOffsetX,ptSize.y-50), "Save Changes", m_pWindow->get_client_area());
	m_pButtonCancel = new CL_Button(CL_Point(buttonOffsetX+100,ptSize.y-50), "Cancel", m_pWindow->get_client_area());
	CL_Button *pButtonClear = new CL_Button(CL_Point(buttonOffsetX+200,ptSize.y-50), "Clear", m_pWindow->get_client_area());

	m_slots.connect(m_pButtonOK->sig_clicked(), this, &EntCollisionEditor::OnOk);
	m_slots.connect(m_pButtonCancel->sig_clicked(), this, &EntCollisionEditor::OnCancel);
	m_slots.connect(pButtonClear->sig_clicked(), this, &EntCollisionEditor::OnClear);

	int heightOfBut = 16;

	m_pRadioArray[e_modeAddCreate] = new CL_RadioButton(CL_Point(2,2), "Add Verts", m_pWindow->get_client_area());
	m_pRadioArray[e_modeAdjust] = new CL_RadioButton(CL_Point(2,2+ heightOfBut), "Adjust Verts", m_pWindow->get_client_area());
	//m_pRadioArray[e_modeDelete] = new CL_RadioButton(CL_Point(2,2 + (heightOfBut*2)), "Delete", m_pWindow->get_client_area());
	
	m_pRadioGroup = new CL_RadioGroup();
	
	for (int i=0; i < e_modeCount; i++)
	{
		m_pRadioGroup->add(m_pRadioArray[i], true);
	}

	m_slots.connect(m_pRadioGroup->sig_selection_changed(), this, &EntCollisionEditor::OnRadioButtonChange);

	CL_Rect r = m_pWindow->get_children_rect();
	//r.set_size(r.get_size() - CL_Size(5,28));

	CL_Size sz(140,120);
	int rightEdgeOffset = 10;
	int bottomEdgeOffset = 30;

	r.left = r.get_width()-sz.width;
	r.right -= rightEdgeOffset;

	r.top = r.get_height() - sz.height;
	r.bottom -= bottomEdgeOffset;

	m_pListType = new CL_ListBox(r, m_pWindow->get_client_area());
	
	//populate with our materials that were set from lua
	for (int i=0; i < g_materialManager.GetCount(); i++)
	{
		m_pListType->insert_item(g_materialManager.GetMaterial(i)->GetName());
	}
	
	//setup our own tip display
	m_pLabel = new CL_Label(CL_Point(100,1),"", m_pWindow->get_client_area());

	new CL_Label(CL_Point(ptSize.x-230,50),"Material Type:", m_pWindow->get_client_area());

	m_pWindow->set_event_passing(false);

	m_slots.connect( m_pListType->sig_selection_changed(), this, &EntCollisionEditor::OnChangeLineType);
	CL_Point offset = CL_Point(2,40);
	m_pCheckBoxSnap = new CL_CheckBox(offset, "Snap", m_pWindow->get_client_area());
	m_pCheckBoxSnap->set_checked(true);
	m_pCheckBoxSnap->set_focusable(false);

	offset.y += 15;
	m_pCheckBoxClipToImage = new CL_CheckBox(offset, "Clip to image", m_pWindow->get_client_area());
	
	m_pCheckBoxClipToImage ->set_focusable(false);


	offset.y += 15;

	CL_Button *pBut;
	pBut = new CL_Button(CL_Point(2,offset.y), "<", m_pWindow->get_client_area());
	m_slots.connect(pBut->sig_clicked(), this, &EntCollisionEditor::OnPrevLine);

	pBut = new CL_Button(CL_Point(2 + 2 + pBut->get_width(),offset.y), ">", m_pWindow->get_client_area());
	m_slots.connect(pBut->sig_clicked(), this, &EntCollisionEditor::OnNextLine);

	m_pLabelCurLine = new CL_Label(CL_Point(pBut->get_client_x() + pBut->get_width() + 2, offset.y), "",  m_pWindow->get_client_area());

	SetCollisionLineLabel();

	m_curLine = 1;

	SetupLineForEdit(m_curLine);

	SetClippedDefaultBasedOnLine();

}

void EntCollisionEditor::SetSnap(bool bOn)
{
	m_pCheckBoxSnap->set_checked(bOn);
}

void EntCollisionEditor::SetClip(bool bOn)
{
	m_pCheckBoxClipToImage->set_checked(bOn);
}

void EntCollisionEditor::SetClippedDefaultBasedOnLine()
{
	m_pCheckBoxClipToImage ->set_checked(true);
}

void EntCollisionEditor::SetupLineForEdit(int line)
{
	m_curLine = line;

	//find the correct one to edit, adding as we go

	line_list::iterator itor = m_col.GetLineList()->begin();

	for (int i=0;;i++)
	{
		if (itor == m_col.GetLineList()->end())
		{
			PointList pl;
			m_col.GetLineList()->push_back(pl);
			itor = m_col.GetLineList()->end();
			itor--;

			if (i == m_curLine-1)
			{
				break; //this is it
			}
			
			continue;
		}
		if (i == m_curLine-1)
		{
			break; //this is it
		}
		itor++;
	}
	
	m_pActiveLine = &(*itor);

	if (m_pActiveLine->GetPointList()->size() == 0)
	{
		m_mode = e_modeAddCreate;
	} else
	{
		m_mode = e_modeAdjust;
	}

	m_pListType->clear_selection();
	m_pListType->set_selected(m_pActiveLine->GetType(), true);
	
	m_pRadioGroup->set_checked(m_pRadioArray[m_mode]); //this will call OnRadioButtonChanged and set the help text too

	SetCollisionLineLabel();
}

void EntCollisionEditor::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext*) pTarget;
	DrawBullsEyeWorld(m_trueEntityPosition, CL_Color(255,255,0,200), 10, pGC);

	CL_Color col(255,0,0,255);
	RenderVectorCollisionData(m_pos, m_col, pGC, true, &col);
}

void EntCollisionEditor::Update(float step)
{
}


void EntCollisionEditor::OnPrevLine()
{
	int line = m_curLine-1;
	if (line == 0) line = m_col.GetLineList()->size();
	SetupLineForEdit(line);
}

void EntCollisionEditor::OnNextLine()
{
	SetupLineForEdit(++m_curLine);
}



void RenderVectorPointList(const CL_Vector2 &vecPos, PointList &pl, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride)
{

	CL_Vector2 a,b, firstVert;
	CL_Color col;

	col.color = g_materialManager.GetMaterial(pl.GetType())->GetColor();
	
	if (pColorOveride) col = *pColorOveride;

	unsigned int totalVerts = pl.GetPointList()->size();
	if ( totalVerts == 0)
	{
		return;
	}


	a = pl.GetPointList()->at(0);
	a += vecPos;
	
	
	a = g_pMapManager->GetActiveMapCache()->WorldToScreen(a);
    firstVert = a;

	if (bRenderVertBoxes)
	{
		DrawCenteredBox(a, C_COL_VERT_SIZE, col, pGC);
	}

	if (totalVerts == 1) return;

	unsigned int i=1;
	while(i < totalVerts)
	{
		b = pl.GetPointList()->at(i);
		b += vecPos;


		b = g_pMapManager->GetActiveMapCache()->WorldToScreen(b);

		if (bRenderVertBoxes)
		{
			DrawCenteredBox(b, C_COL_VERT_SIZE, col, pGC);
		}

		pGC->draw_line(a.x,a.y,b.x,b.y, col);
		a = b;
		i++;
	}
	
	//draw the last line to connect the last vert with the first
	
	a = firstVert;
	pGC->draw_line(b.x,b.y,a.x,a.y, col);

}



void EntCollisionEditor::ModifyShapePosition(CL_Point pt)
{

	int mult = 1;

	if (CL_Keyboard::get_keycode(CL_KEY_SHIFT))
	{
		mult = 5; //move much more if shift is pressed
	}

	m_col.ApplyOffsetToAll(CL_Vector2(-pt.x, -pt.y)*mult);

}

void RenderVectorCollisionData(const CL_Vector2 &vecPos, CollisionData &col, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride)
{
	line_list *pLineList = col.GetLineList();
	line_list::iterator itor = pLineList->begin();

	while (itor != pLineList->end())
	{
		RenderVectorPointList(vecPos, (*itor), pGC, bRenderVertBoxes, pColorOveride);
		itor++;
	}
}

void RenderTileListCollisionData(tile_list &tileList, CL_GraphicContext *pGC, bool bRenderVertBoxes, CL_Color *pColorOveride)
{
	
	tile_list::iterator itor = tileList.begin();

	CollisionData col, *pCol;

	while (itor != tileList.end())
	{
		CL_Vector2 vPos = (*itor)->GetPos();

		if ( (*itor)->GetType() == C_TILE_TYPE_ENTITY)
		{
			CollisionData *pCol = ((TileEntity*) (*itor))->GetEntity()->GetCollisionData();
			if (!pCol)
			{
				//no valid collision data, this can happen
				itor++;
				continue;
			}
				
		}
	
		if ((*itor)->UsesTileProperties() && (*itor)->GetType() == C_TILE_TYPE_PIC)
		{
			//we need a customized version
			CreateCollisionDataWithTileProperties((*itor), col);
			RenderVectorCollisionData(vPos, col, pGC, bRenderVertBoxes, pColorOveride);
		} else
		{

			pCol = (*itor)->GetCollisionData();
			if (pCol)
			RenderVectorCollisionData(vPos, *pCol, pGC, bRenderVertBoxes, pColorOveride);
		}
	
		itor++;
	}
}
