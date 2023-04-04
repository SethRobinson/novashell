#include "AppPrecomp.h"
#include "Camera.h"
#include "Tile.h"
#include "Screen.h"
#include "GameLogic.h"
#include "EntMapCache.h"
#include "MovingEntity.h"
#include "AppUtils.h"

#define C_DEFAULT_LERP 0.3f

Camera::Camera()
{
	Reset();
}

Camera::~Camera()
{
}

void Camera::SetCameraSettings(CameraSetting &camSetting)
{
  m_vecPos = camSetting.VecPos();
  m_vecTargetPos = camSetting.VecTargetPos();
  m_vecScale = ClampScaleToRange(camSetting.VecScale());
  m_vecScaleTarget = ClampScaleToRange(camSetting.VecTargetScale());
}

CameraSetting Camera::GetCameraSettings()
{
	CameraSetting camSettingOut;

	camSettingOut.VecPos() = m_vecPos;
	camSettingOut.VecTargetPos() = m_vecTargetPos;
	camSettingOut.VecScale() = m_vecScale;
	camSettingOut.VecTargetScale() = m_vecScaleTarget;

	return camSettingOut;
}

CL_Rect Camera::GetScreenRect()
{
	return CL_Rect(0,0, GetScreenX, GetScreenY);
}

CL_Rectf Camera::GetViewRectWorld()

{
	CL_Rectf r;
	CL_Vector2 v;
	v = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
	r.right = v.x;
	r.bottom = v.y;

	r.left = m_vecPos.x;
	r.top = m_vecPos.y;
	return r;
}

CL_Rect Camera::GetViewRectWorldInt()

{
	CL_Rect r;
	CL_Vector2 v;
	v = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
	
	if (v.x > 20000)
	{
		assert(0);
		v = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
		
	}
	
	r.right = v.x;
	r.bottom = v.y;

	r.left = m_vecPos.x;
	r.top = m_vecPos.y;
	
	return r;
}

void Camera::Reset()
{
	m_entityTrackingOffset = CL_Vector2(0,40);
	m_bInstantUpdateASAP = false;
	m_vecPos = g_mapDefaultCenterPos;
	m_vecTargetPos = g_mapDefaultCenterPos;
	m_vecScale.x = m_vecScale.y = 1;
	m_vecScaleTarget = m_vecScale;
	m_entTrackID = 0;
	m_moveLerp = C_DEFAULT_LERP;
	m_scaleLerp = C_DEFAULT_LERP;
	m_bLimitToMapArea = false;
}

void Camera::SetPos(CL_Vector2 vecPos)
{
	m_vecPos = vecPos;
	m_vecTargetPos = vecPos;
}

void Camera::SetScaleRaw(CL_Vector2 vecScale)
{
	m_vecScale = vecScale;
	m_vecScaleTarget = m_vecScale;
}

void Camera::SetScaleTarget(CL_Vector2 vecScale)
{
	if (!g_pMapManager->GetActiveMap()) return;

	m_vecScaleTarget = ClampScaleToRange(vecScale);
}


CL_Vector2 Camera::ClampScaleToRange(CL_Vector2 vecScale)
{

	const float maxZoom = 40.0f; //larger the number, the closer we can zoom in
	const float minZoom = 0.010f; //smaller the #, the farther we can zoom out

	//make sure current zoom level is within acceptable range
	vecScale.x = min(vecScale.x, maxZoom);
	vecScale.y = min(vecScale.y, maxZoom);

	vecScale.x = max(vecScale.x, minZoom);
	vecScale.y = max(vecScale.y, minZoom);

	return vecScale;
}

void Camera::SetScale(CL_Vector2 vecScale)
{
	if (!g_pMapManager->GetActiveMap()) return;
	CL_Vector2 vecPosBefore;
	if (m_entTrackID == 0)
	{
		vecPosBefore = g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY));
	}
	
	m_vecScale = ClampScaleToRange(vecScale);

	m_vecScaleTarget = m_vecScale;
	if (m_entTrackID == 0)
	{
		m_vecTargetPos +=  (vecPosBefore-g_pMapManager->GetActiveMapCache()->ScreenToWorld(CL_Vector2(GetScreenX, GetScreenY)))/2;
	}

}

void Camera::SetEntTracking(int entID)
{
	m_entTrackID = entID;
}


void Camera::UpdateTarget()
{
	if (m_entTrackID != 0)
	{
		MovingEntity *pEnt = ((MovingEntity*) EntityMgr->GetEntityFromID(m_entTrackID));
		if (pEnt)
		{
			CL_Vector2 vPos = pEnt->GetPos();
			
			vPos.y -= (m_entityTrackingOffset.y*pEnt->GetScale().y);
			vPos.x -= (m_entityTrackingOffset.x*pEnt->GetScale().x);
			SetTargetPosCentered(vPos);
		} else
		{
			LogMsg("Camera can't find Entity %d, disabling camera tracking.", m_entTrackID);
			m_entTrackID = 0;
		}
	} 

	

}

