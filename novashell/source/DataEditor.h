//  ***************************************************************
//  DataEditor - Creation date: 02/13/2008
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2008 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef DataEditor_h__
#define DataEditor_h__

class DataManager;
class CL_ListBox;
class DataObject;

class DataEditor
{
public:
	DataEditor();
	virtual ~DataEditor();

	bool Init(const string sTitle, const string sDescription, DataManager *pData);
	
protected:

	enum eExitMode
	{
		CLICKED_CANCEL,
		CLICKED_OK
	};

void SetExitMode(eExitMode exitMode);

private:

	eExitMode m_exitMode;
};

//some misc global helpers to avoid code duplication

void OnPropertiesEditData(const CL_InputEvent &input, CL_ListBox *pList);
void PropertiesSetDataManagerFromListBox(DataManager *pData, CL_ListBox &listBox);
string PropertyMakeItemString(DataObject &o);
void CreateEditDataDialog(DataObject &o);
void OnPropertiesRemoveData(CL_ListBox *pListBox);
void OnPropertiesAddData(CL_ListBox *pListBox);




#endif // DataEditor_h__
