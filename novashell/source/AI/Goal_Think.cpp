#include "AppPrecomp.h"
#include "Goal_Think.h"
#include "ObjectEnumerations.h"
#include "Goal_MoveToPosition.h"
#include "Goal_Types.h"
//#include "Goal_AttackTarget.h"
//#include "AttackTargetGoal_Evaluator.h"
#include "Goal_Delay.h"
#include "Goal_RunScript.h"
#include "Goal_MoveToMapAndPos.h"
#include "Goal_Say.h"
#include "Goal_Approach.h"

Goal_Think::Goal_Think(MovingEntity* pBot, const string &goalName):Goal_Composite<MovingEntity>(pBot, goal_think)
{

	SetName(goalName);
	//create the evaluator objects
//  m_Evaluators.push_back(new AttackTargetGoal_Evaluator(AttackBias));
}

//----------------------------- dtor ------------------------------------------
//-----------------------------------------------------------------------------
Goal_Think::~Goal_Think()
{
  GoalEvaluators::iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
    delete *curDes;
  }
}


//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_Think::Activate()
{
	  m_iStatus = active;
}

void Goal_Think::Kill()
{
	RemoveAllSubgoals();
	m_iStatus = completed;
}


//------------------------------ Process --------------------------------------
//
//  processes the subgoals
//-----------------------------------------------------------------------------
int Goal_Think::Process()
{
  ActivateIfInactive();
  
  int SubgoalStatus = ProcessSubgoals();

 
  if (SubgoalStatus == completed || SubgoalStatus == failed)
  {

	if (m_SubGoals.size() == 1)	 
	{
		//this is the last thing on the list and it's done
		if (m_goalName == "Base")
		{
			//nothing else to do right now.  Kill the thing too
			//RemoveAllSubgoals();
		    m_iStatus = inactive;
		} else
		{
			//LogMsg("Ended subgoal %s", m_goalName.c_str());
			m_iStatus = SubgoalStatus;
		}
		
	}
  }

  return m_iStatus;
}

int Goal_Think::GetGoalCount()
{

	return m_SubGoals.size();
}

//----------------------------- Update ----------------------------------------
// 
//  this method iterates through each goal option to determine which one has
//  the highest desirability.
//-----------------------------------------------------------------------------


//---------------------------- notPresent --------------------------------------
//
//  returns true if the goal type passed as a parameter is the same as this
//  goal or any of its subgoals
//-----------------------------------------------------------------------------
bool Goal_Think::notPresent(unsigned int GoalType)const
{
  if (!m_SubGoals.empty())
  {
    return m_SubGoals.front()->GetType() != GoalType;
  }

  return true;
}


void Goal_Think::PushMoveToTag(TagObject *pTag)
{
	if (!pTag)
	{
		LogError("PushMoveToTag: Invalid Tag");
		return;
	}

	AddSubgoal( new Goal_MoveToMapAndPos(m_pOwner, pTag->GetPos(), pTag->m_pWorld));
}

void Goal_Think::AddMoveToTag(TagObject *pTag)
{
	if (!pTag)
	{
		LogError("AddMoveToTag: Invalid Tag");
		return;
	}

	AddBackSubgoal( new Goal_MoveToMapAndPos(m_pOwner, pTag->GetPos(), pTag->m_pWorld));
}


void Goal_Think::PushAttackTarget()
{
  if (notPresent(goal_attack_target))
  {
    RemoveAllSubgoals();
    //AddSubgoal( new Goal_AttackTarget(m_pOwner));
  }
}

void Goal_Think::Terminate()
{
}

void Goal_Think::PushRunScriptString(const string &scriptString)
{
	AddSubgoal( new Goal_RunScript(m_pOwner, scriptString));
}

void Goal_Think::AddRunScriptString(const string &scriptString)
{
	AddBackSubgoal( new Goal_RunScript(m_pOwner, scriptString));
}

void Goal_Think::PushDelay(int timeMS)
{
	AddSubgoal( new Goal_Delay(m_pOwner,  timeMS));
}

void Goal_Think::AddDelay(int timeMS)
{
	AddBackSubgoal( new Goal_Delay(m_pOwner,  timeMS));
}

void Goal_Think::AddSay(const string &msg, int entToFaceID, int delayMS)
{
	AddSayByID(msg, m_pOwner->ID(), entToFaceID, delayMS);
}

void Goal_Think::AddApproach(int entToFaceID, int distanceRequired)
{
	AddBackSubgoal( new Goal_Approach(m_pOwner,  entToFaceID, distanceRequired));
}

void Goal_Think::AddApproachAndSay(const string &msg, int entToFaceID, int distanceRequired)
{
	AddBackSubgoal( new Goal_Approach(m_pOwner,  entToFaceID, distanceRequired));
	AddBackSubgoal( new Goal_Say(m_pOwner, msg, m_pOwner->ID(), entToFaceID,-1));
}

