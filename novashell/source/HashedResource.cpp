#include "AppPrecomp.h"
#include "CollisionData.h"
#include "HashedResource.h"
#include "AppUtils.h"
#include "GameLogic.h"

HashedResource::HashedResource()
{
	m_pImage = NULL;
	m_bColorKeyActive = false;
	m_colorKey = CL_Color::white.color;
	m_bSaveRequested = false;
}

CollisionData * HashedResource::GetCollisionDataByRect(const CL_Rect &rectSource)
{
	if (!m_pImage)
	{
		//need to load stuff first
		Init();
	}
	
	unsigned int hash = CL_MAKELONG(rectSource.left, rectSource.top);
	CollisionDataMap::iterator colItor = m_collisionMap.find(hash);
	if (colItor == m_collisionMap.end()) 
	{

		//LogMsg("Adding new collision resource");
		std::vector<CollisionData*> colVec;
		CollisionData *pColData = new CollisionData(rectSource);
		colVec.push_back(pColData);
		m_collisionMap.insert(std::make_pair(hash, colVec));
		return pColData;
	}

	//found it, but now we need to locate the exact one if there are more than one

	std::vector<CollisionData*> *pColVec = &colItor->second;
	for (unsigned int i =0; i < pColVec->size(); i++)
	{
		if (rectSource == pColVec->at(i)->GetRect())
		{
			//this is it
			return pColVec->at(i);
		}
	}

	//if we got here, it means we don't have data on this tile/subtile piece.  Create an empty one.
	CollisionData *pColData = new CollisionData(rectSource);
	pColVec->push_back(pColData);
	return pColData;
}

bool HashedResource::HasDataToSave()
{
	CollisionDataMap::iterator ent = m_collisionMap.begin();
	std::vector<CollisionData*> *pColVec;
	unsigned int i;
	for (ent; ent != m_collisionMap.end(); ++ent)
	{
		pColVec = &ent->second;	
		for (i=0; i < pColVec->size(); i++)
		{
			if (pColVec->at(i)->HasData() || pColVec->at(i)->GetDataChanged())	
			{
				//has data to save
				return true;
			}
		}
	}

	if (m_bColorKeyActive) return true;

	return false;		
}

string HashedResource::GetCollisionDataFileName()
{
	return m_strFilename.substr(0,m_strFilename.rfind('.')) + ".dat";
}

bool HashedResource::LoadDefaults()
{
	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(GetCollisionDataFileName());
	} catch(CL_Error error)
	{
		SAFE_DELETE(pFile);
		return false;
	}

	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	unsigned int chunkType;

	try
	{
		helper.process(chunkType);
		while (chunkType != C_HASHED_RESOURCE_END_OF_CHUNKS)
		{
			switch (chunkType)
			{
			case C_HASHED_RESOURCE_COLLISION_CHUNK:
				//load it from the file
				{
					CollisionData col;
					col.Serialize(helper);
					col.CheckForErrors(GetCollisionDataFileName(), true);
					*GetCollisionDataByRect(col.GetRect()) = col;
				}
				break;

			case C_HASHED_RESOURCE_COLORKEY_CHUNK:
				//load it from the file
				{
					helper.process(m_colorKey);
					m_bColorKeyActive = true;
				}
				break;
				
			default:
				assert(!"Unknown chunk type");
				break;
			}

			helper.process(chunkType);
		}

	}
	catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage(error.message, "Error loading collision data for graphic.  " + GetCollisionDataFileName() + " Corrupted?");
		SAFE_DELETE(pFile);
		return true;
	}

	SAFE_DELETE(pFile);
	return true; //success
}

