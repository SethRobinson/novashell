
/* -------------------------------------------------
* Copyright 2005 Robinson Technologies
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created 29:12:2005   9:59
*/


#ifndef Camera_HEADER_INCLUDED // include guard
#define Camera_HEADER_INCLUDED  // include guard

class CameraSetting;
class Camera
{
public:

    Camera();
    virtual ~Camera();

	const CL_Vector2 & GetPos() {return m_vecPos;}
	const CL_Vector2 & GetScale() {return m_vecScale;}
	CL_Vector2 GetPosCentered();
	CL_Rectf GetViewRectWorld(); //we we're actually looking at
	CL_Rect GetViewRectWorldInt();
	CL_Rect GetScreenRect();

	void SetPos(CL_Vector2 vecPos);
	void SetPosCentered(CL_Vector2 vecPos);
	void SetTargetPos(CL_Vector2 vecTarget); //where the upper-left of the camera should be
	void SetTargetPosCentered(CL_Vector2 vecTarget); //Centered in on this spot by calculating the offset needed
	CameraSetting GetCameraSettings();
	void SetCameraSettings(CameraSetting &camSetting); //i want to const it but then my CameraSetting stuff will be a hassle...
	void SetScaleRaw(CL_Vector2 vecScale); //no bounds checking, no auto centering the camera to the new scale
	void SetScale(CL_Vector2 vecScale);
	void SetScaleTarget(CL_Vector2 vecScale);
	void Reset();
	void SetEntTracking(int entID); //set to 0 to disable
	void SetInstantUpdateOnNextFrame(bool bNew){m_bInstantUpdateASAP = bNew;}
	void Update(float step);

	void SetMoveLerp(float lerp){m_moveLerp = lerp;}
	void SetScaleLerp(float lerp){m_scaleLerp = lerp;}
	void InstantUpdate(); //move camera instantly without lerping, useful after a focus change

	int GetEntTracking(){return m_entTrackID;};
	void SetEntityTrackingOffset(CL_Vector2 offset);
	void SetLimitToMapArea(bool bNew);
	bool GetLimitToMapArea() {return m_bLimitToMapArea;}

protected:

	void UpdateTarget();
	CL_Vector2 ClampScaleToRange(CL_Vector2 vecScale);
	CL_Vector2 CalculateLimitToMapArea(CL_Vector2 v);

	CL_Vector2 m_vecPos;
	CL_Vector2 m_vecTargetPos; 
	CL_Vector2 m_vecScale; //controls the zoom.  1.0 is normal
	CL_Vector2 m_vecScaleTarget;
	int m_entTrackID;
	bool m_bInstantUpdateASAP;
	float m_moveLerp, m_scaleLerp;
	CL_Vector2 m_entityTrackingOffset;
	bool m_bLimitToMapArea;
};

#endif                  // include guard
