#include "AppPrecomp.h"
#include "LayerManager.h"
#include "GameLogic.h"



Layer::Layer()
{
	m_stName = "Unnamed";
	for (int i=0; i < e_byteCount; i++) m_byteArray[i] = 0;
	for (int i=0; i < e_intCount; i++) m_intArray[i] = 0;
	for (int i=0; i < e_uintCount; i++) m_uintArray[i] = 0;
	for (int i=0; i < e_floatCount; i++) m_floatArray[i] = 0;

	SetUseInThumbnail(true);
	SetIsDisplayed(true);
	SetIsEditActive(true);
	
}
void Layer::SetScrollMod(const CL_Vector2 &scrollMod)
{
	m_floatArray[e_floatScrollModX] = scrollMod.x;
	m_floatArray[e_floatScrollModY] = scrollMod.y;
}

void Layer::Serialize(CL_FileHelper &helper)
{
	helper.process(m_stName);

	helper.process_smart_array(m_intArray, e_intCount);
	helper.process_smart_array(m_byteArray, e_byteCount);
	helper.process_smart_array(m_floatArray, e_floatCount);
	helper.process_smart_array(m_uintArray, e_uintCount);
}


LayerManager::LayerManager()
{
	
}

int LayerManager::GetLayerIDByName(const string &name)
{
	for (unsigned int i=0; i < GetLayerCount(); i++)
	{
		if (m_layerVec[i].GetName() == name)
			return i;
	}

	return C_LAYER_NONE;
}

LayerManager::~LayerManager()
{
}

void LayerManager::PopulateIDVectorWithAllLayers(vector<unsigned int> &layerIDVecOut)
{
	assert(layerIDVecOut.size() == 0 && "Don't send this in populated");

	for (unsigned int i=0; i < GetLayerCount(); i++)
	{
		layerIDVecOut.push_back(i);
	}
}

void LayerManager::Remove(int layerID)
{
	int i = 0;
	for (layer_vector::iterator itor = m_layerVec.begin(); 
		itor != m_layerVec.end(); i++, itor++)
	{
		if (i == layerID)
		{
			m_layerVec.erase(itor);
			return;
		}
	}
}

void LayerManager::Add(Layer layer)
{
	layer.SetSort(m_layerVec.size());
	m_layerVec.push_back(layer);
	BuildLists();
}

void LayerManager::BuildLists()
{
  //build the draw and edit lists in the order we need them in

	m_drawList.clear();
	m_editActiveList.clear();
	m_allList.clear();
	m_collisionList.clear();

   for (unsigned int i=0; i < GetLayerCount(); i++)
   {
	  if (m_layerVec[i].IsDisplayed()) m_drawList.push_back(i);
	  if (m_layerVec[i].IsEditActive()) m_editActiveList.push_back(i);
	  m_allList.push_back(i);
	  if (m_layerVec[i].GetHasCollisionData()) m_collisionList.push_back(i);
   }
  
}

