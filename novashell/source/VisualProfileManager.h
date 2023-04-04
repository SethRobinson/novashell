
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 27:2:2006   11:00
*/

//visual profiles let us easily define how an entity will look or change its look on  the fly,
//the profile covers things like idle, run, jump animations.  They are setup in .xml profiles
//that are loaded on demand.  There is no way to unload them but it would be pretty easy to add


#ifndef VisualProfileManager_HEADER_INCLUDED // include guard
#define VisualProfileManager_HEADER_INCLUDED  // include guard

#include "VisualProfile.h"
#include "VisualResource.h"

typedef std::map<unsigned int, VisualResource*> VisualResourceMap;

class VisualProfileManager
{
public:

    VisualProfileManager();
    virtual ~VisualProfileManager();

	void Kill();

	VisualResource * GetVisualResource(const string &fileName);
	VisualResource * GetVisualResourceByHashedID(unsigned int resourceID);

protected:

private:
	VisualResourceMap m_hashedResourceMap;

};


CL_Vector2 FacingToVector(int facing);
int VectorToFacing(const CL_Vector2 &v);

#endif                  // include guard
