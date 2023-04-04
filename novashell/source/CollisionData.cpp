#include "AppPrecomp.h"
#include "CollisionData.h"
#include "GameLogic.h"
#include "MaterialManager.h"
CollisionData::CollisionData(const CL_Rect &rec)
{
	SetDefaults(); 
	m_rect = rec;
}

void CollisionData::SetDefaults()
{
	m_dataChanged = false;
	m_vecScale = CL_Vector2(1,1);
	m_bNeedsRecalc = true;
	m_vecCombinedOffset = CL_Vector2(0,0);

}


CollisionData::CollisionData()
{
	SetDefaults();
}

CollisionData::~CollisionData()
{
	SaveIfNeeded();
}

const CL_Rectf & CollisionData::GetCombinedCollisionRect()
{
	if (m_bNeedsRecalc)
	{
		RecalculateCombinedOffsets();
	}

	return m_collisionRect;
}

void CollisionData::SetScale(const CL_Vector2 &vScale)
{
	
	if (m_vecScale != CL_Vector2(1,1))
	{
		//remove current scaling
		ApplyScaleToAll(CL_Vector2(1 /m_vecScale.x, 1 / m_vecScale.y));
	}
	m_vecScale = vScale;

	if (m_vecScale != CL_Vector2(1,1))
	{
		ApplyScaleToAll(m_vecScale);

	}
	SetRequestRectRecalculation();
}

void CollisionData::Load(const string &fileName)
{
	m_dataChanged = false;
	SetRequestRectRecalculation();
	m_fileName = fileName;
	m_vecScale = CL_Vector2(1,1);

	CL_InputSource *pFile = g_VFManager.GetFile(fileName);

	if (!pFile)
	{
		return;
	}

	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

    Serialize(helper);
	SAFE_DELETE(pFile);

	//check it..

	CheckForErrors(fileName, true);
	


}

bool CollisionData::CheckForErrors(string extraText, bool bShow)
{
	//check them for errors?
	bool bOk = true;
	line_list::iterator listItor = m_lineList.begin();
	int lineCount = 0;
	while (listItor != m_lineList.end())
	{
		lineCount++;
		PointList::eVertCheckStatus result = listItor->IsValidBox2DPolygon();

		if (result != PointList::STATUS_OK)
		{
			bOk = false;
			if (bShow)
			{
				LogError("Warning: Box2d doesn't like collision line %d in %s. (%s)", lineCount, extraText.c_str(), listItor->ErrorToString(result).c_str());
			} else
			{
				
			}
		} else
		{
			//LogMsg("%s is ok", extraText.c_str());
		}

		listItor++;
	}
	return bOk;
}

void CollisionData::SaveIfNeeded()
{
	if (m_fileName.empty() || !m_dataChanged) return;

	SetScale(CL_Vector2(1,1)); //remove any scaling operations we had done

    CL_OutputSource *pFile =g_VFManager.PutFile(m_fileName);
	CL_FileHelper helper(pFile); 

	Serialize(helper);

	m_dataChanged = false;
	SAFE_DELETE(pFile);
}


void CollisionData::ApplyScaleToAll(const CL_Vector2 &vScale)
{
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		listItor->ApplyScale(vScale);
		listItor++;
	}

	SetRequestRectRecalculation();
}

void CollisionData::ApplyOffsetToAll(const CL_Vector2 &vOffset)
{
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		listItor->ApplyOffset(vOffset);
		listItor++;
	}

	SetRequestRectRecalculation();
}


const CL_Vector2 & CollisionData::GetCombinedOffsets()
{
	assert(m_vecCombinedOffset == CL_Vector2::ZERO);
	return m_vecCombinedOffset;
	/*
	if (m_bNeedsRecalc)
	{
		RecalculateCombinedOffsets();
	}
	
	return m_vecCombinedOffset;
	*/
}
int CollisionData::GetVertCount()
{
	line_list::iterator listItor = m_lineList.begin();

	int count = 0;

	while (listItor != m_lineList.end())
	{
		count += listItor->GetPointList()->size();
		listItor++;
	}

	return count;
}

void CollisionData::RecalculateCombinedOffsets()
{
	//m_vecCombinedOffset = CL_Vector2(100000,100000);
	m_collisionRect = CL_Rectf(FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX);

	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		
		if (listItor->HasData())
		{
		///	m_vecCombinedOffset.x = min(m_vecCombinedOffset.x, listItor->GetOffset().x);
		//	m_vecCombinedOffset.y = min(m_vecCombinedOffset.y, listItor->GetOffset().y);

			
			if (g_materialManager.GetMaterial(listItor->GetType())->GetType() == CMaterial::C_MATERIAL_TYPE_NORMAL)
			{
				//it's a hardness shape, include it in our overall rect
				m_collisionRect.left = min(m_collisionRect.left, listItor->GetRect().left);
				m_collisionRect.right = max(m_collisionRect.right, listItor->GetRect().right);

				m_collisionRect.top = min(m_collisionRect.top, listItor->GetRect().top);
				m_collisionRect.bottom = max(m_collisionRect.bottom, listItor->GetRect().bottom);

			}
		}
		listItor++;
	}

	m_bNeedsRecalc = false;

	if (m_collisionRect.left == FLT_MAX)
	{
		//uh oh, has no collision data.  At least let's make it 0's
		m_collisionRect = CL_Rectf(0,0,0,0);
	}

	/*
	if (m_vecCombinedOffset.x == 100000)
	{
		//must be invalid data
		m_vecCombinedOffset = CL_Vector2(0,0);
	}
	*/
	
}



