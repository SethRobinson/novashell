
#ifndef GOAL_THINK_H
#define GOAL_THINK_H

#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_Think .h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to arbitrate between a collection of high level goals, and
//          to process those goals.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include "Goal_Composite.h"
#include "Goal_Evaluator.h"

class Goal_Think : public Goal_Composite<MovingEntity>
{
private:
  
  typedef std::vector<Goal_Evaluator*>   GoalEvaluators;

private:
  
  GoalEvaluators  m_Evaluators;

public:

  Goal_Think(MovingEntity* pBot,const string &goalName);
  ~Goal_Think();

  //this method iterates through each goal evaluator and selects the one
  //that has the highest score as the current goal

  //returns true if the given goal is not at the front of the subgoal list
  bool notPresent(unsigned int GoalType)const;

  //the usual suspects
  int  Process();
  void Activate();
  
  //top level goal types
  void PushMoveToPosition(CL_Vector2 pos);
  void PushAttackTarget();
  void PushRunScriptString(const string &scriptString);
  void AddRunScriptString(const string &scriptString);

  void AddApproachAndSay(const string &msg, int entToFaceID, int distanceRequired);
  void PushApproachAndSay(const string &msg, int entToFaceID, int distanceRequired);

  void PushDelay(int timeMS);
  void AddDelay(int timeMS);
  void PushApproach(int entToFaceID, int distanceRequired);
  void AddApproach(int entToFaceID, int distanceRequired);
  Goal_Think * PushNewGoal(const string &goalName);
  Goal_Think * AddNewGoal(const string &goalName);
  Goal_Think * GetActiveGoal();
  Goal_Think * GetGoalByName(const string &goalName);

  //this adds the MoveToPosition goal to the *back* of the subgoal list.
  void AddMoveToPosition(CL_Vector2 pos);
  void AddMoveToMapAndPosition(Map * pMap, CL_Vector2 pos);
  void PushMoveToMapAndPosition(Map *pMap, CL_Vector2 pos);

  void PushMoveToTag(TagObject *pTag);
  void AddMoveToTag(TagObject *pTag);

  void AddSay(const string &msg, int entToFaceID, int delayMS);
  void AddSay(const string &msg, int entToFaceID) {AddSay(msg, entToFaceID, -1);} //for luabind

  void AddSayByID(const string &msg, int entID, int entToFaceID, int delayMS);
  void AddSayByID(const string &msg, int entID, int entToFaceID) {AddSayByID(msg, entID, entToFaceID, -1);}

  //the entToFaceID can be an entity ID or a direction (FACING_LEFT, etc) or FACING_NONE to not look anywhere
  void PushSay(const string &msg, int entToFaceID, int delayMS);
  void PushSay(const string &msg, int entToFaceID) {PushSay(msg, entToFaceID, -1);}

  void PushSayByID(const string &msg, int entID, int entToFaceID, int delayMS);
  void PushSayByID(const string &msg, int entID, int entToFaceID){return PushSayByID(msg, entID, entToFaceID, -1);}

  bool IsGoalActiveByName(const string &name);
	
  void Kill();
  int GetGoalCount();

  //this renders the evaluations (goal scores) at the specified location
  void  RenderEvaluations(int left, int top)const;
  void  Render();
  void Terminate();
};


#endif
