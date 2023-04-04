#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "EntMapCache.h"
#include "EntCollisionEditor.h"
#include "ScriptManager.h"
#include "BrainShake.h"
#include "VisualProfileManager.h"
#include "MaterialManager.h"
#include "AI/Goal_Think.h"
#include "AI/PathPlanner.h"
#include "AI/WatchManager.h"
#include "PhysicsManager.h"
#include "EntCreationUtils.h"

#define C_ANIM_FRAME_LAST -1
#define C_GROUND_RELAX_TIME_MS 150
#define C_DEFAULT_SCRIPT "system/ent_default.lua"
#define C_DISTANCE_NOT_ON_SAME_MAP 2000000000
//there are issues with sliding along walls thinking it's a ground when they are built out of small blocks, this
//provides a work around
#define C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND 1

#define C_DEFAULT_ENTITY_ACCELERATION 0.5;

#define C_DEFAULT_TURN_SPEED 0.1f

#define C_DEFAULT_DENSITY 1.0f

#define C_GRAVITY_OVERRIDE_DISABLED 9999999

#define C_DEFAULT_DESIRED_WALK_SPEED 7
#define C_DEFAULT_MAX_WALK_SPEED 3000

#define C_DEFAULT_DAMPENING 0.0f //only applied when gravity is 0

Zone g_ZoneEmpty;

class ZoneEmptyInit
{
public:
	ZoneEmptyInit()
	{
		g_ZoneEmpty.m_entityID = C_ENTITY_NONE;
		g_ZoneEmpty.m_materialID = CMaterial::C_MATERIAL_NONE;
		g_ZoneEmpty.m_vPos = CL_Vector2::ZERO;

	}
};



ZoneEmptyInit zoneEmpty;

MovingEntity::MovingEntity(): BaseGameEntity(BaseGameEntity::GetNextValidID())
{
	m_pBody = NULL;
	m_pScriptObject = NULL;
	m_pPathPlanner = NULL;
	m_pGoalManager = NULL;
	m_mainScript = C_DEFAULT_SCRIPT;
	m_pFont = NULL;
	m_defaultTextColor = CL_Color(255,100,200,255);
	m_bUsingCustomCollisionData = false;
	m_brainManager.SetParent(this);
	m_hashedName = 0;
    m_drawID = 0;
	m_moveToAtEndOfFrame  = m_pos = CL_Vector2::ZERO;
	


	SetDefaults();
}

MovingEntity::~MovingEntity()
{
	Kill();
	
}

bool MovingEntity::IsOnScreen()
{

	if (g_pMapManager->GetActiveMap() != GetMap()) return false;

	//we're on the map, but are we actually on the visible portion?
	CL_Rectf r  = GetCamera->GetViewRectWorld();
	CL_Rectf mr = GetWorldCollisionRect();

	if (mr.left > r.right) return false;
	if (mr.right < r.left) return false;
	if (mr.top > r.bottom) return false;
	if (mr.bottom < r.top) return false;
	return true;
}

unsigned int MovingEntity::CalculateTimeToReachPosition(const CL_Vector2 &pos)
{
	//Note- we're doing a very crappy job here, we should take into account acceleration/current speed
	//and such as well
	unsigned int timeMS = ((Vec2DDistance(GetPos(), pos)*14)  / cl_max(1, (m_desiredSpeed/3)));

    //LogMsg("Should take %d MS", timeMS);
	return timeMS;
}


PathPlanner * MovingEntity::GetPathPlanner()
{
	if (!m_pPathPlanner)
	{
		m_pPathPlanner = new PathPlanner(this);
	}
	
	return m_pPathPlanner;
}


bool MovingEntity::IsFacingTarget(float tolerance)
{
	float angle = GetAngleBetweenVectorFacings(GetVectorFacing(), GetVectorFacingTarget());
	return fabs(angle) <= tolerance;
}

float MovingEntity::GetDistanceFromEntityByID(int id)
{
	MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(id);

	if (!pEnt)
	{
		LogMsg("GetDistanceFrom sent invalid ID: %d", id);
		return -1;
	}

	if (GetMap() != pEnt->GetMap()) return C_DISTANCE_NOT_ON_SAME_MAP;

	return (GetPos()-pEnt->GetPos()).length();
}

float MovingEntity::GetDistanceFromPosition(const CL_Vector2 &pos)
{
	return (GetPos()-pos).length();
}

CL_Vector2 MovingEntity::GetVectorToEntityByID(int entID)
{
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(entID);
	if (pEnt) return GetVectorToEntity(pEnt);
	
	LogMsg("Ent %d (%s) can't get angle to entity %d, doesn't exist", ID(), GetName().c_str(),
		entID);
	return CL_Vector2(0,0);
}

bool MovingEntity::FunctionExists(const char * pFunctionName)
{
	if (!m_pScriptObject) return false;
	return m_pScriptObject->FunctionExists(pFunctionName);
}

bool MovingEntity::VariableExists(const char * pVarName)
{
	if (!m_pScriptObject) return false;
	return m_pScriptObject->VariableExists(pVarName);
}



CL_Vector2 MovingEntity::GetVectorToEntity(MovingEntity *pEnt)
{
	CL_Vector2 v = pEnt->GetPos()-GetPos();
	if (v == CL_Vector2::ZERO)
	{
		//well, returning 0,0 can cause havok in the path planner, so
		//let's guess
		return m_vecFacing;
	}
	v.unitize();

	//check for NAN, it happens under gcc
	if (v.x != v.x || v.y != v.y)
		{
			//well, returning 0,0 can cause havok in the path planner, so
			//let's guess
			return m_vecFacing;
		}
			
	return v;
}

CL_Vector2 MovingEntity::GetVectorToPosition(const CL_Vector2 &pos)
{
	CL_Vector2 v = pos-GetPos();
	//avoid NAN on osx/gcc
	if (v.x != v.x || v.y != v.y)
	{
		//well, returning 0,0 can cause havok in the path planner, so
		//let's guess
		return m_vecFacing;
	}
	
	v.unitize();
	//check for NAN, it happens under gcc
	if (v.x != v.x || v.y != v.y)
	{
		//well, returning 0,0 can cause havok in the path planner, so
		//let's guess
		return m_vecFacing;
	}
	return v;
}

bool MovingEntity::IsOnSameMapAsEntityByID(int entID)
{
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(entID);
	if (!pEnt) return false;
	return (GetMap() == pEnt->GetMap());
}

bool MovingEntity::IsCloseToEntity(MovingEntity *pEnt, int dist)
{
	if (!pEnt) return false;
	if (GetMap() != pEnt->GetMap()) return false;
	return ( dist >   (GetPos()-pEnt->GetPos()).length()  );
}

bool MovingEntity::IsCloseToEntityByID(int entID, int dist)
{
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(entID);
	if (!pEnt) return false;

	return IsCloseToEntity(pEnt, dist);
}

CL_Vector2 GetPointOutsideOfRectFromInside(const CL_Rectf &r, const CL_Vector2 v, float padding)
{
	//i'm sure there is a much better mathy way of doing this, but this will do.  we need to throw a ray
	//from the center of this rect until we hit the edge
	CL_Vector2 pos = CL_Vector2::ZERO;

	while (r.is_inside( *(CL_Pointf*)&pos))
	{
		pos += v*2;
	}

	return pos + v * padding;
}

bool MovingEntity::GetApproachPosition(MovingEntity *pEnt, int distance, CL_Vector2 &pOutputPos)
{
	CL_Rectf c;

	if (this->GetCollisionData() && distance >= 0) //a -1 means they want to be "inside"
	{
		c = this->GetTile()->GetWorldColRect()- *(CL_Pointf*)&this->GetPos();
	} else
	{
		//well, let's get the direction we want another way
		CL_Vector2 pos = this->GetPos();

		if (distance <= 0)
		{
			//they want to stand directly on top, we don't need to calculate anything fancy
			pOutputPos = pos;
			return true;
		}

		c = CL_Rectf(pos.x - 5, pos.y - 5, pos.x+5, pos.y + 5);
	}

	CL_Vector2 vecAngleFromTarget;

	if (this->GetMap() == pEnt->GetMap())
	{
		vecAngleFromTarget = this->GetVectorToEntity(pEnt);
	} else
	{
		//we don't care which way you approach it, no preference
		vecAngleFromTarget = CL_Vector2(0,-1); //up
	}

	CL_Rectf ourRect = pEnt->GetTile()->GetWorldColRect();

	float padding = max(ourRect.get_height(), ourRect.get_width()) + 1;

	//is this a valid point?
	int tries = 9;
	while(tries--)
	{

		pOutputPos = GetPointOutsideOfRectFromInside(c, vecAngleFromTarget, padding) + this->GetPos();

	if (GetGameLogic()->GetShowPathfinding())
		LogMsg("Approach of %s: Trying angle %s at pos %s", pEnt->GetName().c_str(), PrintVector(vecAngleFromTarget).c_str(), 
			PrintVector(pOutputPos).c_str());


		//found a potential, let's do further checking for more accuracy?
#ifdef _DEBUG
		//		DrawBullsEyeWorld(m_vDestination, CL_Color::red, 20, CL_Display::get_current_window()->get_gc());
#endif

		if (pEnt->IsValidPosition(this->GetMap(), pOutputPos, true))
		{
			//cool
			break;
		}

		if (!tries)
		{
			//give up
			LogMsg("Approach: Ent %d (%s) Failed to find acceptable position close to entity %d (%s), everything blocked?  Let's just skip this.",
				pEnt->ID(), pEnt->GetName().c_str(), this->ID(), this->GetName().c_str());
			return false;
		}

		//well, we failed.  Perhaps there is a wall or something here blocking us.  Let's try from another angle.

		float angle = atan2(vecAngleFromTarget.x, vecAngleFromTarget.y)- (PI/4);
		angle = fmod(angle+PI, PI*2)-PI;
		vecAngleFromTarget.x = sin(angle);
		vecAngleFromTarget.y = cos(angle);
		vecAngleFromTarget.unitize();

	}

	return true; //success
}

void MovingEntity::SetCollisionScale(const CL_Vector2 &vScale)
{
	if (!m_pCollisionData)
	{
		//LogError("Error, can't ScaleCollisionInfo, there is no collision data here");
		return;
	}

	if (m_bUsingTilePicCollision && vScale != CL_Vector2(1,1))
	{
		if (!m_bUsingCustomCollisionData)
		{
			//switch to custom collision from a copy of the shared tilepic stuff
			CollisionData *pNewCol = new CollisionData(*m_pCollisionData);
			m_pCollisionData = pNewCol;
			m_bUsingCustomCollisionData = true;
			m_pCollisionData->SetDataChanged(false);
		}
	}

	if (m_bUsingCustomCollisionData)
	{
		m_pCollisionData->SetScale(vScale);
		InitializeBody(); //have to reinit our whole body for this
	}
}


int MovingEntity::GetLayerID()
{
	 assert(m_pTile);
	 
	 if (m_requestNewLayerID != C_LAYER_NONE) return m_requestNewLayerID;
	 return m_pTile->GetLayer();

}

void MovingEntity::SetLayerByName(const string &name)
{
	if (!IsPlaced())
	{
		LogError("Entity %d (%s) cannot use SetLayerByName yet, he hasn't been placed on the map.  Use this in OnPostInit instead.",
			ID(), GetName().c_str());
		return;
	}
	SetLayerID(GetMap()->GetLayerManager().GetLayerIDByName(name));
}


void MovingEntity::SetLayerID(int id)
 {
	 assert(m_pTile);
	
	if (!m_pTile->GetParentScreen())
	{
		LogError("You can't use SetLayerID() in OnInit, it hasn't been placed on a map  yet.  Use OnPostInit.");
		return;
	}
	if (id != m_pTile->GetLayer())
	{
		m_requestNewLayerID = id;
		m_bMovedFlag = true;

	}

}

