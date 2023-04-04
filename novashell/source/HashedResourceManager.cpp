#include "AppPrecomp.h"
#include "HashedResourceManager.h"
#include "GameLogic.h"
#include "TileEditOperation.h"
#include "CollisionData.h"
#include "HashedResource.h"

#define C_USED_RESOURCES_FILENAME "used_resources.dat"

HashedResourceManager::HashedResourceManager()
{
	
}

HashedResourceManager::~HashedResourceManager()
{
	Kill();
}

bool HashedResourceManager::ScanPathForResources(const string &stPath)
{

	//scan map directory for tile maps we can possibly load
	CL_DirectoryScanner scanner;

	scanner.scan(stPath, "*");
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (scanner.is_directory() && scanner.get_name() != "." && scanner.get_name() != "..")

		{
			//Found a world dir.  Any images in here?

			CL_DirectoryScanner scanPic;
			scanPic.scan(stPath+file);
			while (scanPic.next())
			{
				string fileExtension = CL_String::get_extension(scanPic.get_name());

				if (!scanPic.is_directory())
				{
					if (fileExtension == "dat") continue; //not an image, skip it
					if (fileExtension == "chunk") continue; //not an image, skip it
					if (fileExtension == "txt") continue; //not an image, skip it
					if (fileExtension == "psd") continue; //not an image, skip it
					if (fileExtension == "max") continue; //not an image, skip it
					//note, I build this whole path because .get_filename includes a backslash in the middle instead of
					//a forward slash which screws up the addgraphic function
					if (!AddGraphic(stPath+file+string("/")+scanPic.get_name()))
					{
						return false;
					}
				}
			}

		}
	}

	return true;
}

bool HashedResourceManager::Init()
{
	Kill();

	vector<string> vecPaths;
	g_VFManager.GetMountedDirectories(&vecPaths);

	vector<string>::reverse_iterator itor = vecPaths.rbegin();

	for (;itor != vecPaths.rend(); itor++)
	{
		if (!ScanPathForResources( (*itor) +"/"+GetGameLogic()->GetBaseMapPath()))
		{
			return false;
		}
	}
	return true; //success
}

bool HashedResourceManager::AddGraphic(string str)
{
	//LogMsg("Adding %s to tile graphics", str.c_str());

	unsigned int resourceID = FileNameToID(str.c_str());

	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);
	
	if (itor == m_hashedResourceMap.end()) 
	{
		HashedResource *pResource = new HashedResource;
		pResource->m_strFilename = str;

		m_hashedResourceMap.insert(std::make_pair(resourceID, pResource));
	} else
	{
		//now here one of two things happened:
		//1. The hash conflicted with another art piece, bad!
		//2. A mod has the same graphic, overriding something it's modding.   In this
		//case we should just ignore it.
		
		if (CL_String::get_filename(itor->second->m_strFilename) == CL_String::get_filename(str))
		{
			return true;
		}
		throw CL_Error("Hash conflict with graphic resource " + str + ".  Rename it!");
		return false;
	}
	return true;
}

void HashedResourceManager::PrintStatistics()
{
	LogMsg("");
	LogMsg("  ** Hashed Resource Manager **");

	LogMsg("    %d items hashed.", m_hashedResourceMap.size());
}

void HashedResourceManager::Kill()
{
	HashedResourceMap::iterator ent = m_hashedResourceMap.begin();
	for (ent; ent != m_hashedResourceMap.end(); ++ent)
	{
		delete (*ent).second;
	}

	m_hashedResourceMap.clear();
}

CL_Surface * HashedResourceManager::GetResourceByHashedID(unsigned int resourceID)
{
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		ShowResourceNotFoundError(resourceID);
		return NULL;
	}
	
	return (*itor).second->GetImage();
}

bool HashedResourceManager::HashedIDExists(unsigned int resourceID)
{
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		return false;
	}

	return true;
}


HashedResource * HashedResourceManager::GetResourceClassByHashedID(unsigned int resourceID)
{
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		ShowResourceNotFoundError(resourceID);
		return NULL;
	}

	return (*itor).second;
}


void HashedResourceManager::PutSubGraphicIntoTileBuffer(TilePic *pTile, TileEditOperation &op, CL_Rect srcRect)
{
	op.ClearSelection();
	CL_Surface *pPic = GetResourceByHashedID(pTile->m_resourceID);

	if (!pPic) 
	{
		throw CL_Error("Unable to get hashed resource" + CL_String::from_int(pTile->m_resourceID));
	}
	CL_Rect imageRect = CL_Rect(0,0, pPic->get_width(),pPic->get_height());
	
	//make sure we're within bounds
	srcRect.apply_alignment(origin_top_left, -pTile->m_rectSrc.left, -pTile->m_rectSrc.top);
	srcRect.left = max(srcRect.left, imageRect.left);
	srcRect.right = min(srcRect.right, imageRect.right);
	srcRect.top = max(srcRect.top, imageRect.top);
	srcRect.bottom = min(srcRect.bottom, imageRect.bottom);
	TilePic *pTilePic = (TilePic*)pTile->CreateClone();
	pTilePic->m_rectSrc = srcRect;
	op.AddTileToSelection(TileEditOperation::C_OPERATION_ADD, false, pTilePic);
}

