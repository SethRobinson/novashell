//don't change anything in this file for a mod, it might be modified later.  Better to add/override things in another file that is including after this one.

//Register some global enums

//these are used with GetApp():GetPlatform()
C_PLATFORM_WINDOWS = 0;
C_PLATFORM_OSX = 1;
C_PLATFORM_LINUX = 2;

//These are used with Tile:GetType()
C_TILE_TYPE_BLANK = 0;
C_TILE_TYPE_PIC = 1;
C_TILE_TYPE_ENTITY = 3;

C_RAY_ENTITIES = 0;
C_RAY_TILE_PIC = 1;
C_RAY_EVERYTHING = 2;
C_RAY_DEBUG = 3; //use this and it draws lines showing where the rays will go, without hitting anything, helpful for debugging


C_ENTITY_NONE = 0;
C_ENTITY_CAMERA = -1;

C_FONT_SMALL = 0;
C_FONT_DEFAULT = 1;

C_FACING_NONE = -1;
C_FACING_LEFT = 0;
C_FACING_UP_LEFT = 1;
C_FACING_UP = 2;
C_FACING_UP_RIGHT = 3;
C_FACING_RIGHT = 4;
C_FACING_DOWN_RIGHT = 5;
C_FACING_DOWN = 6;
C_FACING_DOWN_LEFT = 7;

C_FACING_COUNT = 8; //how many directions we can use for sprite display (must be 8)

C_ANIM_FRAME_LAST = -1 //means the last frame of the anim, wherever that is

//things for alignment
C_ORIGIN_TOP_LEFT = 0;
C_ORIGIN_TOP_CENTER = 1;
C_ORIGIN_TOP_RIGHT = 2;
C_ORIGIN_CENTER_LEFT = 3;
C_ORIGIN_CENTER = 4;
C_ORIGIN_CENTER_RIGHT = 5;
C_ORIGIN_BOTTOM_LEFT = 6;
C_ORIGIN_BOTTOM_CENTER = 7;
C_ORIGIN_BOTTOM_RIGHT = 8;

//these virtual states have special meanings, to allow AI code to activate correct animations if available

C_VISUAL_STATE_NONE = -1;
C_VISUAL_STATE_IDLE = 0;
C_VISUAL_STATE_WALK = 1;
C_VISUAL_STATE_RUN = 2;
C_VISUAL_STATE_PAIN = 3;
C_VISUAL_STATE_DIE = 4;
C_VISUAL_STATE_ATTACK1 = 5;
C_VISUAL_STATE_ATTACK2 = 6;
C_VISUAL_STATE_SPECIAL1 = 7;
C_VISUAL_STATE_TURN = 8;
C_VISUAL_STATE_JUMP = 9;

C_TRIGGER_TYPE_NONE = 0;
C_TRIGGER_TYPE_REGION_IMAGE = 1;

C_TRIGGER_BEHAVIOR_NORMAL = 0;
C_TRIGGER_BEHAVIOR_PULSE = 1;

//special flags used with navigation and warps
C_NODE_TYPE_NORMAL = 0
C_NODE_TYPE_WARP = 1

//globals
//g_playerID (set by system, 0 if no player active)
g_playerID = 0;
GetPlayer = nil;
//GetPlayer (set by system, will be nil if no player is active)
//g_isDebug (true if the -debug parm was passed into the .exe)
//g_musicEnabled (true unless the -nomusic parm was passed into the .exe)

//the engine will set these...
g_editorActive = false;
g_consoleActive = false;

//C_SCREEN_X, C_SCREEN_Y (set by system)

//DON'T CHANGE THESE #'s, they are matched to the main engine

//collision masks, we can have up to 16 (0-15) but I define some names to make script sharing easier

//the engine will automatically set each entity/tilepic created to one of these categories:
C_CATEGORY_ENTITY = 0; //everything is either a tilepic (simple) or an entity (complex)
C_CATEGORY_TILEPIC = 1; //tilepic category, or if you prefer, static
//optional categories for your own use, keep in mind something can be a member of more than one category at once
C_CATEGORY_PLAYER = 2;
C_CATEGORY_DAMAGE = 3;
C_CATEGORY_PLATFORM = 4;
C_CATEGORY_ENEMY = 5;
C_CATEGORY_CHARACTER = 6;
C_CATEGORY_BULLET = 7;
C_CATEGORY_POWER_UP = 8;

//for setting the whole mask at once, only to be used with *Categories() functions.

C_CATEGORIES_NONE = 0; //no bits set
C_CATEGORIES_ALL = 65535; //all 16 bits set

