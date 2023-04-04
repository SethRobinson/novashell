#include "AppPrecomp.h"
#include "EntWorldDialog.h"
#include "GameLogic.h"

int g_defaultWorldDialogSelection = 0;



bool LocateWorldPath(string m_path, string &pathOut)
{
	
	if (FileExists( (m_path+ "." + string(C_WORLD_INFO_EXTENSION)) ))
	{
		pathOut = m_path;
		return true;
	}
	
	
	string modInfoFile = m_path+ "." + string(C_WORLD_INFO_EXTENSION);

	if (!FileExists( modInfoFile) )
	{
		//try another way
		modInfoFile = GetGameLogic()->GetWorldsDirPath() + "/"  +m_path+ "." + string(C_WORLD_INFO_EXTENSION);
		if (FileExists(modInfoFile))
		{
			//yep, use this
			pathOut = GetGameLogic()->GetWorldsDirPath() + "/"  +m_path;
			return true;
		} else
		{
			//try another way
			modInfoFile = "worlds/" +m_path+ "." + string(C_WORLD_INFO_EXTENSION);
			if (FileExists(modInfoFile))
			{
				//yep, use this
				pathOut = "worlds/" +m_path;
				return true;
			}
		}
	}


	if (!FileExists( (modInfoFile+"." + string(C_WORLD_INFO_EXTENSION))) )
	{
		LogError("Unable to locate %s.  World is missing perhaps?",  modInfoFile.c_str());
		return false;
	} else
	{
		pathOut = m_path;
	}

	return true;
}

void SetupModPathsFromWorldInfo(string modPath)
{
	//get data about it
	ModInfoItem modInfo;
	
	if (CL_String::get_extension(modPath) == C_WORLD_INFO_EXTENSION)
	{
		//remove the .novashell part
		modPath = modPath.substr(0, modPath.size()- (strlen(C_WORLD_INFO_EXTENSION)+1));
	}

	string modLocation;
	if (LocateWorldPath(modPath, modLocation))
	{
		ReadWorldInfoFile(&modInfo, modLocation+"."+C_WORLD_INFO_EXTENSION);

		//add mod required mod paths
		for (unsigned int i=0; i < modInfo.m_requestedResources.size(); i++)
		{
			//check versions here later?	
			string pathOut;
			if (!LocateWorldPath(modInfo.m_requestedResources[i].m_modPath, pathOut))
			{
				char st[512];
				sprintf(st, "This world requires another world, %s (v%.2f) which is missing.\n\nThis world will probably not run right.",
					modInfo.m_requestedResources[i].m_modPath.c_str(), modInfo.m_requestedResources[i].m_requestedVersion);
					ShowMessage("Can't find a world dependency", st);
			} else
			{			
				GetGameLogic()->AddModPath(modInfo.m_requestedResources[i].m_modPath);
			}
		}


		if (modInfo.m_engineVersionRequested > GetApp()->GetEngineVersion())
		{
			char st[512];
			sprintf(st, "This world requires version %.02f of Novashell.\n\nYou only have version %0.2f, please upgrade.\n\nWe'll try anyway though.",
				modInfo.m_engineVersionRequested, GetApp()->GetEngineVersion());
			ShowMessage("Can't find a world dependency", st);

		}

		
		GetGameLogic()->AddModPath(modPath);
		GetApp()->SetWindowTitle(modInfo.m_stDisplayName);

	}
}


EntWorldDialog::EntWorldDialog(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pListWorld = NULL;
	m_pWindow = NULL;

	m_slots.connect (CL_Keyboard::sig_key_down(), this, &EntWorldDialog::OnButtonDown);

	SetName("ChooseWorldDialog");
	ScanDirectoriesForModInfo();
	BuildWorldListBox();

}

EntWorldDialog::~EntWorldDialog()
{
	SAFE_DELETE(m_pListWorld);
	SAFE_DELETE(m_pWindow);
}


string EntWorldDialog::HandleMessageString(const std::string &msg)
{

	if (msg == "BootUpWorld")
	{
		GetGameLogic()->ClearModPaths();
		SetupModPathsFromWorldInfo(m_modInfo[g_defaultWorldDialogSelection].m_stDirName);
		GetGameLogic()->SetRestartEngineFlag(true);
		SetDeleteFlag(true);
	} else
	{
		LogError("Unknown message: %s", msg.c_str());
	}

	return "";
	
}

void EntWorldDialog::ChangeSelection(int offset)
{
	int selected = m_pListWorld->get_current_item();
	selected = altmod(selected + offset, m_pListWorld->get_count());
	m_pListWorld->set_current_item(selected);

}

void EntWorldDialog::OnButtonDown(const CL_InputEvent &key)
{
	switch(key.id)
	{
	case CL_KEY_UP:
		ChangeSelection(-1);
		break;

	case CL_KEY_DOWN:
		ChangeSelection(1);
		break;

	}
}

