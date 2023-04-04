#ifndef INVAABBOX2D_H
#define INVAABBOX2D_H
//-----------------------------------------------------------------------------
//
//  Name:   InvertedAABBox2D.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   v simple inverted (y increases down screen) axis aligned bounding
//          box class
//-----------------------------------------------------------------------------

class InvertedAABBox2D
{
private:
  
  CL_Vector2  m_vTopLeft;
  CL_Vector2  m_vBottomRight;

  CL_Vector2  m_vCenter;
  
public:

  InvertedAABBox2D(CL_Vector2 tl,
                   CL_Vector2 br):m_vTopLeft(tl),
                                m_vBottomRight(br),
                                m_vCenter((tl+br)/2.0)
  {}

  //returns true if the bbox described by other intersects with this one
  bool isOverlappedWith(const InvertedAABBox2D& other)const
  {
    return !((other.Top() > this->Bottom()) ||
           (other.Bottom() < this->Top()) ||
           (other.Left() > this->Right()) ||
           (other.Right() < this->Left()));
  }

  
  CL_Vector2 TopLeft()const{return m_vTopLeft;}
  CL_Vector2 BottomRight()const{return m_vBottomRight;}

  double    Top()const{return m_vTopLeft.y;}
  double    Left()const{return m_vTopLeft.x;}
  double    Bottom()const{return m_vBottomRight.y;}
  double    Right()const{return m_vBottomRight.x;}
  CL_Vector2 Center()const{return m_vCenter;}

  void     Render(bool RenderCenter = false)const
  {
/*  
	  gdi->Line((int)Left(), (int)Top(), (int)Right(), (int)Top() );
    gdi->Line((int)Left(), (int)Bottom(), (int)Right(), (int)Bottom() );
    gdi->Line((int)Left(), (int)Top(), (int)Left(), (int)Bottom() );
    gdi->Line((int)Right(), (int)Top(), (int)Right(), (int)Bottom() );


    if (RenderCenter)
    {
      gdi->Circle(m_vCenter, 5);
    }
  */
  }

};
  
#endif