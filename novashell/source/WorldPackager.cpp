#include "AppPrecomp.h"
#include "WorldPackager.h"
#include "GameLogic.h"
#include "EntEditor.h"
#include "Console.h"

#define C_PACKER_KEY_COMMAND_LINE "_PACKER_COMMAND_LINE"
#define C_PACKER_COMMAND_LINE_DEFAULT "-res 1024 768"


WorldPackager::WorldPackager()
{
	m_buttonPushed = NONE;
}

WorldPackager::~WorldPackager()
{
}

bool WorldPackager::Init()
{
	
	if (GetGameLogic()->GetActiveWorldName() == "base")
	{
		CL_MessageBox::info("You must first load a world to package anything up.", GetApp()->GetGUI());
		return true;
	}
	
	CL_SlotContainer slots;
	CL_Point ptSize(630,450);
	int indent = 20;
	int buttonOffsetX = 10;
	int width = ptSize.x - (indent+buttonOffsetX)*2;
	CL_Window window(CL_Rect(0, 0, ptSize.x, ptSize.y), "World Packaging & Distribution", CL_Window::close_button, GetApp()->GetGUI());
	window.set_event_passing(false);
	window.set_position( (GetScreenX-ptSize.x)/2, C_EDITOR_MAIN_MENU_BAR_HEIGHT);

	CL_Button buttonCancel (CL_Point(buttonOffsetX+200,ptSize.y-45), "Cancel", window.get_client_area());
	CL_Button buttonOk (CL_Point(buttonOffsetX+100,ptSize.y-45), "Package", window.get_client_area());
	
	slots.connect(buttonCancel.sig_clicked(), (CL_Component*)&window, &CL_Component::quit);
	
	slots.connect(buttonOk.sig_clicked(), this, &WorldPackager::OnPackage, &window);

	//check boxes
	int checkX = buttonOffsetX+indent;
	int checkY = ptSize.y - 80;
	
	CL_CheckBox checkWinStandAlone (CL_Point(checkX,checkY),"Build Windows Standalone", window.get_client_area());
	checkWinStandAlone.set_checked(true); checkY -= 30;

	CL_CheckBox checkMacStandAlone (CL_Point(checkX,checkY),"Build Mac Standalone", window.get_client_area());
	checkMacStandAlone.set_checked(true); checkY -= 30;

	CL_CheckBox checkNovaworld (CL_Point(checkX,checkY),"Build .novazip", window.get_client_area());
	checkNovaworld.set_checked(true); checkY -= 30;
	
	CL_Label label1(CL_Point(buttonOffsetX, checkY), "Output Options:", window.get_client_area()); checkY -= 30;

	checkY -= 10;

	if ( GetGameLogic()->WorldData()->Exists(C_PACKER_KEY_COMMAND_LINE))
	{
		m_commandLine = GetGameLogic()->WorldData()->Get(C_PACKER_KEY_COMMAND_LINE);
	} else
	{
		m_commandLine = C_PACKER_COMMAND_LINE_DEFAULT;
	}
		
	CL_InputBox commandLine (CL_Rect(0,0, width, 24), m_commandLine, window.get_client_area());
	commandLine.set_position(checkX, checkY); checkY -= 20;

	CL_Label label4(CL_Point(checkX, checkY), "Extra Commandline parms:", window.get_client_area()); checkY -= 30;


	CL_CheckBox checkRetail (CL_Point(checkX,checkY),"Retail (doesn't include .lua source or allow the editor to function)", window.get_client_area());
	checkRetail.set_checked(true); checkY -= 30;

	CL_CheckBox checkWindowed (CL_Point(checkX,checkY),"Start in a window", window.get_client_area());
	checkY -= 30;

	CL_InputBox filesToIgnore (CL_Rect(0,0, width, 24), "*.psd *.sfk *.sfl Thumbs.db *.bak *.max", window.get_client_area());
	filesToIgnore.set_position(checkX, checkY); checkY -= 20;
	CL_Label label3(CL_Point(checkX, checkY), "File Extensions to ignore while compressing:", window.get_client_area()); checkY -= 30;


	CL_Label label2(CL_Point(buttonOffsetX, checkY), "Build Options:", window.get_client_area()); checkY -= 30;

	
	checkY -= 40;

	CL_Label label5(CL_Point(buttonOffsetX, checkY), 
		"This utility packages your game into ready to play formats for both Windows and OSX.\n\n" \
		".novazip - Anyone who already has novashell installed can just double click this to install and play your world.\n" \
		"Windows .zip - Makes a stand-alone zipped file that is ready to play on any Windows system.\n" \
		"Mac .zip - Makes a universal-binary based stand-alone zipped .app file that is ready to play on any OSX 10.4+ system."
	
		, window.get_client_area()); checkY -= 30;

	window.run();	//loop in the menu until quit() is called by hitting a button

	if (m_buttonPushed != PACKAGE) return true;
	
	if (!checkMacStandAlone.is_checked() && !checkWinStandAlone.is_checked() && !checkNovaworld.is_checked())
	{	
		CL_MessageBox::info("Um, you'd have to check one of the output options to actually do anything.\n\nIf an option is grayed out, that means support for building that isn't available on this platform yet.", GetApp()->GetGUI());
		return true;
	}

	m_commandLine = commandLine.get_text();

	if (m_commandLine != C_PACKER_COMMAND_LINE_DEFAULT)
	{
		//go ahead and save this for later
		GetGameLogic()->WorldData()->Set(C_PACKER_KEY_COMMAND_LINE, m_commandLine);
	}

	CL_String::trim_whitespace(m_commandLine);
	if (!m_commandLine.empty()) m_commandLine += " ";
	
	m_bRetail = checkRetail.is_checked();

	//add extra parms

	if (checkWindowed.is_checked()) m_commandLine += "-window ";
	if (m_bRetail) m_commandLine += "-retail ";
	m_commandLine += GetGameLogic()->GetActiveWorldName()+"." + string(C_WORLD_INFO_EXTENSION);
	m_commandLineTemp = "temp.tmp";

	RemoveFile(m_commandLineTemp);
	add_text(m_commandLine.c_str(), m_commandLineTemp.c_str());
	window.quit();

	ConvertFilesToIgnore(filesToIgnore.get_text());

	g_Console.SetOnScreen(true);
	UpdateLuaFiles();
	
	LogMsg("World path is %s", GetGameLogic()->GetWorldsDirPath().c_str());
	LogMsg("Active world is %s", GetGameLogic()->GetActiveWorldName().c_str());

	string outputDir = "";

	if (GetGameLogic()->GetWorldsDirPath() == "worlds")
	{
		//they are compressing a game from the novashell internal world directory
		if (GetApp()->GetPlatform() == C_PLATFORM_OSX)
		{
			outputDir = "../../../"; //build stuff out of our original .app thing
		}
	} else
	{
		//external novashell directory I guess
		outputDir = GetGameLogic()->GetWorldsDirPath()+"/";
	}

	if (checkNovaworld.is_checked())
	{
		string outputFile = outputDir+GetGameLogic()->GetActiveWorldName()+".novazip";
		PackageNovaZipVersion(outputFile);
	}

	if (checkWinStandAlone.is_checked())
	{
		string outputFile = outputDir+GetGameLogic()->GetActiveWorldName()+"_Win.zip";
		m_appDirName = GetGameLogic()->GetActiveWorldName();
		string locationOfWinExe = "packaging/win/game.exe";
	
		#ifdef _WINDOWS
			locationOfWinExe = "game.exe"; //we can just our own exe, can save on filesize
		#endif

		if (!FileExists(locationOfWinExe))
		{
			CL_MessageBox::info("It looks like this version of Novashell is missing the stuff to make a windows version. (packaging/win/game.exe is missing)", GetApp()->GetGUI());
			LogMsg("Aborting making windows version.\n");
		} else
		{
			PackageWindowsVersion(outputFile, locationOfWinExe);
		}

	}

	if (checkMacStandAlone.is_checked())
	{
		string outputFile = outputDir+GetGameLogic()->GetActiveWorldName()+"_Mac.zip";
		m_appDirName = GetGameLogic()->GetActiveWorldName()+".app";
		string locationOfMacFiles = "packaging/mac/";

		string locationOfMacBinaries = locationOfMacFiles;

#ifdef __APPLE__
		locationOfMacBinaries = "../"; //just reuse the real files, saves on installer size
#endif

		if (!FileExists(locationOfMacBinaries + "MacOS/novashell"))
		{
			CL_MessageBox::info("It looks like this version of Novashell is missing the stuff to make a mac version. (packaging/mac dir is missing)", GetApp()->GetGUI());
			LogMsg("Aborting making mac version, unable to find %s.\n", (locationOfMacBinaries + "MacOS/novashell").c_str());
			
		} else
		{
			PackageMacVersion(outputFile, locationOfMacFiles, locationOfMacBinaries);
		}
	}

	LogMsg("***********");
	if (outputDir.empty()) outputDir = GetApp()->GetBaseDirectory();
	LogMsg("Finished.  Output directory is %s.", outputDir.c_str());
	RemoveFile(m_commandLineTemp);

	
	return true; //success
}

