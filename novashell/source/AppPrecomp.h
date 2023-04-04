//precompiled headers that shouldn't change much

#ifndef _APPPRE
#define _APPPRE

#ifdef __cplusplus

#include <API/core.h>
#include <API/application.h>

#define Zone CarbonZone
#define check CarbonCheck

//for linux header conflicts
#define Screen X11Screen

#include <API/display.h>

#include <API/gl.h> 
#include <misc/MiscUtils.h>
#include <API/gui.h>
#include <API/guistylesilver.h>
#include <API/signals.h>
#undef Zone
#undef check
#undef Screen

#include "lua/etc/lua.hpp"

#ifdef WIN32
//couldn't get macs to work with luabind in the precompiled header.. !
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

#ifndef __APPLE__
#include <float.h> //need FLT_MAX const
#endif

#include "misc/CL_VirtualFileManager.h"
#include "EntityManager.h"

using namespace std;

//#include "All.h"
//#include "std_all.h"
//#include "CBit8.h"

#include <list>
#include <string>
#include <vector>
#include <map>

#pragma warning (disable:4244)
#endif

#endif
