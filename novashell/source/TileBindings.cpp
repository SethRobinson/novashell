
#include "AppPrecomp.h"
#include "TileBindings.h"
#include "Tile.h"
#include "TileEntity.h"
#include "MovingEntity.h"

#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif



using namespace luabind;

MovingEntity * GetTileAsEntity(Tile *pTile)
{
	if (pTile->GetType() != C_TILE_TYPE_ENTITY)
	{
		LogError("Tile:GetAsEntity failed, this thing isn't an entity.");
		return NULL;
	}

	return ((TileEntity*)pTile)->GetEntity();
}

string TileToString(Tile * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "A Tile.");
	return string(stTemp);
}

void luabindTile(lua_State *pState)
{
	module(pState)
		[

			class_<Tile>("Tile")
		
			/*
			Object: Tile
			Anything placeable on the map is housed in a <Tile>.

A Tile can hold a <TilePic> or an <Entity> inside of it.

Note:

When you use <Entity::SetPos> or <Entity::SetBaseColor>, internally, these functions are called.

The reason we use a Tile interface at times is that it lets us perform operations on an <Entity> or <TilePic> without knowing or caring what it really is.

			Group: Member Functions
		
			*/
			
			

			.def("GetType", &Tile::GetType)
			
			/*
			func: GetType
			(code)
			number GetType()
			(end)

			Returns:

			One of the <C_TILE_TYPE_CONSTANTS>.  If it is a <C_TILE_TYPE_ENTITY>, you know you can use the <GetAsEntity> function safely.
			*/

			
			.def("SetPos", &Tile::SetPos)

			/*
			func: SetPos
			(code)
			nil SetPos(Vector2 vPos)
			(end)

			Parameters:

			vPos - a <Vector2> object containing the position to move to.
			*/


			.def("GetPos", &Tile::GetPosSafe)
			/*
			func: GetPos
			(code)
			Vector2 GetPos()
			(end)

			Returns:

			A <Vector2> object containing the tile's position.
			*/
	
			.def("SetLayerID", &Tile::SetLayer)
			
			/*
			func: SetLayerID
			(code)
			nil SetLayerID(number layerID)
			(end)

			Allows you to change the layer of an entity.

			Note that the actual movement will not happen until the end of that game logic cycle.

			Usage:
			(code)
			this:SetLayerID(this:GetLayerID()+1); //move up one layer
			//Note, this may or may not affect our sorting level as expected, depends on the layer's settings.
			(end)
			*/


			.def("GetLayerID", &Tile::GetLayer)
			
			/*
			func: GetLayerID
			(code)
			number GetLayerID()
			(end)

			Usage:
			(code)
			local layerID = this:GetLayerID();
			LogMsg("We're on layer " .. layerID);			
			(end)

			Returns:

			The layer ID that we're on.
			*/

			
			.def("SetBaseColor", &Tile::SetColor)

			/*
			func: SetBaseColor
			(code)
			nil SetBaseColor(Color color);
			(end)

			Using this you can change the color of an entity/tilepic.  Technically, you're just setting how much of red, green, blue can get through.

			So if you start with white, you can make any color by removing the colors you don't want.

			A <Color> object also contains an alpha value, this lets you set the translucency.  (This happens on top of whatever per-pixel alpha the image already has)

			Usage:
			(code)
			myTile:SetBaseColor(Color(255,20,20,255)); //we should now look very red, since we're not
			//letting much green or blue through.

			//another example

			Color c = Color(255,255,255,110); //full color, but only about half the alpha
			myTile:SetBaseColor(c); //we're now about 50% invisible
			(end)

			Parameters:

			color - A <Color> object.
			*/


			.def("GetBaseColor", &Tile::GetColor)
		

			/*
			func: GetBaseColor
			(code)
			Color GetBaseColor();
			(end)

			Returns:

			A <Color> object containing the tile's base color.
			*/
			
			.def("GetAsEntity", &GetTileAsEntity)
			.def("__tostring", &TileToString)

			/*
			func: GetAsEntity
			(code)
			Entity GetAsEntity();
			(end)

			Returns:

			The <Entity> object interface inside the tile, or nil if this isn't an entity.
			*/

		];
/*
Section: Tile Related Constants


Group: C_TILE_TYPE_CONSTANTS
Use with <Tile::GetType> to see what a <Tile> is actually housing inside.

constants: C_TILE_TYPE_BLANK
A blank/invalid tile.  Should never see this,

constants: C_TILE_TYPE_PIC
This means the <Tile> houses a <TilePic> inside and <Tile::GetAsTilePic> is safe to use.

constants: C_TILE_TYPE_ENTITY
This means the <Tile> houses an <Entity> inside and <Tile::GetAsEntity> is safe to use.

*/

}