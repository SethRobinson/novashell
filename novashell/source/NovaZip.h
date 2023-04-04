//  ***************************************************************
//  NovaWorld - Creation date: 05/12/2008
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2008 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef NovaWorld_h__
#define NovaWorld_h__


class ResourceInfoItem
{
public:

	string m_modPath;
	float m_requestedVersion;
};

class ModInfoItem
{
public:

	string m_stDirName;
	string m_stDisplayName;
	float m_engineVersionRequested;

	vector<ResourceInfoItem> m_requestedResources;

};

class NovaZip
{
public:
	NovaZip();
	virtual ~NovaZip();

	
	bool InstallWorld(string fileName, string *pWorldDirOut );

protected:

private:
};

void ParseWorldInfoFile(ModInfoItem *pModInfo, CL_InputSource *pFile);
bool ReadWorldInfoFile(ModInfoItem *pModInfo, const string fileName);


#endif // NovaWorld_h__