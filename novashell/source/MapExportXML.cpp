#include "AppPrecomp.h"
#include "MapExportXML.h"
#include "GameLogic.h"
#include "EntEditor.h"
#include "Console.h"
#include "HashedResource.h"

#define C_EXPORT_XML_PATH "_EXPORT_XML_PATH"
#define C_EXPORT_XML_FILENAME "_EXPORT_XML_FILENAME"

MapExportXML::MapExportXML()
{
	m_indentLevel = 0;
	m_buttonPushed = NONE;
}

MapExportXML::~MapExportXML()
{
}

bool MapExportXML::Init()
{
	if (GetGameLogic()->GetActiveWorldName() == "base" || !g_pMapManager->GetActiveMap())
	{
		CL_MessageBox::info("You must first load a world and map to export.", GetApp()->GetGUI());
		return true;
	}

	m_pMap = g_pMapManager->GetActiveMap();
	m_pWorldDB = GetGameLogic()->WorldData();
	m_pMapDB = m_pMap->GetDB();

	CL_SlotContainer slots;
	CL_Point ptSize(630,450);
	int indent = 20;
	int itemSeparatorY = 30;
	int buttonOffsetX = 10;
	int width = ptSize.x - (indent+buttonOffsetX)*2;
	CL_Window window(CL_Rect(0, 0, ptSize.x, ptSize.y), "XML Map Exporter", CL_Window::close_button, GetApp()->GetGUI());
	window.set_event_passing(false);
	window.set_position( (GetScreenX-ptSize.x)/2, C_EDITOR_MAIN_MENU_BAR_HEIGHT);

	CL_Button buttonCancel (CL_Point(buttonOffsetX+200,ptSize.y-45), "Cancel", window.get_client_area());
	CL_Button buttonOk (CL_Point(buttonOffsetX+100,ptSize.y-45), "Export", window.get_client_area());

	slots.connect(buttonCancel.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);

	slots.connect(buttonOk.sig_clicked(), this, &MapExportXML::OnPackage, &window);

	//check boxes
	
	int checkX = buttonOffsetX+indent;
	int checkY = ptSize.y - 20;

	checkY = 20;

	CL_Label label1(CL_Point(buttonOffsetX, checkY), "Output Options:", window.get_client_area()); checkY += itemSeparatorY;
	checkY += 10;
	if ( m_pWorldDB->Exists(C_EXPORT_XML_PATH))
	{
		m_path = m_pWorldDB->Get(C_EXPORT_XML_PATH);
		StringReplace("\\", "/", m_path);
		
		
		if (m_path.size() > 0)
		if (m_path[m_path.size()-1] != '/')
		{
			//remove trailing slash
			m_path += "/";
		}
	} else
	{
		m_path = GetApp()->GetBaseDirectory();
	}
	CL_Label labelPath(CL_Point(checkX, checkY), "Export file path:", window.get_client_area()); checkY += itemSeparatorY;
	CL_InputBox pathLine (CL_Rect(0,0, width, 24), m_path, window.get_client_area());
	pathLine.set_position(checkX, checkY); checkY += itemSeparatorY;
	
	//filename
	if ( m_pMapDB->Exists(C_EXPORT_XML_FILENAME))
	{
		m_filename = m_pMapDB->Get(C_EXPORT_XML_FILENAME);
	} else
	{
		m_filename = m_pMap->GetName()+".oel";
	}

	CL_Label labelFileName(CL_Point(checkX, checkY), "Export file name:", window.get_client_area()); checkY += itemSeparatorY;
	CL_InputBox FileNameLine (CL_Rect(0,0, width, 24), m_filename, window.get_client_area());
	FileNameLine.set_position(checkX, checkY); checkY += itemSeparatorY;

	
	checkY += 30;
	CL_Label label5(CL_Point(buttonOffsetX, checkY), 
		"This exports the current map in .XML format.  The format is mostly the same as the Ogmo editor.\n\n" \
		"* Layers marked as 'show in editor only' are ignored.  Layers marked as 'have collision' done as collision rects.\n\n" \
		"* Entities are exported with their name if it exists, or the entity data _export_name if it exists.\n\n"\
		"* For additional properties per layer, add them to a map user data value with the key\n export_layer_parm_<layer name>.\n\n" \
		"* To export per entity data, just add the key/value pair in the entities properties.\n\n" \
		"* When exporting on a Mac, use something like /Volumes/Macintosh HD/Users/Seth for the dir."
		, window.get_client_area()); checkY += 30;

	window.run();	//loop in the menu until quit() is called by hitting a button

	if (m_buttonPushed != PACKAGE) return true;
	
//	CL_MessageBox::info("Um, you'd have to check one of the output options to actually do anything.\n\nIf an option is grayed out, that means support for building that isn't available on this platform yet.", GetApp()->GetGUI());
//		return true;
		
	m_path = pathLine.get_text();
	m_filename = FileNameLine.get_text();

	

	//go ahead and save this for later
	m_pWorldDB->Set(C_EXPORT_XML_PATH, m_path);
	m_pMapDB->Set(C_EXPORT_XML_FILENAME, m_filename);
	
	if (m_path == "" || m_path[0] == '.')
	{
		m_path = GetGameLogic()->GetWorldsDirPath()+"/"+m_path;
	}

	StringReplace("\\", "/", m_path);
	if (m_path.size() > 0)
		if (m_path[m_path.size()-1] != '/')
		{
			//remove trailing slash
			m_path += "/";
		}
	
	m_pMap->SetModified(true); //insure that this will be saved

	g_Console.SetOnScreen(true);
	
	LogMsg("World path is %s", GetGameLogic()->GetWorldsDirPath().c_str());
	LogMsg("Active world is %s", GetGameLogic()->GetActiveWorldName().c_str());

	ExportMap();
	LogMsg("***********");
	LogMsg("Map exported to %s.", (m_path + m_filename).c_str());

	return true; //success
}