Map * MovingEntity::GetMap()
{
	if (!m_pTile->GetParentScreen())
	{
		LogError("Can't use GetMap() on Entity %d (%s) yet, it doesn't exist on a map", ID(), GetName().c_str());
		return NULL;
	}

	return m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
}

const string & MovingEntity::GetRequestedMapName()
{
	return m_strWorldToMoveTo;
}

CL_Vector2 MovingEntity::GetCollisionScale()
{
	if (!m_pCollisionData)
	{
		LogError("Error, can't GetCollisionScale, there is no collision data here");
		return CL_Vector2(0,0);
	}

	return m_pCollisionData->GetScale();
}

void MovingEntity::ClearColorMods()
{
	m_colorModRed = 0;
	m_colorModGreen = 0;
	m_colorModBlue = 0;
	m_colorModAlpha = 0;
}

void MovingEntity::Kill()
{
	//tell any entities that were attached to us that we're dying and to also kill themselves
	

	if (m_pScriptObject && IsPlaced())
	{
		RunFunctionIfExists("OnKill");
	}

	g_inputManager.RemoveBindingsByEntity(this);

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->SetDeleteFlag(true);
				pEnt->SetAttach(C_ENTITY_NONE, CL_Vector2(0,0));

			} 
			itor++;
		}
	}

	if (m_bRequestsVisibilityNotifications)
	{
			g_watchManager.RemoveFromVisibilityList(this);
	}

	g_watchManager.OnEntityDeleted( this);

	SAFE_DELETE(m_pPathPlanner);
	SAFE_DELETE(m_pGoalManager);
	m_brainManager.Kill(); //reset it

	m_pVisualProfile = NULL;
	
	if (m_bUsingCustomCollisionData)
	{
		//there is a special case we need to check, if we're using a shared tilepic yet scaled it our own way..
		if (m_bUsingTilePicCollision)
		{
			if (m_pCollisionData && m_pCollisionData->GetDataChanged())
			{
				m_pCollisionData->SetDataChanged(false);
				//yes, we need to copy this to the real tilepics stuff..
				CollisionData *pColData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(GetImageID(), GetImageClipRect());
				if (pColData)
				{
					*pColData = *m_pCollisionData; //copy it over
					pColData->SetScale(CL_Vector2(1,1));
					pColData->SetDataChanged(true);
					
					} else
				{
					assert(!"that was weird");
				}
			}
		}

		
		SAFE_DELETE(m_pCollisionData);
		m_bUsingCustomCollisionData = false;
		m_bUsingTilePicCollision = false;
	}
	
	SAFE_DELETE(m_pSprite);
	SAFE_DELETE(m_pScriptObject);
	SAFE_DELETE(m_pFont);
	KillBody();

	SetDefaults();
	
	
}

void MovingEntity::KillBody()
{
	if (m_pBody)
		GetPhysicsManager()->GetBox2D()->DestroyBody(m_pBody);
	m_pBody = NULL;

}

bool MovingEntity::HandleMessage(const Message &msg)
{

	if (m_pGoalManager)
	{
		return m_pGoalManager->HandleMessage(msg);
	}
	return false; //didn't handle
}

std::string MovingEntity::HandleMessageString( const string &msg )
{
	if (GetScriptObject())
	{
		GetScriptObject()->RunString(msg.c_str());
	} else
	{
		LogMsg("Can't deliver message %s, script not active in ent %d",
			msg.c_str(), ID());
	}

	return "";
}

void MovingEntity::SetDefaults()
{
	assert(!m_pBody);
	m_bSizeOverride = false;
	m_filterData.categoryBits = 0x0001;
	m_filterData.maskBits = 0xFFFF;
	m_filterData.groupIndex = 0;

	m_strWorldToMoveTo.clear();
	//m_pos = CL_Vector2::ZERO; //don't want to actually set this...
	m_angle = 0;
	m_bAnimCallbackActive = false;
	m_bRunUpdateEveryFrame = false;
	m_accel = C_DEFAULT_ENTITY_ACCELERATION;
	m_bUsingTilePicCollision = false;
	m_bLockedScale = false;
	m_attachEntID = C_ENTITY_NONE;
	m_text.clear();
	m_turnSpeed = C_DEFAULT_TURN_SPEED;
	m_textRect = CL_Rect(0,0, 1024, 1024);
	ClearColorMods();
	m_requestNewLayerID = C_LAYER_NONE;
	m_bCanRotate = false;
	SetMaxWalkSpeed(C_DEFAULT_MAX_WALK_SPEED);
	SetDesiredSpeed(C_DEFAULT_DESIRED_WALK_SPEED);
	m_floorMaterialID = -1;
	m_bUsingSimpleSprite = false;
	m_fDensity = C_DEFAULT_DENSITY; //default
	m_bIsAtRest = false;
	m_groundTimer = 0;
	m_bMovedFlag = false;
	assert(m_pScriptObject == NULL);
	m_timeLastActive = INT_MAX;
	m_pSprite = NULL;
	m_pVisualProfile = NULL; //note, we don't init or delete this ourself, visual manager handles it
	m_pSpriteLastUsed = NULL;
	m_pCollisionData = NULL;
	m_ticksOnGround = 0;
	m_navNodeType = C_NODE_TYPE_NORMAL;
	m_bAnimPaused = false;
	m_collisionListenBits = 0; //don't listen to anything
	
	m_visualState = VisualProfile::VISUAL_STATE_IDLE;
	m_animID = 0;
	SetFacing(VisualProfile::FACING_LEFT);
	m_trigger.Reset();
	m_bHasRunOnInit = false;
	m_bHasRunPostInit = false;
	m_bHasRunOnMapInsert = false;
	m_bRequestsVisibilityNotifications = false;
	m_lastVisibilityNotificationID = INT_MAX; //it starts at 0, so max_int is better to use than 0 as the default
	m_bOnScreen = false;
	m_customDampening = -1;
	m_gravityOverride = C_GRAVITY_OVERRIDE_DISABLED;
	m_blendMode = C_BLEND_MODE_NORMAL;
	m_visualStateOverride = VisualProfile::VISUAL_STATE_NONE;
	SetIsCreature(false);
	m_contacts.clear();

}

void MovingEntity::SetNavNodeType(int n)
{
	if (IsInitted())
	{
		LogError("Cannot use SetNavNodeType except in the OnInit() function");
		return;
	}

	m_navNodeType = n;
}

void MovingEntity::SetHasPathNode(bool bHasNode)
{
	if (IsInitted())
	{
		LogError("Cannot use SetHasPathNode except in the OnInit() function");
		return;
	}

	m_pTile->SetBit(Tile::e_pathNode, bHasNode);
}


Goal_Think * MovingEntity::GetGoalManager()
{
	//create it if it doesn't exist
	if (m_pGoalManager) return m_pGoalManager;
	return (m_pGoalManager = new Goal_Think(this, "Base"));
}

void MovingEntity::SetFont(const string &resource_id)
{
		SAFE_DELETE(m_pFont);
		m_pFont = new CL_Font(resource_id, GetGameLogic()->GetGUIStyle()->get_resources());
		if (m_pFont) m_pFont->set_alignment(origin_center,0,0);
}

void MovingEntity::SetFont(int font_id)
{
		SAFE_DELETE(m_pFont);
		m_pFont = new CL_Font(*GetApp()->GetFont(font_id));
		if (m_pFont) m_pFont->set_alignment(origin_center,0,0);
}

void MovingEntity::SetText(const string &text)
{
	m_text = text;
	StringReplace("\\n", "\n", m_text);

	if (m_pFont)
	{
		//SAFE_DELETE(m_pFont);
	} else
	{
		if (!m_pFont)
		{
			if (GetGameLogic()->GetGUIStyle())
			{
				m_pFont = new CL_Font("font_label", GetGameLogic()->GetGUIStyle()->get_resources());
			} else
			{
				m_pFont = new CL_Font(*GetApp()->GetFont(C_FONT_NORMAL));
			}
		}

		m_pFont->set_alignment(origin_center,0,0);
	}
}

CL_Vector2 MovingEntity::GetTextBounds()
{
	if (m_pFont)
	{
		float x,y;
		m_pFont->get_scale(x,y);
		//try to work around a clanlib bounds bug when dealing with small sizes
		if (x < 1)
		{
			m_pFont->set_scale(x* (1+( (1-x)*0.3   )), y);
		}

		CL_Size size = m_pFont->get_size(m_text);

		//put it back
		m_pFont->set_scale(x,y);

		return CL_Vector2(size.width, size.height);
	} else
	{
		LogError("Can't use GetTextBounds(), entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
	}

	return CL_Vector2(0,0);
}

void MovingEntity::SetTextRect(const CL_Rect &r)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextRect, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_textRect = r;
	
}

void MovingEntity::SetTextAlignment(int alignment, CL_Vector2 v)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextAlignment, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_pFont->set_alignment(CL_Origin(alignment), v.x, v.y);
}

void MovingEntity::SetTextColor(CL_Color col)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextColor, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_pFont->set_color(col);
}

