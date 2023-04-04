//  ***************************************************************
//  EntWorldDialog - Creation date: 09/15/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef EntWorldDialog_h__
#define EntWorldDialog_h__

#include "Map.h"
#include "BaseGameEntity.h"
#include "NovaZip.h"

class EntWorldDialog: public BaseGameEntity
{
public:
	EntWorldDialog();
	virtual ~EntWorldDialog();
	virtual string HandleMessageString(const std::string &msg);


private:

	void OnButtonDown(const CL_InputEvent &key);
	void BuildWorldListBox();
	void ScanDirectoryForModInfo(const string &path);
	void ScanDirectoriesForModInfo();
	void OnClickLoad();
	void OnSelected(int selItem);
	void ChangeSelection(int offset);
	bool WorldAlreadyInList(const ModInfoItem &m);
	void OnClickConnect();

	CL_Window *m_pWindow;
	CL_ListBox *m_pListWorld; //control which worlds are drawn

	vector<ModInfoItem> m_modInfo;
	CL_SlotContainer m_slots; //generic one, easier

};

void SetupModPathsFromWorldInfo(string modPath);
bool LocateWorldPath(string m_path, string &pathOut);

#endif // EntWorldDialog_h__