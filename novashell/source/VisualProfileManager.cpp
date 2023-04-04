#include "AppPrecomp.h"
#include "VisualProfileManager.h"
#include "GameLogic.h"

VisualProfileManager::VisualProfileManager()
{
}

VisualProfileManager::~VisualProfileManager()
{
	Kill();
}

void VisualProfileManager::Kill()
{

	VisualResourceMap::iterator ent = m_hashedResourceMap.begin();
	for (; ent != m_hashedResourceMap.end(); ++ent)
	{
		delete (*ent).second;
	}

	m_hashedResourceMap.clear();
}
VisualResource * VisualProfileManager::GetVisualResourceByHashedID(unsigned int resourceID)
{
	VisualResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		return NULL;
	}

	return (*itor).second;
}

VisualResource * VisualProfileManager::GetVisualResource(const string &fileName)
{
	unsigned int resourceID = HashString(fileName.c_str());

	VisualResource *pRes = GetVisualResourceByHashedID(resourceID);

	if (pRes) return pRes;

	//if we got here, it means this resource hasn't been loaded yet and we need to load and init it
	string fullFileName = fileName;
	g_VFManager.LocateFile(fullFileName);

	pRes = new VisualResource;
	if (!pRes || !pRes->Init(fullFileName))
	{
		SAFE_DELETE(pRes);
		LogMsg("Unable to load visual profile %s", fileName.c_str());
		return NULL;
	}

	//add it to our map
	VisualResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		m_hashedResourceMap.insert(std::make_pair(resourceID, pRes));
	} else
	{
		throw CL_Error("Hash conflict with VisualResource " + fileName);
	}

	return pRes; //error
}

//utils

int VectorToFacing(const CL_Vector2 &v)
{
	double angle = atan2(v.y, v.x);
	angle += PI;  //make it start 0ish
	angle += PI/VisualProfile::FACING_COUNT;  //move it 22.5 degrees in the case of 8 dirs
	angle = fmod(angle, PI*2); //wrap it if needed
	angle = angle/ (PI*2); //get the ratio of the angle
	
	//LogMsg("Facing %.2f, which maps to angle %d", angle, int(angle * VisualProfile::FACING_COUNT)); 

	//note-  This is a hack that assumes our direction enums are in the correct order (0 to num dirs -1)
	return int(angle * VisualProfile::FACING_COUNT);
}

#define C_ANGLE_VEC (sqrt(2.0)/2)

CL_Vector2 FacingToVector(int facing)
{
	switch (facing)	
	{
	case VisualProfile::FACING_LEFT:
		return CL_Vector2(-1,0);

	case VisualProfile::FACING_RIGHT:
		return CL_Vector2(1,0);

	case VisualProfile::FACING_UP:
		return CL_Vector2(0,-1);

	case VisualProfile::FACING_DOWN:
		return CL_Vector2(0,1);

	case VisualProfile::FACING_UP_LEFT:
			return CL_Vector2(-C_ANGLE_VEC,-C_ANGLE_VEC);

	case VisualProfile::FACING_DOWN_LEFT:
		return CL_Vector2(-C_ANGLE_VEC,C_ANGLE_VEC);


	case VisualProfile::FACING_UP_RIGHT:
		return CL_Vector2(C_ANGLE_VEC,-C_ANGLE_VEC);

	case VisualProfile::FACING_DOWN_RIGHT:
		return CL_Vector2(C_ANGLE_VEC,C_ANGLE_VEC);

	default:

		throw CL_Error("Unknown facing: " + CL_String::from_int(facing));
		break;
	}

	return CL_Vector2(0,0);
}
