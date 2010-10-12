/*
** Lua binding: global
** Generated automatically by tolua++-1.0.92 on Sun Dec 13 15:12:49 2009.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_global_open (lua_State* tolua_S);

#include "enigma.hh"
using namespace enigma;

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: DefineFont */
#ifndef TOLUA_DISABLE_tolua_global_DefineFont00
static int tolua_global_DefineFont00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isstring(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,1,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,1,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* ttf_name = ((const char*)  tolua_tostring(tolua_S,2,0));
  int ttf_size = ((int)  tolua_tonumber(tolua_S,3,0));
  const char* bmf_name = ((const char*)  tolua_tostring(tolua_S,4,0));
  int r = ((int)  tolua_tonumber(tolua_S,5,255));
  int g = ((int)  tolua_tonumber(tolua_S,6,255));
  int b = ((int)  tolua_tonumber(tolua_S,7,255));
  {
   DefineFont(name,ttf_name,ttf_size,bmf_name,r,g,b);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineFont'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_global_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_function(tolua_S,"DefineFont",tolua_global_DefineFont00);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_global (lua_State* tolua_S) {
 return tolua_global_open(tolua_S);
};
#endif

