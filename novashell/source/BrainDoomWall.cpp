#include "AppPrecomp.h"
#include "BrainDoomWall.h"
#include "MovingEntity.h"

BrainDoomWall registryInstanceBrainDoomWall(NULL); //self register ourselves in the brain registry

BrainDoomWall::BrainDoomWall(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}
}

BrainDoomWall::~BrainDoomWall()
{
}

void BrainDoomWall::Update(float step)
{

}

void BrainDoomWall::OnRemove()
{
	m_pParent->SetSizeOverride(false, CL_Vector2(0,0));
}

void BrainDoomWall::ChangeRandomCells(int count, bool bNew)
{
	int x,y;

	for (int i=0; i < count; i++)
	{

		//to let it spawn blocks around the edges too
		//x = random_range(m_gridSizeSide);
		//y = random(m_gridSizeSide);

		x = random_range(1, m_gridSizeSide-2);
		y = random_range(1, m_gridSizeSide-2);
		m_grid[GetCellIndex(x,y)] = bNew;
	}
}
void BrainDoomWall::OnAdd()
{
	m_pParent->SetSizeOverride(true, CL_Vector2(2000,2000)); //big enough for whatever, so we will always get rendered
	m_gridSizeSide = 0;
	m_cellSize = 1;
	m_gridSizeTotal = m_gridSizeSide*m_gridSizeSide;
	m_highlight.x = -1; m_highlight.y = -1;
	m_shapeSizeSide = 0;
	m_timeLimit = 10000;
	m_farDist = -40;
	m_dieDist = 10;
	m_mode = MODE_PLAY;
	m_endTimeLimit = 1000;
	m_endDieTimeLimit = 1000;
	m_color = CL_Color(255,0,0,255);
}

bool BrainDoomWall::IsSolvedAtThisPosition(int gridX, int gridY, int &hitsOut, int &missesOut)
{
	int misses = 0;
	int hits = 0;

	for (int x=0; x < m_shapeSizeSide; x++)
		for (int y=0; y < m_shapeSizeSide; y++)
		{
			if (m_shape[GetShapeCellIndex(x,y)])
			{
				if (!IsValidGridCoord(gridX+x, gridY+y)) continue;
			} else
			{
				continue;
			}

			if (m_grid[GetCellIndex(gridX+x, gridY+y)])
			{
				misses++;
			} else
			{
				hits++;
			}
		}

		hitsOut = hits;
		missesOut = misses;
		return hits == m_shapeCellsActive;
}


bool BrainDoomWall::CanBeSolvedInOneMoveFromThisPosition(int x, int y)
{

	int hits,misses;

	if (IsSolvedAtThisPosition(x,y, hits, misses))
	{
		return true;
	}

	if (misses == 1 && m_shapeCellsActive-hits <= misses)
	{
		//it is possible to complete this with one more piece
		return true;
	}

	return false;
}

bool BrainDoomWall::CanBeSolvedInOneMove()
{
	//slow as hell, but doesn't really matter, does it?
	
	for (int x=0; x < m_gridSizeSide; x++)
	{
		for (int y=0; y < m_gridSizeSide; y++)
		{
			if (CanBeSolvedInOneMoveFromThisPosition(x,y))
			{
				return true;
			}
		}
	}

	return false; //nope, can't be done
}

bool BrainDoomWall::IsSolved()
{
	//slow as hell, but doesn't really matter, does it?
	int hits, misses;

	for (int x=0; x < m_gridSizeSide; x++)
	{
		for (int y=0; y < m_gridSizeSide; y++)
		{
			if (IsSolvedAtThisPosition(x,y, hits, misses))
			{
				m_finalX = x;
				m_finalY = y;

				return true;
			}
		}
	}

	return false; //nope, can't be done
}

void BrainDoomWall::CountShapeCells()
{
	m_shapeCellsActive = 0;

	for (int i=0; i < m_shapeSizeSide*m_shapeSizeSide; i++)
	{
		if (m_shape[i]) m_shapeCellsActive++;
	}
}

