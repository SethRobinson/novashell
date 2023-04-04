#ifndef MOVING_ENTITY
#define MOVING_ENTITY

#include "BaseGameEntity.h"
#include "CollisionData.h"
#include "Screen.h"
#include "GameLogic.h"
#include "TileEntity.h"
#include "DataManager.h"
#include "BrainManager.h"
#include "Trigger.h"
#include "Material.h"
#include "linearparticle/sources/L_EffectManager.h"
class Goal_Think;
class PathPlanner;
class b2Body;
//#include "AI/Goal_Think.h"

#define C_DEFAULT_SCRIPT_PATH "script/"

#define C_MAX_FALLING_DOWN_SPEED 20 //gravity won't be applied to objects going faster than this
#define C_ENT_TILE_PIC_ID_KEY "_TilePicID" //custom data to signal we use a simple single tilepic for our image
#define C_ENT_TILE_RECT_KEY "_TileRect" //custom data that goes along with the tile ID
#define C_ENT_DENSITY_KEY "_Density" //custom data to override density


#define C_ENT_ROTATE_KEY "_CanRotate" //custom data to override density

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif


enum
{
	C_BLEND_MODE_NORMAL,
	C_BLEND_MODE_ADDITIVE,
	C_BLEND_MODE_NEGATIVE

};

class Brain;
class VisualProfile;


//this is attached to every 2dbox shape and deleted with its shape is
class ShapeUserData
{
public:
	MovingEntity *pOwnerEnt;
	Tile *pOwnerTile;
	int materialID;

};

class MovingEntity : public BaseGameEntity
{
public:

  void InitCollisionDataBySize(float x, float y);
  MovingEntity();
  virtual ~MovingEntity();

  void Kill();
  bool Init();
  virtual void SetName(const std::string &name);
  void SetNameEx(const std::string &name, bool bRemoveOldTag);

  CL_Vector2 GetPos() {return m_pos;}
  CL_Vector2 GetPosSafe() {return m_pos;}
  void  SetPos(const CL_Vector2 &new_pos);
  void SetPosAndMap(const CL_Vector2 &new_pos, const string &worldName);
  int GetSizeX(){ if (m_bSizeOverride) return m_ptSizeOverride.x; if (m_pSprite->get_current_frame() == -1) return 0; return int(m_pSprite->get_width() * m_pTile->GetScale().x); };
  int GetSizeY(){ if (m_bSizeOverride) return m_ptSizeOverride.y; if (m_pSprite->get_current_frame() == -1) return 0; return int(m_pSprite->get_height() * m_pTile->GetScale().y); };
  CL_Vector2 GetScale() {return m_pTile->GetScale();}
  void SetScale(const CL_Vector2 &vScale);
  CL_Vector2 GetCollisionScale();
  CL_Rectf GetCollisionRect();
  CL_Rectf GetWorldCollisionRect();
  virtual CL_Rectf GetWorldRect();
  const CL_Rect & GetBoundsRect();
  CL_Rectf GetBoundsRectf();
  void SetSizeOverride( bool bActive, CL_Vector2 v);
  bool GetSizeOverride() {return m_bSizeOverride;}

  PhysicsManager * GetPhysicsManager() {return GetMap()->GetPhysicsManager();}

  float GetBoundingCollisionRadius();
  
  bool IsValidPosition(Map *pMap, const CL_Vector2 &pos, bool bIgnoreLivingCreatures);
  void Stop();
  void StopY();
  void StopX();

