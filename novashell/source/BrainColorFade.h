//  ***************************************************************
//  BrainColorFade - Creation date: 10/04/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

//this is a brain file.  By including it in the project,
//it will automatically register itself and be available
//from lua script as a brain behavior.

#ifndef BrainColorFade_h__
#define BrainColorFade_h__

#include "Brain.h"

class BrainColorFade: public Brain
{
public:
	BrainColorFade(MovingEntity *pParent);
	virtual ~BrainColorFade();
	virtual void Update(float step);
	virtual const char * GetName() {return "ColorFade";};
	virtual Brain * CreateInstance(MovingEntity *pParent) {return new BrainColorFade(pParent);}
	virtual void HandleMsg(const string &msg);

protected:
	

private:

	class ColorData
	{
	public:
		
		ColorData();
		void Reset();
		void SetColorTarget(short target, int startingColor);
		int Update(float modSpeed);
		bool m_bActive;
		short m_colorTarget;
		float m_curColor;
	};

	enum
	{
		COLOR_R,
		COLOR_G,
		COLOR_B,
		COLOR_A,
		COLOR_COUNT
	};
	
	int m_fadeTimeMS;
	int m_timeCreated;

	ColorData m_color[COLOR_COUNT];

	bool m_bRemove_brain;


};

#endif // BrainColorFade_h__