void RenderCube(float x, float y, float z, float size, CL_Color col = CL_Color(255,255,255,255), bool *bEdgeInfo=NULL)
{
	size /= 2;

	glPushMatrix();
	glTranslatef(x,-y,z);			// Move Into The Screen And Left
	
	static GLfloat vNormals[] = {
		0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0,
		0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
		0.0, -1.0, 0.0,   0.0, -1.0, 0.0,   0.0, -1.0, 0.0,   0.0, -1.0, 0.0,
		1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		-1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0
	};

	const float vVertices[] = 
	{
		-size, -size,  size, size, -size,  size, size,  size,  size, -size,  size,  size, //front
		-size, -size, -size, -size,  size, -size, size,  size, -size, size, -size, -size, //back
		-size,  size, -size, -size,  size,  size , size,  size,  size,  size,  size, -size, //top
		-size, -size, -size, size, -size, -size, size, -size,  size, -size, -size,  size, //bottom
		size, -size, -size, size,  size, -size,  size,  size,  size,  size, -size,  size, //right
		-size, -size, -size, -size, -size,  size, -size,  size,  size, -size,  size, -size //left
	};

	//glDisable(GL_CULL_FACE);
	glVertexPointer(3, GL_FLOAT,  0, vVertices);
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisable(GL_TEXTURE_2D);
	
	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer(GL_FLOAT, 0, vNormals);
	
	
	if (col != CL_Color(255,255,255,255))
	{
		//need to handle custom coloring
		//why doesn't glColor4b work like I thought?
		glColor4f( float(col.get_red())/255,  float(col.get_green())/255, float(col.get_blue())/255, float(col.get_alpha())/255);
		
		if (col.get_alpha() != 255)
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_SRC_COLOR  );
			glEnable(GL_ALPHA);
			glDepthMask(GL_FALSE);
			glDisable(GL_DEPTH_TEST);
			}
		}
	
	
	if (bEdgeInfo && col.get_alpha() == 255)
	{
		const int topOffset = 4*2;
		const int bottompOffset = 4*3;
		const int rightOffset = 4*4;
		const int leftOffset = 4*5;

		glDrawArrays(GL_QUADS, 0, 4*2); //first just draw the top and bottom

		//only render the sides that need it, to stop zfighting and allow stuff to look cooler
		if (bEdgeInfo[0]) glDrawArrays(GL_QUADS, topOffset, 4); //first just draw the top and bottom
		if (bEdgeInfo[1]) glDrawArrays(GL_QUADS, bottompOffset, 4); //first just draw the top and bottom
		if (bEdgeInfo[2]) glDrawArrays(GL_QUADS, rightOffset, 4); //first just draw the top and bottom
		if (bEdgeInfo[3]) glDrawArrays(GL_QUADS, leftOffset, 4); //first just draw the top and bottom
	} else
	{
		glDrawArrays(GL_QUADS, 0, 4*6);
	}

	
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnable(GL_TEXTURE_2D);
	glDisableClientState(GL_NORMAL_ARRAY);	

	if (col != CL_Color(255,255,255,255))
	{
		glDisable(GL_ALPHA);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glColor4f( 1,  1, 1, 1);
		glEnable(GL_LIGHTING);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glPopMatrix();
}

void BrainDoomWall::SetupForCustomRender()
{
	CL_Display::flush();
	
	glEnable(GL_DEPTH_TEST);				
	glClear(GL_DEPTH_BUFFER_BIT);		
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	GLfloat LightAmbient[]= { 0.5f, 0.5f, 0.5f, 1.0f }; 
	GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };	
	GLfloat LightPosition[]= { 0.0f, 0.0f, 10.0f, 1.0f };	
	
	glShadeModel (GL_SMOOTH);
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);	
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);

	glEnable(GL_LIGHT1);
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)GetScreenX/(GLfloat)GetScreenY,0.1f,500.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();						// Reset The View
	glEnable ( GL_COLOR_MATERIAL ) ;
	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;


}
void BrainDoomWall::EndCustomRender()
{
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_DEPTH_TEST);				// Enables Depth Testing
	glDisable(GL_LIGHTING);
}

