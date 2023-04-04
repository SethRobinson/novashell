//------------------------------------------------------------------------------------------------ 
////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// 
// Created by    : OR - 07/02/2004 12:42:04
// 
// Copyright (C) : 2004 Rebellion, All Rights Reserved.
// 
////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Description   : 
// --------------- 
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////// 
//------------------------------------------------------------------------------------------------ 

//These are left over from Olii's PollyColy code


#ifndef __CONTACT_H__
#define __CONTACT_H__


class CMaterial;
#include <string>
using namespace std;

/*
Object: Material
Stores data and properties for an individual material.

About:

You can only create and access materials through the <MaterialManager>.

Group: Member Functions
*/

/*
func: SetType
(code)
nil SetType(number materialType)
(end)
Sets what kind of basic behavior this should have.  For properties like ladders and warps, you need to set this. This is done for you in setup_constants.lua.

Parameters:

materialType - Should be sent one of the <C_MATERIAL_TYPE_CONSTANTS>.

*/

/*
func: GetType
(code)
number GetType()
(end)

Returns:

One of the <C_MATERIAL_TYPE_CONSTANTS>.
*/

/*
func: SetSpecial
(code)
nil SetSpecial(number materialSpecial)
(end)
Sets extra properties for a material.  Not really used yet, but might be useful for designating special kinds of collisions later.

Parameters:

materialSpecial - Should be sent one of the <C_MATERIAL_SPECIAL_CONSTANTS>.

*/

/*
func: GetSpecial
(code)
number GetSpecial()
(end)

Returns:

One of the <C_MATERIAL_SPECIAL_CONSTANTS>.
*/



class CMaterial
{
public:
		CMaterial(float fCoF = 0.2f, float fCoR = 0.3f, float fCoS = 0.4f, float fSep=0.5f)

		: m_fCoF(fCoF)
	, m_fCoR(fCoR)
	, m_fCoS(fCoS)
	, m_fSep(fSep)
	{
		m_id = 0;
		m_type = C_MATERIAL_TYPE_NORMAL;
		m_special = C_MATERIAL_SPECIAL_NONE;
	}

	void SetSeparation		(float fSep) { m_fSep = fSep; }
	void SetFriction		(float fCoF) { m_fCoF = fCoF; }
	void SetStaticFriction	(float fCoS) { m_fCoS = fCoS; }
	void SetRestitution		(float fCoR) { m_fCoR = fCoR; }

	float GetSeparation		() const { return m_fSep; }
	float GetFriction		() const { return m_fCoF; }
	float GetStaticFriction	() const { return m_fCoS; }
	float GetRestitution	() const { return m_fCoR; }
	
	void SetName(const string &name) {m_name = name;}
	const string & GetName(){return m_name;}

	void SetColor(unsigned int col) {m_lineColor = col;}
	unsigned int GetColor() {return m_lineColor;}

	void SetID(unsigned int id){m_id = id;}
	int GetID() {return m_id;};

	void SetSpecial(int special)
	{
		m_special = special;
	}
	int GetSpecial() {return m_special;}

	void SetType(int matType)
	{
		if (matType < 0 || matType >= C_MATERIAL_TYPE_COUNT)
		{
			LogMsg("SetType failed, bad material type");
			return;
		}
		m_type = matType;
	}
	int GetType() {return m_type;}

/*

Title: Material Related Constants


group: C_MATERIAL_TYPE_CONSTANTS

constant: C_MATERIAL_TYPE_NONE
Means invalid material.  Signals a zone with no data when using GetWallByRay()

constant: C_MATERIAL_TYPE_NORMAL
The default.  Just a normal thing you can bump into.  Any other material type will not perform real collisions, just report them.

constant: C_MATERIAL_TYPE_VERTICAL_LADDER
For designating shapes as ladders.  Defined in setup_constants.lua and used in the treeworld scripts.

constant: C_MATERIAL_TYPE_WARP
For designating shapes as warps.  Defined in setup_constants.lua and used in the treeworld scripts.

constant: C_MATERIAL_TYPE_DUMMY
For tweaking sorting and smart shadows.  The engine completely ignores it for collision but will still calculate the depth dot (lowest point of collision) and shadow information based on it.  Good for tweaking those things, make sure it's the first line in a collision shape.
*/

	enum
	{
		C_MATERIAL_NONE = -1
		
	};

	enum
	{
		C_MATERIAL_TYPE_NONE = -1,
		C_MATERIAL_TYPE_NORMAL = 0,
		C_MATERIAL_TYPE_VERTICAL_LADDER,
		C_MATERIAL_TYPE_WARP,
		C_MATERIAL_TYPE_DUMMY,

		//add more above here
		C_MATERIAL_TYPE_COUNT

	};
	/*
	group: C_MATERIAL_SPECIAL_CONSTANTS

	constant: C_MATERIAL_SPECIAL_NONE
	Default, no special properties.


	*/
	enum
	{
		C_MATERIAL_SPECIAL_NONE = 0,
		C_MATERIAL_SPECIAL_FLOOR
	};

private:

	float m_fCoF, m_fCoR, m_fCoS, m_fSep;
	unsigned int m_lineColor;
	string m_name;
	int m_id;
	int m_type;
	int m_special;
};


#endif//__CONTACT_H__
