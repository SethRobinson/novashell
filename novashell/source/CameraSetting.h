
/* -------------------------------------------------
* Copyright 2006 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 13:1:2006   20:43
*/


#ifndef CameraSetting_HEADER_INCLUDED // include guard
#define CameraSetting_HEADER_INCLUDED  // include guard

class CameraSetting
{
public:

    CameraSetting();
    virtual ~CameraSetting();

	CL_Vector2 & VecPos(){return m_vec2Array[e_vec2Pos];}
	CL_Vector2 & VecTargetPos(){return m_vec2Array[e_vec2TargetPos];}
	CL_Vector2 & VecScale(){return m_vec2Array[e_vec2Scale];}
	CL_Vector2 & VecTargetScale(){return m_vec2Array[e_vec2TargetScale];}

	int & TrackID(){return m_intArray[e_intTrackID];};
	bool Serialize(CL_FileHelper &helper);

protected:

	//stuff to help me serialize to disk with smart endianness
	enum
	{
		e_intTrackID = 0,
		e_intCount
	};

	enum
	{
		e_vec2Pos = 0,
		e_vec2TargetPos,
		e_vec2Scale,
		e_vec2TargetScale,
		e_vec2Count
	};

	CL_Vector2 m_vec2Array[e_vec2Count];
	cl_int32 m_intArray[e_intCount];

};

#endif                  // include guard
