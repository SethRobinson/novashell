//  ***************************************************************
//  BrainDoomWall - Creation date: 04/18/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainDoomWall_h__
#define BrainDoomWall_h__

#include "Brain.h"

class BrainDoomWall: public Brain
{
public:
	BrainDoomWall(MovingEntity *pParent);
	virtual ~BrainDoomWall();
	virtual void Update(float step);
	virtual const char * GetName() {return "DoomWall";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainDoomWall(pParent);}
	virtual void Render(void *pTarget);
	virtual void OnAdd(); //called once when brain is inserted
	virtual void OnRemove();
	virtual void HandleMsg(const string &msg);
	virtual string HandleAskMsg(const string &msg);
	bool IsValidGridCoord(int x, int y) {if (x < 0 || y < 0 || x >= m_gridSizeSide || y >= m_gridSizeSide) return false; return true;}

	
protected:
	
void EndCustomRender();
int GetCellIndex(int x, int y) {return y*m_gridSizeSide+x;}
int GetShapeCellIndex(int x, int y) {return y*m_shapeSizeSide+x;}
void ChangeRandomCells(int count, bool bNew);
void SetupForCustomRender();
void RenderWall(CL_Color col, float explodePercent = 0);
void RenderHighlight(CL_Point tilePos);
void ConstructLevel();
bool CanBeSolvedInOneMove();
bool CanBeSolvedInOneMoveFromThisPosition(int x, int y);
bool IsSolvedAtThisPosition(int gridX, int gridY, int &hitsOut, int &missesOut);
void CountShapeCells();
bool IsSolved();
void RenderGUIOverlay();
void Draw2DShape(int posX, int posY, int cellSize);
void RenderPlay();
void RenderWon();
void RenderShape(CL_Color c);
void RenderLose();
void ComputeEdgeDataForBlock(int x, int y, bool *bEdgeData);
bool BlockExistsAt(int x, int y);

enum eMode
{
	MODE_PLAY,
	MODE_WON,
	MODE_LOST
};

private:
	
	vector<bool> m_grid;
	vector<bool> m_shape;

	int m_gridSizeSide;
	int m_gridSizeTotal;
	float m_cellSize;
	int m_shapeSizeSide;
	int m_shapeCellsActive; //how many are actually turned on
	float m_farDist, m_dieDist;
	int m_timeLimit;
	int m_ApproachTimer;
	CL_Point m_highlight;
	eMode m_mode;
	int m_endTimeLimit;
	bool m_bWallsBlock; //if true, the cursor can't navigate through holes
	int m_finalX, m_finalY; //where our block is going to hit
	unsigned int m_blockStoppedTimer;
	int m_endDieTimeLimit;
	CL_Color m_color;
};

#endif // BrainDoomWall_h__