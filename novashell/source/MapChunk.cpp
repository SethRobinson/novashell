#include "AppPrecomp.h"
#include "MapChunk.h"
#include "Screen.h"
#include "Map.h"

MapChunk::MapChunk(Map *pParent)
{
	m_pParent = pParent;
	m_bIsEmpty = true;
	m_pScreen = NULL;
	m_bChunkDataChanged = false;
	for (int i=0; i < e_byteCount; i++) m_byteArray[i] = 0;
	for (int i=0; i < e_intCount; i++) m_intArray[i] = 0;
	for (int i=0; i < e_uintCount; i++) m_uintArray[i] = 0;
	
	m_pThumb = NULL;
	m_pThumbPixelData = NULL;	
}

MapChunk::~MapChunk()
{
	SAFE_DELETE(m_pScreen);
	KillThumbnail();
}

void MapChunk::KillThumbnail()
{
	SAFE_DELETE(m_pThumb);
	SAFE_DELETE(m_pThumbPixelData);
}
void MapChunk::SetDataChanged(bool bNeedsSaving)
{
	m_byteArray[e_byteDataChanged] = bNeedsSaving;
	
	if (m_pScreen)
	{
		m_pScreen->SetRequestIsEmptyRefreshCheck(true);
	}
}

void MapChunk::SetNeedsThumbnailRefresh(bool bNeedsRefresh)
{
	m_byteArray[e_byteNeedsThumbNailRefreshed] = bNeedsRefresh;
	
}

void MapChunk::SetThumbNail(CL_PixelBuffer *pPixBuffer)
{
	KillThumbnail();
	m_pThumb = pPixBuffer;
	m_bChunkDataChanged = true;

}

void MapChunk::SetScreen(Screen *pScreen)
{
	SAFE_DELETE(m_pScreen);
	m_pScreen = pScreen;
}

void MapChunk::UnloadScreen()
{
	//save memory by killing off parts of the map we aren't using
	if (m_pScreen)
	{
		//LogMsg("Unloading screen %d", GetScreenID());
		m_bIsEmpty = m_pScreen->IsEmpty();
		SAFE_DELETE(m_pScreen);
		 
	}
}

bool MapChunk::Serialize(CL_OutputSource *pOutput)
{
	
	CL_FileHelper helper(pOutput); //will autodetect if we're loading or saving

	//they load it
	helper.process_smart_array(m_byteArray, e_byteCount);
	helper.process_smart_array(m_intArray, e_intCount);
	helper.process_smart_array(m_uintArray, e_uintCount);

	//pixel buffer stuff
	if (m_pThumb)
	{
		//save out thumb info
		helper.process_const(C_DATA_PIXEL_DATA_CHUNK);
		helper.process(m_pThumb, C_THUMBNAIL_FORMAT);
	} else
	{
		helper.process_const(C_DATA_NO_PIXEL_DATA);
		
	}

	//screen will get saved later automatically when we kill ourself
	m_bChunkDataChanged = false;

	return true;
}

bool MapChunk::Serialize(CL_InputSource *pInput)
{
	
	assert(!m_pThumbPixelData && !m_pThumb && !m_pScreen && "This should not be initted!");
	CL_FileHelper helper(pInput); //will autodetect if we're loading or saving

	helper.process_smart_array(m_byteArray, e_byteCount);
	helper.process_smart_array(m_intArray, e_intCount);
	helper.process_smart_array(m_uintArray, e_uintCount);
	
	cl_uint32 chunkType;
	helper.process(chunkType);

	switch (chunkType)
	{
	case C_DATA_NO_PIXEL_DATA:
		break;

	case C_DATA_PIXEL_DATA_CHUNK:
		helper.process(&m_pThumb, &m_pThumbPixelData, C_THUMBNAIL_FORMAT);
		break;

	default:

		assert(!"Unknown chunk type");

	}

	//if we just loaded it, we can assume it's not an empty map or it would never have
	//been saved
	m_bIsEmpty = false;
	m_bChunkDataChanged = false;

	return true;
}

Screen * MapChunk::GetScreen(bool bLoadOnDemand)
{
	if (m_pScreen) return m_pScreen;
	
	if (!bLoadOnDemand) return NULL; //not loaded yet or doesn't exist

	//no screen exists so we'd better load/init it now
	
	m_pScreen = new Screen(this);
	if (!m_bIsEmpty)
	{
		//I know we're checking out possibly outdated cached version, this is on purpose
		//as our info will be more updated then the screens because we just new'ed it

		m_pScreen->Load();
	}
	
	return m_pScreen;
}

bool MapChunk::IsEmpty()
{
	if (m_pScreen) 
	{
		return m_pScreen->IsEmpty();
	} 

	return m_bIsEmpty; //our cached version
}

void MapChunk::SetScreenID(ScreenID screenID)
{
	 m_intArray[e_intScreenID] = screenID;

	 //cache our bounds in world coordinates
	 CL_Vector2 vecUpLeft = m_pParent->ScreenIDToWorldPos(screenID);
	 m_rect.left = vecUpLeft.x;
	 m_rect.top = vecUpLeft.y;
	 m_rect.set_size( CL_Size(m_pParent->GetMapChunkPixelSize(),m_pParent->GetMapChunkPixelSize()));
}
const CL_Rect & MapChunk::GetRect()
{
	return m_rect;
}