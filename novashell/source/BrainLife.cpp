#include "AppPrecomp.h"
#include "BrainLife.h"
#include "MovingEntity.h"

BrainLife registryInstanceBrainLife(NULL); //self register ourselves i nthe brain registry

BrainLife::BrainLife(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

BrainLife::~BrainLife()
{
}


void BrainLife::OnRemove()
{
	m_pParent->SetSizeOverride(false, CL_Vector2(0,0));
}
void BrainLife::OnAdd()
{
	m_pParent->SetAlignment(origin_top_left, CL_Vector2(0,0));
	m_thinkSpeedMS = 1;
	Init(100, 10);
	
}

void BrainLife::Init(int gridSizeSide, int cellSize)
{
	m_gridSizeSide = gridSizeSide;
	m_cellSize = cellSize;
	m_gridSizeTotal = m_gridSizeSide*m_gridSizeSide;
	m_grid.clear();
	m_grid.resize(m_gridSizeTotal, false);
	m_pParent->SetSizeOverride(true, CL_Vector2(m_gridSizeSide*m_cellSize,m_gridSizeSide*m_cellSize));
	m_thinkTimer = GetApp()->GetGameTick() + m_thinkSpeedMS;
}

void BrainLife::Update(float step)
{

	if (m_thinkSpeedMS == 0) return;

	if (m_thinkTimer < GetApp()->GetGameTick())
	{

		//time to think again
		m_thinkTimer = GetApp()->GetGameTick()+m_thinkSpeedMS;
		int neighbors;
		m_gridBack = m_grid;
		for (int x=1; x < m_gridSizeSide-2; x++)
		{
			for (int y=1; y < m_gridSizeSide-2; y++)
			{
				neighbors = 0;
				
				if (m_gridBack[GetCellIndex(x-1,y)]) neighbors++; //found an active neighbor
				if (m_gridBack[GetCellIndex(x+1,y)]) neighbors++; //found an active neighbor
				if (m_gridBack[GetCellIndex(x,y-1)]) neighbors++; //found an active neighbor
				if (m_gridBack[GetCellIndex(x,y+1)]) neighbors++; //found an active neighbor

				//diagonals
				if (m_gridBack[GetCellIndex(x-1,y-1)]) neighbors++; //found an active neighbor
				if (m_gridBack[GetCellIndex(x+1,y-1)]) neighbors++; //found an active neighbor

				if (m_gridBack[GetCellIndex(x-1,y+1)]) neighbors++; //found an active neighbor
				if (m_gridBack[GetCellIndex(x+1,y+1)]) neighbors++; //found an active neighbor

				if (neighbors < 2 || neighbors > 3)
				{
					m_grid[GetCellIndex(x,y)] = false; //dies from under or over population
					continue;
				}

				if (neighbors == 3)
				{
					m_grid[GetCellIndex(x,y)] = true; //spawn a new one, assuming it wasn't already alive
					continue;
				}

			}
		}
	}
}


void BrainLife::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	
	CL_Vector2 vVisualPos;
	CL_Vector2 vFinalScale;

	vFinalScale = GetCamera->GetScale();
	vFinalScale.x *= GetParent()->GetScale().x;
	vFinalScale.y *= GetParent()->GetScale().y;

	EntMapCache *pWorldCache = GetParent()->GetMap()->GetMyMapCache();

	float scaledCellSize = m_cellSize*GetParent()->GetScale().x;

	CL_Rectf r(0,0, m_cellSize*vFinalScale.x,m_cellSize*vFinalScale.y);
	 
	
	for (int x=0; x < m_gridSizeSide; x++)
	{
		for (int y=0; y < m_gridSizeSide; y++)
		{
			vVisualPos = pWorldCache->WorldToScreen(GetParent()->GetPos() + CL_Vector2(x*scaledCellSize, y*scaledCellSize));

			if (m_grid[GetCellIndex(x,y)])
			{
				//cell is alive
				pGC->fill_rect(r + CL_Pointf(vVisualPos.x, vVisualPos.y), CL_Color(255,255,255,255));
				//pGC->draw_rect(r + CL_Pointf(vVisualPos.x, vVisualPos.y), CL_Color(0,0,0,255));
			} else
			{
				//cell is dead
			}

		}
	}
	
}

void BrainLife::AddRandomCells(int count)
{
	int x,y;
	
	for (int i=0; i < count; i++)
	{
		x = random(m_gridSizeSide);
		y = random(m_gridSizeSide);
		m_grid[GetCellIndex(x,y)] = true;
	}
}


void BrainLife::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);
		
			if (words[0] == "init")
			{
				Init(CL_String::to_int(words[1]), CL_String::to_int(words[2]));

			} else
				if (words[0] == "add_random")
				{
					
					AddRandomCells(CL_String::to_int(words[1]));

				} else
					if (words[0] == "set_cell")
					{
						m_grid[GetCellIndex(CL_String::to_int(words[1]), CL_String::to_int(words[2]))] = CL_String::to_bool( words[3]);

					} else
						if (words[0] == "set_update_ms")
						{
							m_thinkSpeedMS = CL_String::to_int(words[1]);
							m_thinkTimer = GetApp()->GetGameTick()+m_thinkSpeedMS;

						} else

					{
						LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
					}
	}
}

