
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 3/27/2005 11:20:35 AM
*/

#pragma once

#include "GeneratorInterface.h"

class Screen;

class GeneratorSimple: public GeneratorInterface
{
public:

    GeneratorSimple();

    virtual void GenerateInit();
    virtual bool GenerateStep();


protected:

    void GenerateScreen(Screen *pScreen, int x, int y);

	int m_genX, m_genY;

	CL_Rect m_worldRect;
};