bool EntWorldDialog::WorldAlreadyInList(const ModInfoItem &m)
{
	for (unsigned int i=0; i < m_modInfo.size(); i++)
	{
		if (m_modInfo[i].m_stDisplayName == m.m_stDisplayName)
		{
			return true;
		}
	}

	return false;
}

void EntWorldDialog::ScanDirectoryForModInfo(const string &path)
{

	//scan map directory for available maps
	CL_DirectoryScanner scanner;

	scanner.scan(path, string("*.")+C_WORLD_INFO_EXTENSION);
	while (scanner.next())
	{
		std::string file = scanner.get_name();
			if (scanner.get_name()[0] != '_')
				if (scanner.get_name()[0] != '.')
				{
				
						//no underscore at the start, is this a world?
						ModInfoItem m;

						m.m_stDirName = scanner.get_name();
						if (ReadWorldInfoFile(&m, path +"/"+scanner.get_name()))
						{
							//						LogMsg("Found %s", scanner.get_name().c_str());
				
							if (!WorldAlreadyInList(m))
							{
								m_modInfo.push_back(m);
							}
						}
				}

		
	}
}

void EntWorldDialog::ScanDirectoriesForModInfo()
{
	m_modInfo.clear();
	
	ScanDirectoryForModInfo(GetGameLogic()->GetWorldsDirPath());

	if (GetGameLogic()->GetWorldsDirPath() != "worlds")
	{
		//might as well add our local worlds too
		ScanDirectoryForModInfo("worlds");
	}
}

void EntWorldDialog::BuildWorldListBox()
{

	SAFE_DELETE(m_pListWorld); //just in case it was already initted
	SAFE_DELETE(m_pWindow);
	
	CL_Rect rectSize = CL_Rect(0,0,400,300);
	CL_Point ptPos = CL_Point((GetScreenX/2) - rectSize.get_width()/2 , (GetScreenY/2) - rectSize.get_height()/2);

	m_pWindow = new CL_Window(rectSize+ptPos, "Novashell Game Creation System " + GetApp()->GetEngineVersionAsString() + " - Choose world to load", CL_Window::close_button, GetApp()->GetGUI()->get_client_area());

	int buttonCount = 2;

	CL_Rect rectListBox = rectSize;
	int borderOffset = 5;
	rectListBox.left = borderOffset;
	rectListBox.top =  borderOffset;
	rectListBox.right -= borderOffset*2;
	rectListBox.bottom -= borderOffset*2;
	rectListBox.bottom -= m_pWindow->get_component_at(0,0)->get_height();

	rectListBox.bottom -= (26*buttonCount);

	//offset more, so we have a place to draw the buttons

	m_pListWorld = new CL_ListBox(rectListBox, m_pWindow->get_client_area());
	
	

	CL_ListItem *pItem;
	
	for (unsigned int i=0; i < m_modInfo.size(); i++)
	{
		pItem = new CL_ListItem;
		pItem->str = m_modInfo[i].m_stDisplayName;
		pItem->user_data = i;
		m_pListWorld->insert_item(pItem,-1,true);
	}

	if (m_modInfo.size() == 0)
	{
		pItem = new CL_ListItem;
		pItem->str = "No worlds installed";
		pItem->user_data = -1;
		m_pListWorld->insert_item(pItem,-1,true);
	}

	
	//make sure the default is in a valid range
	g_defaultWorldDialogSelection = min(g_defaultWorldDialogSelection, m_pListWorld->get_count());

	m_pListWorld->set_current_item(g_defaultWorldDialogSelection);
	rectListBox.top = rectListBox.bottom + borderOffset;
	rectListBox.bottom = rectListBox.top + 22;
	
	CL_Button *pBut = new CL_Button(rectListBox, "Play", m_pWindow->get_client_area());

	//link to things so we know when they are clicked
	m_slots.connect( m_pListWorld->sig_activated(), this, &EntWorldDialog::OnSelected);
	m_slots.connect( pBut->sig_clicked(), this, &EntWorldDialog::OnClickLoad);

	rectListBox.apply_alignment(origin_top_left,0, -26);
	CL_Button *pButOnline = new CL_Button(rectListBox, "Connect online to download more", m_pWindow->get_client_area());
	m_slots.connect( pButOnline->sig_clicked(), this, &EntWorldDialog::OnClickConnect);
	
	m_pListWorld->get_children().front()->set_focus(); //set focus to the real client area
	
}

void EntWorldDialog::OnClickLoad()
{
	OnSelected(-1);
}

void EntWorldDialog::OnClickConnect()
{
	ShowMessage("Seth is lazy", "In-game browsing/rating not functional yet!");
}

void EntWorldDialog::OnSelected(int selItem)
{
	
	    //let's load the world
	
		CL_ListItem *pItem = m_pListWorld->get_item(m_pListWorld->get_current_item());

		int modID = pItem->user_data;

		if (modID == -1)
		{
			LogError("No world to load.");
			return;
		}

		g_defaultWorldDialogSelection = modID;
		
		//don't load it right away, wait for the mouse message to pass first

		ScheduleSystem(150, ID(), "BootUpWorld");

		m_pWindow->quit();


}