CL_Vector2 Camera::CalculateLimitToMapArea(CL_Vector2 v)
{
		if (g_pMapManager->GetActiveMap())
		{
		//limit to map area
			CL_Rectf r = g_pMapManager->GetActiveMap()->GetWorldRectExact();
			//LogMsg("Limiting to %s", PrintRectInt(r).c_str());

			if (v.y + (float(GetScreenY))/m_vecScale.y > r.bottom)
			{
				v.y = r.bottom - (float(GetScreenY))/m_vecScale.y;
			} else
				if (v.y < r.top)
				{
					//we're too high

					v.y = r.top;

					if (v.y + (float(GetScreenY))/m_vecScale.y > r.bottom)
					{
						//oh no, now we broke the other side.  The map is too small for this screen size...
						//force it down I guess
						v.y = r.bottom - (float(GetScreenY))/m_vecScale.y;
					}

				}


				//also check the left and right bounds
				if (v.x + (float(GetScreenX))/m_vecScale.x > r.right)
				{
					v.x = r.right - (float(GetScreenX))/m_vecScale.x;
				} else
					if (v.x < r.left)
					{
						//we're too high
						v.x = r.left;

						if (v.x + (float(GetScreenX))/m_vecScale.x > r.right)
						{
							//oh no, now we broke the other side.  The map is too small for this screen size...
							//force it left I guess
							v.x = r.right - (float(GetScreenX))/m_vecScale.x;
						}


					}


		}

	return v;
}

void Camera::SetTargetPosCentered(CL_Vector2 vecTarget)
{
	m_vecTargetPos = vecTarget;
	m_vecTargetPos.x -= (float(GetScreenX/2))/m_vecScale.x;
	m_vecTargetPos.y -= (float(GetScreenY/2))/m_vecScale.y;

	//m_vecTargetPos.x = RoundNearest(m_vecTargetPos.x,1.0f);
	//m_vecTargetPos.y = RoundNearest(m_vecTargetPos.y,1.0f);

}

CL_Vector2 Camera::GetPosCentered()
{
	return m_vecPos + CL_Vector2((float(GetScreenX/2))/m_vecScale.x, (float(GetScreenX/2))/m_vecScale.y );
}

void Camera::SetPosCentered(CL_Vector2 vecPos)
{
	m_vecPos = vecPos;
	m_vecPos.x -= (float(GetScreenX/2))/m_vecScale.x;
	m_vecPos.y -= (float(GetScreenY/2))/m_vecScale.y;
	m_vecTargetPos = m_vecPos;
}

void Camera::SetTargetPos(CL_Vector2 vecTarget)
{
	m_vecTargetPos = vecTarget;
}
void Camera::InstantUpdate()
{
	UpdateTarget();
	SetScale(m_vecScaleTarget);
	SetPos(m_vecTargetPos);
}

void Camera::Update(float step)
{
	assert(step != 0);
	
	m_vecScale = Lerp(m_vecScale, m_vecScaleTarget, cl_min(m_scaleLerp*step, 1));

	
	UpdateTarget();
  	
	if (m_bInstantUpdateASAP)
	{
		m_bInstantUpdateASAP = false;
		InstantUpdate();
	}
	
	m_vecPos = Lerp(m_vecPos,m_vecTargetPos, cl_min(m_moveLerp*step, 1));

	static const float fuzz = 0.01f;

	if (m_vecScaleTarget.x < m_vecScale.x+fuzz && m_vecScaleTarget.x > m_vecScale.x-fuzz)
	{
		m_vecPos.x = RoundNearest(m_vecPos.x,1.0f);
		m_vecPos.y = RoundNearest(m_vecPos.y,1.0f);
	}


	if (m_bLimitToMapArea && !GetGameLogic()->GetEditorActive())
	{
		m_vecPos = CalculateLimitToMapArea(m_vecPos);
	}


}

void Camera::SetEntityTrackingOffset( CL_Vector2 offset )
{
	m_entityTrackingOffset = offset;
}