CL_Color MovingEntity::GetTextColor()
{
	if (!m_pFont)
	{
		LogError("Can't use GetTextColor, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return CL_Color(0,0,0);
	}

	return m_pFont->get_color();
}

void MovingEntity::SetTextScale(const CL_Vector2 &vecScale)
{
	if (!m_pFont)
	{
		LogError("Can't use SetTextScale, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return;
	}

	m_pFont->set_scale(vecScale.x, vecScale.y);
}

CL_Vector2 MovingEntity::GetTextScale()
{
	if (!m_pFont)
	{
		LogError("Can't use GetTextScale, entity %d (%s) doesn't have any font assigned (Use SetText first)", ID(), GetName().c_str());
		return CL_Vector2(0,0);
	}

	float x,y;
	m_pFont->get_scale(x,y);
	return CL_Vector2(x,y);
}



MovingEntity * MovingEntity::CreateEntity(Map *pMap, CL_Vector2 pos, const string &script)
{
	if (!pMap)
	{
		//use our own active map, none was specific
		pMap = GetMap();
	}

	return ::CreateEntity(pMap, pos, ProcessPathNoScript(script));
}


void MovingEntity::SetFacingTarget(int facing)
{
	SetVectorFacingTarget(FacingToVector(facing));
}

int MovingEntity::GetFacingTarget()
{
	return VectorToFacing(m_vecFacingTarget);
}

void MovingEntity::SetVisualStateOverride(int visualState)
{
	if (visualState != m_visualStateOverride)
	{
		m_bRestartAnim = true;
	}

	m_visualStateOverride = visualState;
}

int MovingEntity::GetVisualState()
{
	if (m_visualStateOverride != VisualProfile::VISUAL_STATE_NONE)
	{
		return m_visualStateOverride;
	}

	return m_visualState;
}

int MovingEntity::GetVisualStateOverride()
{
		return m_visualStateOverride;
}	

void MovingEntity::SetVisualState(int visualState)
{
	if (m_visualStateOverride == VisualProfile::VISUAL_STATE_NONE && m_visualState != visualState) 
	{
		m_bRestartAnim = true;
	}
	m_visualState = visualState;
}

void MovingEntity::SetFacing(int facing)
{
	m_facing = facing;
	m_vecFacing = m_vecFacingTarget = FacingToVector(m_facing);
}

void MovingEntity::SetVectorFacing(const CL_Vector2 &v)
{
	m_vecFacingTarget = m_vecFacing = v;
	m_facing = VectorToFacing(v);
}

void MovingEntity::SetVectorFacingTarget(const CL_Vector2 &v)
{
	m_vecFacingTarget = v;
}

CL_Vector2 MovingEntity::GetVectorFacing()
{
	return m_vecFacing;
}

CL_Vector2 MovingEntity::GetVectorFacingTarget()
{
	return m_vecFacingTarget;
}

void MovingEntity::SetName(const std::string &name)
{
	
	SetNameEx(name, true);

}
void MovingEntity::SetNameEx(const std::string &name, bool bRemoveOldTag)
{
	//setting a name has a special meaning when done in a moving entity.  It
	//means this entity can be tracked by its name from anywhere in the game, it's
	//stored in a special database that is loaded even when the entities world
	//isn't.  So don't set this unless an entity specifically needs a trackable
	//identity
	
	if (IsPlaced())
	{
		if (bRemoveOldTag)
		g_TagManager.Remove(this);
		BaseGameEntity::SetName(name);

		if (name.empty())
		{
			m_hashedName = 0;
		} else
		{
			m_hashedName = HashString(name.c_str());
			g_TagManager.Update(GetMap(), this);
		}
	} else
	{
		BaseGameEntity::SetName(name);
		m_hashedName = HashString(name.c_str());
	}
	
}

void MovingEntity::Serialize(CL_FileHelper &helper)
{
	
	if (helper.IsWriting() && m_pScriptObject && IsPlaced() && m_bHasRunPostInit && !GetDeleteFlag())
	{
		RunFunctionIfExists("OnSave");
	}

	
	//load/save needed data
	cl_uint8 ver = m_pTile->GetParentScreen()->GetVersion();

	float orientation = GetRotation();

	if (helper.IsWriting() || ver > 0)
	{
		helper.process(m_vecFacing);
		helper.process(m_facing);
		helper.process(orientation);
	}


    helper.process(m_mainScript);

	string temp;

	if (helper.IsWriting())
	{

#ifdef _DEBUG
		if (!GetName().empty())
		{
			//named entity.  Let's make sure the tagcache data is right
			TagObject * pTag = g_TagManager.GetFromHash(m_hashedName);
			if (!pTag)
			{
				LogError("%s's Tagcache data missing", GetName().c_str());
			} else
			{

				if (pTag->GetPos() != GetPos())
				{
					LogError("%s doesn't match his tagcache's position", GetName().c_str());
				}
				if (pTag->m_pWorld != GetMap())
				{
					LogError("%s doesn't match his tagcache's world", GetName().c_str());
				}
			}
		}
#endif
		
		temp = GetName();
		helper.process(temp);
	} else
	{
		helper.process(temp);
		SetName(temp);
	}

	//LogMsg("Loading version %d", ver);
    GetData()->Serialize(helper);
	SetVectorFacing(m_vecFacing);

	if (!helper.IsWriting())
	{
		//we need to init this too, as we just loaded it.
		Init();
		SetRotation(orientation);
		g_pMapManager->AddToEntityUpdateList(this); //this will allow it to get a visibility notification or something

		//SetSpriteByVisualStateAndFacing();
	}

}

void MovingEntity::SetMainScriptFileName(const string &fileName)
{
	m_mainScript = fileName;
}

BaseGameEntity * MovingEntity::CreateClone(TileEntity *pTile)
{
	if (m_pScriptObject && IsPlaced() && m_bHasRunPostInit)
	{
		RunFunctionIfExists("OnSave");
	}
	
	MovingEntity *pNew = new MovingEntity;
	pTile->SetEntity(pNew);

	pNew->SetMainScriptFileName(m_mainScript);
	pNew->SetName(GetName());
	pNew->SetPos(GetPos());
	*pNew->GetData() = m_dataManager; //copy our entity specific vars too
	
	pNew->SetVectorFacing(m_vecFacing);
	
	CL_Origin o;
	int x,y;
	GetAlignment(o, x, y);
	pTile->SetAlignment(o, x , y);
	pNew->Init();
	pNew->SetRotation(GetRotation());

	//InitEntity(pNew);
	return pNew;
}

CL_Rectf MovingEntity::GetWorldRect()
{
	return GetBoundsRectf()+CL_Pointf(GetPos().x, GetPos().y);
}

CL_Rectf MovingEntity::GetBoundsRectf()
{
	//OPTIMIZE:  This is called many times during a frame PER entity, we can probably get a big speed increase
	//by caching this info out with a changed flag or something

	if (!m_pSprite)
	{
		return CL_Rectf(0,0,32,32); //no sprite set yet...
	}

	int sizeX, sizeY;

	if (m_bSizeOverride)
	{
		sizeX = m_ptSizeOverride.x;
		sizeY = m_ptSizeOverride.y;
	} else
	{
		sizeX = m_pSprite->get_width();
		sizeY = m_pSprite->get_height();
	}

	CL_Origin o;
	int x,y;

	m_pSprite->get_alignment(o, x, y);

	CL_Vector2 vPos = CL_Vector2(0,0);
	CL_Pointf pt = calc_origin(o, CL_Size(sizeX*m_pTile->GetScale().x, sizeY * m_pTile->GetScale().y) );

	vPos.x -= pt.x;
	vPos.y -= pt.y;

	CL_Rectf r(vPos.x, vPos.y, 
		vPos.x + sizeX*m_pTile->GetScale().x, vPos.y + sizeY* m_pTile->GetScale().y);


	r.apply_alignment(origin_top_left,- (x*m_pTile->GetScale().x) , -y* m_pTile->GetScale().y);
	//r.apply_alignment(origin_top_left,- (x*m_pTile->GetScale().x) , -y* m_pTile->GetScale().y);
	/*
	//I have no idea why I need this as a special case??? But also too lazy to look into it.
	if (o == origin_top_left)
	{
	} else
	{
		r.apply_alignment(origin_top_left,- (x*m_pTile->GetScale().x) , -y* m_pTile->GetScale().y);
	}
	*/

	return r;
}

const CL_Rect & MovingEntity::GetBoundsRect()
{
	
	static CL_Rect rc;
	rc = CL_Rect(GetBoundsRectf());
	return rc;
}

string MovingEntity::ProcessPath(const string &st)
{
	if (st[0] == '~')
	{
		return C_DEFAULT_SCRIPT_PATH + CL_String::get_path(m_mainScript) + (st.c_str()+1);
	}

	return st;
}

string MovingEntity::ProcessPathNoScript(const string &st)
{
	if (st[0] == '~')
	{
		return CL_String::get_path(m_mainScript) + (st.c_str()+1);
	}

	return st;
}

void MovingEntity::UpdateSoundByPosition(int soundID, float minHearing, float maxHearing, float volMod)
{
	if (g_pMapManager->GetActiveMap() != GetMap())
	{
		//LogMsg("%s is not on the same map, ignoring sound", GetName().c_str());
		//we're not in the same map
		g_pSoundManager->SetVolume(soundID, 0);
		return;
	}
	//calculate the volume/pan based on camera position

	CL_Vector2 vDist = GetPos() - GetCamera->GetPosCentered();
	float dist = vDist.length();

	if (dist > maxHearing)
	{
	
		g_pSoundManager->SetVolume(soundID, 0);
		return;
	}

	if (dist < minHearing)
	{
		g_pSoundManager->SetVolume(soundID, 0);
	}

	//smoothly transition
	float percentCloseness = 1 - ( (dist - minHearing) / (maxHearing-minHearing));

	//volMod = min(volMod, 1);
	assert(volMod >= 0 && volMod <= 1 && "huh?");
	g_pSoundManager->SetVolume(soundID, percentCloseness * volMod);
}

const float C_DEFAULT_MIN_HEARING = 100;
const float C_DEFAULT_MAX_HEARING = 800;

int MovingEntity::PlaySoundPositioned(const string &fName)
{
	if (g_pMapManager->GetActiveMap() != GetMap())
	{
		//we're not in the same map
		return C_SOUND_NONE;
	}
	
	if (!g_pSoundManager) return C_SOUND_NONE;

	int hHandle = g_pSoundManager->Play(ProcessPath(fName).c_str());
	
	UpdateSoundByPosition(hHandle, C_DEFAULT_MIN_HEARING, C_DEFAULT_MAX_HEARING, 1);
	return hHandle;
}

int MovingEntity::PlaySound(const string &fName)
{
	if (!g_pSoundManager) return C_SOUND_NONE;

	int hHandle = g_pSoundManager->Play(ProcessPath(fName).c_str());
	return hHandle;
}

bool MovingEntity::SetVisualProfile(string resourceFileName, const string &profileName)
{
	resourceFileName = C_DEFAULT_SCRIPT_PATH + ProcessPathNoScript(resourceFileName);
	VisualResource *pResource = GetVisualProfileManager->GetVisualResource(resourceFileName);
	if (!pResource) return false; //failed that big time

	//now we need to get the actual profile
	m_pVisualProfile = pResource->GetProfile(profileName);
	
	SetIsCreature(true); //a guess here
	return true; //success
}

void MovingEntity::InitializeBody()
{
	
/*
	if (!m_bHasRunPostInit)
	{
		//don't let the physics start doing crap until the scripts get a chance to do their thing
		return;
	}
	*/

	if (!m_pTile->GetParentScreen())
	{
		//we're not on a real map yet
		return;
	}
	KillBody();

if (!m_pCollisionData || !m_pCollisionData->HasData()) return;

	//LogMsg("initializing body (%d) (%s) with density %.2f", ID(), m_mainScript.c_str(), m_fDensity);
	//MAKE BODY
	b2BodyDef bd;

	bd.userData = this;
	//bd.isBullet = true; //keep in mind this breaks SetXForm
	bd.fixedRotation = !m_bCanRotate;
	bd.position = ToPhysicsSpace(GetPos());
	m_pBody = GetPhysicsManager()->GetBox2D()->CreateBody(&bd);

	if (m_pCollisionData->GetLineList()->size() > 0) 
	{
		//do this for each line...

		line_list::iterator itor = m_pCollisionData->GetLineList()->begin();

		for (; itor != m_pCollisionData->GetLineList()->end(); itor++)
		{
			PointList * pPointList = &(*itor);
			CMaterial *pMat = g_materialManager.GetMaterial(pPointList->GetType());
			if (pMat->GetType() == CMaterial::C_MATERIAL_TYPE_DUMMY)		
			{
				//LogMsg("Ignoring dummy or warp..");
				continue;
			}


			if (pPointList->IsValidBox2DPolygon() != PointList::STATUS_OK)
			{
				continue;
			}

			b2PolygonDef shapeDef;

			pPointList->GetAsPolygonDef(&shapeDef);
			shapeDef.density = m_fDensity;
			shapeDef.filter = m_filterData;
				
			if (pMat->GetType() == CMaterial::C_MATERIAL_TYPE_NORMAL)
			{
				ShapeUserData *pShapeUserData = new ShapeUserData();
				pShapeUserData->pOwnerEnt = this;
				pShapeUserData->pOwnerTile = NULL;
				pShapeUserData->materialID = pPointList->GetType();

				shapeDef.userData = pShapeUserData;
				
				m_pBody->CreateShape(&shapeDef);
				
				m_pBody->SetMassFromShapes();

			}
			
			//create sensor version which we may or may not need
			ShapeUserData *pShapeUserData = new ShapeUserData();
			pShapeUserData->pOwnerEnt = this;
			pShapeUserData->pOwnerTile = NULL;
			pShapeUserData->materialID = pPointList->GetType();
			
			shapeDef.userData = pShapeUserData;
			
			shapeDef.density = 0;
			//shapeDef.friction = 0;
			shapeDef.isSensor = true;

			m_pBody->CreateShape(&shapeDef);

			m_pBody->WakeUp();
			
		}
		
	}

}

void MovingEntity::SetPos(const CL_Vector2 &new_pos)
{
	
	m_pos = new_pos;
//	LogMsg("SetPos: Setting ent %d to %s", ID(), PrintVector(m_pos).c_str());
	m_bMovedFlag = true;

	if (m_bRanApplyPhysics)
	{
		UpdatePosToPhysics();
	}

	if (!m_strWorldToMoveTo.empty())
	{
		m_moveToAtEndOfFrame = new_pos;
		//LogMsg("Warning: SetPos() command issued to Entity %d (%s) ignored, because a map-move request was scheduled this frame",
		//	ID(), GetName().c_str());
	}
}

bool MovingEntity::SetPosAndMapByTagName(const string &name)
{
	TagObject *pO = g_TagManager.GetFromString(name);

	if (!pO)
	{
		LogMsg("SetPosAndMapByName: Unable to locate entity by name: %s", name.c_str());
		return false;
	} 


	SetPosAndMap(pO->m_pos, pO->m_pWorld->GetName());
	return true; 
}


void MovingEntity::ForceUpdatingPeriod()
{
	
	//if (!g_pMapManager->IsOnEntityUpdateList(this))
	{
#ifdef _DEBUG
	LogMsg("Forcing update for %s", GetName().c_str());
#endif
		g_watchManager.AddSilently(this, 100);
		RunPostInitIfNeeded();
	}
	

}

void MovingEntity::SetPosAndMap(const CL_Vector2 &new_pos, const string &worldName)
{

	if (worldName != m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap()->GetName())
	{
		//LogMsg("Changing world to %s at %s", worldName.c_str(), PrintVector(new_pos).c_str());
		//we also need to move it to a new world
		m_strWorldToMoveTo = worldName; //can't do it now, but we'll do it asap
		//we can't just move now, because we still need to finish drawing this frame
		m_moveToAtEndOfFrame = new_pos;
		
#ifdef _DEBUG
		LogMsg("Warping at end of logic ent %s...", GetName().c_str());
#endif
	
	} else
	{
		//m_pos = new_pos;
		SetPos(new_pos);
#ifdef _DEBUG
		LogMsg("Moving ent %s...", GetName().c_str());
#endif
		if (GetCamera->GetEntTracking() == ID())
		{
			GetCamera->SetInstantUpdateOnNextFrame(true);
		}
	

	}
	
	ForceUpdatingPeriod();

	m_bMovedFlag = true;
	
}


Zone MovingEntity::GetNearbyZoneByCollisionRectAndType(int matType)
{
	return GetMap()->GetZoneByRectAndType(GetWorldCollisionRect(), matType);
}


Zone MovingEntity::GetNearbyZoneByPointAndType(const CL_Vector2 &vPos, int matType)
{
	return GetMap()->GetZoneByPointAndType(vPos, matType);
}

void MovingEntity::GetAlignment(CL_Origin &origin, int &x, int &y)
{
	if (m_pSprite && m_pSprite->get_frame_count() > 0)
	{
		m_pSprite->get_alignment(origin, x, y);
	} else
	{
		assert(m_pTile);
		m_pTile->GetBaseAlignment(origin, x, y);
	}
}



void MovingEntity::SetAlignment(int origin, CL_Vector2 v)
{

	if (m_pSprite && m_pSprite->get_frame_count() != 0)
	{
		m_pSprite->set_alignment(CL_Origin(origin), v.x, v.y);
	} else
	{
		LogError("Entity %d (%s) can't set alignment, it has no sprite visual yet.", ID(), GetName().c_str());
	}
}


//Note, this is NOT safe to use except in the post update functions.  An entity might have
//been deleted otherwise.



CL_Rect MovingEntity::GetImageClipRect()
{
	return StringToRect(m_dataManager.Get(C_ENT_TILE_RECT_KEY));
}

unsigned int MovingEntity::GetImageID()
{
	return CL_String::to_int(m_dataManager.Get(C_ENT_TILE_PIC_ID_KEY));
}

void MovingEntity::SetImage(const string &fileName, CL_Rect *pSrcRect)
{
	unsigned int imageID = FileNameToID(fileName.c_str());


	if (!GetHashedResourceManager->HashedIDExists(imageID))
	{
		//The image doesn't exist in our resource manager.  But let's try to add it...
	
		string fullFileName = ProcessPath(fileName);
		if (g_VFManager.LocateFile(fullFileName))
		{
			//LogMsg("Located file %s", fullFileName.c_str()); 
			GetHashedResourceManager->AddGraphic(fullFileName);
		}
	}

	SetImageByID(imageID, pSrcRect);

}


void MovingEntity::AddImageToMapCache(const string &fileName)
{
	unsigned int imageID = FileNameToID(fileName.c_str());


	if (!GetHashedResourceManager->HashedIDExists(imageID))
	{
		//The image doesn't exist in our resource manager.  But let's try to add it...

		string fullFileName = ProcessPath(fileName);
		if (g_VFManager.LocateFile(fullFileName))
		{
			//LogMsg("Located file %s", fullFileName.c_str()); 
			GetHashedResourceManager->AddGraphic(fullFileName);
		}
	}
}

/*
void MovingEntity::SetClipRect(CL_Rect *pSrcRect)
{
	if (!m_pSprite || m_pSprite->get_frame_count() == 0)
	{
		LogError("Entity:SetClipRect requires that an image be loaded before using.  Like with SetImage or something.");
		return;

	}
	
	if (!m_bUsingSimpleSprite)
	{
		LogError("Entity:SetClipRect can currently only be used with simple tilepic images, such as those set with SetImage.");
		return;
	}

	m_pSprite->get

}
*/

bool MovingEntity::SetImageByID(unsigned int picID, CL_Rect *pSrcRect)
{

	CL_Surface *pSurf = GetHashedResourceManager->GetResourceByHashedID(picID);
	if (!pSurf)
	{
		LogError("Error, entity %d (%s) is unable to find image with hash %u to use.", ID(), GetName().c_str(), picID);
		return false;
	} 

	int x = 0;
	int y = 0;
	CL_Origin align;

	GetTile()->GetAlignment(align,x,y);

/*
	if (m_pSprite && m_pSprite->get_frame_count() != 0)
	{
		//also check if a TilePic is set and steal its alignment???
		m_pSprite->get_alignment(align, x,y);
	}
*/
	SAFE_DELETE(m_pSprite);

	m_pSprite = new CL_Sprite();
	static CL_Rect imageRect;
	if (pSrcRect == NULL )
	{
		//use default image size
		imageRect = CL_Rect(0,0, pSurf->get_width(), pSurf->get_height());
		pSrcRect = &imageRect;
	}
	
	//check if it's valid
	if (pSurf->get_width() < pSrcRect->right || pSurf->get_height() < pSrcRect->bottom)
	{
		if (pSurf->get_width() < pSrcRect->right)
		pSrcRect->right = pSurf->get_width();
		

		if (pSurf->get_height() < pSrcRect->bottom)
		pSrcRect->bottom = pSurf->get_height();
		LogMsg("Ent %d (%s) tilePic size illegal, truncated to match the size of image", ID(), GetName().c_str());
	}

	m_pSprite->add_frame(*pSurf, *pSrcRect);
	m_pSprite->set_alignment(align,x,y);

	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
		m_bUsingCustomCollisionData = false;
	}

	SetCollisionInfoFromPic(picID, *pSrcRect);

	m_bUsingSimpleSprite = true;
	m_dataManager.Set(C_ENT_TILE_PIC_ID_KEY, CL_String::from_int(picID));
	m_dataManager.Set(C_ENT_TILE_RECT_KEY, RectToString(*pSrcRect));

	
	return true;
}


bool MovingEntity::Init()
{
	m_bSetAnimByName = false;
	assert(!m_pSprite);

	unsigned int picID = CL_String::to_int(m_dataManager.Get(C_ENT_TILE_PIC_ID_KEY));
	
	if (picID != 0)
	{
			CL_Rect picSrcRect = StringToRect(m_dataManager.Get(C_ENT_TILE_RECT_KEY));
		
			if (SetImageByID(picID, &picSrcRect))
			{
				CL_Origin o;
				int x,y;
				GetTile()->GetBaseAlignment(o,x,y);
				m_pSprite->set_alignment(o,x,y); 
			}
	}

	if (!m_pSprite)
	m_pSprite = new CL_Sprite(); //visual data will be set on it later


	//override settings with certain things found
	if (m_dataManager.HasData())
	{

		if (m_dataManager.Exists(C_ENT_DENSITY_KEY))
		{
			if (GetCollisionData() && GetCollisionData()->HasData())
			{
				SetDensity(CL_String::to_float(m_dataManager.Get(C_ENT_DENSITY_KEY)));
			}
		}

		if (m_dataManager.Exists(C_ENT_ROTATE_KEY))
		{
			if (GetCollisionData() && GetCollisionData()->HasData())
			{
				EnableRotation(CL_String::to_bool(m_dataManager.Get(C_ENT_ROTATE_KEY)));
			}

		}
	}


	string stEmergencyMessage;

	if (!m_mainScript.empty())
	if (!LoadScript(m_mainScript.c_str()))
	{
		stEmergencyMessage = "Ent "+CL_String::from_int(ID()) + " ("+GetName()+") error while loading lua script " + m_mainScript +".";
		LogError(stEmergencyMessage.c_str());
		LoadScript(C_DEFAULT_SCRIPT);
	}

	if (m_pSprite->is_null())
	{
		string sFile = "system/system.xml";

		//avoid a crash
		if (!m_pVisualProfile)
		{
			
			if (m_mainScript.empty())
			{
				//LogError("(giving entity %d (%s) the default script so you can see it to delete it)", ID(), GetName().c_str());
			} else
			{
				LogError("No visual profile was set in script %s's init!  Maybe it had a syntax error, go check the log. (trying to load default)", m_mainScript.c_str());
			}
			
			g_VFManager.LocateFile(sFile);
			SetVisualProfile(sFile,"ent_default");
		}

		if (!m_pVisualProfile->GetSprite(VisualProfile::VISUAL_STATE_IDLE, VisualProfile::FACING_LEFT))
		{
			LogError("Unable to set default anim of idle_left in profile %s!", m_pVisualProfile->GetName().c_str());
			g_VFManager.LocateFile(sFile);
			SetVisualProfile(sFile,"ent_default");


		}
	}

	if (GetVisualProfile() && !m_bSetAnimByName)
	{
		SetSpriteByVisualStateAndFacing();
	}
	
	m_bHasRunOnInit = true;
	return true;
}

void MovingEntity::AddEffect(L_ParticleEffect *pEffect)
{
	pEffect->trigger();
	pEffect->initialize();
	m_effectManager.add(pEffect);
}

void MovingEntity::ResetEffects()
{
	if (m_effectManager.effect_list.size() > 0)
	{

		std::list<L_ParticleEffect*>::iterator effectItor = m_effectManager.effect_list.begin();

		for (; effectItor != m_effectManager.effect_list.end(); effectItor++)
		{
			(*effectItor)->clear();

		}

	}
}


void MovingEntity::RunPostInitIfNeeded()
{
	assert(IsPlaced());
	if (!m_bHasRunPostInit)
	{

		if (m_pScriptObject)
		{
			if (m_pScriptObject->FunctionExists("OnPostInit"))
				m_pScriptObject->RunFunction("OnPostInit");
		} 
		m_bHasRunPostInit = true;
		InitializeBody();

	}
}

void MovingEntity::RunOnMapInsertIfNeeded()
{
	if (!m_bHasRunOnMapInsert)
	{
		InitializeBody();
		
		if (m_pScriptObject)
		{
			if (m_pScriptObject->FunctionExists("OnMapInsert"))
				m_pScriptObject->RunFunction("OnMapInsert");
		}

		m_bHasRunOnMapInsert = true;
	}

	
}

bool MovingEntity::LoadScript(const char *pFileName)
{
	SAFE_DELETE(m_pScriptObject);
		
	m_pScriptObject = new ScriptObject();
	string s = C_DEFAULT_SCRIPT_PATH;
	s += pFileName;

	StringReplace("\\", "/", s);
	if (s[s.length()-1] == 'a') s+="c"; //try to grab the compiled version first
	if (!g_VFManager.LocateFile(s))
	{
		//try again
		s = s.substr(0, s.length()-1); //cut the .c off and try again
		if (!g_VFManager.LocateFile(s))
		{
			return false;
		}
	}

	if (!m_pScriptObject->Load(s.c_str()))
	{
		return false;
	}

	//set this script objects "this" variable to point to an instance of this class
	luabind::globals(m_pScriptObject->GetState())["this"] = this;
	m_pScriptObject->RunFunction("OnInit");
	
	
	return true;
}

bool MovingEntity::ScriptNotReady(const string &func, bool bShowError)
{

	if (!m_pScriptObject)
	{
		if (bShowError)
		{
			LogError("Can't RunFunction %s on entity %d (%s), no script is attached.",
				func.c_str(), ID(), GetName().c_str());
		}

		return true; //signal error condition
	}

	if (!GetScriptObject()->FunctionExists(func.c_str()))
		GetScriptManager->SetStrict(false);

	return false; //no error
}

luabind::object MovingEntity::RunFunction(const string &func)
{
	luabind::object ret;
	
	if (ScriptNotReady(func)) return ret; //also shows an error for us
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str());
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);

	return ret;
}


luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1)
{
	luabind::object ret;
	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2)
{

	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}


luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}

luabind::object MovingEntity::RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7, luabind::object obj8)
{
	luabind::object ret;

	if (ScriptNotReady(func)) return ret; //also shows an error for us
	
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	GetScriptManager->SetStrict(true);
return ret;
}



luabind::object MovingEntity::RunFunctionIfExists(const string &func)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error
	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str());
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}


luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1)
{
	luabind::object ret;
	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2)
{

	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}


luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6)
{
	luabind::object ret;
	
	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

luabind::object MovingEntity::RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7, luabind::object obj8)
{
	luabind::object ret;

	if (ScriptNotReady(func, false)) return ret; //also shows an error for us
	if (!GetScriptObject()->FunctionExists(func.c_str())) return ret; //silent error

	try {ret = luabind::call_function<luabind::object>(m_pScriptObject->GetState(), func.c_str(), obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8);
	} LUABIND_ENT_CATCH( ("Error while calling function " + func).c_str());
	return ret;
}

void MovingEntity::UpdateTilePosition()
{
		assert(m_pTile);
		assert(m_pTile->GetParentScreen());


		bool bReInitBody = false;

	if (m_bMovedFlag)
	{
		Map *pWorldToMoveTo = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
		
		if (!m_strWorldToMoveTo.empty())
		{
			 MapInfo *pInfo = g_pMapManager->GetMapInfoByName(m_strWorldToMoveTo);

			 if (pInfo)
			 {
				
				 KillBody();
				 bReInitBody = true;

				//load it if needed
				 if (!pInfo->m_world.IsInitted())
				 {
					 g_pMapManager->LoadMap(pInfo->m_world.GetDirPath(), false);
					 pInfo->m_world.PreloadMap();

				 }
				 m_pos = m_moveToAtEndOfFrame;
				pWorldToMoveTo = &pInfo->m_world;
				
				//assert(!"Don't handle moving between maps yet..");
				
				SAFE_DELETE(m_pPathPlanner); //this would be invalid now

				if (GetCamera->GetEntTracking() == ID())
				{
					 //the camera should follow the entity
					CameraSetting cs = GetCamera->GetCameraSettings();
					g_pMapManager->SetActiveMapByPath(pInfo->m_world.GetDirPath(),&cs);
				}
				ResetEffects();

			 } else
			 {

				 LogMsg("Error: Entity unable to move, world %s not found", m_strWorldToMoveTo.c_str());
				 m_bMovedFlag = false;
				 m_strWorldToMoveTo.clear();
				 return;
			 }

			 m_strWorldToMoveTo.clear();	 
		} 
		
		m_pTile->SetEntity(NULL); //so it won't delete us
		TileEntity *pTileEnt = (TileEntity*) m_pTile->CreateClone();
		
		m_pTile->GetParentScreen()->RemoveTileByPointer(m_pTile);


		//add it back
		pTileEnt->SetEntity(this); //this will set our m_pTile automatically
		
		if (m_requestNewLayerID != C_LAYER_NONE)
		{
			m_pTile->SetLayer(m_requestNewLayerID);
			m_requestNewLayerID = C_LAYER_NONE;
		}

		pWorldToMoveTo->AddTile(m_pTile);
		m_bMovedFlag = false;
		if (bReInitBody)
		{
			InitializeBody();
		}
	
	} 

}

