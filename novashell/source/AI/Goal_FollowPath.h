#ifndef GOAL_FOLLOWPATH_H
#define GOAL_FOLLOWPATH_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_FollowPath.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:
//-----------------------------------------------------------------------------
#include "Goal_Composite.h"
#include "Goal_Types.h"
#include "NavGraphManager.h"
#include "../MovingEntity.h"
#include "PathPlanner.h"

class Goal_FollowPath : public Goal_Composite<MovingEntity>
{
private:

  //a local copy of the path returned by the path planner
  std::list<PathEdge>  m_Path;

public:

  Goal_FollowPath(MovingEntity* pBot, std::list<PathEdge> path);

  //the usual suspects
  void Activate();
  int Process();
  void Render();
  void Terminate(){}
  bool HandleMessage(const Message& msg);

};

#endif