void MapExportXML::UpdateScreen()
{
	CL_System::keep_alive();
	CL_Display::clear();
	g_Console.Render();
	//GetGameLogic()->GetGameGUI()->show();
	CL_Display::flip(); //show it now
}

string MapExportXML::GetExtraXMLParmsFromDB(DataManager &db, string searchKey, bool bFromEntity)
{
	//go through each parm
	dataList *dList = db.GetList();
	dataList::iterator ditor = dList->begin();
	
	string out;

	while (ditor != dList->end())
	{

		string key = ditor->second.m_key;
		
		if (key == "export_name")
		{
			ditor++;
			continue;
		}
		if (bFromEntity)
		{
			if (key[0] != '_')
			{
				//yeah, let's export this parm, whatever it is
				out += " "+ditor->second.m_key+"=\""+ditor->second.Get()+"\"";
			}
			
		} else
		{
			//setting layer data
			if (key == searchKey)
			{
				//import layer setting
				vector<string> words = CL_String::tokenize(ditor->second.Get(), "=");
				if (words.size() < 2)
				{
					LogError("Layer key %s has bad data, should be something=something as the data.", key.c_str());
					ditor++;
					continue;
				}
				out += " "+ditor->second.Get();
			}
		}
		
		ditor++;
	}

	return out;
}

void MapExportXML::ExportLayer(Layer &lay, int layerID, CL_Rect &worldArea)
{
	
	tile_list tileList;
	
	vector<unsigned int> layersToInclude;
	layersToInclude.push_back(layerID);

	
	string layerName = lay.GetName();
	StringReplace(" ", "_", layerName);

	g_pMapManager->GetActiveMapCache()->AddTilesByRect(worldArea, &tileList, layersToInclude);

	if (tileList.empty())
	{
		LogMsg("Ignoring layer %s, it's empty", layerName.c_str());
		return;
	}
	
	if (lay.GetHasCollisionData())
	{
		LogMsg("Exporting tile layer %s as collision (noticed its has collision flag)", layerName.c_str());
	} else
	{
		LogMsg("Exporting tile layer %s as tiles/objects", layerName.c_str());
	}

	//try both ways?
	string extraData = GetExtraXMLParmsFromDB(*m_pMap->GetDB(), "export_layer_parm_"+lay.GetName(), false);
	if (lay.GetName() != layerName)
		extraData += GetExtraXMLParmsFromDB(*m_pMap->GetDB(), "export_layer_parm_"+layerName, false);

	if (lay.GetHasCollisionData())
	{
		extraData += " collision=\"yes\"";
	}

	extraData += " tilesizex=\""+toString(g_pMapManager->GetActiveMap()->GetDefaultTileSizeX())+"\"";
	extraData += " tilesizey=\""+toString(g_pMapManager->GetActiveMap()->GetDefaultTileSizeY())+"\"";

	extraData += " sort=\""+toString(lay.GetSort())+"\"";


	tile_list::iterator itor = tileList.begin();

	if (itor != tileList.end())
	{
		Tile *pTile = (*itor);
		if (pTile->GetType() == C_TILE_TYPE_PIC)
		{	
			
			TilePic *pTilePic = (TilePic*)pTile;
			HashedResource *pHashedResource = GetHashedResourceManager->GetResourceClassByHashedID(pTilePic->m_resourceID);
			extraData += " filename=\""+CL_String::get_filename(pHashedResource->m_strFilename)+"\"";
		}

	}

	StartGroup(layerName+extraData);

	while (itor != tileList.end())
	{
		Tile *pTile = (*itor);

		//LogMsg("Tile at %s", PrintVector(pTile->GetPos()).c_str());

		if (lay.GetHasCollisionData())
		{
		//collision data, slightly different format for this
	
		string temp = "<rect x=\""+toString(pTile->GetPos().x)+"\" y=\""+toString(pTile->GetPos().y)+"\"";
		temp +=" w=\""+toString(pTile->GetBoundsSize().x)+"\" h=\""+toString(pTile->GetBoundsSize().y)+ +"\"/>";
		Add(temp);
		} else
		{
	
			//tiles or objects
			if (pTile->GetType() == C_TILE_TYPE_PIC)
			{	
			TilePic *pTilePic = (TilePic*)pTile;
			string temp = "<tile tx=\""+toString(pTilePic->m_rectSrc.left)+"\" ty=\""+toString(pTilePic->m_rectSrc.top)+"\"";
			temp += " x=\""+toString(pTile->GetPos().x)+"\" y=\""+toString(pTile->GetPos().y)+"\"/>";
			Add(temp);

			} else if (pTile->GetType() == C_TILE_TYPE_ENTITY)
			{
				TileEntity *pTileEntity = (TileEntity*)pTile;
				MovingEntity *pEnt = pTileEntity->GetEntity();

				string name = pEnt->GetName();
				if (pEnt->GetData()->Exists("export_name"))
				{
					name = pEnt->GetData()->Get("export_name");
				}

				if (name.empty())
				{
					LogError("Warning: Ignoring entity because it has no name.  Edit its name property or add an _export_name user var to it.");
					itor++;
					continue;
				}
				string temp = "<"+name+" x=\""+toString(pTile->GetPos().x)+"\" y=\""+toString(pTile->GetPos().y)+"\"";

				string extraData = GetExtraXMLParmsFromDB(*pEnt->GetData(), "", true);

					temp += extraData+"/>";
				Add(temp);

				
			} else
			{
				LogMsg("Unknown tile type");
			}
	}

		itor++;
	}
	EndGroup(layerName);

}

