
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 27:2:2006   14:00
*/


#ifndef VisualProfile_HEADER_INCLUDED // include guard
#define VisualProfile_HEADER_INCLUDED  // include guard

class VisualResource;


class ProfileAnim
{
public:

	//assignment and copy constructor for use by the stl vector
	ProfileAnim& operator=(const ProfileAnim &rhs)
   {
	   SAFE_DELETE(m_pSprite);
		if (rhs.m_pSprite)
		{
			m_pSprite = new CL_Sprite(*rhs.m_pSprite);
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
			//clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

		}
		m_name = rhs.m_name;
		m_spriteName = rhs.m_spriteName;
	   return *this;
   }
	
	ProfileAnim(const ProfileAnim &copyFrom)
	{
		m_pSprite = NULL;
		*this = copyFrom;
	}

	ProfileAnim()
	{
		m_pSprite = NULL;
	}

	
	~ProfileAnim()
	{
		SAFE_DELETE(m_pSprite);
	}
	
	CL_Sprite * m_pSprite;
	string m_name;
	string m_spriteName;
};


class VisualProfile
{
public:


	enum eFacing
	{

	 //do not change this order!  	
	 FACING_NONE = -1,

	 FACING_LEFT = 0,
	 FACING_UP_LEFT,
	 FACING_UP,
	 FACING_UP_RIGHT,
	 FACING_RIGHT,
	 FACING_DOWN_RIGHT,
	 FACING_DOWN,
	 FACING_DOWN_LEFT,

	 FACING_COUNT //how many directions we support
	};

	enum eState
	{
		VISUAL_STATE_NONE = -1,
		VISUAL_STATE_IDLE = 0,
		VISUAL_STATE_WALK,
		VISUAL_STATE_RUN,
		VISUAL_STATE_PAIN,
		VISUAL_STATE_DIE,
		VISUAL_STATE_ATTACK1,
		VISUAL_STATE_ATTACK2,
		VISUAL_STATE_SPECIAL1,
		VISUAL_STATE_TURN,
		VISUAL_STATE_JUMP,
	};

	enum eAnim
	{
		IDLE_LEFT = 0,
		IDLE_UP_LEFT,
		IDLE_UP,
		IDLE_UP_RIGHT,
		IDLE_RIGHT,
		IDLE_DOWN_RIGHT,
		IDLE_DOWN,
		IDLE_DOWN_LEFT,

		WALK_LEFT,
		WALK_UP_LEFT,
		WALK_UP,
		WALK_UP_RIGHT,
		WALK_RIGHT,
		WALK_DOWN_RIGHT,
		WALK_DOWN,
		WALK_DOWN_LEFT,

		RUN_LEFT,
		RUN_UP_LEFT,
		RUN_UP,
		RUN_UP_RIGHT,
		RUN_RIGHT,
		RUN_DOWN_RIGHT,
		RUN_DOWN,
		RUN_DOWN_LEFT,

		PAIN_LEFT,
		PAIN_UP_LEFT,
		PAIN_UP,
		PAIN_UP_RIGHT,
		PAIN_RIGHT,
		PAIN_DOWN_RIGHT,
		PAIN_DOWN,
		PAIN_DOWN_LEFT,

		DIE_LEFT,
		DIE_UP_LEFT,
		DIE_UP,
		DIE_UP_RIGHT,
		DIE_RIGHT,
		DIE_DOWN_RIGHT,
		DIE_DOWN,
		DIE_DOWN_LEFT,

		ATTACK1_LEFT,
		ATTACK1_UP_LEFT,
		ATTACK1_UP,
		ATTACK1_UP_RIGHT,
		ATTACK1_RIGHT,
		ATTACK1_DOWN_RIGHT,
		ATTACK1_DOWN,
		ATTACK1_DOWN_LEFT,

		ATTACK2_LEFT,
		ATTACK2_UP_LEFT,
		ATTACK2_UP,
		ATTACK2_UP_RIGHT,
		ATTACK2_RIGHT,
		ATTACK2_DOWN_RIGHT,
		ATTACK2_DOWN,
		ATTACK2_DOWN_LEFT,

		SPECIAL1_LEFT,
		SPECIAL1_UP_LEFT,
		SPECIAL1_UP,
		SPECIAL1_UP_RIGHT,
		SPECIAL1_RIGHT,
		SPECIAL1_DOWN_RIGHT,
		SPECIAL1_DOWN,
		SPECIAL1_DOWN_LEFT,

		TURN_LEFT,
		TURN_UP_LEFT,
		TURN_UP,
		TURN_UP_RIGHT,
		TURN_RIGHT,
		TURN_DOWN_RIGHT,
		TURN_DOWN,
		TURN_DOWN_LEFT,

		JUMP_LEFT,
		JUMP_UP_LEFT,
		JUMP_UP,
		JUMP_UP_RIGHT,
		JUMP_RIGHT,
		JUMP_DOWN_RIGHT,
		JUMP_DOWN,
		JUMP_DOWN_LEFT,


		BUILTIN_ANIM_COUNT
	};

    VisualProfile();
    virtual ~VisualProfile();

	bool Init(VisualResource *pVisualResource, const string &profileName);
	CL_Sprite * GetSprite(int eStatem, int eFacing);
	CL_Sprite * GetSpriteByAnimID(int animID);

	const string & GetName() {return m_name;}
	bool IsActive(int stateID); //check if a certain anim state actually has data in it or not
	int StateNameToAnimID(const string & stState); //returns -1 if anim id doesn't exist
	VisualResource * GetParentVisualResource() {return m_pParent;}
	void UpdateToDocument(CL_DomDocument &document);
    	vector<string> GetListOfActiveAnims();
	int SpriteNameToAnimID(const string & stState, bool bShowErrors = false);
	int SpriteNameToAnimID(CL_Sprite *pSprite); //compares by memory address
	string AnimIDToText(int animID){return m_animArray[animID].m_name;}
	int GetAnimID(int eState, int eFacing);
	int GetFirstValidAnimIndex();

protected:

	void AddAnimInfo(CL_DomElement &node);
	int TextToAnimIDCreatedIfNeeded(const string & stState);
	void UpdateDocumentSpriteFromAnim(CL_DomNode *node, ProfileAnim *anim);
	void UpdateProfileAnim(CL_DomElement &node);
	void UpdateProfile(CL_DomNode *pNode);
	void UpdateSprite(CL_DomNode *pNode);

	string m_name;
	VisualResource *m_pParent;
	vector<ProfileAnim> m_animArray;
};

#endif                  // include guard