void WorldPackager::PackageWindowsVersion(string outputFile, string locationOfWinExe)
{
	LogMsg("Creating windows version...");
	UpdateScreen();

	Scan("base", true, m_appDirName+"/base/"); //special case for the base library

	string worldDestinationDir = m_appDirName+"/worlds/";
	ScanWorlds(worldDestinationDir); //add all world files

	//add files into the base directory
	m_zip.add_file(locationOfWinExe,  m_appDirName+"/game.exe", true);

	m_zip.add_file(m_commandLineTemp,  m_appDirName+"/command_line.txt", true);

	LogMsg("Creating %s...", outputFile.c_str());
	UpdateScreen();
	m_zip.save(outputFile);
	m_zip = CL_Zip_Archive(); //clear it out
	LogMsg("Successfully built windows version.\n");
}

void WorldPackager::PackageMacVersion(string outputFile, string locationOfMacFiles, string locationOfMacBinaries)
{
	LogMsg("Creating mac version...");
	UpdateScreen();

	Scan("base", true,  m_appDirName+"/Contents/Resources/base/"); //special case for the base library
	
	Scan(locationOfMacBinaries+"MacOS", true, m_appDirName+"/Contents/MacOS/");
	Scan(locationOfMacBinaries+"Frameworks", true, m_appDirName+"/Contents/Frameworks/");
	
	
	m_zip.add_file(locationOfMacFiles+"Info.plist", m_appDirName+"/Contents/Info.plist", true);
	m_zip.add_file(locationOfMacFiles+"PkgInfo", m_appDirName+"/Contents/PkgInfo", true);
	m_zip.add_file(locationOfMacFiles+"Resources/app.icns", m_appDirName+"/Contents/Resources/app.icns", true);
	m_zip.add_file(m_commandLineTemp, m_appDirName+"/Contents/Resources/command_line.txt", true);

	string worldDestinationDir = m_appDirName+"/Contents/Resources/worlds/";
	ScanWorlds(worldDestinationDir); //add all world files

	//add files into the base directory
	
	LogMsg("Creating %s...", outputFile.c_str());
	UpdateScreen();
	m_zip.save(outputFile);
	m_zip = CL_Zip_Archive(); //clear it out

	LogMsg("Successfully built mac version.\n");
}