void BrainDoomWall::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);
		if (words[0] == "set_shape_size")
		{
			m_shapeSizeSide = CL_String::to_int(words[1]);
			m_shape.clear();
			m_shape.resize(m_shapeSizeSide*m_shapeSizeSide, false);

		} else

		if (words[0] == "set_shape")
		{
			m_shape[GetShapeCellIndex(CL_String::to_int(words[1]), CL_String::to_int(words[2]))]=true;

		} else
			if (words[0] == "set_color")
			{
				m_color = CL_Color(CL_String::to_int(words[1]), CL_String::to_int(words[2]), CL_String::to_int(words[3]), CL_String::to_int(words[4]));

			} else
		if (words[0] == "walls_block")
			{
				m_bWallsBlock = CL_String::to_bool(words[1]);

			} else

		if (words[0] == "gridSize")
		{
			m_gridSizeSide = CL_String::to_int(words[1]);
			m_gridSizeTotal = m_gridSizeSide*m_gridSizeSide;
			ConstructLevel();

		} else
			if (words[0] == "x")
			{
				m_highlight.x = CL_String::to_int(words[1]);
			} else

				if (words[0] == "y")
				{
					m_highlight.y = CL_String::to_int(words[1]);

				} else
					
				{
					LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
				}
	}
}

bool BrainDoomWall::BlockExistsAt(int x, int y)
{
	return (IsValidGridCoord(x,y) && m_grid[GetCellIndex(x,y)]);
}

void BrainDoomWall::ComputeEdgeDataForBlock(int x, int y, bool *bEdgeData)
{
	bEdgeData[0] = !BlockExistsAt(x,y-1);  //top
	bEdgeData[1] = !BlockExistsAt(x,y+1);  //bottom
	bEdgeData[2] = !BlockExistsAt(x+1,y); //right
	bEdgeData[3] = !BlockExistsAt(x-1,y); //left
}

void BrainDoomWall::RenderWall(CL_Color c, float explodePercent)
{
	for (int x=0; x < m_gridSizeSide; x++)
	{
		for (int y=0; y < m_gridSizeSide; y++)
		{
			if (m_grid[GetCellIndex(x,y)])
			{
				if (explodePercent == 0)
				{
					bool bEdgeData[4];
					ComputeEdgeDataForBlock(x,y, bEdgeData);
					RenderCube(x*m_cellSize, y*m_cellSize, 0, m_cellSize, c, bEdgeData);
				} else
				{
					//special way to explode...
					float myX = x*m_cellSize;
					float myY = y*m_cellSize;

					float xPercent = (x / (float(m_gridSizeSide)/2));
					float yPercent = (y / (float(m_gridSizeSide)/2));

					xPercent -= 1;
					yPercent -= 1;
					glPushMatrix();
					glRotatef(xPercent*explodePercent*1, 1,0,0);
					glRotatef(xPercent*explodePercent*2, 0,1,0);
					glRotatef(xPercent*explodePercent*3, 0,0,1);

					RenderCube(myX + (xPercent*explodePercent), myY +(yPercent*explodePercent), 0, m_cellSize, c);

					glPopMatrix();
				}
				
			} else
			{
			}

		}
	}
}

void BrainDoomWall::RenderHighlight(CL_Point tilePos)
{
	if (tilePos.x < 0 || tilePos.y < 0) return; //invalid

	RenderCube(tilePos.x*m_cellSize, tilePos.y*m_cellSize, m_cellSize*0.3, m_cellSize, CL_Color(255,0,0,100));
}

void BrainDoomWall::ConstructLevel()
{
	m_grid.clear();
	m_grid.resize(m_gridSizeTotal, true);

	m_cellSize = 10/float(m_gridSizeSide); //auto scale things to fit the screen better

	CountShapeCells();
	m_pParent->SetAlignment(origin_top_left, CL_Vector2(0,0));
	m_pParent->SetSizeOverride(true, CL_Vector2(m_gridSizeSide*m_cellSize,m_gridSizeSide*m_cellSize));
	m_highlight = CL_Point(m_gridSizeTotal/2,m_gridSizeTotal/2);

	while (!CanBeSolvedInOneMove())
	{
		ChangeRandomCells(1, false);
	}
	m_ApproachTimer = GetApp()->GetGameTick()+m_timeLimit;
	m_mode = MODE_PLAY;
}

