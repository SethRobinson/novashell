#include "AppPrecomp.h"
#include "GeneratorDink.h"
#include "GameLogic.h"

#define mapX 32
#define mapY 24
#define tileSize 50
#define tilesPerScreenX 12
#define tilesPerScreenY 8
#define tilesPerTilePage 128

GeneratorDink::GeneratorDink()
{
}

void GeneratorDink::GenerateInit()
{
	assert(g_pMapManager->GetActiveMap() && "World must be initted before this is called");
	string st = C_BASE_MAP_PATH +string("/_base_graphics/dink.dat");

	//grab the map header

	FILE *fp = fopen(st.c_str(), "rb");
	if (!fp) throw CL_Error("Unable to find "+st);
	fread(&m_mapInfo,sizeof(dink_map_info),1,fp);
	fclose(fp);
	m_mapIndex = 0;

	m_vecMapUpperLeft = g_pMapManager->GetActiveMap()->ScreenIDToWorldPos(g_pMapManager->GetActiveMap()->GetScreenID(C_DEFAULT_SCREEN_ID,C_DEFAULT_SCREEN_ID));
}


void GeneratorDink::LoadDinkScreen(dink_small_map *pDinkScreen, int mapIndex)
{
    //LogMsg("Loading map %d...",mapIndex);
	string st = C_BASE_MAP_PATH + string("/_base_graphics/map.dat");

	FILE * fp = fopen( st.c_str(), "rb");
	if (!fp) throw CL_Error("Can't open" + st);

	fseek( fp, sizeof(dink_small_map)* (mapIndex-1), SEEK_SET);
	fread(pDinkScreen, sizeof(dink_small_map), 1, fp);    
	fclose(fp);
}

void GeneratorDink::ProcessDinkScreen(CL_Vector2 &vecOffset, dink_small_map &screenData)
{
	
	dink_tile *pDinkTile = NULL;
	TilePic *pTile = NULL;
	CL_Rect rectSrc;

	for (int x=0; x < tilesPerScreenX; x++)
	{
		for (int y=0; y < tilesPerScreenY; y++)
		{
			pDinkTile = &screenData.t[x+(y*tilesPerScreenX)];

			int tilePageNum = pDinkTile->num / tilesPerTilePage;
			int tileIndex = pDinkTile->num - (tilePageNum*tilesPerTilePage);
			if (tilePageNum > 42)
			{
				LogMsg("Invalid tilepage of %d found, ignoring");
				continue; //bad data?
			}

			rectSrc.top = (tileIndex/tilesPerScreenX)* tileSize;
			rectSrc.left = (tileIndex - ((rectSrc.top/tileSize) * tilesPerScreenX)) *tileSize;
			rectSrc.right = rectSrc.left + tileSize;
			rectSrc.bottom = rectSrc.top + tileSize;

			tilePageNum += 1;
			string st("ts");
			if (tilePageNum < 10) st += "0";
			st += CL_String::from_int(tilePageNum);
		
			//LogMsg("Tile Page %d, tile index %d.", tilePageNum, tileIndex);
	        //LogMsg("Formatted source image filename of %s", st.c_str());
			
			pTile = new TilePic;
			CL_Vector2 localDestPos = CL_Vector2(x*tileSize, y*tileSize);
			
			pTile->SetPos(localDestPos+vecOffset);
			
			pTile->m_rectSrc = rectSrc;
			pTile->m_resourceID = FileNameToID(st.c_str());
			if (!GetHashedResourceManager->GetResourceByHashedID(pTile->m_resourceID))
			{
				throw CL_Error("Unable to locate tilescreen "+st);
			}
			//figure out where to put it

			g_pMapManager->GetActiveMap()->AddTile(pTile);
		}
	}
	//LogMsg("This screen is %s", screenData.name);

}

//returns false when finished generating

bool GeneratorDink::GenerateStep()
{
	//process
	CL_Vector2 vecOffsetOfScreen;
	int realMapFileIndex;

	if (m_mapIndex < 768)
	{
		realMapFileIndex = m_mapInfo.loc[m_mapIndex+1]; 
		//figure out upper left of this screen
		if (realMapFileIndex != 0)
		{
			//LogMsg("World index %d has data", m_mapIndex);
			vecOffsetOfScreen.y = m_mapIndex/mapX;
			vecOffsetOfScreen.x = m_mapIndex- (vecOffsetOfScreen.y * mapX);
			vecOffsetOfScreen.y *= tilesPerScreenY * tileSize;
			vecOffsetOfScreen.x *= tilesPerScreenX * tileSize;
			vecOffsetOfScreen += m_vecMapUpperLeft;
			
			//load the screen data itself
			dink_small_map screenData;
			LoadDinkScreen(&screenData, realMapFileIndex);
			ProcessDinkScreen(vecOffsetOfScreen, screenData);

		} else
		{
			//LogMsg("World index %d is BLANK", m_mapIndex);
		}
		
		m_mapIndex++;
	} else
	{
		return false; //all done
	}


  //we're still working
  return true;
}