//each materialID has a member called 'type':
C_MATERIAL_TYPE_NONE = -1
C_MATERIAL_TYPE_NORMAL = 0 //solid, all the rest will not collide with anything
C_MATERIAL_TYPE_VERTICAL_LADDER = 1 //will not collide
C_MATERIAL_TYPE_WARP = 2 //will not collide
C_MATERIAL_TYPE_DUMMY = 3 //special case, will not collide, but does affect starting point of shadows/depth sorting

//Don't change the order of these, the editor references them by index.  But you can add your own, but better
//not to do it from this file, as it will be changed.


//Actually create our material ID's

//Addmaterial (friction, restitution, unused, unused, color, name in editor)

C_MATERIAL_NONE = -1
C_MATERIAL_DEFAULT =     GetMaterialManager:AddMaterial(0.3,0.2, 0, 0, Color(200,200,100,200), "Default");
C_MATERIAL_NO_BOUNCE = 	 GetMaterialManager:AddMaterial(0.3, 0.0, 0, 0, Color(100,255,200,255), "No Bounce");
C_MATERIAL_BOUNCY =      GetMaterialManager:AddMaterial(0.5, 0.5, 0, 0, Color(255,100,200,255), "Bouncy");
C_MATERIAL_SLIPPERY =    GetMaterialManager:AddMaterial(0.01, 0.2, 0, 0, Color(200,200,200,255), "Ice");

C_MATERIAL_VERTICAL_LADDER =    GetMaterialManager:AddMaterial(0.0, 0.0, 0.0, 0.0, Color(0,255,0,255), "V Ladder");
GetMaterialManager:GetMaterial(C_MATERIAL_VERTICAL_LADDER):SetType(C_MATERIAL_TYPE_VERTICAL_LADDER);

C_MATERIAL_WARP =    GetMaterialManager:AddMaterial(0.0, 0.0, 0.0, 0.0, Color(255,255,0,255), "Warp");
GetMaterialManager:GetMaterial(C_MATERIAL_WARP):SetType(C_MATERIAL_TYPE_WARP);

C_MATERIAL_DUMMY =    GetMaterialManager:AddMaterial(0.0, 0.0, 0.0, 0.0, Color(255,200,50,255), "Dummy");
GetMaterialManager:GetMaterial(C_MATERIAL_DUMMY):SetType(C_MATERIAL_TYPE_DUMMY);

C_MATERIAL_NO_BOUNCE_SLIPPERY = GetMaterialManager:AddMaterial(0.01, 0.0, 0, 0, Color(100,100,255,255), "Ice no Bounce");

C_MATERIAL_HIGH_FRICTION = GetMaterialManager:AddMaterial(0.9, 0.2, 0, 0, Color(255,50,50,255), "High Friction");

//misc definitions to make scripts more readable

C_DISTANCE_TALK = 40 //how close someone should be for talking
C_DISTANCE_YELL = 80 //how close to be if you want to yell something to someone
C_DISTANCE_CLOSE = 0 //how close "close" is
C_DISTANCE_NOT_ON_SAME_MAP = 2000000000 //GetDistanceToEntityByID() can return this
C_DISTANCE_INSIDE = -1; //means go "inside" the AddApproach target, good for monsters
//that need to cause touch damage

C_TIME_FOREVER = 2000000000; //not really forever, but close enough
C_SOUND_NONE = -1;

//ID's for sound effects
C_EFFECT_FADE = 0;

C_GRAVITY_OVERRIDE_DISABLED = 9999999; //reserved to signal "no gravity override"

C_LAYER_NONE = -1;
C_LAYER_TOP = 1000;

//Used with DrawManager
C_DRAW_SCREEN_COORDS = 0;
C_DRAW_WORLD_COORDS = 1;

//these are used with Entity:SetBlendMode()
C_BLEND_MODE_NORMAL = 0;
C_BLEND_MODE_ADDITIVE = 1; //similar to photoshop's "Screen"
C_BLEND_MODE_NEGATIVE = 2; //uh.. just draws a black silhouette

g_allowStrict = true;

//for particles/effects
C_PROBABILITY_DEFAULT = -1; //used with ParticleEffect:AddParticle()

//For use with Entity:GetType/EntitySetType, only used on the scripting side

C_TYPE_UNKNOWN = 0;
C_TYPE_CHARACTER = 1;
C_TYPE_DAMAGE = 2;
C_TYPE_PLATFORM = 3;
C_TYPE_ENEMY = 4;

//If you want to add more types on your own, start above 100 to be safe, in case I add more here.

//for use with Entity:SetPriorityLevel().  If you set to C_PRIORITY_LEVEL_HIGH, it will function while 'paused'.
C_PRIORITY_LEVEL_NORMAL = 0;
C_PRIORITY_LEVEL_HIGH = 100;