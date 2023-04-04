
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 2:2:2006   15:10
*/


#ifndef LayerManager_HEADER_INCLUDED // include guard
#define LayerManager_HEADER_INCLUDED  // include guard

#define C_LAYER_NONE -1
#define C_TOP_LAYER 1000
#define C_LAYER_FILENAME "layer.dat"
#define C_MAP_DB_FILENAME "userdb.dat" //generic per-map db for user data

const int C_LAYER_VERSION = 0;
//some help to name these for generation functions, not really hardcoded though
enum
{
	C_LAYER_BACKGROUND1 = 0,
	C_LAYER_BACKGROUND2,
	C_LAYER_BACKGROUND3,
	C_LAYER_MAIN,
	C_LAYER_DETAIL1,
	C_LAYER_DETAIL2,
	C_LAYER_ENTITY,
	C_LAYER_OVERLAY1,
	C_LAYER_OVERLAY2,
	C_LAYER_HIDDEN_DATA,
	C_LAYER_HIDDEN_DATA2,
	C_LAYER_GUI1,
	C_LAYER_GUI2,
	C_LAYER_GUI3,

	//add more above here
	C_LAYER_DEFAULT_COUNT
};

class Layer
{
public:
	Layer();

	cl_uint8 IsDisplayed(){return m_byteArray[e_byteIsDisplayed];}
	void SetIsDisplayed(cl_uint8 isDisplayed){m_byteArray[e_byteIsDisplayed] = isDisplayed;}

	cl_uint8 IsEditActive(){return m_byteArray[e_byteEditActive];}
	void SetIsEditActive(cl_uint8 isEditActive){m_byteArray[e_byteEditActive] = isEditActive;}

	cl_uint8 GetShowInEditorOnly() {return m_byteArray[e_byteShowInEditorOnly];};
	void SetShowInEditorOnly(cl_uint8 showInEditorOnly) {m_byteArray[e_byteShowInEditorOnly] = showInEditorOnly;}

	cl_uint8 GetUseInThumbnail() {return m_byteArray[e_byteUseInThumbnail];};
	void SetUseInThumbnail(cl_uint8 useInThumbnail) {m_byteArray[e_byteUseInThumbnail] = useInThumbnail;}

	cl_uint8 GetUseParallaxInThumbnail() {return m_byteArray[e_byteUseParallaxInThumbnail];};
	void SetUseParallaxInThumbnail(cl_uint8 useParallaxInThumbnail) {m_byteArray[e_byteUseParallaxInThumbnail] = useParallaxInThumbnail;}

	cl_uint8 GetHasCollisionData() {return m_byteArray[e_byteHasCollisionData];};
	void SetHasCollisionData(cl_uint8 hasColData) {m_byteArray[e_byteHasCollisionData] = hasColData;}

	cl_uint8 GetDepthSortWithinLayer() {return m_byteArray[e_byteDepthSortWithinLayer];};
	void SetDepthSortWithinLayer(cl_uint8 depthSort) {m_byteArray[e_byteDepthSortWithinLayer] = depthSort;}

	const string & GetName(){return m_stName;}
	void SetName(const string st){m_stName = st;}
	
	void SetSort(int sort) { m_intArray[e_intSort] = sort;}
	int GetSort() {return m_intArray[e_intSort];}
	CL_Vector2 GetScrollMod() { return CL_Vector2(m_floatArray[e_floatScrollModX], m_floatArray[e_floatScrollModY]);}
	bool RequiresParallax() {return (m_floatArray[e_floatScrollModX] != 0 || m_floatArray[e_floatScrollModY] != 0);}

	void SetScrollMod(const CL_Vector2 &scrollMod);
	void Serialize(CL_FileHelper &helper); //handles loading and saving to a stream
	
private:

	
	enum
	{
		e_intSort = 0,
		
		//add new vars above here
		e_intCount
	};
	enum
	{
		e_byteIsDisplayed = 0,
		e_byteEditActive,
		e_byteShowInEditorOnly,
		e_byteUseInThumbnail,
		e_byteUseParallaxInThumbnail,
		e_byteHasCollisionData,
		e_byteDepthSortWithinLayer,

		//add new vars above here
		e_byteCount
	};

	enum
	{
		e_floatScrollModX = 0,
		e_floatScrollModY,

		//add new vars above here
		e_floatCount
	};

	enum
	{
		e_uintPlaceHolder = 0, 

		//add new vars above here
		e_uintCount
	};

	cl_uint8 m_byteArray[e_byteCount];
	cl_int32 m_intArray[e_intCount];
	cl_uint32 m_uintArray[e_uintCount];
	float m_floatArray[e_floatCount];

	string m_stName;
	
};

typedef vector<Layer> layer_vector;

class LayerManager
{
public:

    LayerManager();
    virtual ~LayerManager();
	unsigned int GetLayerCount() {return m_layerVec.size();}
	Layer & GetLayerInfo(unsigned int layerID) {return m_layerVec[layerID];}
	int GetLayerIDByName(const string &name);
	const vector<unsigned int> & GetDrawList() {return m_drawList;}
	const vector<unsigned int> & GetEditActiveList() {return m_editActiveList;}
	const vector<unsigned int> & GetAllList() {return m_allList;} //indexes of all layers
	const vector<unsigned int> & GetCollisionList() {return m_collisionList;} //indexes of all layers
	vector<unsigned int> GetCollisionListNoConst() {return m_collisionList;} //indexes of all layers
	vector<unsigned int> GetAllListNoConst() {return m_allList;} //indexes of all layers
	vector<unsigned int> GetDrawListNoConst() {return m_drawList;}

	void BuildLists();
	void PopulateIDVectorWithAllLayers(vector<unsigned int> &layerIDVecOut);
	void Remove(int layerID);
	void Add(Layer layer);
	void Save(const string &fileName);
	void Load(const string &filename);

protected:

	void BuildDefaultLayers();

	layer_vector m_layerVec;
	vector<unsigned int> m_drawList, m_editActiveList;
	vector<unsigned int> m_allList, m_collisionList;
};


#endif                  // include guard

/*
Object: LayerList
Contains a list of layerID's.

Group: Member Functions

func: Add
(code)
nil Add(number layerID)
(end)

Some functions require a list of layers to operate on.  Besides grabbing pre-built lists from the <LayerManager>, you can contruct your own.

Usage:
(code)
myLayerList = LayerList(); //create a new object

myLayerList.Add(someMap:GetLayerManager:GetLayerIDByName("Overlay 1"));
myLayerList.Add(someMap:GetLayerManager:GetLayerIDByName("GUI 2"));
(end)

Now we can use myLayerList with <Map::GetTilesByRect> to scan only these two layers.

Parameters:

The layerID to add.  LayerID's are just numbers.
*/


/*
func: Get
(code)
number Get(number index)
(end)

Parameters:

index - The <LayerList> slot you want the number from.  (Starts at 0, must be less than <GetCount>.)

Returns:

The layerID at this slot.
*/

/*
func: GetCount
(code)
number GetCount()
(end)

Returns:

How many layerID's are in this list.
*/