void MovingEntity::SetAnimByName(const string &name)
{
	if (GetVisualProfile())
	{
		//LogMsg("Setting anim %s", name.c_str());
		m_animID = GetVisualProfile()->StateNameToAnimID(name);
		SetSpriteData(GetVisualProfile()->GetSpriteByAnimID(m_animID));
		m_bSetAnimByName = true;
	}
}
string MovingEntity::GetAnimName()
{
	if (GetVisualProfile())
	{
		return GetVisualProfile()->AnimIDToText(m_animID);
	}
	return "";
}

void MovingEntity::SetAnimFrame(int frame)
{
	if (GetSprite() && GetSprite()->get_frame_count() > 0)
	{
		if (frame == C_ANIM_FRAME_LAST)
		{
			frame = GetSprite()->get_frame_count()-1;
		}
		GetSprite()->restart(); //clear finished flags, timing
		GetSprite()->set_frame(frame);	
		
	} else
	{
		LogMsg("Error, ent %d (%s) can't set anim frame, no visual seems to be assigned to it. (Use OnPostInit maybe?)", ID(), GetName().c_str());
	}

	m_bRestartAnim = false; //if this had been set, we probably don't want it activated now
}

int MovingEntity::GetAnimFrame()
{
	if (GetSprite())
	{
		return GetSprite()->get_current_frame();

	} else
	{
		LogMsg("Error, ent %d (%s) can't get anim frame, no visual seems to be assigned to it. (Use OnPostInit maybe?)", ID(), GetName().c_str());
	}

	return 0;
}


void MovingEntity::SetSpriteByVisualStateAndFacing()
{
	
	if (GetVisualProfile())
	{
		m_animID = GetVisualProfile()->GetAnimID(GetVisualState(), GetFacing());
		if (m_animID == -1)
		{
			//unable to get a valid anim
			
			/*
			string sFile = "system/system.xml";
			g_VFManager.LocateFile(sFile);
			SetVisualProfile(sFile,"ent_default");
			*/
			
			return;
		}
		SetSpriteData(GetVisualProfile()->GetSpriteByAnimID(m_animID));
	}
}

void MovingEntity::LastCollisionWasInvalidated()
{
	m_bTouchedAGroundThisFrame = m_bOldTouchedAGroundThisFrame;
	m_floorMaterialID = m_oldFloorMaterialID;
}

void MovingEntity::SetIsOnGround(bool bOnGround)
{
	if (bOnGround)
	{
		m_groundTimer = GetApp()->GetGameTick()+C_GROUND_RELAX_TIME_MS;
		m_bTouchedAGroundThisFrame = true;
	} else
	{
		m_groundTimer = 0;
		m_ticksOnGround = 0;
	}
}

void MovingEntity::SetDensity(float fDensity)
{
	m_fDensity = fDensity;
	
	if (GetBody())
	{
		InitializeBody();
	}

}

void MovingEntity::EnableRotation(bool bRotate)
{
	if (bRotate != m_bCanRotate)
	{
		m_bCanRotate = bRotate;
		InitializeBody();

	}
}

bool MovingEntity::GetEnableRotation()
{
	return m_bCanRotate;
}

void MovingEntity::SetScale(const CL_Vector2 &vScale)
{
	m_pTile->SetScale(vScale);
}

void MovingEntity::LoadCollisionInfo(const string &fileName)
{
	if (m_bUsingCustomCollisionData)
	{
  		SAFE_DELETE(m_pCollisionData);
	}
	
	m_bUsingTilePicCollision = false;

	if (fileName == "")	
	{
		//guess they really don't want collision
		m_bUsingCustomCollisionData = false;
		
		KillBody();
		return;
	}
	
	m_pCollisionData = new CollisionData;
    m_bUsingCustomCollisionData = true;
	
	m_pCollisionData->Load(C_DEFAULT_SCRIPT_PATH + ProcessPathNoScript(fileName)); //it will init a default if the file is not found, and automatically handle

	PointList pl;
	PointList *pActiveLine;

	if (m_pCollisionData->GetLineList()->size() == 0)
	{
		//add default data
		m_pCollisionData->GetLineList()->push_back(pl);
		pActiveLine = &(*m_pCollisionData->GetLineList()->begin());
	
		//add our lines by hand
		pActiveLine->GetPointList()->resize(4);
		pActiveLine->GetPointList()->at(0) = CL_Vector2(-10,-10);
		pActiveLine->GetPointList()->at(1) = CL_Vector2(10,-10);
		pActiveLine->GetPointList()->at(2) = CL_Vector2(10,10);
		pActiveLine->GetPointList()->at(3) = CL_Vector2(-10,10);
		pActiveLine->CalculateOffsets();
	}

	pActiveLine = &(*m_pCollisionData->GetLineList()->begin());

	//link to data correctly
	
		//we don't want a physics presence until we give it some time to initialize itself
		InitializeBody();
	//pActiveLine->GetAsBody(CL_Vector2(0,0), &m_body);
	//SetDensity(m_fDensity);
	
	m_pCollisionData->SetScale(GetScale());
}

void MovingEntity::SetCollisionInfoFromPic(unsigned picID, const CL_Rect &recPic)
{
	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
	}

	m_bUsingTilePicCollision = true;
	m_bUsingCustomCollisionData = false;
	
	//copy it
	m_pCollisionData = GetHashedResourceManager->GetCollisionDataByHashedIDAndRect(picID, recPic);
	
	if (m_pTile->GetScale().x != 1 || m_pTile->GetScale().y != 1)
	{
		//we need to keep our own transformed version

		if (m_pCollisionData->GetLineList()->size() > 0)
		{
			//well, this has real data most likely, so let's create our own copy and apply scale to it
			CollisionData *pNewCol = new CollisionData(*m_pCollisionData);
			m_pCollisionData = pNewCol;
			m_pCollisionData->SetDataChanged(false);
			m_bUsingCustomCollisionData = true;
			m_pCollisionData->SetScale(GetScale());
		}
	}

/*
	PointList pl;
	PointList *pActiveLine = NULL;

	bool bHadNoData = false;
	if (m_pCollisionData->GetLineList()->size() != 0)
	{
		pActiveLine = &(*m_pCollisionData->GetLineList()->begin());
		if (!pActiveLine->HasData())
		{
			//we'll, we see a line but no points so this won't work well
			pActiveLine = NULL;
		}
	}

	//link to data correctly
	
	if (pActiveLine)
	{
	} else
	{
		SetDensity(0); //immovable
	}
	*/

	InitializeBody();

}

void MovingEntity::InitCollisionDataBySize(float x, float y)
{
	if (m_bUsingCustomCollisionData)
	{
		SAFE_DELETE(m_pCollisionData);
	}

	if (x == 0 && y == 0)
	{
		//let's guess here
		x = GetSizeX();
		y = GetSizeY();
	}

	CL_Origin origin;
	int offsetX,offsetY;
	m_pSprite->get_alignment(origin, offsetX, offsetY);
	CL_Pointf vecOrigin = calc_origin(origin,CL_Size(x,y));


	CL_Rect colRect(-vecOrigin.x, -vecOrigin.y, x - vecOrigin.x, y - vecOrigin.y);

	m_pCollisionData = new CollisionData;
	
	PointList pl;
	m_pCollisionData->GetLineList()->push_back(pl);
	PointList *pActiveLine;
	pActiveLine = &(*m_pCollisionData->GetLineList()->begin());

	//add our lines by hand
    pActiveLine->GetPointList()->resize(4);
	pActiveLine->GetPointList()->at(0) = CL_Vector2(colRect.left,colRect.top);
	pActiveLine->GetPointList()->at(1) = CL_Vector2(colRect.right,colRect.top);
	pActiveLine->GetPointList()->at(2) = CL_Vector2(colRect.right, colRect.bottom);
	pActiveLine->GetPointList()->at(3) = CL_Vector2(colRect.left, colRect.bottom);
	pActiveLine->CalculateOffsets();
	//get notified of collisions
	m_bUsingCustomCollisionData = true;
	//link to data correctly
	InitializeBody();
	SetDensity(1);
}