void Camera::SetLimitToMapArea( bool bNew )
{
	m_bLimitToMapArea = bNew;

}
/*
Object: Camera
Controls what part of the world is currently on the screen, and at what zoom level.

About:
This is a global object that can always be accessed.

Usage:
(code)
GetCamera:Reset();
(end)

Changing Map:

Use <MapManager::SetActiveMapByName> to change which map the <Camera> is showing.

Group: Position And Scale

func: SetPos
(code)
nil SetPos(Vector2 vPos)
(end)

Parameters:

vPos - A <Vector2> object containing the new position of the <Camera> in world coordinates.  This point will be the upper-left of the screen.

func: GetPos
(code)
Vector2 SetPos()
(end)

Returns:

A <Vector2> with the world coordinates of where the upper-left of the screen is located.

func: SetPosCentered
(code)
nil SetPosCentered(Vector2 vPos)
(end)
Like <SetPos> but automatically centers the camera on it, taking into account the current screen size.

Parameters:

vPos - A <Vector2> object containing the new position that the <Camera> should center on in world coordinates.

func: GetPosCentered
(code)
Vector2 GetPosCentered()
(end)

Returns:

The world coordinates of the exact middle of the screen.

func: SetScale
(code)
nil SetScale(Vector2 vPos)
(end)

Scale determines how "zoomed in/zoomed out" the <Camera> is.  A scale of 1,1 means exactly the size of the pixels. (normally what you want)

Note:

If the X/Y scale that are sent are not the same, this creates a "stretched" effect on the whole screen, could be a useful effect for something.

Parameters:

vPos - A <Vector2> object containing the new magnification.  Will be clipped to the valid range of 40,40 (zoomed waaay in) to 0.01,0.01 (zoomed waaay out).

func: GetScale
(code)
Vector2 GetScale()
(end)

Returns:

The current scale magnification level.

Group: Tracking And Control

func: SetEntityTrackingByID
(code)
nil SetEntityTrackingByID(number entityID)
(end)

This tells the camera to follow around an <Entity>.  It attempts to keep the <Entity> in the middle of the screen, by lerping to its position.

Use <SetMoveLerp> to make the camera more or less snappy/responsive.

This always overrides <SetPosTarget>.

Note:

The <Camera> is smart enough to follow entities through doors/warps even if they move to a new <Map>.

Parameters:

entityID - The ID of an <Entity> you'd like the <Camera> to follow around.  Send <C_ENTITY_NONE> to disable entity tracking.

func: GetEntityTrackingByID
(code)
number GetEntityTrackingByID()
(end)

Returns:

The entityID of the <Entity> we're tracking or <C_ENTITY_NONE> if none.


func: SetEntityTrackingOffset
(code)
nil SetEntityTrackingOffset(Vector2 vOffset)
(end)


When you've set <SetEntityTrackingByID> you may wish to fine tune what the camera focuses on.  By default, this is set to 0,40.  In other words, it looks above the feet by 40 units, so when we zoom in we don't see a close-up of the feet in the treeworld example.

If your game is top view, you probably should set this to Vector2(0,0) so the true center of the entity (as determined by its collision usually) is used.

Note:

This value is automatically scaled by the cameras current scale/zoom.

Parameters:

vOffset - a <Vector2> object containing the offset the camera should use.

func: GetEntityTrackingByID
(code)
number GetEntityTrackingByID()
(end)

Returns:

The entityID of the <Entity> we're tracking or <C_ENTITY_NONE> if none.


func: SetLimitToMapArea
(code)
nil SetLimitToMapArea(boolean bLimitToMapArea)
(end)

If your player is standing on the bottom of a map, but it annoys you that he's still centered in the camera, this function is for you.

If enabled, it will automatically scan each map at load time and figure out its dimensions.  To manually set the viewable area, use <Map:SetWorldRect> or
<Map:ComputeWorldRect> to re-scan if the map size changes.

Parameters:

bLimitToMapArea - if true, the camera will attempt to limit the view to the active areas of the map

func: GetLimitToMapArea
(code)
boolean GetLimitToMapArea()
(end)

Returns:

True if the camera is currently limiting its view to the active map area.


func: SetPosTarget
(code)
nil SetPosTarget(Vector2 vPos)
(end)

This is like <SetPos> except instead of being instantaneous, the <Camera> smoothly moves to the new position.

Parameters:

vPos - A <Vector2> object containing the target position.

func: SetPosCenteredTarget
(code)
nil GetPosCenteredTarget(Vector2 vPos)
(end)

This is like <SetPosCentered> except instead of being instantaneous, the <Camera> smoothly moves to the new position.

Parameters:

vPos - A <Vector2> object containing the target position.

func: SetScaleTarget
(code)
nil SetScaleTarget(Vector2 vPos)
(end)

This is like <SetScale> except instead of being instantaneous, the <Camera> smoothly scales up/down to the new scale.

Parameters:

vPos - A <Vector2> object containing the new X and Y scale.

Group: Interpolation Settings

func: SetMoveLerp
(code)
nil SetMoveLerp(number lerpSpeed)
(end)

This controls how fast the <Camera> moves when <SetPosTarget> or <SetEntityTrackingByID> is used.

Parameters:

lerpSpeed - 1 for instant, 0.01 for slow.  Default is 0.3.

func: SetScaleLerp
(code)
nil SetScaleLerp(number lerpSpeed)
(end)

This controls how fast the <Camera> scales in and out when <SetScaleTarget> is used.

Parameters:

lerpSpeed - 1 for instant, 0.01 for slow.  Default is 0.3.

Group: Miscellaneous

func: Reset
(code)
nil Reset()
(end)

Resets everything to the default <Camera> settings and moves the position to 0,0.


func: InstantUpdate
(code)
nil InstantUpdate()
(end)

Makes the camera "catch up" with any tracking/targeting that is active, without waiting for the lerping to happen.

func: SetCameraSettings
(code)
nil SetCameraSettings(CameraSettings camSettings)
(end)

Set's the <Camera> to specific state, that was previously remembered with <GetCameraSettings>.

Parameters:

camSettings - A <CameraSettings> object.

func: GetCameraSettings
(code)
CameraSettings GetCameraSettings()
(end)

If you'd like to save the state information about the camera so you can quickly return to that view, this is an easy way to do it.

Returns:

A <CameraSettings> object.

*/