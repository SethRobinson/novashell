
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 7:1:2006   9:40
*/


#ifndef WorldChunk_HEADER_INCLUDED // include guard
#define WorldChunk_HEADER_INCLUDED  // include guard

class Screen;
class Map;
typedef int ScreenID;



#define C_WORLDCHUNK_PADDING_SIZE 32 
#define C_THUMBNAIL_FORMAT CL_PixelFormat::rgb888

class MapChunk
{
public:

    MapChunk(Map *pParent);
    virtual ~MapChunk();

	void SetScreen(Screen *pScreen);

	void SetDataChanged(bool bNeedsSaving); //aplies to the screen class we hold
	cl_uint8 GetDataChanged() {return m_byteArray[e_byteDataChanged];}

	void SetNeedsThumbnailRefresh(bool bNeedsRefresh);
	cl_uint8 GetNeedsThumbnailRefresh() {return m_byteArray[e_byteNeedsThumbNailRefreshed];}
	bool Serialize(CL_OutputSource *pOutput);
	bool Serialize(CL_InputSource *pInput);

	void SetThumbNail(CL_PixelBuffer *pPixBuffer);

	CL_PixelBuffer * GetThumbnail() {return m_pThumb;}
	ScreenID GetScreenID() {return m_intArray[e_intScreenID];}
	void SetScreenID(ScreenID screenID);
	Screen * GetScreen( bool bLoadOnDemand = true);
	bool IsScreenLoaded() {return m_pScreen != NULL;}
	bool IsEmpty(); //if true, this map is totally empty, no need to load/save it
	void UnloadScreen();
	Map * GetParentMap() {return m_pParent;}
	void KillThumbnail();
	const CL_Rect & GetRect();
	bool GetChunkDataChanged() {return m_bChunkDataChanged;} //applies only to this class

protected:

	//this weird variable system is so I can automatically have them serialized to and
	//from disk in an endian-correct fasion, while remaining backwards compatibility.
	
    enum
	{
		C_DATA_NO_PIXEL_DATA,
		C_DATA_PIXEL_DATA_CHUNK
	};

	enum
	{
	 e_byteDataChanged = 0,
	 e_byteNeedsThumbNailRefreshed,
	 e_byteCount
	};

	enum
	{
	 e_intScreenID = 0,
	 e_intCount
	};

	enum
	{
		e_uintPlaceholder = 0,
		e_uintCount
	};


	Map * m_pParent;
	cl_uint8 m_byteArray[e_byteCount];
	cl_int32 m_intArray[e_intCount];
	cl_uint32 m_uintArray[e_uintCount];
	Screen * m_pScreen; //null if not loaded yet (they don't have to be)
	CL_PixelBuffer *m_pThumb; //thumbnail of the screenshot
	cl_uint8 *m_pThumbPixelData; //the raw data, PixelBuffers won't delete this for us when we
	//construct them manually so we have to remember this data for later
	bool m_bIsEmpty;
	CL_Rect m_rect; //the bounds of this screen
	bool m_bChunkDataChanged;


};

#endif                  // include guard
