
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 23:1:2006   12:34
*/

 
#ifndef CollisionData_HEADER_INCLUDED // include guard
#define CollisionData_HEADER_INCLUDED  // include guard

using namespace std;

#include "PointList.h"
#include "Tile.h"


typedef std::list<PointList> line_list;

class CollisionData
{
public:

	CollisionData();
	CollisionData(const CL_Rect &rec);
    ~CollisionData();
	void Clear();
	int GetVertCount(); //total verts of all shapes
	const CL_Rect & GetRect() {return m_rect;} //resources use this, don't touch it
	const CL_Rectf & GetCombinedCollisionRect();
	line_list * GetLineList() {return &m_lineList;}
	void Serialize(CL_FileHelper &helper);
	bool HasData();
	void RemoveOffsets();
	void Load(const string &fileName);
	void SetDataChanged(bool bNew) {m_dataChanged = bNew;}
	void SetScale(const CL_Vector2 &vScale);
	const CL_Vector2 & GetScale() {return m_vecScale;}
	bool GetDataChanged() {return m_dataChanged;}
	const CL_Vector2 &GetCombinedOffsets();
	bool CheckForErrors(string extraText, bool bShow);
	void ApplyOffsetToAll(const CL_Vector2 &vOffset); //applied to every vertex individually

protected:

	void SaveIfNeeded();
	void ApplyScaleToAll(const CL_Vector2 &vScale);
	int CountValidLineLists();
	void SetRequestRectRecalculation() {m_bNeedsRecalc = true;}
	void RecalculateCombinedOffsets();
	void SetDefaults();

private:
	CL_Rect m_rect;
	CL_Rectf m_collisionRect;
	line_list m_lineList;
	string m_fileName; //if not empty, it means we were loaded from disk and should also
	//save if changed
	bool m_dataChanged;
	CL_Vector2 m_vecScale;
	bool m_bNeedsRecalc;
	CL_Vector2 m_vecCombinedOffset;


};

void CreateCollisionDataWithTileProperties(Tile *pTile, CollisionData &colOut);


#endif                  // include guard
