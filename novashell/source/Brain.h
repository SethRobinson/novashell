
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 25:2:2006   15:05
*/


//Meant to be subclassed by brains. 

//Note, the difference between doing something in the OnRemove and in the destructor is in the OnRemove,
//it's guaranteed that everything the brain relys on is still active and ready for use.

#ifndef Brain_HEADER_INCLUDED // include guard
#define Brain_HEADER_INCLUDED  // include guard

class MovingEntity;


enum
{
	C_BRAIN_MESSAGE_DONT_SET_VISUAL //used if we don't want the base brain to specify a visual for some reason
};

class Brain
{
public:

	Brain(MovingEntity * pParent);
	virtual ~Brain();
	virtual void Update(float step) = 0;
	virtual void PostUpdate(float step) {};
	virtual const char * GetName()=0;
	virtual Brain * CreateInstance(MovingEntity *pParent)=0;
	int GetSort() const {return m_sort;}
	void SetSort(int sort){m_sort = sort;}
	virtual void OnAdd(){}; //called once when brain is inserted
	virtual void OnRemove(){};
	virtual void HandleMsg(const string &msg) {return;}
	virtual string HandleAskMsg(const string &msg) {return "";}
	virtual int HandleSimpleMessage(int message, int user1, int user2) {return 0;}
	virtual void AddWeightedForce(const CL_Vector2 & force){assert(!"This brain not setup to be a base brain!");};
	virtual unsigned int GetKeys(){return 0;}
	void SetDeleteFlag(bool bFlag) {m_bDeleteFlag = bFlag;}
	bool GetDeleteFlag() {return m_bDeleteFlag;}
	virtual void Render(void *pTarget) {};
	MovingEntity * GetParent() {return m_pParent;}

protected:

	void RegisterClass();

	int m_sort; //higher # gets run first
	MovingEntity *m_pParent;
	bool m_bDeleteFlag;
};

#endif                  // include guard