void LayerManager::BuildDefaultLayers()
{
	m_layerVec.clear();
	//setup default layers
	m_layerVec.resize(C_LAYER_DEFAULT_COUNT);

	m_layerVec[C_LAYER_BACKGROUND1].SetName("Background 1");
	m_layerVec[C_LAYER_BACKGROUND1].SetSort(C_LAYER_BACKGROUND1);
	//m_layerVec[C_LAYER_BACKGROUND1].SetScrollMod(CL_Vector2(0.6f, 0.6f));
	m_layerVec[C_LAYER_BACKGROUND1].SetUseInThumbnail(false);

	m_layerVec[C_LAYER_BACKGROUND2].SetName("Background 2");
	m_layerVec[C_LAYER_BACKGROUND2].SetSort(C_LAYER_BACKGROUND2);
	//m_layerVec[C_LAYER_BACKGROUND2].SetScrollMod(CL_Vector2(0.5f, 0.5f));
	m_layerVec[C_LAYER_BACKGROUND2].SetUseInThumbnail(false);

	m_layerVec[C_LAYER_BACKGROUND3].SetName("Background 3");
	m_layerVec[C_LAYER_BACKGROUND3].SetSort(C_LAYER_BACKGROUND3);
	//m_layerVec[C_LAYER_BACKGROUND3].SetScrollMod(CL_Vector2(0.3f, 0.3f));
	m_layerVec[C_LAYER_BACKGROUND3].SetUseInThumbnail(false);

	m_layerVec[C_LAYER_MAIN].SetName("Main");
	m_layerVec[C_LAYER_MAIN].SetSort(C_LAYER_MAIN);
	m_layerVec[C_LAYER_MAIN].SetHasCollisionData(true);

	m_layerVec[C_LAYER_DETAIL1].SetName("Detail 1");
	m_layerVec[C_LAYER_DETAIL1].SetSort(C_LAYER_DETAIL1);
	m_layerVec[C_LAYER_DETAIL1].SetHasCollisionData(true);

	m_layerVec[C_LAYER_DETAIL2].SetName("Detail 2");
	m_layerVec[C_LAYER_DETAIL2].SetSort(C_LAYER_DETAIL2);
	m_layerVec[C_LAYER_DETAIL2].SetUseInThumbnail(false);
	m_layerVec[C_LAYER_DETAIL2].SetHasCollisionData(true);

	m_layerVec[C_LAYER_ENTITY].SetName("Entity");
	m_layerVec[C_LAYER_ENTITY].SetSort(C_LAYER_ENTITY);
	m_layerVec[C_LAYER_ENTITY].SetHasCollisionData(true);
	m_layerVec[C_LAYER_ENTITY].SetDepthSortWithinLayer(true);


	m_layerVec[C_LAYER_OVERLAY1].SetName("Overlay 1");
	m_layerVec[C_LAYER_OVERLAY1].SetSort(C_LAYER_OVERLAY1);
	m_layerVec[C_LAYER_OVERLAY1].SetHasCollisionData(true);
	m_layerVec[C_LAYER_OVERLAY1].SetDepthSortWithinLayer(true);

	m_layerVec[C_LAYER_OVERLAY2].SetName("Overlay 2");
	m_layerVec[C_LAYER_OVERLAY2].SetSort(C_LAYER_OVERLAY2);
	//m_layerVec[C_LAYER_OVERLAY2].SetScrollMod(CL_Vector2(-0.07f, -0.07f));
	m_layerVec[C_LAYER_OVERLAY2].SetDepthSortWithinLayer(true);

	m_layerVec[C_LAYER_HIDDEN_DATA].SetName("Hidden Data");
	m_layerVec[C_LAYER_HIDDEN_DATA].SetSort(C_LAYER_HIDDEN_DATA);
	m_layerVec[C_LAYER_HIDDEN_DATA].SetShowInEditorOnly(true);
	m_layerVec[C_LAYER_HIDDEN_DATA].SetUseInThumbnail(false);
	m_layerVec[C_LAYER_HIDDEN_DATA].SetHasCollisionData(true);


	m_layerVec[C_LAYER_HIDDEN_DATA2].SetName("Hidden Data 2");
	m_layerVec[C_LAYER_HIDDEN_DATA2].SetSort(C_LAYER_HIDDEN_DATA2);
	m_layerVec[C_LAYER_HIDDEN_DATA2].SetShowInEditorOnly(true);
	m_layerVec[C_LAYER_HIDDEN_DATA2].SetUseInThumbnail(false);
	m_layerVec[C_LAYER_HIDDEN_DATA2].SetHasCollisionData(true);
	m_layerVec[C_LAYER_HIDDEN_DATA2].SetIsEditActive(false);
	m_layerVec[C_LAYER_HIDDEN_DATA2].SetIsDisplayed(false);


	m_layerVec[C_LAYER_GUI1].SetName("GUI 1");
	m_layerVec[C_LAYER_GUI1].SetSort(C_LAYER_GUI1);
	m_layerVec[C_LAYER_GUI1].SetHasCollisionData(false);
	m_layerVec[C_LAYER_GUI1].SetUseInThumbnail(false);
	m_layerVec[C_LAYER_GUI1].SetIsEditActive(true);
	m_layerVec[C_LAYER_GUI1].SetIsDisplayed(true);

	m_layerVec[C_LAYER_GUI2].SetName("GUI 2");
	m_layerVec[C_LAYER_GUI2].SetSort(C_LAYER_GUI2);
	m_layerVec[C_LAYER_GUI2].SetHasCollisionData(false);
	m_layerVec[C_LAYER_GUI2].SetUseInThumbnail(false);
	m_layerVec[C_LAYER_GUI2].SetIsEditActive(true);
	m_layerVec[C_LAYER_GUI2].SetIsDisplayed(true);

	m_layerVec[C_LAYER_GUI3].SetName("GUI 3");
	m_layerVec[C_LAYER_GUI3].SetSort(C_LAYER_GUI3);
	m_layerVec[C_LAYER_GUI3].SetHasCollisionData(false);
	m_layerVec[C_LAYER_GUI3].SetUseInThumbnail(false);
	m_layerVec[C_LAYER_GUI3].SetIsEditActive(true);
	m_layerVec[C_LAYER_GUI3].SetIsDisplayed(true);


	BuildLists();
}
void LayerManager::Load(const string &filename)
{
	CL_InputSource *pFile = g_VFManager.GetFile(filename);

	if (!pFile)
	{
		//use default 
		BuildDefaultLayers();
		return; //nothing to load
	}


	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	//load everything we can find

	m_layerVec.clear();
	try
	{

	int version;
	helper.process(version);

	unsigned int count;
	helper.process(count);

	while (count--)
	{
		Layer l;
		l.Serialize(helper);
		m_layerVec.push_back(l);
	}
	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		ShowMessage(error.message, "Error loading layer data.");
		SAFE_DELETE(pFile);
	}

	SAFE_DELETE(pFile);
	BuildLists();
}