void MovingEntity::SetSpriteData(CL_Sprite *pSprite)
{
	if (m_pSpriteLastUsed != pSprite)
	{
		int frameTemp = 0;
		if (m_bRestartAnim)
		{
			if (m_pSprite && !m_pSprite->is_null()) 
			{
				m_pSprite->restart();
			}
			m_bRestartAnim = false;
		} else
		{
			if (m_pSprite && m_pSprite->get_frame_count() > 0) frameTemp = m_pSprite->get_current_frame();
		}

		m_pSprite->set_image_data(*pSprite);
		m_pSpriteLastUsed = pSprite;
		CL_Origin origin;
		int x,y;

		m_pSprite->set_angle(pSprite->get_angle());
		m_pSprite->set_angle_pitch(pSprite->get_angle_pitch());
		m_pSprite->set_angle_yaw(pSprite->get_angle_yaw());
		pSprite->get_alignment(origin, x,y);
		m_pSprite->set_alignment(origin, x, y);
		m_pSprite->set_show_on_finish(pSprite->get_show_on_finish());
		
		pSprite->get_rotation_hotspot(origin, x, y);
		m_pSprite->set_rotation_hotspot(origin, x, y);
		m_pSprite->set_frame(frameTemp);
		
	}
}



CL_Vector2 MovingEntity::GetVisualOffset()
{
	return CL_Vector2(0,0);
}

void MovingEntity::RenderShadow(void *pTarget)
{
	static CL_Vector2 vecPos;
	static Map *pWorld;
	static EntMapCache *pWorldCache;

	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	CL_Vector2 vVisualPos = GetPos();

	pWorld = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
	pWorldCache = pWorld->GetMyMapCache();

	vecPos = pWorldCache->WorldToScreen(vVisualPos);

	if (m_bLockedScale)
	{
		m_pSprite->set_scale( m_pTile->GetScale().x, m_pTile->GetScale().y);
	} else
	{
		m_pSprite->set_scale(GetCamera->GetScale().x * m_pTile->GetScale().x, GetCamera->GetScale().y * m_pTile->GetScale().y);
	}

	short a = min(50, 255 + m_colorModAlpha);
	
	m_pSprite->set_color(CL_Color(0,0,0,a));

	static CL_Surface_DrawParams1 params1;
	m_pSprite->setup_draw_params(vecPos.x, vecPos.y, params1, true);
	AddShadowToParam1(params1, m_pTile);
	m_pSprite->draw(params1, pGC);

}

void MovingEntity::Stop()
{
	GetBody()->SetAngularVelocity(0);
	GetBody()->SetLinearVelocity(b2Vec2(0,0));
}

void MovingEntity::StopY()
{
	GetBody()->SetLinearVelocity(b2Vec2(GetBody()->GetLinearVelocity().x,0));
}

void MovingEntity::StopX()
{
	GetBody()->SetLinearVelocity(b2Vec2(0, GetBody()->GetLinearVelocity().y));
}


void MovingEntity::SetRotation(float angle)
{
	m_angle = angle;
}
float MovingEntity::GetRotation()
{
	return m_angle;
}

CL_Vector2 MovingEntity::GetRawScreenPosition(bool &bRootIsCam)
{

	bRootIsCam = false;

	if (m_attachEntID == C_ENTITY_CAMERA)
	{
		bRootIsCam = true;
		return m_attachOffset;
	}

	if (m_attachEntID == 0)
	{
		//not attached to anything...
		return GetPos()-GetCamera->GetPos();
	}
	MovingEntity *pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(m_attachEntID);
	return pEnt->GetRawScreenPosition(bRootIsCam)+m_attachOffset;
}

void MovingEntity::Render(void *pTarget)
{
	if (!m_pSprite) return;
	m_lastVisibilityNotificationID = g_watchManager.GetVisibilityID();

	if (m_pSprite->get_current_frame() == -1)
	{
		LogError("Sprite %d (%s) has no valid sprite frame assigned.  Don't set 1 frame anims to ping pong? Turn looping on?", ID(), GetName().c_str());
		return;
	}
	
	CL_GraphicContext *pGC = (CL_GraphicContext *)pTarget;
	static float yawHold, pitchHold;

	CL_Vector2 vVisualPos = GetPos();
	CL_Vector2 vFinalScale = m_pTile->GetScale();
		
	if (!m_bLockedScale)
	{
		vFinalScale.x *= GetCamera->GetScale().x;
		vFinalScale.y *= GetCamera->GetScale().y;
	}

	yawHold = m_pSprite->get_angle_yaw();
	pitchHold = m_pSprite->get_angle_pitch();

	bool bNeedsReversed = false;

	if (m_pTile->GetBit(Tile::e_flippedX))
	{
		m_pSprite->set_angle_yaw(m_pSprite->get_angle_yaw() -180);
	}

	if (m_pTile->GetBit(Tile::e_flippedY))
	{
		m_pSprite->set_angle_pitch(m_pSprite->get_angle_pitch() -180);
	}


	if (m_pSprite->get_angle_yaw() != 0)
	{
		bNeedsReversed = !bNeedsReversed;
	}

	if (m_pSprite->get_angle_pitch() != 0)
	{
		bNeedsReversed = !bNeedsReversed;
	}

	if (m_angle < -100 || m_angle > 100) 
	{
		LogMsg("fixing bad angle in %d, angle is %.2f", ID(), m_angle);
		m_angle = 0;
	}
	if (!bNeedsReversed)
	{
		m_pSprite->set_base_angle( -RadiansToDegrees(GetRotation()));
	} else
	{
		m_pSprite->set_base_angle( RadiansToDegrees(GetRotation()));
	}
	
	//construct the final color with tinting mods, insuring it is in range
	short r,g,b,a;

	r = m_pTile->GetColor().get_red() + m_colorModRed;
	g = m_pTile->GetColor().get_green() + m_colorModGreen;
	b = m_pTile->GetColor().get_blue() + m_colorModBlue;
	a = m_pTile->GetColor().get_alpha() + m_colorModAlpha;

	//force them to be within range
	r = cl_min(r, 255); r = cl_max(0, r);
	g = cl_min(g, 255); g = cl_max(0, g);
	b = cl_min(b, 255); b = cl_max(0, b);
	a = cl_min(a, 255); a = cl_max(0, a);
	
	static CL_Vector2 vecPos;

	if (a > 0)
	{
	
		static bool bUseParallax;
		static Layer *pLayer;
		static Map *pWorld;
		pWorld = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap();
		static EntMapCache *pWorldCache;
	
		pWorldCache = pWorld->GetMyMapCache();

		if (GetGameLogic()->GetParallaxActive())
		{
			bUseParallax = true;
			pLayer = &pWorld->GetLayerManager().GetLayerInfo(m_pTile->GetLayer());
			if (pLayer->RequiresParallax())
			{

			if (GetGameLogic()->GetMakingThumbnail())
			{
				if (!pLayer->GetUseParallaxInThumbnail())
				{
					bUseParallax = false;
				}
			}
			} else
			{
				bUseParallax = false;
			}
		} else
		{
			bUseParallax = false;
		}

		if (bUseParallax)
		{
			vecPos = pWorldCache->WorldToScreen(pWorldCache->WorldToModifiedWorld(vVisualPos, 
				pLayer->GetScrollMod()));
		} else
		{
			vecPos = pWorldCache->WorldToScreen(vVisualPos);
		}

		if (m_bLockedScale && m_attachEntID != 0)
		{
			bool bRootIsCam;
			
			CL_Vector2 vTemp = GetRawScreenPosition(bRootIsCam);
			if (bRootIsCam)
			{
				vecPos = vTemp;
			}
		}

		CL_OpenGLState state(pGC);
		state.set_active();
		clBindTexture(CL_TEXTURE_2D, m_pSprite->get_frame_surface(m_pSprite->get_current_frame()).get_handle());

		// Check out whether the Use Linear Filter flag is set.
		if (GetTile()->GetBit(Tile::e_linearFilter))
		{
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_LINEAR);
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_LINEAR);
		}
		else
		{
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
			clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);
		}

//		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
//		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

		m_pSprite->set_scale(vFinalScale.x, vFinalScale.y);
		//do the real blit
		m_pSprite->set_color(CL_Color(r,g,b,a));

		if (m_blendMode != C_BLEND_MODE_NORMAL)
		{
			CL_BlendFunc src,dest;
			m_pSprite->get_blend_func(src, dest);
			switch (m_blendMode)
			{
			case C_BLEND_MODE_ADDITIVE:

				m_pSprite->set_blend_func(blend_src_alpha, blend_one); //screen/additive
				break;
			case C_BLEND_MODE_NEGATIVE:
				m_pSprite->set_blend_func(blend_src_alpha_saturate, blend_one_minus_src_alpha);  //Negative?
				break;
			default:
				LogError("Unknown blend mode: %d", m_blendMode);
			}

			m_pSprite->draw_subpixel( int(vecPos.x), int(vecPos.y), pGC);
			//m_pSprite->draw_subpixel( vecPos.x, vecPos.y, pGC);
			m_pSprite->set_blend_func(src, dest); //put it back how it was

		} else
		{
			m_pSprite->draw_subpixel( int(vecPos.x), int(vecPos.y), pGC);
			//m_pSprite->draw_subpixel( vecPos.x, vecPos.y, pGC);
		}

		//return things back to normal
		m_pSprite->set_angle_yaw(yawHold);
		m_pSprite->set_angle_pitch(pitchHold);
	
		if (m_effectManager.effect_list.size() > 0)
		{
			std::list<L_ParticleEffect*>::iterator effectItor = m_effectManager.effect_list.begin();

			for (; effectItor != m_effectManager.effect_list.end(); effectItor++)
			{
				(*effectItor)->draw(0,0,vFinalScale.x, vFinalScale.y);

			}
		}
	}

	/*
	//***** debug, draw rect
	CL_Rectf worldRect =GetWorldRect();
	CL_Vector2 vecStart(worldRect.left, worldRect.top);
	CL_Vector2 vecStop(worldRect.right, worldRect.bottom);
	DrawRectFromWorldCoordinates(vecStart, vecStop, CL_Color(255,255,255,180), pGC);
	//*********
	*/

	/*
	if (GetGameLogic()->GetMakingThumbnail())
	{
		return;
	}
	*/

	if (!m_text.empty())
	{
		//draw some text too
		vecPos = GetMap()->GetMyMapCache()->WorldToScreen(vVisualPos);
		
		float textScaleX, textScaleY;
		m_pFont->get_scale(textScaleX, textScaleY);

		CL_Origin originTemp; 
		int offsetX, offsetY;

		m_pFont->get_alignment(originTemp, offsetX, offsetY);

		if (m_bLockedScale)
		{
			m_pFont->set_scale(textScaleX , textScaleY);
			m_pFont->set_alignment(originTemp, offsetX , offsetY);

		} else
		{
			m_pFont->set_scale(textScaleX * GetCamera->GetScale().x, textScaleY *GetCamera->GetScale().y);
			m_pFont->set_alignment(originTemp, offsetX * GetCamera->GetScale().x, offsetY * GetCamera->GetScale().y);
		}

		if (m_attachEntID != 0)
		{
			bool bRootIsCam;

			CL_Vector2 vTemp = GetRawScreenPosition(bRootIsCam);
			if (bRootIsCam)
			{
				vecPos = vTemp;
			}
		} else
		{
			//we're not attached to anything			
			//vecPos = m_attachOffset;
		}

		float oldAlpha = m_pFont->get_alpha();
		a = m_pFont->get_color().get_alpha() + m_colorModAlpha;
		a = cl_min(a, 255); a = cl_max(0, a);

		m_pFont->set_alpha(float(a)/255);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MAG_FILTER, CL_NEAREST);
		clTexParameteri(CL_TEXTURE_2D, CL_TEXTURE_MIN_FILTER, CL_NEAREST);

		m_pFont->draw( vecPos.x, vecPos.y, m_text, pGC);

		//put it back to how it was
		m_pFont->set_scale(textScaleX, textScaleY);
		m_pFont->set_alpha(oldAlpha);
		m_pFont->set_alignment(originTemp, offsetX, offsetY);
	}
	m_brainManager.Render(pTarget);

}