void BrainDoomWall::RenderPlay()
{
	float percent = float(m_ApproachTimer-GetApp()->GetGameTick())/float(m_timeLimit);
	//LogMsg("Percent: %.2f", percent);
	float dist = m_dieDist+ ((m_farDist-m_dieDist)*percent);
	if (m_shapeSizeSide < 1)
	{
		dist = 0; //let it go forever, it's attract mode
	}
	
	glTranslatef(-5,5, dist);	 //camera location

	//for looks only
	glPushMatrix();

	glRotatef(SinGamePulseByMS(35000)*20, 0,0,1);

	glTranslatef(-80, 80, -200);	 //camera location
	glScalef(30,(SinGamePulseByMS(8000)*6) +30,30);
	RenderWall(CL_Color(255,0, (SinGamePulseByMS(1000)*80) +80,(SinGamePulseByMS(4000)*20)+40));
	glPopMatrix();

	//game field
	glPushMatrix();
	glRotatef(SinGamePulseByMS(2000)*5, 1,0,0);
	glRotatef(SinGamePulseByMS(3000)*5, 0,1,0);

	if (m_shapeSizeSide != 0) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		RenderWall(m_color);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//RenderWall(CL_Color(0,0,255,255));
		RenderHighlight(m_highlight);
	} else
	{
		//attract mode?
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopMatrix();
}


void BrainDoomWall::RenderShape(CL_Color c)
{

	for (int x=0; x < m_shapeSizeSide; x++)
	{
		for (int y=0; y < m_shapeSizeSide; y++)
		{
			if (m_shape[GetShapeCellIndex(x,y)])
			{
				RenderCube((m_finalX+x)*m_cellSize, (m_finalY+y)*m_cellSize, 0, m_cellSize, c);
			} 
		}
	}
}

void BrainDoomWall::RenderWon()
{
	float percent = float(m_ApproachTimer-GetApp()->GetGameTick())/float(m_endTimeLimit);
	//LogMsg("Percent: %.2f", percent);
	float dist = -3+ ((6)*percent);
	

	//for looks only, that weird bg thing
	glPushMatrix();
	glRotatef(SinGamePulseByMS(25000)*20, 0,0,1);
	glRotatef(SinGamePulseByMS(15000)*20, 0,1,0);

	glTranslatef(-80, 80, -200);	 //camera location
	glScalef(30,(SinGamePulseByMS(8000)*6) +30,30);
	RenderWall(CL_Color(255,0, (SinGamePulseByMS(1000)*80) +80,(SinGamePulseByMS(4000)*20)+40));
	glPopMatrix();


	glTranslatef(dist,9, -50);	 //camera location
	glRotatef(290, 0,1,0);
	glScalef(2.6, 2.6,2.6);

	
	//game field
	glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	RenderWall(m_color);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glTranslatef(0, 0, dist);	 //camera location

	RenderShape(CL_Color(255,255,0,255));

	glPopMatrix();
}


void BrainDoomWall::RenderLose()
{

	float percent = float(m_ApproachTimer-int(GetApp()->GetGameTick()))/float(m_endDieTimeLimit);
	//LogMsg("Percent: %.2f", percent);
	float dist = 3 *percent;

	float distTemp = dist;
	dist = cl_max(1, dist); //make it stop in the middle
	float expPercent = 0;
	
	if (dist != distTemp)
	{
		if (m_blockStoppedTimer == 0)
		{
			m_blockStoppedTimer = GetApp()->GetGameTick();
		}
		expPercent = ( float(GetApp()->GetGameTick() - m_blockStoppedTimer)) / 100;
	}
	
	//for looks only, that weird bg thing
	glPushMatrix();
	glRotatef(SinGamePulseByMS(25000)*20, 0,0,1);
	glRotatef(SinGamePulseByMS(15000)*20, 0,1,0);
	
	
	glTranslatef(-80, 80, -200);	 //camera location
	glScalef(30,(SinGamePulseByMS(8000)*6) +30,30);
	RenderWall(CL_Color(255,0, (SinGamePulseByMS(1000)*80) +80,(SinGamePulseByMS(4000)*20)+40));
	glPopMatrix();


	glTranslatef(dist,9, -50);	 //camera location
	glRotatef(290, 0,1,0);
	glScalef(2.6, 2.6,2.6);


	//game field
	glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	RenderWall(m_color, expPercent);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glTranslatef(0, 0, dist);	 //camera location

	RenderShape(CL_Color(255,255,0,255));

	
	glPopMatrix();
}

