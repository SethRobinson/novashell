// ***************************************************************
//  TextManager - date: 04/18/2006
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
//  Copyright (C) 2006 Robinson Technologies - All Rights Reserved

#ifndef TextManager_HEADER_INCLUDED // include guard
#define TextManager_HEADER_INCLUDED  // include guard

//this class helps us display the player or NPC's text in a controlled way, so it stays
//on the screen and is always readable

class MovingEntity;
class TextManager;

class TextObject
{
public:

	TextObject(TextManager *pTextManager);

	void EntDeleted(int ID);
	int Init(const string &text, MovingEntity *pEnt, int fontID);
	void Render();
	bool Update(); //return false to delete
	unsigned int GetTimeCreated() const {return m_timeCreated;}
	bool GetVisible() {return m_bVisible;} 
	const MovingEntity * GetEntity(){return m_pEntity;}
	const CL_Rect & GetTextRect(){return m_rect;}
	bool GetColorOdd() {return m_bColorOdd;};
	void InitCustom(const string &text, MovingEntity * pEnt, const CL_Vector2 &vecPos,
		const CL_Vector2 &vecMovement, const CL_Color &col, int timetoShowMS, int fontID);

	enum Mode
	{
		DIALOG,
		CUSTOM,
		CUSTOM_SCREEN
	};

	void SetMode(Mode mode) {m_mode = mode;}
	Mode GetMode(){return m_mode;}
	

private:

	CL_Point CalculateTextAvoidenceOffset();
	bool UpdateDialog(bool bFancyPositioning = true);
	bool UpdateCustom();

	unsigned int m_timeCreated;
	unsigned int m_timeToShowMS;
	string m_text;
	CL_Point m_pos;
	CL_Rect m_boundingRect; //0 based rect of the text rect
	CL_Rect m_rect; //final place to render the text
	MovingEntity *m_pEntity; //who wrote us, NULL if nobody
	TextManager *m_pTextManager;
	CL_SlotContainer m_slots;
	bool m_bVisible;
	CL_Color m_color;
	float m_alpha;
	bool m_bColorOdd; //i want to alternate the color sometimes
	Mode m_mode;
	CL_Vector2 m_worldPos, m_vecMovement; //applicable to CUSTOM mode
	int m_fontID;
	CL_Vector2 m_vecDisplacement;
};

typedef std::list<TextObject> textObject_list;


class TextManager
{
public:

	TextManager();
	virtual ~TextManager();

	int Add(const string &text, MovingEntity *pEnt); //returns MS it will be shown on screen
	void Update(float step);
	void Render();
	textObject_list & GetTextList(){return m_textList;};
	int GetCountOfTextActiveForEntity(const MovingEntity *pEnt);
	TextObject * GetLastTextForEntity(const MovingEntity *pEnt);
	void Reset();
	
	//Like Add, but gives more control, allows you to set the font, color, scrolling speed
	void AddCustom(const string &text, const MovingEntity *pEnt, const CL_Vector2 &vecPos,
		const CL_Vector2 &vecMovement, const CL_Color &col, int timeToShowMS, int fontID);
	
	//Like AddCustom but uses screen coordinates, won't scroll with world movement
	void AddCustomScreen(const string &text, const CL_Vector2 &vecPos,
		const CL_Vector2 &vecMovement, const CL_Color &col, int timeToShowMS, int fontID);

protected:

private:

	textObject_list m_textList;
};

extern TextManager g_textManager;

#endif                  // include guard
