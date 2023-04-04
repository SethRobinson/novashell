
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 24:1:2006   15:08
*/


#ifndef HashedResource_HEADER_INCLUDED // include guard
#define HashedResource_HEADER_INCLUDED  // include guard

//we want to cache the pointers so we use a pointer to CollisionData instead of CollisionData to
//avoid complications with random STL realocations that could screw us up
class CollisionData;
typedef std::map<unsigned int, std::vector<CollisionData*> > CollisionDataMap;

class HashedResource
{
public:

	HashedResource();
	~HashedResource();

	bool Init(); //load the image and whatnot
	void Kill();
	void SaveDefaults();
	bool LoadDefaults();
	string GetCollisionDataFileName();
	CollisionData * GetCollisionDataByRect(const CL_Rect &rectSource);
	CL_Surface * GetImage();
	bool HasColorKey() {return m_bColorKeyActive;}
	void SetHasColorKey(bool bActive, CL_Color col);
	CL_Color GetColorKey() {assert(sizeof(CL_Color) == 4); return *(CL_Color*)&m_colorKey;}
	void PopulateListBoxWithCollisionData(CL_ListBox *pList);
	void DeleteCollisionDataByRect(CL_Rect r);

	string m_strFilename; //good to remember the original filename

private:

	bool LoadImage();
	bool HasDataToSave(); //slow check

	CollisionDataMap m_collisionMap;
	CL_Surface *m_pImage;
	bool m_bColorKeyActive;
	unsigned int m_colorKey;
	bool m_bSaveRequested;

	enum
	{
		//chunk descriptions
		C_HASHED_RESOURCE_COLLISION_CHUNK = 0,
		C_HASHED_RESOURCE_END_OF_CHUNKS,
		C_HASHED_RESOURCE_COLORKEY_CHUNK,
	};
};


#endif                  // include guard