CollisionData * HashedResourceManager::GetCollisionDataByHashedIDAndRect(unsigned int resourceID, const CL_Rect &rectSource)
{
	//first get the correct resource
	
	HashedResourceMap::iterator itor = m_hashedResourceMap.find(resourceID);

	if (itor == m_hashedResourceMap.end()) 
	{
		ShowResourceNotFoundError(resourceID);
		//throw ("Error finding hashed resource " + CL_String::from_int(resourceID));
		return NULL;
	}
  
  HashedResource *pResource = itor->second;
	//convert the rect info into a single number to use as a hash

  return pResource->GetCollisionDataByRect(rectSource);
}

//here we manually break up a vanilla image into tiles and put it in the buffer given for easy copy and
//pasting into a real map
void HashedResourceManager::PutGraphicIntoTileBuffer(int resourceID, TileEditOperation &op, CL_Vector2 gridSizeInPixels)
{
	CL_Size sizeGrid(gridSizeInPixels.x, gridSizeInPixels.y);
	op.ClearSelection();
	CL_Surface *pPic = GetResourceByHashedID(resourceID);
	
	if (!pPic) 
	{
		throw CL_Error("Unable to get hashed resource" + CL_String::from_int(resourceID));
	}
	CL_Size sz = CL_Size(pPic->get_width(),pPic->get_height());
	TilePic *pTilePic;

	if (gridSizeInPixels.x == 0)
	{
		//just one giant tile is fine
		sizeGrid = sz;
	}

	for (int x = 0; x < sz.width; x += sizeGrid.width)
	{
		for (int y = 0; y < sz.height; y += sizeGrid.height)
		{
			pTilePic = new TilePic();
			pTilePic->m_rectSrc = CL_Rect(x,y,x+sizeGrid.width,y+sizeGrid.height);
			pTilePic->m_rectSrc.right = min(pTilePic->m_rectSrc.right, pPic->get_width());
			pTilePic->m_rectSrc.bottom = min(pTilePic->m_rectSrc.bottom, pPic->get_height());
			
			pTilePic->m_resourceID = resourceID;
			pTilePic->SetLayer(C_LAYER_ENTITY);
			pTilePic->SetPos(CL_Vector2(pTilePic->m_rectSrc.left,pTilePic->m_rectSrc.top));
			op.AddTileToSelection(TileEditOperation::C_OPERATION_ADD, false, pTilePic);
			
		}
	}
}

//the only problem with using # hashes for everything is if an image is missing, 237827382 doesn't tell
//you a whole lot about where to find it.  We'll save out the textures used, these will only be read and
//checked if there is an error.  We can figure out filenames this way.

void HashedResourceManager::SaveUsedResources(Map *pWorld, string path)
{

	//save out our globals
	CL_OutputSource *pSource = g_VFManager.PutFile(path+C_USED_RESOURCES_FILENAME);
	CL_FileHelper helper(pSource);


	HashedResourceMap::iterator ent = m_hashedResourceMap.begin();
	for (ent; ent != m_hashedResourceMap.end(); ++ent)
	{
		//LogMsg("Found %s", ent->second->m_strFilename.c_str());
		helper.process_const(ent->first);
		helper.process_const(ent->second->m_strFilename.c_str());
	}

//	helper.process_const(C_PROFILE_DAT_VERSION);
	SAFE_DELETE(pSource);
	
}


void HashedResourceManager::ShowResourceNotFoundError(unsigned int resourceID)
{
	//do a slow look up to find the resource.  It's not like this should be called much or speed matters.
	//If it ever is, load the file into a hash-map and keep it cached after the first call.
	
	Map *pWorld = g_pMapManager->GetActiveMap();
	string stExtraInfo = "extra info not available";
	
	if (pWorld)
	{
		//does extra data exist?	
		CL_InputSource *pSource = g_VFManager.GetFile(pWorld->GetDirPath()+ C_USED_RESOURCES_FILENAME);

		if (pSource)
		{
			CL_FileHelper helper(pSource);

			string stTemp;
			unsigned int storedResourceID;

			while (pSource->tell() < pSource->size())
			{
				helper.process(storedResourceID);
				helper.process(stTemp);

				if (storedResourceID == resourceID)
				{
					//this is it
					stExtraInfo = stTemp;
					break; //don't scan anymore
				}
			}

			SAFE_DELETE(pSource);
		}
	}

		//couldn't find extra data
		LogError("Unable to find resourceID %u (%s)", resourceID, stExtraInfo.c_str());
		return;
	

	//scan for more info

	

}