void Goal_Think::PushApproachAndSay(const string &msg, int entToFaceID, int distanceRequired)
{
	AddSubgoal( new Goal_Say(m_pOwner, msg,m_pOwner->ID(), entToFaceID,-1));
	AddSubgoal( new Goal_Approach(m_pOwner,  entToFaceID, distanceRequired));
}

void Goal_Think::PushApproach(int entToFaceID, int distanceRequired)
{
	AddSubgoal( new Goal_Approach(m_pOwner,  entToFaceID, distanceRequired));
}

void Goal_Think::AddSayByID(const string &msg, int entID, int entToFaceID, int delayMS)
{
	AddBackSubgoal( new Goal_Say(m_pOwner,  msg, entID, entToFaceID, delayMS));
}

void Goal_Think::PushSay(const string &msg, int entToFaceID, int delayMS)
{
	PushSayByID(msg, m_pOwner->ID(), entToFaceID, delayMS);
}

void Goal_Think::PushSayByID(const string &msg, int entID, int entToFaceID, int delayMS)
{
	AddSubgoal( new Goal_Say(m_pOwner,  msg, entID, entToFaceID, delayMS));
}

Goal_Think * Goal_Think::PushNewGoal(const string &goalName)
{
	Goal_Think *pGoal = new Goal_Think(m_pOwner, goalName);
	AddSubgoal( pGoal);
	return pGoal;
}

Goal_Think * Goal_Think::AddNewGoal(const string &goalName)
{
	Goal_Think *pGoal = new Goal_Think(m_pOwner, goalName);
	AddBackSubgoal( pGoal);
	return pGoal;
}

void Goal_Think::PushMoveToPosition(CL_Vector2 pos)
{
	AddSubgoal( new Goal_MoveToMapAndPos(m_pOwner, pos, m_pOwner->GetMap()));
}


void Goal_Think::PushMoveToMapAndPosition(Map *pMap, CL_Vector2 pos)
{
	AddSubgoal(new Goal_MoveToMapAndPos(m_pOwner, pos, pMap));
}

//-------------------------- Queue Goals --------------------------------------
//-----------------------------------------------------------------------------
void Goal_Think::AddMoveToPosition(CL_Vector2 pos)
{
	AddBackSubgoal(new Goal_MoveToMapAndPos(m_pOwner, pos, m_pOwner->GetMap()));
}

void Goal_Think::AddMoveToMapAndPosition(Map *pMap, CL_Vector2 pos)
{
	AddBackSubgoal(new Goal_MoveToMapAndPos(m_pOwner, pos, pMap));
}

//----------------------- RenderEvaluations -----------------------------------
//-----------------------------------------------------------------------------
void Goal_Think::RenderEvaluations(int left, int top)const
{
  /*
	gdi->TextColor(Cgdi::black);
  
  std::vector<Goal_Evaluator*>::const_iterator curDes = m_Evaluators.begin();
  for (curDes; curDes != m_Evaluators.end(); ++curDes)
  {
    (*curDes)->RenderInfo(CL_Vector2(left, top), m_pOwner);

    left += 75;
  }
  */
}

Goal_Think * Goal_Think::GetActiveGoal()
{
	//recursively figure out what is the active goal
	if (!m_SubGoals.empty())
	{
		if (m_SubGoals.front()->GetType() == goal_think) return ((Goal_Think*)m_SubGoals.front())->GetActiveGoal();
	}
	
	//special case for the base to pass NIL

	if (m_SubGoals.size() == 1 && m_goalName == "Base") return NULL;
	
	//otherwise, we're it
	return this;
}

Goal_Think * Goal_Think::GetGoalByName(const string &name)
{
	if (name == m_goalName) return this;
	Goal_Think *pTempGoal = NULL;

	std::list<Goal<MovingEntity>*>::iterator curG;
	//scan all goals under us..
	for (curG=m_SubGoals.begin(); curG != m_SubGoals.end(); ++curG)
	{
		//is this subgoal the right one?
		if ((*curG)->GetType() == goal_think)
		{
			pTempGoal = ((Goal_Think*)(*curG))->GetGoalByName(name);
			if (pTempGoal) return pTempGoal; //found it I guess
		}
	}

	return NULL;
}

bool Goal_Think::IsGoalActiveByName(const string &name)
{
	if (name == m_goalName) return true;
	if (!m_SubGoals.empty())
	{
		if (m_SubGoals.front()->GetName() == name) return true;
		if (m_SubGoals.front()->GetType() == goal_think)
		{
			return ((Goal_Think*)m_SubGoals.front())->IsGoalActiveByName(name);
		}
	}

		
	return false;
}