void WorldPackager::PackageNovaZipVersion(string outputFile)
{
	LogMsg("Creating .novazip version...");
	UpdateScreen();

	ScanWorlds("/", true); //add all world files

	LogMsg("Creating %s...", outputFile.c_str());
	UpdateScreen();
	m_zip.save(outputFile);
	m_zip = CL_Zip_Archive(); //clear it out
	LogMsg("Successfully built .novazip version.\n");

}

bool WorldPackager::ScanWorlds(string destinationDirectory, bool bIsNovaZip)
{
	vector<string> modPaths = GetGameLogic()->GetModPaths();

	for (unsigned int i=0; i < modPaths.size(); i++)
	{
		LogMsg("Scanning %s...", modPaths[i].c_str());
		UpdateScreen();
		Scan(modPaths[i], false, destinationDirectory+CL_String::get_filename(modPaths[i])+"/", 0, "", bIsNovaZip);
	}

	LogMsg("");
	return true;
}

void WorldPackager::ConvertFilesToIgnore(string fileList)
{
	vector<string> words = CL_String::tokenize(fileList, " ", true);

	for (unsigned int i=0; i < words.size(); i++)
	{
		if (words[i][0] == '*' && words[i][1] == '.')
		{
			m_fileExtensionsToIgnore.push_back(CL_String::to_lower(words[i].substr(2, words[i].length()-2)));
		} else
		{
			m_filesToIgnore.push_back(CL_String::to_lower(words[i]));
		}
	}
}