void MovingEntity::SetTrigger(int triggerType, int typeVar, int triggerBehavior, int behaviorVar)
{
	m_trigger.Set(this, triggerType, typeVar, triggerBehavior, behaviorVar);
}

void MovingEntity::DumpScriptInfo()
{
	if (m_pScriptObject)
	{
		LogMsg("Script %s loaded in entity %d (%s).", m_mainScript.c_str(), ID(), GetName().c_str());
		DumpTable(m_pScriptObject->GetState());
	} else
	{
		LogMsg("No script is loaded in ent %d. (%s)", ID(), GetName().c_str());
	}
}

void MovingEntity::RotateTowardsVectorDirection(const CL_Vector2 &vecTargetfloat, float maxTurn)
{
	if (RotateVectorTowardsAnotherVector(m_vecFacing, vecTargetfloat, maxTurn))
	{
		//actual change was made, update
		m_facing = VectorToFacing(m_vecFacing);
	}

}

void MovingEntity::UpdateTriggers(float step)
{
	m_trigger.Update(step);
}

void MovingEntity::SetRunStringASAP(const string &command)
{
	if (m_runScriptASAP.empty())
	{
		m_runScriptASAP = command;
	} else
	{
		LogError("Can't queue up %s, %s is already set to run.", command.c_str(), m_runScriptASAP.c_str());
	}
}

void MovingEntity::ProcessPendingMoveAndDeletionOperations()
{

	if (GetDeleteFlag())
	{
		//remove tile completely
		assert(GetTile());
		GetTile()->GetParentScreen()->RemoveTileByPointer(GetTile());
		return;
	} else
	{
		UpdateTilePosition();
	}

}

void MovingEntity::CheckVisibilityNotifications()
{

	if (GetGameLogic()->GetGamePaused()) return;

	SetIsOnScreen(IsOnScreen());
}

//run first
void MovingEntity::Update(float step)
{
	
	m_contacts.clear();

	if (UnderPriorityLevel()) 
	{
		UpdatePosToPhysics();
		return;
	}

	assert(IsPlaced());

	m_bRanPostUpdate = false;
	m_bRanApplyPhysics = false;
	
	ClearColorMods();

	RunPostInitIfNeeded();

	if (m_bRunUpdateEveryFrame)
	{
		if (!GetScriptObject()->FunctionExists("Update"))
			GetScriptManager->SetStrict(false);
	try {luabind::call_function<void>(m_pScriptObject->GetState(), "Update", step);
		} LUABIND_ENT_CATCH("Error while calling function Update");
		GetScriptManager->SetStrict(true);

	}
	
	UpdateTriggers(step);

	if (m_pGoalManager)
	{
		m_pGoalManager->Process();
		if (!m_runScriptASAP.empty())
		{
			m_pScriptObject->RunString(m_runScriptASAP.c_str());
			m_runScriptASAP.clear();
		}
	}

	m_brainManager.Update(step);

	if (!m_bAnimPaused && m_pSprite->get_current_frame() != -1)
	{
		m_pSprite->update( float(GetApp()->GetGameLogicSpeed()) / 1000.0f );
		if (m_bAnimCallbackActive)
		{
				if (m_pSprite->is_finished() || m_pSprite->is_looping())
				{
					RunFunction("OnAnimLoop");
				}

		}
	}

	if (m_effectManager.effect_list.size() > 0)
		{
			std::list<L_ParticleEffect*>::iterator effectItor = m_effectManager.effect_list.begin();

			for (; effectItor != m_effectManager.effect_list.end(); effectItor++)
			{
				(*effectItor)->set_position(GetPos().x, GetPos().y);
				(*effectItor)->set_velocity(L_Vector(GetLinearVelocity()/4));
			}
		}
	
	m_effectManager.run( (GetApp()->GetGameLogicSpeed()), false);

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->Update(step);
				g_pMapManager->AddToEntityUpdateList(pEnt);
			} else
			{
				itor = m_attachedEntities.erase(itor);
				continue;
			}
			itor++;
		}
	}
}


void MovingEntity::UpdateBodyCollisionMode()
{
	if (m_pBody)
	{
		UpdateBodyFilterData(m_pBody, m_filterData);
	}
}

void MovingEntity::UpdatePosToPhysics()
{
	if (m_pBody)
	{
		//LogMsg("Setting physics %d to %s", ID(), PrintVector(m_pos).c_str());
		m_pBody->SetXForm(ToPhysicsSpace(m_pos), m_angle);
	}

}

void MovingEntity::UpdatePhysicsToPos()
{
	if (m_pBody)
	{
		b2Shape *pShape = m_pBody->GetShapeList();
		if (pShape)
		{
			if (
				pShape->GetFilterData().categoryBits != m_filterData.categoryBits
				||
				pShape->GetFilterData().maskBits != m_filterData.maskBits
				||
				pShape->GetFilterData().groupIndex != m_filterData.groupIndex
				)
			{
				UpdateBodyFilterData(m_pBody, m_filterData);
			}
		}


		if (!m_pBody->IsStatic()) 
		{
			m_angle = m_pBody->GetAngle();
			if ( m_strWorldToMoveTo.empty())
			{
				//LogMsg("Setting local..");
				SetPos(FromPhysicsSpace(m_pBody->GetPosition()));
				//LogMsg("Setting local %d to %s", ID(), PrintVector(m_pos).c_str());

			}


			if (!UnderPriorityLevel())
			{


				if (m_customDampening!= -1)
				{
					AddForceBurst( -(GetLinearVelocity()*m_customDampening));
				} else
				{
					//default dampening
					if (GetMap()->GetGravity() == 0)
					{
						AddForceBurst( -(GetLinearVelocity()*C_DEFAULT_DAMPENING));

					}
				}
				if (m_gravityOverride != C_GRAVITY_OVERRIDE_DISABLED)
				{
					//remove the real gravity
					AddForce(-CL_Vector2(0, GetMap()->GetGravity()));

					//add our own
					AddForce(CL_Vector2(0, m_gravityOverride));
				}
			}


		} else
		{
			//put it to sleep, if it is bumped, it will be woken up and accept collisions
			m_pBody->PutToSleep();
		}
	}

}

//run after physics steps



void MovingEntity::PostUpdate(float step)
{

	if (GetDeleteFlag())
	{
		return;
	}
	if (!m_bHasRunOnInit) return;

	UpdatePhysicsToPos();

	if (UnderPriorityLevel()) return;

	if (m_bRanPostUpdate) return; else m_bRanPostUpdate = true;
	
	if (m_attachEntID != C_ENTITY_NONE)
	{
		if (m_attachEntID != C_ENTITY_CAMERA)
		{
			MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);

			if (pEnt)
			{
				if (!pEnt->HasRunPostUpdate())
				{
					return; //don't update yet
				}
			} else
			{
				assert(!"Huh?  Our parent was deleted and we weren't notified?");
			}
		}
	}

	if (m_attachEntID != C_ENTITY_NONE)
	{
		UpdatePositionFromParent();
	} 
	
	if (m_bRequestsVisibilityNotifications)
	{
			SetIsOnScreen(IsOnScreen());
			g_watchManager.AddEntityToVisibilityList(this);
	}

	HandleContacts();
	
	m_brainManager.PostUpdate(step);

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->PostUpdate(step);
				g_pMapManager->AddToEntityUpdateList(pEnt);

			} else
			{
				itor = m_attachedEntities.erase(itor);
				continue;
			}
			itor++;
		}
	}

	
}


//run right before physics are applied
void MovingEntity::ApplyToPhysics(float step)
{
	if (UnderPriorityLevel()) return;
	if (m_bRanApplyPhysics) return; else m_bRanApplyPhysics = true;

	if (m_attachEntID != C_ENTITY_NONE)
	{
		if (m_attachEntID != C_ENTITY_CAMERA)
		{
			//we are attached to another entity.  It makes sense to not update us until AFTER it updates
			MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);
			if (!pEnt->HasRunPhysics())
			{
				return; //don't update yet
			}
		}
	}
	
	//get ready for next frame, if we plan on one coming..
	if (!m_pBody || !m_pBody->IsSleeping())
	{
		m_bTouchedAGroundThisFrame = false;
	}

	UpdatePosToPhysics();

	m_zoneVec.clear();

	if (m_attachEntID != C_ENTITY_NONE)
	{
		UpdatePositionFromParent();
	} 

	//while we're at it, if any entities are attached *to* us, make sure they "update".  Don't worry, it's smart enough
	//not to allow something to update twice if it's already done it

	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->ApplyToPhysics(step);
				g_pMapManager->AddToEntityUpdateList(pEnt);

			} else
			{
				itor = m_attachedEntities.erase(itor);
				continue;
			}
			itor++;
		}
	}
}

void MovingEntity::AddForce(CL_Vector2 force)
{
	assert(GetApp()->GetDelta() != 0);
	//m_pBody-> ApplyForce(  *(b2Vec2*)& ( m_pBody->GetMass() * ToPhysicsSpace(force)), m_pBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));
	m_pBody-> ApplyForce(  *(b2Vec2*)&  (force*GetMass()) , m_pBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));
}

void MovingEntity::AddForceBurst(CL_Vector2 force)
{
	assert(GetApp()->GetDelta() != 0);
	//m_pBody-> ApplyImpulse(  ToPhysicsSpace(force), m_pBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));
	//the * mass undos the / mass calculation that is done in apply impulse, allowing us to evenly apply force to objects
	//with different masses
	m_pBody-> ApplyImpulse(  *(b2Vec2*)& (force * m_pBody->GetMass()), m_pBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));

}

void MovingEntity::AddForceAndTorque(CL_Vector2 force, CL_Vector2 torque)
{
	LogError("AddForceAndTorque not implemented right now");
	//m_pBody-> ApplyForce( *(b2Vec2*)&force* m_pBody->GetMass(), m_pBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));
}

void MovingEntity::AddForceAndTorqueBurst(CL_Vector2 force, CL_Vector2 torque)
{
	LogError("AddForceAndTorqueBurst not implemented right now");
	//m_pBody-> ApplyForce( *(b2Vec2*)&force* m_pBody->GetMass(), m_pBody->GetWorldPoint(b2Vec2(0.0f, 0.0f)));
}

void MovingEntity::OnWatchListTimeout(bool bIsOnScreen)
{
	if (!bIsOnScreen)
	{
		if (GetVisibilityNotifications())
		{
			//hack so the onlosescreen won't run again
			g_watchManager.RemoveFromVisibilityList(this);
		}

		if (m_pScriptObject->FunctionExists("OnWatchTimeout"))
		{
			try {luabind::call_function<void>(GetScriptObject()->GetState(), "OnWatchTimeout", bIsOnScreen);
			} LUABIND_ENT_CATCH( "Error while calling function OnWatchTimeout(bool bIsOnScreen)");
		}
	}
}

