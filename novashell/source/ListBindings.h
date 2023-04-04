#ifndef ListClassBindings_HEADER_INCLUDED // include guard
#define ListClassBindings_HEADER_INCLUDED  // include guard

#include <vector>


class TileList
{

	public:

	TileList();
	TileList(tile_list *pTileList);

	Tile * GetNext();
	void ResetNext();
	int GetCount();

	private:


		tile_list m_tileList; //the real list
		tile_list::iterator m_itor;
		bool m_bItorInitted;
};


void luabindList(lua_State *pState);





#endif                  // include guard