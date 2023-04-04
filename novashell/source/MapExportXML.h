//  ***************************************************************
//  MapExportXML - Creation date: 05/03/2011
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2011 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef MapExportXML_h__
#define MapExportXML_h__

class DataManager;
class Map;
class Layer;
class CL_Rect;

class MapExportXML
{
public:
	MapExportXML();
	virtual ~MapExportXML();

	bool Init();

	void OnPackage(CL_Window *pWindow);
	void UpdateScreen();
	void ExportMap();
	void ExportLayer(Layer &lay, int layerID, CL_Rect &worldArea);
	void Add(string line);
	void StartGroup(string name);
	void EndGroup(string name);
	string GetExtraXMLParmsFromDB(DataManager &db, string searchKey, bool bFromEntity);
	enum eButtonPushed
	{
		NONE,
		PACKAGE
	};
protected:
private:
	eButtonPushed m_buttonPushed;
	string m_path;
	string m_filename;
	
	//shortcut so I don't have to type long things everywhere
	DataManager *m_pMapDB; 
	DataManager *m_pWorldDB;
	Map * m_pMap;
	
	int m_indentLevel;
};

#endif // MapExportXML_h__