void LayerManager::Save(const string &fileName)
{

	//cycle through and save all tag data applicable
	CL_OutputSource *pFile = g_VFManager.PutFile(fileName);
	CL_FileHelper helper(pFile); //will autodetect if we're loading or saving

	helper.process_const(C_LAYER_VERSION);
	helper.process_const((cl_uint32)m_layerVec.size()); //let them know how many are coming

	//cycle through and let them save themselves

	for (unsigned int i=0; i < m_layerVec.size(); i++)
	{
		m_layerVec[i].Serialize(helper);
	}

	SAFE_DELETE(pFile);
		
}

/*
Object: LayerManager
Every <Map> contains its own <LayerManager> which contains information about its layers.

Group: Member Functions

func: GetLayerIDByName
	  (code)
	  number GetLayerIDByName(string layerName)
	  (end)
	  
	  Parameters:

	  layerName - A name of a layer, such as "GUI 1" or "Entity".

	  Returns:

	  The layerID of this layer, or <C_LAYER_NONE> if no layer of that name is found in the map.

 func: GetCollisionLayers
 (code)
 LayerList GetCollisionLayers()
 (end)

 Returns:

 A <LayerList> containing all layers that have "Uses collision" enabled for them.

 func: GetVisibleLayers
 (code)
 LayerList GetVisibleLayers()
 (end)

 Returns:

 A <LayerList> containing all layers that are currently being drawn.

 func: GetAllLayers
 (code)
 LayerList GetAllLayers()
 (end)

 Returns:

 A <LayerList> containing all layers.

 Section: Layer Related Constants


 Group: C_LAYER_CONSTANST
 Used with <LayerManager::GetLayerIDByName>.

 constants: C_LAYER_NONE
 This means an invalid layer.
 
 */