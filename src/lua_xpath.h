#include <lua.h>
#include <lauxlib.h>


#ifdef HAVE_CONFIG_H

#include "config.h"

#define LUA_XPATH_VERSION   PACKAGE_STRING
#define LUA_XPATH_AUTHORS   PACKAGE_BUGREPORT

#else 

#define LUA_XPATH_VERSION   "LuaXPath"
#define LUA_XPATH_AUTHORS   "Hungpu DU <alecdu@gmail.com>"

#endif 

#define LUA_XPATH_COPYRIGHT "Copyright (C) 2014-2016 HungpuDU"


LUALIB_API int luaopen_xpath(lua_State *L);