  void SetCollisionScale(const CL_Vector2 &vScale);
  bool IsAtRest() {return m_bIsAtRest;}
  bool IsOnGround() {return m_groundTimer >= GetApp()->GetGameTick();}
  bool IsOnGroundAccurate() {return m_bTouchedAGroundThisFrame;}
  luabind::object RunFunction(const string &func);
  luabind::object RunFunction(const string &func, luabind::object obj1);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3);

  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7);
  luabind::object RunFunction(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7, luabind::object obj8);

  luabind::object RunFunctionIfExists(const string &func);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3);

  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7);
  luabind::object RunFunctionIfExists(const string &func, luabind::object obj1, luabind::object obj2, luabind::object obj3, luabind::object obj4, luabind::object obj5, luabind::object obj6, luabind::object obj7, luabind::object obj8);


  bool ScriptNotReady(const string &func, bool bShowError = true);

  void GetAlignment(CL_Origin &origin, int &x, int &y);
  void SetAlignment(int origin, CL_Vector2 v);
  void UpdateTilePosition();
  CollisionData * GetCollisionData() {return m_pCollisionData;}
  bool SetVisualProfile(string resourceFileName, const string &profileName);
  virtual void  Update(float step);
  virtual void  Render(void *pTarget);
  void ApplyToPhysics(float step);
  void RenderShadow(void *pTarget);
  BaseGameEntity * CreateClone(TileEntity *pTile);
  void SetIsOnGround(bool bOnGround);
  b2Body * GetBody(){return m_pBody;}
  b2Body * m_pBody;
  void SetSpriteData(CL_Sprite *pSprite);
 // void SetImageClipRect(const CL_Rect &r);
  CL_Rect GetImageClipRect();
  unsigned int GetImageID(); //returns 0 if not applicable
  bool SetImageByID(unsigned int picID, CL_Rect *pSrcRect);
  void SetImage(const string &fileName, CL_Rect *pSrcRect);
  void AddImageToMapCache(const string &fileName);


  bool LoadScript(const char *pFileName);
  void SetMainScriptFileName(const string &fileName);
  const string & GetMainScriptFileName() {return m_mainScript;}
  int GetNavNodeType(){return m_navNodeType;}
  void SetNavNodeType(int n);
  void SetHasPathNode(bool bHasNode);

  VisualProfile * GetVisualProfile() {return m_pVisualProfile;}
  void Serialize(CL_FileHelper &helper);
  void LoadCollisionInfo(const string &fileName);
  void SetCollisionInfoFromPic(unsigned int picID, const CL_Rect &recPic);
  void SetAnimByName(const string &name);
  string GetAnimName();
	void EnableRotation(bool bRotate);
  bool GetEnableRotation();
  void SetRotation(float angle);
  float GetRotation();
  unsigned int GetDrawID() {return m_drawID;}
  float GetMass() {return m_pBody->GetMass();}
	bool IsOnScreen();
  void SetMass(float mass);
  void SetDensity(float fDensity);
  void PostUpdate(float step);
  void SetAnimFrame(int frame);
  int GetAnimFrame();
  void AddForce(CL_Vector2 force);
  void AddForceBurst(CL_Vector2 force);
  void SetCollisionCategory(int category, bool bOn);
  void SetCollisionMask(int category, bool bOn);
  int GetCollisionGroup();
  void SetCollisionGroup(int group);

  void AddForceAndTorque(CL_Vector2 force, CL_Vector2 torque);
  void AddForceAndTorqueBurst(CL_Vector2 force, CL_Vector2 torque);

  CL_Vector2 GetForce() {

	  return  *(CL_Vector2*)&m_pBody->GetLinearVelocity();
	 //   return  FromPhysicsSpace(m_pBody->GetLinearVelocity());
  
  };
  CL_Vector2 GetLinearVelocity() 
  {
	  if (!m_pBody) return CL_Vector2::ZERO;
	  return  *(CL_Vector2*)&m_pBody->GetLinearVelocity();
  // return  FromPhysicsSpace(m_pBody->GetLinearVelocity());
  }
  void SetPersistent(bool bOn){assert(m_pTile); m_pTile->SetBit(Tile::e_notPersistent, !bOn);}
  bool GetPersistent() {assert(m_pTile); return !m_pTile->GetBit(Tile::e_notPersistent);}
  float GetDistanceFromEntityByID(int id);
  float GetDistanceFromPosition(const CL_Vector2 &pos);
  MovingEntity * CreateEntity(Map *pMap, CL_Vector2 pos, const string &script);

  int GetLayerID();
  void SetLayerID(int id);
  void SetLayerByName(const string &name);

  
  ScriptObject * GetScriptObject() {return m_pScriptObject;}
  Zone GetNearbyZoneByPointAndType(const CL_Vector2 &vPos, int matType);
  Zone GetNearbyZoneByCollisionRectAndType(int matType);

  bool InZoneByMaterialType(int matType) {return GetNearbyZoneByCollisionRectAndType(matType).m_materialID != CMaterial::C_MATERIAL_TYPE_NONE;}
  bool InNearbyZoneByMaterialType(const CL_Vector2 &vPos, int matType) {return GetNearbyZoneByPointAndType(vPos, matType).m_materialID != CMaterial::C_MATERIAL_TYPE_NONE;}
  CL_Sprite * GetSprite(){return m_pSprite;}
  int GetAnimID() {return m_animID;}
  void SetAnimPause(bool bPause) { m_bAnimPaused = bPause;}
  bool GetAnimPause() {return m_bAnimPaused;}
  
  virtual string HandleMessageString(const string &msg);
  virtual bool HandleMessage(const Message &msg);

  void SetDefaultTextColor(CL_Color color) {m_defaultTextColor = color;}
  CL_Color GetDefaultTextColor() {return m_defaultTextColor;}
  DataManager * GetData() {return &m_dataManager;}
  unsigned int GetNameHash() {return m_hashedName;}
  bool SetPosAndMapByTagName(const string &name);
  void SetImageFromTilePic(TilePic *pTilePic);
  bool GetUsingImageFromTilePic() {return m_bUsingSimpleSprite;}
  bool UsingCustomCollisionData() {return m_bUsingCustomCollisionData;}
  int GetFloorMaterialID() {return m_floorMaterialID;}
  BrainManager * GetBrainManager() {return (BrainManager*)&m_brainManager;}
  string ProcessPath(const string &st); //replaces ~ with current script path
  string ProcessPathNoScript(const string &st); //doesn't add the "script" part
  void SetFacing(int facing);
  void SetFacingTarget(int facing);
  int GetFacing(){return m_facing;}
  void SetVectorFacing(const CL_Vector2 &v);
  void SetVectorFacingTarget(const CL_Vector2 &v);
  int GetFacingTarget();

  CL_Vector2 GetVectorFacing();
  CL_Vector2 GetVectorFacingTarget();
 
  unsigned int CalculateTimeToReachPosition(const CL_Vector2 &pos);

  int GetVisualState();
  void SetVisualState(int visualState);
  void SetVisualStateOverride(int visualState);
  int GetVisualStateOverride();


  void SetSpriteByVisualStateAndFacing();
  void LastCollisionWasInvalidated();

  void AddColorModRed(short mod) {m_colorModRed += mod;}
  void AddColorModGreen(short mod) {m_colorModGreen += mod;}
  void AddColorModBlue(short mod) {m_colorModBlue += mod;}
  void AddColorModAlpha(short mod) {m_colorModAlpha += mod;}

  void SetBaseColor(const CL_Color &col){m_pTile->SetColor(col);}
  CL_Color GetBaseColor() {return m_pTile->GetColor();}
  void SetTrigger(int triggerType, int typeVar, int triggerBehavior, int behaviorVar);
  void ClearColorMods();
  void DumpScriptInfo();
  CL_Vector2 GetVisualOffset();
  void RotateTowardsVectorDirection(const CL_Vector2 &vecTargetfloat, float maxTurn);
  void UpdateTriggers(float step);
  Goal_Think * GetGoalManager();
  bool IsGoalManagerActive() {return m_pGoalManager != NULL;}
  //normally you'd never use this, but during load I need this to disable the update...
  void SetMovedFlag(bool bNeedToUpdatePosition) {m_bMovedFlag = bNeedToUpdatePosition;}
  void ForceSpriteUpdate() {m_pSpriteLastUsed = NULL;}
  float GetDesiredSpeed() {return m_desiredSpeed;}
  void SetDesiredSpeed(float s) {m_desiredSpeed = s;}

  float GetMaxWalkSpeed() {return m_maxWalkSpeed;}
  void SetMaxWalkSpeed(float s) {m_maxWalkSpeed = s;}
  bool IsPlaced() {return m_pTile->GetParentScreen() != 0;}
  bool IsInitted() {return m_bHasRunOnInit;}
  bool IsFacingTarget(float tolerance); 

  bool IsCloseToEntityByID(int entID, int dist);
  bool IsCloseToEntity(MovingEntity *pEnt, int dist);

  bool CanWalkTo(CL_Vector2 &pos, bool ignoreLivingCreatures);
  bool CanWalkBetween(Map *pMap, CL_Vector2 from, CL_Vector2 to, bool ignoreLivingCreatures);

  CL_Vector2 GetVectorToEntityByID(int entID);
  CL_Vector2 GetVectorToEntity(MovingEntity *pEnt);
  CL_Vector2 GetVectorToPosition(const CL_Vector2 &pos);

  bool IsOnSameMapAsEntityByID(int entID);

  PathPlanner * GetPathPlanner();
  void SetRunStringASAP(const string &command);
  void ProcessPendingMoveAndDeletionOperations();
  Map * GetMap(); //the map we are currently on
  const string & GetRequestedMapName(); //current map name OR the one we're scheduled to move to ASAP
  void RunPostInitIfNeeded();
  void RunOnMapInsertIfNeeded();
  void CheckVisibilityNotifications();
  bool GetVisibilityNotifications() {return m_bRequestsVisibilityNotifications;}
  void SetVisibilityNotifications(bool bNew);
  void OnWatchListTimeout(bool bIsOnScreen);
  int PlaySoundPositioned(const string &fName);
  int PlaySound(const string &fName);
  void SetFont(const string &resource_id);
  void SetFont(int font_id);
  void SetText(const string &text);
  string GetText() {return m_text;}
  CL_Vector2 GetTextBounds();
 
  void SetTextColor(CL_Color color);
  CL_Color GetTextColor();
  void SetTextAlignment(int alignment, CL_Vector2 v);

  void SetTextScale(const CL_Vector2 &vecScale);
  CL_Vector2 GetTextScale();
  void SetTextRect(const CL_Rect &r);
  void SetAttach(int entityID, CL_Vector2 vOffset);
  int GetAttachEntityID() {return m_attachEntID;}
  CL_Vector2 GetAttachOffset() {return m_attachOffset;}
  void SetAttachOffset(const CL_Vector2 &pos) {m_attachOffset = pos;}
  MovingEntity * Clone(Map *pMap, CL_Vector2 vecPos);
  void OnAttachedEntity(int entID);
  void SetRunUpdateEveryFrame(bool bNew) {m_bRunUpdateEveryFrame = bNew;}
  bool GetRunUpdateEveryFrame() {return m_bRunUpdateEveryFrame;}
  void SetDampening(float f) {m_customDampening = f;}
  bool FunctionExists(const char * pFunctionName);
  bool VariableExists(const char * pVarName);

  void SetTurnSpeed(float fNew) {m_turnSpeed = fNew;}
  float GetTurnSpeed() {return m_turnSpeed;}
  void SetGravityOverride(float g) {m_gravityOverride = g;}
  float GetGravityOverride() {return m_gravityOverride;}
  void SetBlendMode(int blendMode) {m_blendMode = blendMode;}
  int GetBlendMode() {return m_blendMode;}
  bool GetIsCreature() {return m_bIsCreature;}
  void SetIsCreature(bool bIsCreature) {m_bIsCreature = bIsCreature;}
  void SetAccel(float fNew) {m_accel = fNew;}
  float GetAccel() {return m_accel;}
  void AddEffect(L_ParticleEffect *pEffect);
  void SetGarbageCollectionOnKill(bool bGarbageCollectOnKill) {m_pScriptObject->SetGarbageCollectOnKill(bGarbageCollectOnKill);}
  void SetLockedScale(bool b) {m_bLockedScale = b;}
  bool GetLockedScale() {return m_bLockedScale;}
  unsigned int GetLastVisibilityID() {return m_lastVisibilityNotificationID;}
  bool HasRunPhysics(){return m_bRanApplyPhysics;}
  bool HasRunPostUpdate(){return m_bRanPostUpdate;}
  void OnMapChange(const string &mapName);
  CL_Vector2 GetRequestPosition() {return m_moveToAtEndOfFrame;}
  void UpdateSoundByPosition(int soundID, float minHearing, float maxHearing, float volMod);
  bool GetApproachPosition(MovingEntity *pEnt, int distance, CL_Vector2 &pOutputPos);
  void SetAnimLoopCallback(bool bNew) {m_bAnimCallbackActive = bNew;}
  bool GetAnimLoopCallback() {return m_bAnimCallbackActive;}
  void AddContact(ContactPoint &cp);
  void SetCollisionListenCategory(int category, bool bOn);
  uint16 GetCollisionListenBits() {return m_collisionListenBits;}
  void SetCollisionCategories(uint16 mask);
  void SetCollisionListenCategories( uint16 mask );
  void SetCategories( uint16 mask );
  enum ListenCollision
{
	//I know I don't have to specify the #'s but it helps me visually keep
	//them syncronized with the lua defines that have to match. So don't
	//be changin' the order ya hear?
	
