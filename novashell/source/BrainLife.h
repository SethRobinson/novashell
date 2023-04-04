//  ***************************************************************
//  BrainLife - Creation date: 04/17/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainLife_h__
#define BrainLife_h__

#include "Brain.h"

class BrainLife: public Brain
{
public:
	BrainLife(MovingEntity *pParent);
	virtual ~BrainLife();
	virtual void Update(float step);
	virtual const char * GetName() {return "Life";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainLife(pParent);}
	virtual void HandleMsg(const string &msg);

	virtual void Render(void *pTarget);
	virtual void OnAdd(); //called once when brain is inserted
	virtual void OnRemove();

protected:
	

private:

	int GetCellIndex(int x, int y) {return y*m_gridSizeSide+x;}
	void AddRandomCells(int count);
	void Init(int gridSizeSide, int cellSize);

	vector<bool> m_grid;
	vector<bool> m_gridBack; //think of it like a video flipping chain?

	int m_gridSizeSide;
	int m_gridSizeTotal;
	float m_cellSize;
	unsigned int m_thinkSpeedMS;
	unsigned int m_thinkTimer;

};

#endif // BrainLife_h__