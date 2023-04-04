
#include "AppPrecomp.h"
#include "MovingEntity.h"
#include "DataManager.h"
#include "ListBindings.h"
#include "VisualProfile.h"
#include "MaterialManager.h"
#include "MessageManager.h"
#include "TextManager.h"
#include "TagManager.h"
#include "InputManager.h"
#include "VisualProfileManager.h"
#include "AI/Goal_Think.h"
#include "AI/WatchManager.h"
#include "EntCreationUtils.h"



#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

TileList::TileList()
{
	m_bItorInitted = false;
}

TileList::TileList(tile_list *pTileList)
{
	
	m_bItorInitted = false;

	tile_list::iterator itor=pTileList->begin();

	while (itor != pTileList->end())
	{
		m_tileList.push_back(*itor);

		itor++;
	}
}

int TileList::GetCount()
{
	return m_tileList.size();
}

Tile * TileList::GetNext()
{
	if (!m_bItorInitted)
	{
		ResetNext();
		m_itor = m_tileList.begin();
	}


	if (m_itor != m_tileList.end())
	{
		
		return * (m_itor++);
	}

	ResetNext();
	return NULL; //all done
}

void TileList::ResetNext()
{
	m_bItorInitted = true;
	m_itor = m_tileList.begin();

}

int NumberGet(vector<unsigned int> *pVec, unsigned int index)
{
	if (index > pVec->size())
	{
		LogError("Index %d is invalid in NumberList, it only has %d items.", index, pVec->size());
			return 0;
	}

	return pVec->at(index);
}

using namespace luabind;


string TileListToString(TileList * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A TileList with %d tiles.", pObj->GetCount());
	return string(stTemp);
}

string LayerListToString( vector<unsigned int> * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A LayerList with %d tiles.", pObj->size());
	return string(stTemp);
}

void luabindList(lua_State *pState)
{
	module(pState)
		[
			/*
			Object: TileList
			Contains a list of tiles.

			Internally, this contains only pointers to tiles.  So if you copy the list, no tiles are copied, only the pointers.

			Group: Member Functions

			func: GetNext
			(code)
			Tile GetNext()
			(end)

			Allows you to run through the list and look at each tile in it.

			When no more tiles are available, nil is returned and <ResetNext> is automatically called internally.

			Usage:
			(code)
			local tile; 

			while true do
				tile = tileList:GetNext();
				if (tile == nil) then break; end;
				LogMsg("Found tile type " .. tile:GetType());
			end
			
			(end)
			
			Returns:

			The next <Tile> in  the list, or nil if the end has been reached.
			*/
			class_<TileList>("TileList")
			.def(constructor<>())
			.def("GetNext", &TileList::GetNext)
			.def("ResetNext", &TileList::ResetNext)
			.def("__tostring", &TileListToString)

			
/*
func: ResetNext
(code)
nil ResetNext()
(end)

Causes the next call to <GetNext> to start on the first tile.

*/

			
			.def("GetCount", &TileList::GetCount)
			,
		
			/*
			func: GetCount
			(code)
			number GetCount()
			(end)

			Returns:

			The number of tiles currently in the list.

			*/


		luabind::class_<std::vector<unsigned int>>("LayerList")
		.def(luabind::constructor<>())
		.def("Add", static_cast<void(*)(std::vector<unsigned int>&, const unsigned int&)>([](std::vector<unsigned int>& vec, const unsigned int& val) { vec.push_back(val); }))
		.def("GetCount", &std::vector<unsigned int>::size)
		.def("Get", &NumberGet)
		.def("__tostring", &LayerListToString)

		/*
		//old way that won't compile
			class_< vector<unsigned int> >("LayerList")
			.def(constructor<>())
			.def("Add", &vector<unsigned int>::push_back)
			.def("GetCount", &vector<unsigned int>::size)
			.def("Get",  &NumberGet)
			.def("__tostring", &LayerListToString)
			*/


		];
}