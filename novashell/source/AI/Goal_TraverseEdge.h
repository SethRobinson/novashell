#ifndef GOAL_TRAVERSE_EDGE_H
#define GOAL_TRAVERSE_EDGE_H
#pragma warning (disable:4786)

#include "Goal.h"
#include "PathPlanner.h"
#include "PathEdge.h"

class Goal_TraverseEdge : public Goal<MovingEntity>
{
private:

  //the edge the bot will follow
  PathEdge  m_Edge;

  //true if m_Edge is the last in the path.
  bool      m_bLastEdgeInPath;

  //the estimated time the bot should take to traverse the edge
  unsigned int     m_timeExpected;
  
  //this records the time this goal was activated
  unsigned int     m_startTime;

  //returns true if the bot gets stuck
  bool      isStuck()const;


public:

  Goal_TraverseEdge(MovingEntity* pBot,
                    PathEdge   edge,
                    bool       LastEdge); 

  //the usual suspects
  void Activate();
  int  Process();
  void Terminate();
  void Render();
  bool HandleMessage(const Message& msg);
  virtual const char * GetName() const {return "TraverseEdge";}
};




#endif

