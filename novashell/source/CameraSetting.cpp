#include "AppPrecomp.h"
#include "CameraSetting.h"
#include "Map.h"

CameraSetting::CameraSetting()
{
	for (int i=0; i < e_vec2Count; i++) m_vec2Array[i] = CL_Vector2(0,0);
	for (int i=0; i < e_intCount; i++) m_intArray[i] = 0;


	VecPos() = g_mapDefaultCenterPos;
	VecTargetPos() = g_mapDefaultCenterPos;
	
	VecTargetScale() = VecScale() = CL_Vector2(1,1);
}

CameraSetting::~CameraSetting()
{
}

bool CameraSetting::Serialize(CL_FileHelper &helper)
{
	helper.process_smart_array(m_intArray, e_intCount);
	helper.process_smart_array(m_vec2Array, e_vec2Count);
	return true;
}


/*
Object: CameraSetting
A simple data object that contains state information for the camera.

None of its functions are publicly accessible at this time, although it can be used with <Camera::GetCameraSettings> and <Camera::SetCameraSettings>.

*/