bool WorldPackager::UpdateLuaFiles()
{
	LogMsg("Checking to make sure all .loac files are up to date...");
	LogMsg("");
	UpdateScreen();

	GetScriptManager->CompileAllLuaFilesRecursively("base");

	vector<string> modPaths = GetGameLogic()->GetModPaths();
	
	for (unsigned int i=0; i < modPaths.size(); i++)
	{
		GetScriptManager->CompileAllLuaFilesRecursively(modPaths[i]);
		UpdateScreen();
	}
	
	return true;
}

void WorldPackager::Scan(string dir, bool bIsBase, string targetDir, int recursionDepth, string originalDir, bool bIsNovaZip)
{
	CL_DirectoryScanner scanner;
	scanner.scan(dir, "*");
	string fileExtension;

	if (recursionDepth == 0)
	{
		originalDir = dir;
	}

	if (!bIsBase && recursionDepth == 0)
	{
		//some hackish things to put the .novashell file one folder up
		int startingIndex = 0;
		StringReplace("\\", "/", targetDir);
		if (targetDir[0] == '/') startingIndex = 1; //skip the first slash
	
		string novaInfoDir = targetDir.substr(startingIndex, targetDir.length()-1);
		
		if (bIsNovaZip)
		{
			m_zip.add_file(dir+"."+string(C_WORLD_INFO_EXTENSION), CL_String::get_filename(dir)+"."+string(C_WORLD_INFO_EXTENSION), true);
		} else
		{
			m_zip.add_file(dir+"."+string(C_WORLD_INFO_EXTENSION), CL_String::get_path(novaInfoDir)+"/"+CL_String::get_filename(dir)+"."+string(C_WORLD_INFO_EXTENSION), true);
		}
	}

	string fileName;
	while (scanner.next())
	{
		std::string file = scanner.get_name();
		if (!scanner.is_directory())
		{
			fileExtension = CL_String::to_lower(CL_String::get_extension(scanner.get_name()));
			if (fileExtension == "lua")
			{
				if (m_bRetail) continue; //don't actually include this file
			}

			//is it on our ignore list?
			bool bIgnoreFile = false;

			for (unsigned int i=0; i < m_fileExtensionsToIgnore.size(); i++)
			{
				if (fileExtension == m_fileExtensionsToIgnore[i])
				{
					bIgnoreFile = true;
					break;
				}
			}
			
			if (!bIgnoreFile)
			{
				fileName = CL_String::to_lower(scanner.get_name());

				for (unsigned int i=0; i < m_filesToIgnore.size(); i++)
				{
					if (fileName == m_filesToIgnore[i])
					{
						bIgnoreFile = true;
						break;
					}
				}
			}


			if (bIgnoreFile) continue;
			//put it in our target folder, but with the relative pathing required by its internal tree
			string zipName = targetDir + scanner.get_pathname().substr(originalDir.length()+1, (scanner.get_pathname().length()+1) - originalDir.length());
			int startingIndex = 0;
			StringReplace("\\", "/", zipName);
			if (zipName[0] == '/') startingIndex = 1; //skip the first slash
			m_zip.add_file(scanner.get_pathname(), &zipName.c_str()[startingIndex], true);

		} else
		{
			//it's a directory, deal with it
			if (scanner.get_name()[0] == '.') continue;
			Scan(dir+"/"+scanner.get_name(), bIsBase, targetDir, ++recursionDepth, originalDir);
			recursionDepth--;
		}
	}	
}


void WorldPackager::UpdateScreen()
{
	CL_System::keep_alive();
	CL_Display::clear();
	g_Console.Render();
	//GetGameLogic()->GetGameGUI()->show();
	CL_Display::flip(); //show it now
}
void WorldPackager::OnPackage(CL_Window *pWindow)
{
	m_buttonPushed = PACKAGE;
	pWindow->quit();
}