void HashedResource::SaveDefaults()
{
	if (!m_pImage) return;
	string fName = GetCollisionDataFileName();

	if (!HasDataToSave())
	{
		//instead of saving, we should destroy ourselves..
		RemoveFile(fName);
		return;
	}

	bool bNeedsToSave = m_bSaveRequested;

	//walk through everything we have and ask them to tell us if they need saving

	CollisionDataMap::iterator ent;
	std::vector<CollisionData*> *pColVec;
	unsigned int i;
	
	if (!bNeedsToSave)
	{
		for (ent = m_collisionMap.begin(); ent != m_collisionMap.end(); ++ent)
		{
			pColVec = &ent->second;
		
			for (i=0; i < pColVec->size(); i++)
			{
				if (pColVec->at(i)->GetDataChanged())
				{
					bNeedsToSave = true;
					break;
				}
			}

		}
	
	if (!bNeedsToSave) return;
	}

		
	//we know there is data, so let's save it out, if it's been modified

	CL_OutputSource_File file(fName);
	CL_FileHelper helper(&file); //will autodetect if we're loading or saving

	LogMsg("Saving coldata %s", fName.c_str());

	for (ent = m_collisionMap.begin(); ent != m_collisionMap.end(); ++ent)
	{
		pColVec = &ent->second;	
		for (i=0; i < pColVec->size(); i++)
		{
			if (pColVec->at(i)->HasData())		
			{
				//has data to save
				//save header
				assert(sizeof(C_HASHED_RESOURCE_COLLISION_CHUNK) == 4 && "This should be an unsigned int.. uh.. ");

				helper.process_const(C_HASHED_RESOURCE_COLLISION_CHUNK);
				pColVec->at(i)->Serialize(helper);
			}
		}
	}

	if (m_bColorKeyActive)
	{
		helper.process_const(C_HASHED_RESOURCE_COLORKEY_CHUNK);
		helper.process(m_colorKey);
	}

	
	//save end of data header
	helper.process_const(C_HASHED_RESOURCE_END_OF_CHUNKS);

}

HashedResource::~HashedResource()
{
	SaveDefaults();
	Kill();
}

void HashedResource::Kill()
{
	m_bSaveRequested = false;
	CollisionDataMap::iterator ent = m_collisionMap.begin();
	unsigned int i;
	for (ent; ent != m_collisionMap.end(); ++ent)
	{
		//run through the vector and delete its stuff, note that this could cause
		//hanging pointers if anyone is still caching it

		for (i=0; i < (*ent).second.size(); i++)
		{
			delete (*ent).second.at(i);
		}
	}
	m_collisionMap.clear();
	SAFE_DELETE(m_pImage);
}

CL_Surface * HashedResource::GetImage()
{
	if (!m_pImage) 
	{
		//BlitMessage("... loading ...");
		Init();
	}
	return m_pImage;
}

void HashedResource::SetHasColorKey(bool bActive, CL_Color col)
{
	if (bActive != m_bColorKeyActive || col.color != m_colorKey)
	{
		m_bSaveRequested = true;
		m_bColorKeyActive = bActive;
		m_colorKey = col.color;
		SaveDefaults();
		LoadImage(); //reinit things
	}

}

bool HashedResource::LoadImage()
{
	GetGameLogic()->ShowLoadingMessage();

	SAFE_DELETE(m_pImage);

	CL_PixelBuffer p = CL_ProviderFactory::load(m_strFilename);
	//use this method gives us a chance to twiddle with the bits a bit
	if (m_bColorKeyActive)
	{
		p.set_colorkey(true, m_colorKey);
	}

	m_pImage = new CL_Surface(p);
	// Commented out due to the Linear Filter change
//	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
//	clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);
	return m_pImage != NULL;
}

bool HashedResource::Init()
{
	Kill();
	assert(m_strFilename.size() > 0 && "Set the filename first");

	if (!LoadImage()) return false;	
	LoadDefaults(); //if applicable
	
	if (m_bColorKeyActive)
	{	
		//OPTIMIZE: load image again, this time it will know to substitute the colorkey.  This is a waste of time, but
		//we need to know the image dimensions (which can change) before loading the col data with the current
		//setup.

		LoadImage();
	}
	return true;
}

void HashedResource::DeleteCollisionDataByRect(CL_Rect r)
{
	CollisionData *pData = GetCollisionDataByRect(r);
	if (pData)
	{
		pData->Clear();
		m_bSaveRequested = true;
	}
}

void HashedResource::PopulateListBoxWithCollisionData(CL_ListBox *pList)
{
	CollisionDataMap::iterator ent;
	std::vector<CollisionData*> *pColVec;
	unsigned int i;

		for (ent = m_collisionMap.begin(); ent != m_collisionMap.end(); ++ent)
		{
			pColVec = &ent->second;

			for (i=0; i < pColVec->size(); i++)
			{
				if (pColVec->at(i)->HasData() )
				{
				
					string extra = "";

					if (!pColVec->at(i)->CheckForErrors("", false))
					{
						extra = " Bad!";
					}
					//add data from here
					int index = pList->insert_item(RectToString(pColVec->at(i)->GetRect())
						+ " has " + CL_String::from_int(pColVec->at(i)->GetLineList()->size())+ " shape(s), (" +
						CL_String::from_int(pColVec->at(i)->GetVertCount())+" verts)" + extra);
					pList->get_item(index)->user_data = i;
				}
			}

		}

}
