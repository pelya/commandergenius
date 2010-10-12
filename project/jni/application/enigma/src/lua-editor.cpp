/*
** Lua binding: editor
** Generated automatically by tolua++-1.0.92 on Sun Dec 13 15:12:49 2009.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_editor_open (lua_State* tolua_S);

#include "editor.hh"
using namespace editor;

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: DefineFloorGroup */
#ifndef TOLUA_DISABLE_tolua_editor_editor_DefineFloorGroup00
static int tolua_editor_editor_DefineFloorGroup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_istable(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* descr = ((const char*)  tolua_tostring(tolua_S,2,0));
  int nentries = ((int)  tolua_tonumber(tolua_S,3,0));
#ifdef __cplusplus
  char** entries = new char*[nentries];
#else
  char** entries = (char**) malloc((nentries)*sizeof(char*));
#endif
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isstringarray(tolua_S,4,nentries,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<nentries;i++)
    entries[i] = ((char*)  tolua_tofieldstring(tolua_S,4,i+1,0));
   }
  }
  {
   DefineFloorGroup(name,descr,nentries,entries);
  }
#ifdef __cplusplus
  delete [] entries;
#else
  free(entries);
#endif
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineFloorGroup'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineItemGroup */
#ifndef TOLUA_DISABLE_tolua_editor_editor_DefineItemGroup00
static int tolua_editor_editor_DefineItemGroup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_istable(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* descr = ((const char*)  tolua_tostring(tolua_S,2,0));
  int nentries = ((int)  tolua_tonumber(tolua_S,3,0));
#ifdef __cplusplus
  char** entries = new char*[nentries];
#else
  char** entries = (char**) malloc((nentries)*sizeof(char*));
#endif
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isstringarray(tolua_S,4,nentries,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<nentries;i++)
    entries[i] = ((char*)  tolua_tofieldstring(tolua_S,4,i+1,0));
   }
  }
  {
   DefineItemGroup(name,descr,nentries,entries);
  }
#ifdef __cplusplus
  delete [] entries;
#else
  free(entries);
#endif
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineItemGroup'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineStoneGroup */
#ifndef TOLUA_DISABLE_tolua_editor_editor_DefineStoneGroup00
static int tolua_editor_editor_DefineStoneGroup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_istable(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* descr = ((const char*)  tolua_tostring(tolua_S,2,0));
  int nentries = ((int)  tolua_tonumber(tolua_S,3,0));
#ifdef __cplusplus
  char** entries = new char*[nentries];
#else
  char** entries = (char**) malloc((nentries)*sizeof(char*));
#endif
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isstringarray(tolua_S,4,nentries,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<nentries;i++)
    entries[i] = ((char*)  tolua_tofieldstring(tolua_S,4,i+1,0));
   }
  }
  {
   DefineStoneGroup(name,descr,nentries,entries);
  }
#ifdef __cplusplus
  delete [] entries;
#else
  free(entries);
#endif
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineStoneGroup'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_editor_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_module(tolua_S,"editor",0);
  tolua_beginmodule(tolua_S,"editor");
   tolua_function(tolua_S,"DefineFloorGroup",tolua_editor_editor_DefineFloorGroup00);
   tolua_function(tolua_S,"DefineItemGroup",tolua_editor_editor_DefineItemGroup00);
   tolua_function(tolua_S,"DefineStoneGroup",tolua_editor_editor_DefineStoneGroup00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_editor (lua_State* tolua_S) {
 return tolua_editor_open(tolua_S);
};
#endif

