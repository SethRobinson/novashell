#ifndef AppUtils_HEADER_INCLUDED // include guard
#define AppUtils_HEADER_INCLUDED  // include guard

#include "Screen.h"
#include "misc/C2DMAtrix.h"

bool IsControlKeyDown(); //is Control or the Command key (OSX only) down?

void BlitMessage(string msg, int posY = -1);
string ColorToString(const CL_Color &colr);
CL_Color StringToColor(const string &stColor);
CL_Vector2 StringToVector(const string &stVec);
CL_Rect StringToRect(const string &stColor);
string RectToString(const CL_Rect &r);
string VectorToString(const CL_Vector2 * pVec);
string VectorToStringEx(const CL_Vector2 * pVec);
string RectToStringEx(const CL_Rect * pR);
string RectfToStringEx(const CL_Rectf * pR);
string OriginToString(CL_Origin o);

CL_Rectf ConvertWorldRectToScreenRect(CL_Rectf rArea);
string GetEntityNameByID(int id);
void RenderVertexList(const CL_Vector2 &pos, CL_Vector2 *pVertArray, int vertCount, CL_Color &colr, CL_GraphicContext *pGC);
CL_Vector2 MakeNormal(CL_Vector2 &a, CL_Vector2 &b);
void RenderVertexListRotated(const CL_Vector2 &pos, CL_Vector2 *pVertArray, int vertCount, CL_Color &colr, CL_GraphicContext *pGC,  float angleRad);
void DrawLineFromWorldCoordinates(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC);
void DrawRectFromWorldCoordinates(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC);
void DrawRectFromWorldCoordinatesRotated(CL_Vector2 vecStart, CL_Vector2 vecStop, CL_Color borderColor, CL_GraphicContext *pGC, float angleRad);
string GetStrippedFilename(string str);
unsigned int FileNameToID(const char * filename);
bool GetTileLineIntersection(const CL_Vector2 &vStart, const CL_Vector2 &vEnd, tile_list &tList, CL_Vector2 *pvColPos, Tile* &pTileOut, const Tile * const pTileToIgnore  = NULL, int limitSearchToThisTileType = C_TILE_TYPE_BLANK, bool bIgnoreMovingCreatures = false);
string PrintVector(CL_Vector2 v);
string PrintRect(CL_Rectf r);
string PrintRectInt(CL_Rect r);

void ResetFont(CL_Font *pFont); //set the centering, color and alpha back to default 
string ExtractFinalDirName(string path); //not tested with paths with filenames yet
bool compareLayerBySort(unsigned int pA, unsigned int pB); //for use with stl::sort
void StringReplace(const std::string& what, const std::string& with, std::string& in);
bool ConfirmMessage(string title, string msg);
bool RotateVectorTowardsAnotherVector(CL_Vector2 &vecFacing, CL_Vector2 vecFacingTarget, float maxTurn);
void DrawCenteredBox(const CL_Vector2 &a, int size, CL_Color col, CL_GraphicContext *pGC);
void DrawCenteredBoxWorld(const CL_Vector2 &a, int size, CL_Color col, CL_GraphicContext *pGC);
double GetAngleBetweenVectorFacings(CL_Vector2 v1, CL_Vector2 v2);
void DrawLineWithArrowWorld(CL_Vector2 from, CL_Vector2 to, double size, CL_Color &col, CL_GraphicContext* pGC);
void DrawLineWithArrow(CL_Vector2 from, CL_Vector2 to, double size, CL_Color &col, CL_GraphicContext* pGC);
CL_Vector2 Vector2Perp(const CL_Vector2 &v);
void CenterComponentOnScreen(CL_Component *pComp);
void DrawWithShadow(int x, int y, const string &msg, CL_Color col = CL_Color(255,255,30));
void DrawBullsEyeWorld(CL_Vector2 vecPos, CL_Color col, int size, CL_GraphicContext *pGC);
void DrawTextBar(int posY, CL_Color barColor, string msg);

string SimplifyNumberString(string num);
float SinGamePulseByMS(int ms);

typedef cl_uint32 CL_DWORD_PTR; //note, for a 64 bit compiler we'll need to make sure this is 64 bits somehow...

#define CL_MAKELONG(a, b)      ((cl_uint32)(((cl_uint16)((CL_DWORD_PTR)(a) & 0xffff)) | ((cl_uint32)((cl_uint16)((CL_DWORD_PTR)(b) & 0xffff))) << 16))
#define CL_LOWORD(l)           ((cl_uint16)((cl_uint32)(l) & 0xffff))
#define CL_HIWORD(l)           ((cl_uint16)((cl_uint32)(l) >> 16))


	#define cl_max(a,b)    (((a) > (b)) ? (a) : (b))
	#define cl_min(a,b)    (((a) < (b)) ? (a) : (b))

class GameTimer
{
public:

	GameTimer()
	{
		m_timer = 0;
		m_bUseSystemTimer = false;
	}
	void Reset();
	void SetIntervalReached();
	bool IntervalReached();
	void SetUseSystemTime(bool bNew) {m_bUseSystemTimer = bNew; Reset();}

	bool IsActive() {return m_timer != 0;}
	
	void SetInterval(int interval)
	{
		m_interval = interval;
		Reset();
	}

	unsigned int m_timer;
	int m_interval;
	bool m_bUseSystemTimer; //if true, doesn't pause with the game
};
int VectorSign(const CL_Vector2 &v1, const CL_Vector2 &v2);

//RoundNearest written by Extrarius - http://www.gamedev.net/community/forums/topic.asp?topic_id=377593&whichpage=1&#2492400
#include <cmath>

const int     MaxInt    = (std::numeric_limits<int>::max)();
const double  MaxDouble = (std::numeric_limits<double>::max)();
const double  MinDouble = (std::numeric_limits<double>::min)();
const float   MaxFloat  = (std::numeric_limits<float>::max)();
const float   MinFloat  = (std::numeric_limits<float>::min)();

inline double Vec2DDistance(const CL_Vector2 &v1, const CL_Vector2 &v2)
{

	double ySeparation = v2.y - v1.y;
	double xSeparation = v2.x - v1.x;

	return sqrt(ySeparation*ySeparation + xSeparation*xSeparation);
}

inline double Vec2DDistanceSq(const CL_Vector2 &v1, const CL_Vector2 &v2)
{

	double ySeparation = v2.y - v1.y;
	double xSeparation = v2.x - v1.x;

	return ySeparation*ySeparation + xSeparation*xSeparation;
}

template <typename NumberType>
const NumberType RoundNearest(
							  const NumberType &p_Number,
							  const NumberType &p_Precision,
							  const NumberType &p_Center = NumberType(0))
{
	if(p_Number >= p_Center)
	{
		const NumberType Intermediate = (p_Number - p_Center) + (p_Precision / NumberType(2));
		return (floor(Intermediate / p_Precision) * p_Precision) + p_Center;
	}
	else
	{
		const NumberType Intermediate = (p_Number - p_Center) - (p_Precision / NumberType(2));
		return (ceil(Intermediate / p_Precision) * p_Precision) + p_Center;
	}
}


#endif                  // include guard