void MapExportXML::ExportMap()
{
	//delete old file
	RemoveFile(m_path+m_filename);

	if (FileExists(m_path+m_filename))
	{
		LogError("Unable to delete old %s.. another app is using it? Oh well, adding to it I guess.", (m_path+"/"+m_filename).c_str());
	}

	//make sure we can write here
	FILE *fp = fopen( (m_path+m_filename).c_str(), "wb");
	if (!fp)
	{
		LogError("Unable to create the file %s. Incorrect path or we don't have access to write there.  Try ../../../%s instead maybe.",
			(m_path+m_filename).c_str(), m_filename.c_str());
		return;
	}
	fclose(fp);

	Add("<!-- Map exported by Novashell "+GetApp()->GetEngineVersionAsString()+" - www.rtsoft.com/novashell --> ");
	StartGroup("level");
	
	CL_Rectf r = m_pMap->ComputeWorldRect(1); //include editor only things.  0 to not

	LogMsg("World rect: %s", PrintRect(r).c_str());
	Add("<width>"+toString(r.get_width())+"</width>");
	Add("<height>"+toString(r.get_height())+"</height>");

	//first, get the layers we're going to save to
	LayerManager &layerMan = g_pMapManager->GetActiveMap()->GetLayerManager();
	vector<unsigned int> layerVec;
	layerMan.PopulateIDVectorWithAllLayers(layerVec);
	//sort it
	std::sort(layerVec.begin(), layerVec.end(), compareLayerBySort);
	
	for (unsigned int i=0; i < layerVec.size(); i++)
	{
		Layer &lay = layerMan.GetLayerInfo(layerVec[i]);

		string name = lay.GetName();
		
		if (!lay.GetShowInEditorOnly())
		{
			CL_Rect rec(r); //GCC has trouble sending this directly...
			ExportLayer(lay,  i,  rec);
		} else
		{
			LogMsg("Ignoring layer %s, it's 'only show in editor' flag is set", lay.GetName().c_str());
		}
	}
	EndGroup("level");
}

void MapExportXML::Add(string line)
{
	string indent;

	for (int i=0; i < m_indentLevel; i++)
	{
		indent += "  "; //two spaces.. or would tabs be better? dunno
	}

	
	line = indent + line + "\r\n";

add_text( line.c_str(), (m_path+m_filename).c_str());
}

void MapExportXML::StartGroup(string name)
{
	Add("<"+name+">");
	m_indentLevel++;
}

void MapExportXML::EndGroup(string name)
{
	m_indentLevel--;
	assert(m_indentLevel >= 0 && "Mismatched indentation, dork");
	Add("</"+name+">");
}

void MapExportXML::OnPackage(CL_Window *pWindow)
{
	m_buttonPushed = PACKAGE;
	pWindow->quit();
}