
#ifndef GlobalScriptFunctionBindings_h__
#define GlobalScriptFunctionBindings_h__


void luabindGlobalFunctions(lua_State *pState);

class Tile;
class Map;

Tile * GetTileByWorldPos(Map *pWorld, CL_Vector2 v, std::vector<unsigned int> layerIDVec, bool bPixelAccurate);

#endif // GlobalScriptFunctionBindings_h__
