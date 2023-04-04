
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 2:3:2006   8:46
*/


#ifndef MaterialManager_HEADER_INCLUDED // include guard
#define MaterialManager_HEADER_INCLUDED  // include guard

#include "Material.h"
//lua doc info for natural docs processer.  Becareful how you move it, it will break..

/*

Object: MaterialManager
This object stores all the information about materials in the game.

About:

This is a global object that can always be accessed.

What's a material?  Well, it's the properties that collision shapes have. For instance, if the shape should be bouncy, a ladder, or a warp.

You can add your own custom materials, then later, during a collision query to see what kind of material you hit.

setup_constants.lua adds material definitions when the engine is started.

Usage:

(code)
C_MATERIAL_ICE = GetMaterialManager:AddMaterial(0.5,0.3, 0.5, 0.3, Color(255,200,50,255), "Ice");
(end)

*/

class MaterialManager
{
public:

    MaterialManager();
    virtual ~MaterialManager();

	int AddMaterial(float fCoF, float fCoR, float fCoS, float fSep, const CL_Color &col, const string &name);
	
	

	CMaterial * GetMaterial(int idx);
	int GetCount(){return m_vecMat.size();}

	void Init();

protected:

private:

	vector<CMaterial> m_vecMat;
};

extern MaterialManager g_materialManager; //I should make the class a singleton. uh.. 

#endif                  // include guard


/*

Group: Member Functions

func: AddMaterial
(code)
number AddMaterial (number friction, number restitution,
number separation, Color color, string name)
(end)

Parameters:
friction - A lower number here means its easier for an entity to slide around.
restitution - how bounce this shape is
separation - how hard the shape tries to push itself out of other shapes.
color - a <Color> object.  This affects the line color in the editor when you show collision.
name - what name you see when you are clicking the collision line type in the collision editor.

Returns:
A number to reference the material you created.  Normally you create your materials once at startup and just keep track of these.

Warning:
If you're going to be messing with these values or creating your own, it's only fair that I warn you it's a very sensitive and flakey system and what seems like it's working might be broken in other situations.

Also, the final result is what happens when TWO shapes collide, so you have to think about the material type of both pieces and how they will interact.
*/


/*
func: GetMaterial
(code)
Material * AddMaterial (number materialID)
(end)

Parameters:
materialID - the materialID you got when the material was created

Returns:
A pointer to a <Material> object

*/