void Goal_Think::Render()
{
  std::list<Goal<MovingEntity>*>::iterator curG;
  for (curG=m_SubGoals.begin(); curG != m_SubGoals.end(); ++curG)
  {
    (*curG)->Render();
  }
}


/*
Object: GoalManager
All <Entity> objects can use their <GoalManager> to allow high-level AI features.

Movement related features in the <GoalManager> require that the <Entity> has a <StandardBase> brain added.

Adding and pushing, what's the difference?:

For each goal that starts with *Add*, there is also a version starting with *Push*.


(code)
this:GetGoalManager():AddSay("Happy birthday...", C_FACING_NONE);
this:GetGoalManager():AddSay("..to you.", C_FACING_NONE);

//if we stopped now, it would sing these.

//Let's use Push to move something to the top of the stack, to happen before everything that is already there.

this:GetGoalManager():PushSay("I will now sing a song for you.", C_FACING_NONE); 
(end)

In this example, the character would first say "I will sing a song for you", and then would sing the song.  

Why use <AddNewGoal> to add more goals to the <GoalManager>?:

Internally, every sub goal added is actually a <GoalManager> that allows more goals to be added 'under' it.

A hierarchical tree of goals can be created.

Complex sub goals like <AddApproach> may create their own sub-goals to complete what is required.

In many cases it makes sense to add, track, and remove goals as a named group of actions, <AddNewGoal> (or PushNewGoal) allows you to do this by adding a custom named dummy goal designed to hold others.

Debugging Goals:

Activate "Display->Show Entity Goal And AI Data" from the main editor menu to see the Goal hierarchy in real-time as the game is played.

The goal in yellow text "has focus".

Group: Sub Goals


func: AddDelay
(code)
nil AddDelay(number delayMS)
(end)
Waits the specified amount of time before continuing to the next action/goal.

If this action is interrupted, the amount of time waited is saved, and continued when it is resumed.

Parameters:

delayMS - How long we should wait, in milliseconds. (1000 = one second)

func: AddMoveToPosition
(code)
nil AddMoveToPosition(Vector2 vPos)
(end)
Causes the <Entity> to move to the specified position using the pathfinding system.

Parameters:

vPos - A <Vector2> object containing the location to move to.  Must be on the same <Map> as the <Entity>.

func: AddMoveToMapAndPosition
(code)
nil AddMoveToPosition(Map map, Vector2 vPos)
(end)
Causes the <Entity> to move to the specified map and position using the pathfinding system.

If there is no door/warp to get to the specified <Map>, this will fail.

Parameters:

map - a <Map> object of the map we want to be on.
vPos - A <Vector2> object containing the location to move to.

func: AddApproach
(code)
nil AddApproach(number entityID, number distance)
(end)
Causes the owner <Entity> to move to (or near) the position of the entity ID sent using the pathfinding system.

Will go through doors, around obstacles as needed.

When a distance is set, the <Entity> will locate a position with a line-of-sight to the target the requested distance away, perfect for getting ready to talk to things/characters.

Parameters:

entityID - an ID of an existing entity anywhere in the world.
distance - How far we want to stand from the target.  Use a number, or one of the <C_DISTANCE_CONSTANTS>.

func: AddApproachAndSay
(code)
nil AddApproachAndSay(string msg, number entityID, number distance)
(end)
Like <AddApproach> but also says a line of text, while facing the entityID sent.

Usage:
(code)
//walk through doors, through mazes, wherever the entity named Tad is, face him and say hi.
this:GetGoalManager():AddApproachAndSay("Hi Tad.", GetEntityIDByName("Tad"), C_DISTANCE_TALK); 
(end)

Parameters:

msg - What the <Entity> should say.  (internally, the <TextManager> is used to say it)
entityID - An ID of an existing entity anywhere in the world.
distance - How far we want to stand from the target.  Use a number, or one of the <C_DISTANCE_CONSTANTS>.


func: AddSay
(code)
nil AddSay(string msg, number entToFaceID, (optional) number delayMS)
(end)

Causes the owner <Entity> to say something, while looking a certain direction.

Usage:
(code)
//Say "Where is Jim" without changing our direction
this:GetGoalManager():AndSay("Where is Jim?", C_FACING_NONE);

//Look to the north
this:GetGoalManager():AndSay("Hello, north", C_FACING_UP);

//Look to the south
this:GetGoalManager():AndSay("Hello, south", C_FACING_DOWN);

//Look at wherever Jim is, and make the text stay on screen for 10 seconds with
//the optional delayMS parm
this:GetGoalManager():AndSay("Hello, Jiiiiim!", entityIDForJim, 10000);
(end)

Parameters:

msg - What the <Entity> should say.  (internally, the <TextManager> is used to say it)
entToFaceID - an entityID, or one of the <C_FACING_CONSTANTS>.
delayMS - (optional) If specified, this sets how long the text should stay on the screen.  (normally, the TextManager decides, based on length)

func: AddSayByID
(code)
nil AddSayByID(string msg, number entityIDToTalk, number entToFaceID, (optional) number delayMS)
(end)
Like <AddSay> but lets you specify which <Entity> talks instead of assuming it's the owner entity.
This is useful to orchestrate complex conversations with only one <GoalManager> controlling everything.

Usage:
(code)
//make a conversation with three people happen using only one guy's GoalManager

local gandolfID =  GetEntityIDByName("Gandolf")
local frodoID =  GetEntityIDByName("Frodo")
local legolasID =  this:GetID(); //let's assume this entity is Legolas, although it could be any other

//make gandolf speak to Legolas
this:GetGoalManager():AndSayByID("NEVER do that again.", gandolfID, legolasID);

//legolas replies to gandalf
this:GetGoalManager():AndSayByID("What are you talking about? Do what?!", legolasID, gandolfID);

//make frodo speak to Legolas
this:GetGoalManager():AndSayByID(
	"He's talking about using that shield as a damn surfboard!  Ruined the movie!", frodoID, legolasID);
(end)

Parameters:

msg - What the <Entity> should say.  (internally, the <TextManager> is used to say it)
entToFaceID - an entityID, or one of the <C_FACING_CONSTANTS>.
delayMS - (optional) If specified, this sets how long the text should stay on the screen.  (normally, the TextManager decides, based on length)

func: AddRunScriptString
(code)
nil AddRunScriptString(string msg)
(end)

This lets you run any piece of lua code in owner <Entity>'s namespace.

You can use it to call a function, change a variable, or anything else.

The string is not parsed until the time of execution.

Usage:

(code)

//run this Entity's function called "WonderAround()"
this:GetGoalManager():AddRunScriptString("WonderAround()");


//Change our name, notice to use quotes inside the quotes, we have to use \".
local someText = "this:SetName(\"Dillweed\");";
//let's change our color too
someText = someText + "this:SetBaseColor(Color(255,0,0,255));";
//actually send the string to the goal manager
this:GetGoalManager():AddRunScriptString(someText);
(end)

msg - The lua string you want to run.  As always, no size limits.

Group: Miscellaneous Functions

func: AddNewGoal
(code)
GoalManager AddNewGoal(string goalName)
(end)
Adds a new subgoal with the desired name and returns a handle to the created goal, allowing you to populate it.
Think of goal as an empty folder that is ready to handle more commands/goals.

When a goal has "focus" it will run all items in it sequentially until empty, then delete itself, unless it is the "Base".

Usage:
(code)
//create a new goal. If we wanted to interrupts what they are doing now, we should
//use PushNewGoal instead of AddNewGoal.

local goal = this:GetGoalManager():AddNewGoal("CleanPlayerBed"); 

//look at the player while telling him we'll do it
goal:AddSay("Fine, I'll go clean the bad.", g_playerID); 

//figure out the entity ID of the bed we want to clean
local bedID = GetEntityIDByName("Hol_Inn_BedPlayer"); 

//walk up to the bed, and say we're cleaning
goal:AddApproachAndSay("<cleaning>",bedID , C_DISTANCE_CLOSE);  
(end)

Parameters:

goalName - The name you'd like the new goal to have, can be anything.  "Kill Tad" or "GetFoodAndEat" for instance.  This name will show up in the game when "Show goal info" is activated.

Returns:

A <GoalManager> connected to the new goal.

func: RemoveAllSubgoals
(code)
nil RemoveAllSubgoals()
(end)
Deletes all goals and subgoals under this <GoalManager>.

This will cause it to delete itself, unless it is the "Base" <GoalManager> of this entity.

func: IsGoalActiveByName
(code)
boolean IsGoalActiveByName(string goalName)
(end)

goalName - The name of the goal you probably added with <AddNewGoal> or PushNewGoal.

Returns:

True if a goal by this name exists somewhere in the hierarchy of this <GoalManager>.

func: GetActiveGoal
(code)
GoalManager GetActiveGoal()
(end)
Returns the <GoalManager> object connected to the active goal.  If nil, no goal is currently active.  (Ie, the base is the only thing active)

Returns:

A <GoalManager> of the active goal or nil if no goal is active.

func: GetGoalCount
(code)
number GetGoalCount()
(end)

Returns:

The number of Sub Goals/Goals under this <GoalManager>. 0 if empty.


func: GetGoalCountByName
(code)
number GetGoalCountByName()
(end)

goalName - The name of a goal(s) you probably added with <AddNewGoal> or PushNewGoal.

Returns:

The number of Goals with this name under this <GoalManager>.


func: GetGoalByName
(code)
GoalManager GetGoalByName(string goalName)
(end)
Allows you to grab a goal object by name.

Returns:

A <GoalManager> of the goal with this name, or nil if nothing is found.
*/