void MovingEntity::SetIsOnScreen(bool bNew)
{
	if (bNew != m_bOnScreen)
	{
		m_bOnScreen = bNew;

		if (m_bOnScreen)
		{
			//LogMsg("Ent %d (%s) has just entered the screen", ID(), GetName().c_str());
			RunFunction("OnGetVisible");
		} else
		{
			RunFunction("OnLoseVisible");
			//LogMsg("Ent %d (%s) has left the screen", ID(), GetName().c_str());
		}
	}
}


void MovingEntity::SetImageFromTilePic(TilePic *pTilePic)
{
	m_dataManager.Set(C_ENT_TILE_PIC_ID_KEY, CL_String::from_int(pTilePic->m_resourceID));
	m_dataManager.Set(C_ENT_TILE_RECT_KEY, RectToString(pTilePic->m_rectSrc));

/*
	m_pCollisionData = pTilePic->GetCollisionData();

	if (m_pCollisionData->GetLineList()->size() == 0)
	{
		//doesn't have any collision data, let's disable physics.  They can always update the density to
		//something higher later.
		m_fDensity = 0;
	} 
*/
	m_dataManager.Set(C_ENT_DENSITY_KEY, CL_String::from_float(m_fDensity));
}

CL_Rectf MovingEntity::GetCollisionRect()
{
	//OPTIMIZE for gods sake, cache this
	if (!m_pTile->GetCollisionData()) return CL_Rectf(0,0,0,0);
	return m_pTile->GetCollisionData()->GetCombinedCollisionRect();
}

CL_Rectf MovingEntity::GetWorldCollisionRect()
{
	//OPTIMIZE for gods sake, cache this
	if (!m_pTile->GetCollisionData()) return CL_Rectf(0,0,0,0);
	return m_pTile->GetWorldColRect();
}

float MovingEntity::GetBoundingCollisionRadius()
{
	
	//OPTIMIZE for gods sake, cache this
	CL_Rectf c = GetCollisionRect();
	return max(c.get_width(), c.get_height());
}
//returns true if this bot can move directly to the given position
//without bumping into any walls
//warning, only tests against edges
bool MovingEntity::CanWalkTo(CL_Vector2 & to, bool ignoreLivingCreatures)
{
	EntMapCache *pWC = m_pTile->GetParentScreen()->GetParentMapChunk()->GetParentMap()->GetMyMapCache();	
	return !pWC->IsPathObstructed(GetPos(), to, GetBoundingCollisionRadius(), m_pTile, ignoreLivingCreatures);
}

//similar to above. Returns true if the bot can move between the two
//given positions without bumping into any walls
//warning, only tests against edges
bool MovingEntity::CanWalkBetween(Map *pMap, CL_Vector2 from, CL_Vector2 to, bool ignoreLivingCreatures)
{
	return !pMap->GetMyMapCache()->IsPathObstructed(from, to, GetBoundingCollisionRadius(), m_pTile, ignoreLivingCreatures);
}

bool MovingEntity::IsValidPosition(Map *pMap, const CL_Vector2 &pos, bool bIgnoreLivingCreatures)
{

	if (!pMap) pMap = GetMap();
	return !pMap->GetMyMapCache()->IsAreaObstructed(pos, GetBoundingCollisionRadius(), bIgnoreLivingCreatures, this);
}


MovingEntity * MovingEntity::Clone(Map *pMap, CL_Vector2 vecPos)
{
	Tile *pNew = m_pTile->CreateClone();
	
	MovingEntity *pNewEnt = ((TileEntity*)pNew)->GetEntity();
	pNewEnt->SetPos(vecPos);
	if (!pMap)
	{
		//use the active map, none was specific
		pMap = g_pMapManager->GetActiveMap();
		if (!pMap)
		{
			pMap = GetMap();
		}
	}
	
	pMap->AddTile(pNew);
	InitEntity(pNewEnt);
	return pNewEnt;
}

void MovingEntity::OnAttachedEntity(int entID)
{
#ifdef _DEBUG
	LogMsg("Entity %d (%s) just had %d attached", ID(), GetName().c_str(), entID);
#endif
	m_attachedEntities.push_back(entID);
}


void MovingEntity::UpdatePositionFromParent()
{
	CL_Vector2 vScaleMod(1,1);
	string mapName;
	CL_Vector2 vPos;

	if (m_bLockedScale)
	{
		vScaleMod = GetCamera->GetScale();
		//LogMsg("Ent %d got cam scale %s.  visualoffset is %s", ID(), PrintVector(vScaleMod).c_str(), PrintVector(GetVisualOffset()).c_str());
	}

	if (m_attachEntID == C_ENTITY_CAMERA)
	{

		vPos = GetCamera->GetPos() + m_attachOffset;

		if (GetMap() != g_pMapManager->GetActiveMap())
		{
			mapName = g_pMapManager->GetActiveMap()->GetName();
		}

	}else

	{
		//we're stuck to a common entity
		MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);
		if (!pEnt)
		{
			//the entity we were attached to no longer exists, so let's just kill ourself
			SetDeleteFlag(true);
		} else
		{

			vPos = pEnt->GetPos() + m_attachOffset;

			if (GetMap() != pEnt->GetMap() || !pEnt->GetRequestedMapName().empty())
			{
				if (!pEnt->GetRequestedMapName().empty())
				{
					mapName = pEnt->GetRequestedMapName();
				} else
				{
					mapName = pEnt->GetMap()->GetName();
				}
				vPos = pEnt->GetRequestPosition() + m_attachOffset;
			} 

		}
	}

	if (!mapName.empty())
	{
		SetPosAndMap(vPos, mapName);
		//LogMsg(" Ent %d changing to map %s at %u.", ID(),  mapName.c_str(), GetApp()->GetGameTick());
	} else
	{
		SetPos(vPos);
	}

}

void MovingEntity::OnMapChange( const string &mapName )
{
	
	if (m_attachEntID == C_ENTITY_NONE)
	{
		//don't care	
		return;
	}

	UpdatePositionFromParent();

	//force children to do the same
	if (!m_attachedEntities.empty())
	{
		list<int>::iterator itor;
		MovingEntity *pEnt;

		for (itor = m_attachedEntities.begin(); itor != m_attachedEntities.end();)
		{
			pEnt = (MovingEntity*) EntityMgr->GetEntityFromID(*itor);
			if (pEnt)
			{
				pEnt->UpdatePositionFromParent();
			} 
			itor++;
		}
	}

}

void MovingEntity::SetAttach(int entityID, CL_Vector2 vOffset)
{
	m_attachEntID = entityID;
	m_attachOffset = vOffset;

	if (entityID == C_ENTITY_NONE)
	{
		return;
	}
	if (entityID == C_ENTITY_CAMERA)
	{
		g_watchManager.Add(this, INT_MAX);
		m_bLockedScale = true;

	} else
	{
		//we're stuck to a common entity
		MovingEntity *pEnt = (MovingEntity*)EntityMgr->GetEntityFromID(m_attachEntID);
		if (!pEnt)
		{
			LogError("Warning: Ent %d (%s) can't attach to entity ID %d, doesn't exist", ID(), GetName().c_str(),
				m_attachEntID);
		} else
		{
			if (!pEnt->GetTile()->GetParentScreen())
			{
				LogMsg("Entity %d (%s) can't attach to Entity %d (%s) because it hasn't been played on a real map yet",
					ID(), GetName().c_str(), pEnt->ID(), pEnt->GetName().c_str());
				return;
			}
			if (GetMap() != pEnt->GetMap())
			{
				SetPosAndMap(pEnt->GetPos()+m_attachOffset, pEnt->GetMap()->GetName());
			} else
			{
				SetPos(pEnt->GetPos()+m_attachOffset);
			}
			
			pEnt->OnAttachedEntity(ID());
		}
	}
}

void MovingEntity::AddContact( ContactPoint &cp)
{
	bool bIgnore = false;
	
	
	//TODO: first, remove duplicates from the same entity.  Speed up/fix up later..
	if (cp.pOwnerEnt)
	for (unsigned int i=0; i < m_contacts.size(); i++)
	{
		if ( m_contacts[i].pOwnerEnt ==  cp.pOwnerEnt)
		{
			bIgnore = true;
			break;
		}
	}
	
	if (!bIgnore)
	{
		m_contacts.push_back(cp);
	}
}


void MovingEntity::HandleContacts()
{
	if (!m_pBody) return;

	for (unsigned int i=0; i < m_contacts.size(); i++)
	{
	
#ifdef _DEBUG
	if (m_contacts[i].pOwnerEnt)
	{
		int id = m_contacts[i].pOwnerEnt->ID(); //get a giant error if this is invalid

	//LogMsg("%d getting message from %d", ID(), m_entContacts[i].pOwnerEnt->ID());

	if (id < 0 || id > 10000)
	{
		assert(0);
	}
	}
#endif
	if (!m_pScriptObject->FunctionExists("OnCollision"))
			GetScriptManager->SetStrict(false);

	try {	luabind::call_function<void>(m_pScriptObject->GetState(), "OnCollision", FromPhysicsSpace(m_contacts[i].position),
		*(CL_Vector2*)&m_contacts[i].velocity, 
		CL_Vector2(m_contacts[i].normal.x, m_contacts[i].normal.y), m_contacts[i].separation, m_contacts[i].materialID, m_contacts[i].pOwnerEnt,
		m_contacts[i].state);
		} LUABIND_ENT_CATCH("Error while calling OnCollision(Vector2, Vector2, Vector2, number, number , Entity, number)");
		GetScriptManager->SetStrict(true);
		//LogMsg("End %d", ID());
	
	}

	if (m_bTouchedAGroundThisFrame)
	{
		m_ticksOnGround++;

		if (m_ticksOnGround >= C_TICKS_ON_GROUND_NEEDED_TO_TRIGGER_GROUND)
		{
			//LogMsg("%s turned on ground true", GetName().c_str());
			SetIsOnGround(true);
		}
	} else
	{
		m_ticksOnGround = 0;
	}

}

void MovingEntity::SetMass( float mass )
{
	if (!m_pBody)
	{
		LogMsg("SetMass: Entity %d (%s) has no collision properties yet, ignoring", ID(), GetName().c_str());
		return;
	}
	LogMsg("Ignoring SetMass() command in Entity %d (%s) - use SetDensity() instead for now", ID(), GetName().c_str());

	//m_pBody->GetMass()
}

void MovingEntity::SetVisibilityNotifications( bool bNew )
{
	if (m_bRequestsVisibilityNotifications != bNew && bNew)
	{
		//set the default state...?  default is off
		//m_bOnScreen = IsOnScreen();
	}
	m_bRequestsVisibilityNotifications = bNew;
}

void MovingEntity::SetCollisionCategory( int category, bool bOn )
{
	SetCollisionCategoryBit(m_filterData, category, bOn);
}

void MovingEntity::SetCollisionMask( int category, bool bOn )
{
	SetCollisionMaskBit(m_filterData, category, bOn);

}

int MovingEntity::GetCollisionGroup()
{
	return m_filterData.groupIndex;
}

void MovingEntity::SetCollisionGroup( int group )
{
	m_filterData.groupIndex = group;
}

void MovingEntity::SetCategories( uint16 mask )
{
	m_filterData.categoryBits = mask;
}

void MovingEntity::SetCollisionListenCategory( int category, bool bOn )
{
	assert(category >= 0 && category < 16);
	if (bOn)
	{
		//set it
		m_collisionListenBits |= SetBitSimple(category);

	} else
	{
		//clear it
		m_collisionListenBits &= ~SetBitSimple(category);
	}
}

void MovingEntity::SetCollisionCategories( uint16 mask )
{
		m_filterData.maskBits = mask;
}

void MovingEntity::SetCollisionListenCategories( uint16 mask )
{
	m_collisionListenBits = mask;
}

void MovingEntity::SetSizeOverride( bool bActive, CL_Vector2 v )
{
	m_bSizeOverride = bActive;
	m_ptSizeOverride.x = v.x;
	m_ptSizeOverride.y = v.y;
}