#ifndef PATHEDGE_H
#define PATHEDGE_H
//-----------------------------------------------------------------------------
//
//  Name:   PathEdge.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   class to represent a path edge. This path can be used by a path
//          planner in the creation of paths. 
//
//-----------------------------------------------------------------------------

class PathEdge
{
private:

  //positions of the source and destination nodes this edge connects
  CL_Vector2 m_vSource;
  CL_Vector2 m_vDestination;

  //the behavior associated with traversing this edge
  int      m_iBehavior;

  int      m_iDoorID;

public:
  
  PathEdge(CL_Vector2 Source,
           CL_Vector2 Destination,
           int      Behavior,
           int      DoorID = 0):m_vSource(Source),
                                m_vDestination(Destination),
                                m_iBehavior(Behavior),
                                m_iDoorID(DoorID)
  {}

  CL_Vector2 Destination()const{return m_vDestination;}
  void     SetDestination(CL_Vector2 NewDest){m_vDestination = NewDest;}
  
  CL_Vector2 Source()const{return m_vSource;}
  void     SetSource(CL_Vector2 NewSource){m_vSource = NewSource;}

  int      DoorID()const{return m_iDoorID;}
  int      Behavior()const{return m_iBehavior;}
};


#endif
