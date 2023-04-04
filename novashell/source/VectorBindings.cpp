
#include "AppPrecomp.h"

#include "AppUtils.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;


void NormalizeVector2(CL_Vector2 * pV)
{
	pV->unitize();
}

CL_Rect CalculateCombinedRect(CL_Rect *r, const CL_Rect &r2)
{
	return CombineRects(*r, r2);
}

CL_Rectf CalculateCombinedRectf(CL_Rectf *r, const CL_Rectf &r2)
{
	return CombineRects(*r, r2);
}

bool RectfIsInside(CL_Rectf *r, const CL_Vector2 &v)
{
	return r->is_inside(CL_Pointf(v.x, v.y));
}

bool RectIsInside(CL_Rect *r, const CL_Vector2 &v)
{
	return r->is_inside(CL_Point(v.x, v.y));
}

void luabindVector(lua_State *pState)
{

	/*
	Title: Math Related Objects

	Object: Vector2
	The Vector2 object.

	About:

	Most functions in Novashell take a <Vector2> instead of an x and y separately.

	Usage:
	(code)
	vPos = Vector2(5,5);
	vPos = vPos + (vPos*2);
	LogMsg("Hey, vPos is " .. tostring(vPos) .. " and the length is " .. vPos:Length());
	//Output is "Hey, vPos is X:15.00 Y: 15.00 and the length is 21.21320343076"

	vPos.x = vPos.x/1.54; //let's only modify the x coordinate

	LogMsg("My god, vPos.x is " .. vPos.x);
	//Output is "My god, vPos.x is 9.7402601242065"

	//Let's turn it into a unit vector
	vPos.Normalize(); 

	(end)

	Note:
	If you use tostring() to show a Vector2, it is formatted and truncated to two decimal points for display.  Internally it's still double accuracy, like all all Lua numbers.
	
	Group: Local Variables

	Variable: x
	The x coordinate.

	Variable: y
	The y coordinate.

	Group: Initialization
	
	func: Vector2
	(code)
	Vector2()
	(end)
	Defaults to 0,0 if initialized with no parameters.

	func: Vector2(Vector2)
	(code)
	Vector2(Vector2 vSomeVec)
	(end)
	A <Vector2> can be initialized with the values from another <Vector2> object.

	func: Vector2(x,y)
	(code)
	Vector2(number x, number, y)
	(end)
	A <Vector2> can be initialized with a standard x and y passed in separately.

	So anytime a function asks for a <Vector2> and you don't have one, keep in mind you can always just type *Vector2(x,y)*. (Replacing x and y with the numbers you want.)

	Group: Member Functions
	*/

	module(pState)
		[

			class_<CL_Vector2>("Vector2")
			.def(constructor<>())
			.def(constructor<CL_Vector2>())
			.def(constructor<float, float>())
			.def_readwrite("x", &CL_Vector2::x)
			.def_readwrite("y", &CL_Vector2::y)
			
			/*
			func: Length
			(code)
			number Length()
			(end)

			Returns:
			The length of the vector.
			*/

			/*
			func: Normalize
			(code)
			number Normalize()
			(end)

			Turns the vector into a unit vector.
			*/

			/*
			func: Dot
			(code)
			number Dot(Vector2 vVec)
			(end)

			Returns:
			The dot product of the operation.
			*/

			/*
			func: Cross
			(code)
			Vector2 Cross()
			(end)

			Returns:
			The cross product of the vector.
			*/


			.def("Normalize", &NormalizeVector2)
			.def("Length", &CL_Vector2::length)
			.def("Dot", &CL_Vector2::dot)
			.def("Cross", &CL_Vector2::cross)
		    .def("__tostring", &VectorToStringEx)
			/*

			Group: Operators
			*/

			/*
			Function: Assignment Operator
			=Vector2

			Example of assignment:
			(code)
			newVec = oldVec
			(end)

			Function: Addition Operator
			+Vector2

			Example of addition:
			(code)
			vPos = vPos + Vector2(1,1); //add 1 to x and y in the vPos vector
			(end)

			Function: Subtraction Operator
			-Vector2

			Example of subtraction:
			(code)
			vPos = vPos - Vector2(1,0); //remove 1 from vPos's x coordinate
			(end)

			Function: Multiply Operator
			*number

			Example of scaler multiplication:
			(code)
			vPos = Vector(1,2);
			vPos = vPos * 10;  //vPos.x is now 10, vPos.y is now 20.
			(end)

			Function: Division Operator
			*number

			Example of scaler division:
			(code)
			vPos = Vector(1,0);
			vPos = vPos /2;  //vPos.x is now 0.5, vPos.y is still 0 of course.
			(end)

			Function: Equality Operator
			==Vector2

			Example of using the equality operator:
			(code)
			if (vPos == vYourPos) then LogMsg("They are the same"); end;
			(end)

			Function: Inequality Operator
			!=Vector2

			Example of using the inequality operator:
			(code)
			if (vPos != vYourPos) then LogMsg("By jove, these are different"); end;
			(end)

			*/

			.def(const_self + CL_Vector2())
			.def(const_self - CL_Vector2())
			.def(const_self / float())
			.def(const_self * float())
			.def(const_self == CL_Vector2())
	
			,
			
/*
Object: Rect
The Rect object.  Holds only whole integer numbers, no decimal points allowed.

Group: Local Variables

Variable: left
The left coordinate.

Variable: top
The top coordinate.

Variable: right
The right coordinate.

Variable: bottom
The bottom coordinate.

Group: Initialization

func: Rect
(code)
Rect()
(end)
Defaults to all zeroes if initialized with no parameters.

func: Rect(Rect)
(code)
Rect(Rect vSomeRect)
(end)
A <Rect> can be initialized with the values from another <Rect> object.

func: Rect(left,top,right,bottom)
(code)
Rect(number left, number top, number right, number bottom)
(end)
A <Rect> can be initialized with four numbers.

Group: Member Functions

*/

			class_<CL_Rect>("Rect")
			.def(constructor<>())
			.def(constructor<CL_Rect>())
			.def(constructor<CL_Rectf>())
			.def(constructor<int, int, int, int>())
			.def_readwrite("left", &CL_Rect::left)
			.def_readwrite("top", &CL_Rect::top)
			.def_readwrite("right", &CL_Rect::right)
			.def_readwrite("bottom", &CL_Rect::bottom)


			/*
			func: GetWidth
			(code)
			number GetWidth()
			(end)
			
			Returns:

			The actual width of the rect.
			*/

		
			.def("GetWidth", &CL_Rect::get_width)
			
			/*
			func: GetHeight
			(code)
			number GetHeight()
			(end)

			Returns:

			The actual height of the rect.
			*/

			.def("GetHeight", &CL_Rect::get_height)
			
			/*
			func: IsOverlapped
			(code)
			boolean IsOverlapped(Rect rectB)
			(end)

			Returns:

			True if the two rectangles overlap at all.
			*/
			.def("IsOverlapped", &CL_Rect::is_overlapped)

			/*
			func: IsInside
			(code)
			boolean IsInside(Vector2 v)
			(end)

			Returns:

			True if the vector is located inside the rectangle.
			*/
			.def("IsInside", &RectIsInside)


		
			/*
			func: CalculateUnion
			(code)
			Rect CalculateUnion(Rect rectB)
			(end)

			Returns:

			A <Rect> of the exact size of the union.
			*/

			.def("CalculateUnion", &CL_Rect::calc_union)

			/*
			func: CalculateCombined
			(code)
			Rect CalculateCombined(Rect rectB)
			(end)

			Returns:

			A <Rect> of the area required to enclose both the original and passed in rectangle.
			*/

			.def("CalculateCombined", &CalculateCombinedRect)


			/*
			Group: Operators

			func: Addition Operator
			+Rect
			
			Usage:
			(code)
			rectArea = rectHouse + Rect(100,0,0,0); //we added 100 to rectArea.left
			(end)

			func: Subtraction Operator
			-Rect

			Usage:
			(code)
			rectArea = rectHouse - rectAreaToRemove;
			(end)

			*/
			
			.def(const_self + CL_Rect())
			.def(const_self - CL_Rect())
				.def(const_self == CL_Rect())
			.def("__tostring", &RectToStringEx)
			,
			
			/*
			Object: Rectf
			The Rectf object.  This version supports fractions and decimal points.

			Group: Local Variables

			Variable: left
			The left coordinate.

			Variable: top
			The top coordinate.

			Variable: right
			The right coordinate.

			Variable: bottom
			The bottom coordinate.

			Group: Initialization

			func: Rectf
			(code)
			Rectf()
			(end)
			Defaults to all zeroes if initialized with no parameters.

			func: Rectf(Rectf)
			(code)
			Rectf(Rectf vSomeRectf)
			(end)
			A <Rectf> can be initialized with the values from another <Rectf> object.

			func: Rectf(left,top,right,bottom)
			(code)
			Rectf(number left, number top, number right, number bottom)
			(end)
			A <Rectf> can be initialized with four numbers.

			Group: Member Functions

			*/

			class_<CL_Rectf>("Rectf")
			.def(constructor<>())
			.def(constructor<CL_Rectf>())
			.def(constructor<float, float, float, float>())
			.def_readwrite("left", &CL_Rectf::left)
			.def_readwrite("top", &CL_Rectf::top)
			.def_readwrite("right", &CL_Rectf::right)
			.def_readwrite("bottom", &CL_Rectf::bottom)

			/*
			func: GetWidth
			(code)
			number GetWidth()
			(end)

			Returns:

			The actual width of the Rectf.
			*/


			.def("GetWidth", &CL_Rectf::get_width)

			/*
			func: GetHeight
			(code)
			number GetHeight()
			(end)

			Returns:

			The actual height of the Rectf.
			*/

			.def("GetHeight", &CL_Rectf::get_height)

			/*
			func: IsOverlapped
			(code)
			boolean IsOverlapped(Rectf RectfB)
			(end)

			Returns:

			True if the two Rectfangles overlap at all.
			*/
			.def("IsOverlapped", &CL_Rectf::is_overlapped)

		
			/*
			func: IsInside
			(code)
			boolean IsInside(Vector2 v)
			(end)

			Returns:

			True if the vector is located inside the rectangle.
			*/
			.def("IsInside", &RectfIsInside)

	

			/*
			func: CalculateUnion
			(code)
			Rectf CalculateUnion(Rectf RectfB)
			(end)

			Returns:

			A <Rectf> the exact size of the union.
			*/

			.def("CalculateUnion", &CL_Rectf::calc_union)

			/*
			func: CalculateCombined
			(code)
			Rectf CalculateCombined(Rectf rectB)
			(end)

			Returns:

			A <Rectf> of the area required to enclose both the original and passed in rectangle.
			*/

			.def("CalculateCombined", &CalculateCombinedRectf)



			/*
			Group: Operators

			func: Addition Operator
			+Rectf

			Usage:
			(code)
			RectfArea = RectfHouse + Rectf(100,0,0,0); //we added 100 to RectfArea.left
			(end)

			func: Subtraction Operator
			-Rectf

			Usage:
			(code)
			RectfArea = RectfHouse - RectfAreaToRemove;
			(end)

			*/

			.def(const_self + CL_Rectf())
			.def(const_self - CL_Rectf())
			.def(const_self == CL_Rectf())
			.def("__tostring", &RectfToStringEx)



		];



}