void CollisionData::RemoveOffsets()
{
	return;
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		listItor->RemoveOffsets();
		listItor++;
	}

	m_vecCombinedOffset = CL_Vector2(0,0);
	SetRequestRectRecalculation();
}

bool CollisionData::HasData()
{
	
	if (m_lineList.empty()) return false;
	line_list::iterator listItor = m_lineList.begin();

	while (listItor != m_lineList.end())
	{
		if (!listItor->GetPointList()->empty()) return true;
		listItor++;
	}

	return false;
}

int CollisionData::CountValidLineLists()
{
	int lineLists = 0;

	line_list::iterator listItor = m_lineList.begin();
	while (listItor != m_lineList.end())
	{
		if ( listItor->GetPointList()->size() > 0)
		{
			lineLists++;
		}
		listItor++;
	}

	return lineLists;
}

void CollisionData::Serialize(CL_FileHelper &helper)
{
  //save out our rect and points
	helper.process(m_rect);

	if (helper.IsWriting())
	{

		cl_uint32 validLines = CountValidLineLists();
		assert(validLines > 0);
		//write how many linelists are coming

		helper.process_const(validLines);

		line_list::iterator listItor = m_lineList.begin();

		while (listItor != m_lineList.end())
		{
			if ( listItor->GetPointList()->size() > 0)
			{
				//write data about this PointList
				helper.process_const(listItor->GetType());
				
				//write how many points are in this list
				helper.process_const(cl_uint32(listItor->GetPointList()->size()));
				CL_Vector2 offsetUnused;
				//helper.process(listItor->GetOffset());
				helper.process(offsetUnused);
				//write 'em
				helper.process_array(&listItor->GetPointList()->at(0), listItor->GetPointList()->size());
			}
		
			listItor++;
		}
	} else
	{
		//we're reading
		//first read how many lists we've got
		unsigned int listCount;
		helper.process(listCount);
		unsigned int vecCount;
		int pointListType;

		CL_Vector2 a,b;

		for (unsigned int i=0; i < listCount; i++)
		{
			PointList ptList;
			helper.process(pointListType);
			ptList.SetType(pointListType);
			helper.process(vecCount); //how many points this list has
			if (vecCount != 0)
			{

				CL_Vector2 vTemp;
				helper.process(vTemp);
				m_lineList.push_back(ptList);
				//m_lineList.rbegin()->SetOffset(vTemp);

				m_lineList.rbegin()->GetPointList()->resize(vecCount);
				helper.process_array(&m_lineList.rbegin()->GetPointList()->at(0), vecCount);


			}
		}
		
	}
}
 
void CollisionData::Clear()
{
	m_lineList.clear();
	SetRequestRectRecalculation();
	SetDataChanged(true);
}


void CreateCollisionDataWithTileProperties(Tile *pTile, CollisionData &colOut)
{
	
	//perform the copy
	colOut = *pTile->GetCollisionData();

	line_list *pLineList = colOut.GetLineList();
	line_list::iterator itor = pLineList->begin();
	CL_Vector2 vecTemp;
	
    //modify it in place

	while (itor != pLineList->end())
	{
		CL_Rectf picBounds = pTile->GetBoundsRect();
		bool bNeedsReverse = false;
		
		if (pTile->GetBit(Tile::e_flippedX))
		{
			bNeedsReverse = !bNeedsReverse;
		}

		if (pTile->GetBit(Tile::e_flippedY))
		{
			bNeedsReverse = !bNeedsReverse;
		}
		for (unsigned int i=0; i < itor->GetPointList()->size(); i++)
		{
			CL_Vector2 *p = &itor->GetPointList()->at(i);
			
			//do each vert
			if (pTile->GetBit(Tile::e_flippedX))
			{
				p->x = picBounds.right + (-p->x);
			}

			if (pTile->GetBit(Tile::e_flippedY))
			{
				p->y = picBounds.bottom + (-p->y);
			}
		}

		if (bNeedsReverse)
		{
			//reverse the winding so box2d won't get mad
			std::reverse(itor->GetPointList()->begin(), itor->GetPointList()->end());
		}
		itor++;
	}
	
}