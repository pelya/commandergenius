/*
** Lua binding: display
** Generated automatically by tolua++-1.0.92 on Sun Dec 13 15:12:49 2009.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_display_open (lua_State* tolua_S);

#include "display.hh"
#include "d_models.hh"
using namespace display;
using ecl::Rect;
using ecl::Surface;

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"Rect");
 tolua_usertype(tolua_S,"Surface");
}

/* function: GetSurface */
#ifndef TOLUA_DISABLE_tolua_display_GetSurface00
static int tolua_display_GetSurface00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   Surface* tolua_ret = (Surface*)  GetSurface(name);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetSurface'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: CropSurface */
#ifndef TOLUA_DISABLE_tolua_display_CropSurface00
static int tolua_display_CropSurface00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Surface",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Surface* s = ((const Surface*)  tolua_tousertype(tolua_S,1,0));
  Rect r = *((Rect*)  tolua_tousertype(tolua_S,2,0));
  {
   Surface* tolua_ret = (Surface*)  CropSurface(s,r);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'CropSurface'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineImageModel */
#ifndef TOLUA_DISABLE_tolua_display_DefineImageModel00
static int tolua_display_DefineImageModel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Surface",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  Surface* s = ((Surface*)  tolua_tousertype(tolua_S,2,0));
  {
   DefineImageModel(name,s);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineImageModel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetFollowMode */
#ifndef TOLUA_DISABLE_tolua_display_display_SetFollowMode00
static int tolua_display_display_SetFollowMode00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  FollowMode m = ((FollowMode) (int)  tolua_tonumber(tolua_S,1,0));
  {
   SetFollowMode(m);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetFollowMode'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetScrollBoundary */
#ifndef TOLUA_DISABLE_tolua_display_display_SetScrollBoundary00
static int tolua_display_display_SetScrollBoundary00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  double boundary = ((double)  tolua_tonumber(tolua_S,1,0));
  {
   SetScrollBoundary(boundary);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetScrollBoundary'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ResizeGameArea */
#ifndef TOLUA_DISABLE_tolua_display_display_ResizeGameArea00
static int tolua_display_display_ResizeGameArea00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int w = ((int)  tolua_tonumber(tolua_S,1,0));
  int h = ((int)  tolua_tonumber(tolua_S,2,0));
  {
   ResizeGameArea(w,h);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ResizeGameArea'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineImage */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineImage00
static int tolua_display_display_DefineImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* fname = ((const char*)  tolua_tostring(tolua_S,2,0));
  int xoff = ((int)  tolua_tonumber(tolua_S,3,0));
  int yoff = ((int)  tolua_tonumber(tolua_S,4,0));
  int padding = ((int)  tolua_tonumber(tolua_S,5,0));
  {
   int tolua_ret = (int)  DefineImage(name,fname,xoff,yoff,padding);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineSubImage */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineSubImage00
static int tolua_display_display_DefineSubImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,5,"Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* fname = ((const char*)  tolua_tostring(tolua_S,2,0));
  int xoff = ((int)  tolua_tonumber(tolua_S,3,0));
  int yoff = ((int)  tolua_tonumber(tolua_S,4,0));
  Rect r = *((Rect*)  tolua_tousertype(tolua_S,5,0));
  {
   int tolua_ret = (int)  DefineSubImage(name,fname,xoff,yoff,r);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineSubImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineRandModel */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineRandModel00
static int tolua_display_display_DefineRandModel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_istable(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  int n = ((int)  tolua_tonumber(tolua_S,2,0));
#ifdef __cplusplus
  char** names = new char*[n];
#else
  char** names = (char**) malloc((n)*sizeof(char*));
#endif
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isstringarray(tolua_S,3,n,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<n;i++)
    names[i] = ((char*)  tolua_tofieldstring(tolua_S,3,i+1,0));
   }
  }
  {
   DefineRandModel(name,n,names);
  }
#ifdef __cplusplus
  delete [] names;
#else
  free(names);
#endif
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineRandModel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineShadedModel */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineShadedModel00
static int tolua_display_display_DefineShadedModel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* model = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* shade = ((const char*)  tolua_tostring(tolua_S,3,0));
  {
   DefineShadedModel(name,model,shade);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineShadedModel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineOverlayImage */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineOverlayImage00
static int tolua_display_display_DefineOverlayImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_istable(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  int n = ((int)  tolua_tonumber(tolua_S,2,0));
#ifdef __cplusplus
  char** images = new char*[n];
#else
  char** images = (char**) malloc((n)*sizeof(char*));
#endif
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isstringarray(tolua_S,3,n,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<n;i++)
    images[i] = ((char*)  tolua_tofieldstring(tolua_S,3,i+1,0));
   }
  }
  {
   DefineOverlayImage(name,n,images);
  }
#ifdef __cplusplus
  delete [] images;
#else
  free(images);
#endif
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineOverlayImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineComposite */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineComposite00
static int tolua_display_display_DefineComposite00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* bgname = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* fgname = ((const char*)  tolua_tostring(tolua_S,3,0));
  {
   DefineComposite(name,bgname,fgname);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineComposite'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineAnim */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineAnim00
static int tolua_display_display_DefineAnim00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  bool loop_p = ((bool)  tolua_toboolean(tolua_S,2,0));
  {
   DefineAnim(name,loop_p);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineAnim'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: AddFrame */
#ifndef TOLUA_DISABLE_tolua_display_display_AddFrame00
static int tolua_display_display_AddFrame00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* model = ((const char*)  tolua_tostring(tolua_S,2,0));
  double time = ((double)  tolua_tonumber(tolua_S,3,0));
  {
   AddFrame(name,model,time);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddFrame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineAlias */
#ifndef TOLUA_DISABLE_tolua_display_display_DefineAlias00
static int tolua_display_display_DefineAlias00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* othername = ((const char*)  tolua_tostring(tolua_S,2,0));
  {
   DefineAlias(name,othername);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineAlias'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_display_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_function(tolua_S,"GetSurface",tolua_display_GetSurface00);
  tolua_function(tolua_S,"CropSurface",tolua_display_CropSurface00);
  tolua_function(tolua_S,"DefineImageModel",tolua_display_DefineImageModel00);
  tolua_module(tolua_S,"display",0);
  tolua_beginmodule(tolua_S,"display");
   tolua_constant(tolua_S,"FOLLOW_SCROLLING",FOLLOW_SCROLLING);
   tolua_constant(tolua_S,"FOLLOW_SCREEN",FOLLOW_SCREEN);
   tolua_constant(tolua_S,"FOLLOW_SCREENSCROLLING",FOLLOW_SCREENSCROLLING);
   tolua_constant(tolua_S,"FOLLOW_SMOOTH",FOLLOW_SMOOTH);
   tolua_function(tolua_S,"SetFollowMode",tolua_display_display_SetFollowMode00);
   tolua_function(tolua_S,"SetScrollBoundary",tolua_display_display_SetScrollBoundary00);
   tolua_function(tolua_S,"ResizeGameArea",tolua_display_display_ResizeGameArea00);
   tolua_function(tolua_S,"DefineImage",tolua_display_display_DefineImage00);
   tolua_function(tolua_S,"DefineSubImage",tolua_display_display_DefineSubImage00);
   tolua_function(tolua_S,"DefineRandModel",tolua_display_display_DefineRandModel00);
   tolua_function(tolua_S,"DefineShadedModel",tolua_display_display_DefineShadedModel00);
   tolua_function(tolua_S,"DefineOverlayImage",tolua_display_display_DefineOverlayImage00);
   tolua_function(tolua_S,"DefineComposite",tolua_display_display_DefineComposite00);
   tolua_function(tolua_S,"DefineAnim",tolua_display_display_DefineAnim00);
   tolua_function(tolua_S,"AddFrame",tolua_display_display_AddFrame00);
   tolua_function(tolua_S,"DefineAlias",tolua_display_display_DefineAlias00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_display (lua_State* tolua_S) {
 return tolua_display_open(tolua_S);
};
#endif

