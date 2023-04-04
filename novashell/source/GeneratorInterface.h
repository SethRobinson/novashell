
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 3/27/2005 11:19:12 AM
*/

#pragma once

class GeneratorInterface
{
public:

    virtual void GenerateInit()=0;
    virtual bool GenerateStep()=0;

};
