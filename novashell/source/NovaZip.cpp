#include "AppPrecomp.h"
#include "NovaZip.h"
#include "AppUtils.h"
#include "GameLogic.h"

void RemoveEndingCR(string &text)
{
	if (*text.rbegin() == 10)
	{
		text.erase(text.size()-1, 1);
	}

	if (*text.rbegin() == '\n')
	{
		text.erase(text.size()-1, 1);
	}

}

void ParseWorldInfoFile(ModInfoItem *pModInfo, CL_InputSource *pFile)
{
	while (pFile->tell() < pFile->size())
	{
		vector<string> tok = CL_String::tokenize(GetNextLineFromFile(pFile), "|", true);

		if (CL_String::compare_nocase(tok[0], "world_name"))
		{
			pModInfo->m_stDisplayName = tok[1];

			RemoveEndingCR(pModInfo->m_stDisplayName);
		}

		if (CL_String::compare_nocase(tok[0], "engine_version_requested"))
		{
			pModInfo->m_engineVersionRequested = CL_String::to_float(tok[1]);
		}

		if (CL_String::compare_nocase(tok[0], "add_world_requirement"))
		{
			ResourceInfoItem res;
			res.m_modPath = tok[1];
			res.m_requestedVersion = CL_String::to_float(tok[2]);
			pModInfo->m_requestedResources.push_back(res);
		}

	}

}


bool ReadWorldInfoFile(ModInfoItem *pModInfo, const string fileName)
{

	CL_InputSource_File *pFile = NULL;

	try
	{
		pFile = new CL_InputSource_File(fileName);

	} catch(CL_Error error)
	{
		//file doesn't exist
		LogMsg("Unable to find %s, ignoring mod", fileName.c_str());
		delete pFile;
		return false;
	}

	ParseWorldInfoFile(pModInfo, pFile);

	delete pFile;
	return true;

}

NovaZip::NovaZip()
{
}

NovaZip::~NovaZip()
{
}

bool NovaZip::InstallWorld( string fileName, string *pWorldDirOut )
{
	*pWorldDirOut = "";
	
	CL_Zip_Archive zip(fileName);
	std::vector<CL_Zip_FileEntry> &file_list = zip.get_file_list();
	int size = file_list.size();
	
	string novashellFileName = "";

	for (int i=0; i<size; i++)
	{
		//LogMsg("Checking %s..", file_list[i].get_filename().c_str()); 
		if (file_list[i].get_filename().find(".novashell") != string::npos)
		{
			novashellFileName = file_list[i].get_filename();

			//make sure its valud
			if (novashellFileName.find_first_of("/\\") != string::npos)
			{
				//this shouldn't happen, they must have it in a weird directory
				novashellFileName = "";
			}
			break;
		}
	}

	if (novashellFileName == "")
	{
		LogError("Unable to find *.novashell file in the %s zip.  It must be in the root directory!",
			fileName.c_str());
	}

	//extract file

	CL_InputSource *input = zip.open_source(novashellFileName);

	ModInfoItem modInfo;
	modInfo.m_stDirName = novashellFileName.substr(0, novashellFileName.find(string(".") + C_WORLD_INFO_EXTENSION));
	ParseWorldInfoFile(&modInfo, input);
	delete input;
	
	if (!ConfirmMessage("Installing " + modInfo.m_stDirName, "Would you like to install \"" + modInfo.m_stDisplayName+"\"?" ))
	{
		return false;
	}

	//got the go ahead
	string deleteDir = GetApp()->GetBaseDirectory() +GetGameLogic()->GetWorldsDirPath() + "/"+modInfo.m_stDirName;
	
	if (FileExists(deleteDir + string(".") + C_WORLD_INFO_EXTENSION))
	{
		if (!ConfirmMessage("World" + modInfo.m_stDirName + " Overwrite Confirmation", "Warning: This will delete your existing version of this world along with any saved games!" ))
		{
			return false;
		}
		LogMsg("Deleting old stuff from %s", deleteDir.c_str());
	}

	//actually write out all the files
	UnzipToDir(zip, GetApp()->GetBaseDirectory() +GetGameLogic()->GetWorldsDirPath());

	*pWorldDirOut = modInfo.m_stDirName;
	return true; //success
}