void BrainDoomWall::Render(void *pTarget)
{
	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	CL_OpenGLState gl_state(CL_Display::get_current_window()->get_gc());

	SetupForCustomRender();

	switch (m_mode)
	{
	case MODE_PLAY:
		RenderPlay();
		break;

	case MODE_WON:
		RenderWon();
		break;

	case MODE_LOST:
		RenderLose();
		break;
	}

	EndCustomRender();
	
	gl_state.set_active();
	RenderGUIOverlay();
}

void BrainDoomWall::Draw2DShape(int posX, int posY, int cellSize)
{
	for (int x=0; x < m_shapeSizeSide; x++)
		for (int y=0; y < m_shapeSizeSide; y++)
		{
			if (!m_shape[GetShapeCellIndex(x,y)]) continue;

			CL_Rectf rec;
			rec.left = posX + x*cellSize;
			rec.top = posY + y*cellSize;
			rec.right = rec.left+cellSize;
			rec.bottom = rec.top + cellSize;
			CL_Display::fill_rect(rec, CL_Color(255,255,0,40));
			CL_Display::draw_rect(rec, CL_Color(255,255,0,180));
		}
}

void BrainDoomWall::RenderGUIOverlay()
{
	if (m_shapeSizeSide < 1) return;

	int x = 10;
	int y = GetScreenY-120;
	//GetApp()->GetFont(C_FONT_NORMAL)->draw(x,y, "Shape");

	Draw2DShape(x, y+20, 95/m_shapeSizeSide);
}

std::string BrainDoomWall::HandleAskMsg( const string &msg )
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	CL_Point desiredHighlight;

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);
		
		if (words[0] == "is_solved")
		{
			if (IsSolved())
			{
				m_mode = MODE_WON;
				m_ApproachTimer = GetApp()->GetGameTick()+m_endTimeLimit;
			} else
			{
				m_mode = MODE_LOST; //we're assuming they only get one chance to kill a block here..
				m_ApproachTimer = GetApp()->GetGameTick()+m_endDieTimeLimit;
				m_blockStoppedTimer = 0;
				//we're we'll have the shape crash into it
				m_finalX = m_highlight.x;
				m_finalY = m_highlight.y;
			}
			
			if (m_mode == MODE_WON)
			{
				return "true";
			}
			return "false";

		} else
		
			if (words[0] == "destroy_highlighted")
		{

			if (m_grid.size() > 0 && m_grid[GetCellIndex(m_highlight.x, m_highlight.y)])
			{
				//let's do it
				m_grid[GetCellIndex(m_highlight.x, m_highlight.y)] = false;
				
				return "true";
			} else
			{
				return "false";
			}
			
		} else

			if (words[0] == "x")
			{
				desiredHighlight.x = CL_String::to_int(words[1]);
			} else

				if (words[0] == "y")
				{
					desiredHighlight.y = CL_String::to_int(words[1]);

				} else
				{
					LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
				}
	}

	//yes we can move here
	
	if (IsValidGridCoord(desiredHighlight.x, desiredHighlight.y))
	{
		if (m_bWallsBlock)
		{
			//this turned out to be too hard, so I made it optional that you can't move through holes
			if (!m_grid[GetCellIndex(desiredHighlight.x, desiredHighlight.y)])
			{
				return "false";
			}
		}
		
		m_highlight = desiredHighlight;
		return "true";
	} 

	return "false";
}