	LISTEN_COLLISION_NONE = 0,
	LISTEN_COLLISION_PLAYER_ONLY = 1,
	LISTEN_COLLISION_ALL_ENTITIES = 2,

	//add more above this
	LISTEN_COLLISION_COUNT
};

enum ListenStaticCollision
{
	//I know I don't have to specify the #'s but it helps me visually keep
	//them syncronized with the lua defines that have to match. So don't
	//be changin' the order ya hear?

	LISTEN_COLLISION_STATIC_NONE = 0,
	LISTEN_COLLISION_STATIC_ALL = 1,

	//add more above this
	LISTEN_COLLISION_STATIC_COUNT
};

protected:

	void SetDefaults();
	void RotateTowardsFacingTarget(float step);
	void SetIsOnScreen(bool bNew);
	void ResetEffects();
	void UpdatePositionFromParent();
	CL_Vector2 GetRawScreenPosition(bool &bRootIsCam);
	void ForceUpdatingPeriod(); //force entity to update for a few frames, no matter where it is
	void InitializeBody();
	void KillBody();
	void HandleContacts();
	void UpdatePhysicsToPos();
	void UpdateBodyCollisionMode();
	void UpdatePosToPhysics();

	CL_Rectf m_scanArea;
	bool m_bIsAtRest;
	bool m_bMovedFlag; //if true, we need to update our tile position
	unsigned int m_groundTimer; //relax when we say they are on the  ground or not to take care of tiny bounces
	CollisionData * m_pCollisionData; //null if we don't collide
	ScriptObject *m_pScriptObject;
	CL_Sprite *m_pSprite, *m_pSpriteLastUsed;
	VisualProfile *m_pVisualProfile;
	string m_mainScript;
	unsigned int m_timeLastActive;
	float m_fDensity; //not in physics space, needs to be converted before sending to a Body
	int m_ticksOnGround; 
	bool m_bTouchedAGroundThisFrame;
	unsigned int m_drawID; //if this matches GetWorldCache->GetUniqueDrawID() it means they have
	//already "thought" this cycle
	vector<Zone> m_zoneVec;
	bool m_bOldTouchedAGroundThisFrame;
	int m_oldFloorMaterialID;
	bool m_bAnimPaused;
	CL_Color m_defaultTextColor;
	string m_strWorldToMoveTo; //if empty, we don't have a world move scheduled
	DataManager m_dataManager;
	unsigned int m_hashedName;
	bool m_bUsingSimpleSprite; //no animation, just a still taken from a tile pic
	bool m_bUsingCustomCollisionData; //if true, we need to delete it
	int m_floorMaterialID;
	BrainManager m_brainManager;
	int m_facing;  //may or may not be used
	int m_visualState; //may or may not be used
	CL_Vector2 m_moveToAtEndOfFrame;
	
	short m_colorModRed;
	short m_colorModGreen;
	short m_colorModBlue;
	short m_colorModAlpha;

	CL_Vector2 m_vecFacing;
	CL_Vector2 m_vecFacingTarget;

	Trigger m_trigger;
	bool m_bRestartAnim;
	int m_animID;

	Goal_Think *m_pGoalManager;
	float m_desiredSpeed;
	float m_maxWalkSpeed;
	PathPlanner * m_pPathPlanner;
	string m_runScriptASAP; //empty if not being used, only used for the RunScript Goal
	int m_navNodeType; //only used if this entity has a pathnode, allows special markings for the graph, like doors or warps

	bool m_bHasRunOnInit; //run before inserted in map
	bool m_bHasRunPostInit; //run during map insertion, for handling warps mostly
	bool m_bHasRunOnMapInsert; //run in the entities think phase, never during a large map load

	unsigned int m_lastVisibilityNotificationID;
	bool m_bRequestsVisibilityNotifications;
	bool m_bOnScreen;
	bool m_bCanRotate;
	int m_requestNewLayerID; //-1 if none requested

	//stuff that may or may not be used for text in this entity
	string m_text; //simple way to show text in  the game
	CL_Font * m_pFont;
	CL_Rect m_textRect;

	int m_attachEntID;
	CL_Vector2 m_attachOffset;
	bool m_bLockedScale; //if true, we don't scale with the camera
	list<int> m_attachedEntities; 
	bool m_bRunUpdateEveryFrame;
	float m_customDampening;
	float m_turnSpeed;
	bool m_bRanPostUpdate;
	bool m_bRanApplyPhysics;
	float m_gravityOverride;
	int m_blendMode;
	bool m_bIsCreature;
	float m_accel; //how fast we can accelerate
	bool m_bUsingTilePicCollision;
	L_EffectManager m_effectManager;
	int m_visualStateOverride; 
	bool m_bAnimCallbackActive;
	CL_Vector2 m_pos;
	float m_angle;
	vector<ContactPoint> m_contacts;
	b2FilterData m_filterData; //shared between all shapes
	uint16 m_collisionListenBits;
	bool m_bSetAnimByName; //when an ent is initted, we'll only do SetSpriteByStateAndDir or whatever it is if the script didn't do a SetAnimByName
	bool m_bSizeOverride; //if true, we are forcing how big the engine things we are
	CL_Point m_ptSizeOverride;
	
};

MovingEntity * CreateEntity(Map *pMap, CL_Vector2 vecPos, string scriptFileName);

void AddShadowToParam1(CL_Surface_DrawParams1 &params1, Tile *pTile);

extern Zone g_ZoneEmpty; //to save time this is preinitted

#endif

/*
Object: Zone
A simple object that contains information about a collision.

Group: Member Variables

Variable: materialID
One of the <C_MATERIAL_TYPE_CONSTANTS> describing what the collision was with, or <C_MATERIAL_TYPE_NONE> if no collision was made.

Variable: entityID
If an <Entity> was hit, this his ID, otherwise will be <C_ENTITY_NONE>.

Variable: vPos
When using <Map::GetCollisionByRay> it is the exact point of collision, in other cases it may be the <Entity>/<TilePic>'s position.

Variable: boundingRect
Usually is the local rectangle of the collision box of the thing we hit.

*/