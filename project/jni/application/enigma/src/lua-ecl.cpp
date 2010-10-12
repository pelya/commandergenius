/*
** Lua binding: px
** Generated automatically by tolua++-1.0.92 on Sun Dec 13 15:12:49 2009.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_px_open (lua_State* tolua_S);

#include "SDL.h"
#include "ecl.hh"
using namespace ecl;

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_Rect (lua_State* tolua_S)
{
 Rect* self = (Rect*) tolua_tousertype(tolua_S,1,0);
	delete self;
	return 0;
}

static int tolua_collect_GS (lua_State* tolua_S)
{
 GS* self = (GS*) tolua_tousertype(tolua_S,1,0);
	delete self;
	return 0;
}

static int tolua_collect_Font (lua_State* tolua_S)
{
 Font* self = (Font*) tolua_tousertype(tolua_S,1,0);
	delete self;
	return 0;
}

static int tolua_collect_Surface (lua_State* tolua_S)
{
 Surface* self = (Surface*) tolua_tousertype(tolua_S,1,0);
	delete self;
	return 0;
}

static int tolua_collect_Drawable (lua_State* tolua_S)
{
 Drawable* self = (Drawable*) tolua_tousertype(tolua_S,1,0);
	delete self;
	return 0;
}

static int tolua_collect_V2 (lua_State* tolua_S)
{
 V2* self = (V2*) tolua_tousertype(tolua_S,1,0);
	delete self;
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"Rect");
 tolua_usertype(tolua_S,"GS");
 tolua_usertype(tolua_S,"Font");
 tolua_usertype(tolua_S,"Drawable");
 tolua_usertype(tolua_S,"Surface");
 tolua_usertype(tolua_S,"Screen");
 tolua_usertype(tolua_S,"V2");
}

/* get function: x of class  Rect */
#ifndef TOLUA_DISABLE_tolua_get_Rect_x
static int tolua_get_Rect_x(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->x);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: x of class  Rect */
#ifndef TOLUA_DISABLE_tolua_set_Rect_x
static int tolua_set_Rect_x(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: y of class  Rect */
#ifndef TOLUA_DISABLE_tolua_get_Rect_y
static int tolua_get_Rect_y(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'y'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: y of class  Rect */
#ifndef TOLUA_DISABLE_tolua_set_Rect_y
static int tolua_set_Rect_y(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'y'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->y = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: w of class  Rect */
#ifndef TOLUA_DISABLE_tolua_get_Rect_w
static int tolua_get_Rect_w(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'w'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->w);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: w of class  Rect */
#ifndef TOLUA_DISABLE_tolua_set_Rect_w
static int tolua_set_Rect_w(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'w'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->w = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: h of class  Rect */
#ifndef TOLUA_DISABLE_tolua_get_Rect_h
static int tolua_get_Rect_h(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'h'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->h);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: h of class  Rect */
#ifndef TOLUA_DISABLE_tolua_set_Rect_h
static int tolua_set_Rect_h(lua_State* tolua_S)
{
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'h'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->h = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Rect */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Rect_new00
static int tolua_px_ecl_Rect_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Rect",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int xx = ((int)  tolua_tonumber(tolua_S,2,0));
  int yy = ((int)  tolua_tonumber(tolua_S,3,0));
  int ww = ((int)  tolua_tonumber(tolua_S,4,0));
  int hh = ((int)  tolua_tonumber(tolua_S,5,0));
  {
   Rect* tolua_ret = (Rect*)  new Rect(xx,yy,ww,hh);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Rect");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Rect */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Rect_new00_local
static int tolua_px_ecl_Rect_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Rect",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int xx = ((int)  tolua_tonumber(tolua_S,2,0));
  int yy = ((int)  tolua_tonumber(tolua_S,3,0));
  int ww = ((int)  tolua_tonumber(tolua_S,4,0));
  int hh = ((int)  tolua_tonumber(tolua_S,5,0));
  {
   Rect* tolua_ret = (Rect*)  new Rect(xx,yy,ww,hh);
   tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"Rect");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  Rect */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Rect_delete00
static int tolua_px_ecl_Rect_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Rect* self = (Rect*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'",NULL);
#endif
  delete self;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  V2 */
#ifndef TOLUA_DISABLE_tolua_px_ecl_V2_new00
static int tolua_px_ecl_V2_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"V2",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   V2* tolua_ret = (V2*)  new V2();
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"V2");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  V2 */
#ifndef TOLUA_DISABLE_tolua_px_ecl_V2_new00_local
static int tolua_px_ecl_V2_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"V2",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   V2* tolua_ret = (V2*)  new V2();
   tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"V2");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  V2 */
#ifndef TOLUA_DISABLE_tolua_px_ecl_V2_new01
static int tolua_px_ecl_V2_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"V2",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  double x = ((double)  tolua_tonumber(tolua_S,2,0));
  double y = ((double)  tolua_tonumber(tolua_S,3,0));
  {
   V2* tolua_ret = (V2*)  new V2(x,y);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"V2");
  }
 }
 return 1;
tolua_lerror:
 return tolua_px_ecl_V2_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  V2 */
#ifndef TOLUA_DISABLE_tolua_px_ecl_V2_new01_local
static int tolua_px_ecl_V2_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"V2",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  double x = ((double)  tolua_tonumber(tolua_S,2,0));
  double y = ((double)  tolua_tonumber(tolua_S,3,0));
  {
   V2* tolua_ret = (V2*)  new V2(x,y);
   tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"V2");
  }
 }
 return 1;
tolua_lerror:
 return tolua_px_ecl_V2_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  V2 */
#ifndef TOLUA_DISABLE_tolua_px_ecl_V2__geti00
static int tolua_px_ecl_V2__geti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"V2",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  V2* self = (V2*)  tolua_tousertype(tolua_S,1,0);
  int idx = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'",NULL);
#endif
  {
   double tolua_ret = (double)  self->operator[](idx);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.geti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  GS */
#ifndef TOLUA_DISABLE_tolua_px_ecl_GS_new00
static int tolua_px_ecl_GS_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GS",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Rect* clipr = ((const Rect*)  tolua_tousertype(tolua_S,2,0));
  {
   GS* tolua_ret = (GS*)  new GS(*clipr);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"GS");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  GS */
#ifndef TOLUA_DISABLE_tolua_px_ecl_GS_new00_local
static int tolua_px_ecl_GS_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GS",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Rect* clipr = ((const Rect*)  tolua_tousertype(tolua_S,2,0));
  {
   GS* tolua_ret = (GS*)  new GS(*clipr);
   tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"GS");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  GS */
#ifndef TOLUA_DISABLE_tolua_px_ecl_GS_delete00
static int tolua_px_ecl_GS_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GS",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GS* self = (GS*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'",NULL);
#endif
  delete self;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: cliprect of class  GS */
#ifndef TOLUA_DISABLE_tolua_get_GS_cliprect
static int tolua_get_GS_cliprect(lua_State* tolua_S)
{
  GS* self = (GS*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cliprect'",NULL);
#endif
  tolua_pushusertype(tolua_S,(void*)&self->cliprect,"Rect");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: cliprect of class  GS */
#ifndef TOLUA_DISABLE_tolua_set_GS_cliprect
static int tolua_set_GS_cliprect(lua_State* tolua_S)
{
  GS* self = (GS*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'cliprect'",NULL);
  if (!tolua_isusertype(tolua_S,2,"Rect",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->cliprect = *((Rect*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: pcolor of class  GS */
#ifndef TOLUA_DISABLE_tolua_get_GS_unsigned_pcolor
static int tolua_get_GS_unsigned_pcolor(lua_State* tolua_S)
{
  GS* self = (GS*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pcolor'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->pcolor);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: pcolor of class  GS */
#ifndef TOLUA_DISABLE_tolua_set_GS_unsigned_pcolor
static int tolua_set_GS_unsigned_pcolor(lua_State* tolua_S)
{
  GS* self = (GS*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'pcolor'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->pcolor = ((  unsigned int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_delete00
static int tolua_px_ecl_Drawable_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'",NULL);
#endif
  delete self;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: map_color of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_map_color00
static int tolua_px_ecl_Drawable_map_color00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  int r = ((int)  tolua_tonumber(tolua_S,2,0));
  int g = ((int)  tolua_tonumber(tolua_S,3,0));
  int b = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'map_color'",NULL);
#endif
  {
    unsigned int tolua_ret = (  unsigned int)  self->map_color(r,g,b);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'map_color'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: map_color of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_map_color01
static int tolua_px_ecl_Drawable_map_color01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  int r = ((int)  tolua_tonumber(tolua_S,2,0));
  int g = ((int)  tolua_tonumber(tolua_S,3,0));
  int b = ((int)  tolua_tonumber(tolua_S,4,0));
  int a = ((int)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'map_color'",NULL);
#endif
  {
    unsigned int tolua_ret = (  unsigned int)  self->map_color(r,g,b,a);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_px_ecl_Drawable_map_color00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: blit of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_blit00
static int tolua_px_ecl_Drawable_blit00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,5,"Surface",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
  Surface* s = ((Surface*)  tolua_tousertype(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'blit'",NULL);
#endif
  {
   self->blit(*gs,x,y,s);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'blit'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: blit of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_blit01
static int tolua_px_ecl_Drawable_blit01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,5,"Surface",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,6,"const Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,7,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
  Surface* s = ((Surface*)  tolua_tousertype(tolua_S,5,0));
  const Rect* r = ((const Rect*)  tolua_tousertype(tolua_S,6,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'blit'",NULL);
#endif
  {
   self->blit(*gs,x,y,s,*r);
  }
 }
 return 0;
tolua_lerror:
 return tolua_px_ecl_Drawable_blit00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: get_pixel of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_get_pixel00
static int tolua_px_ecl_Drawable_get_pixel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'get_pixel'",NULL);
#endif
  {
   unsigned int tolua_ret = ( unsigned int)  self->get_pixel(x,y);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_pixel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: set_pixel of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_set_pixel00
static int tolua_px_ecl_Drawable_set_pixel00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'set_pixel'",NULL);
#endif
  {
   self->set_pixel(*gs,x,y);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_pixel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: set_pixels of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_set_pixels00
static int tolua_px_ecl_Drawable_set_pixels00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int n = ((int)  tolua_tonumber(tolua_S,3,0));
  const int x = ((const int)  tolua_tonumber(tolua_S,4,0));
  const int y = ((const int)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'set_pixels'",NULL);
#endif
  {
   self->set_pixels(*gs,n,&x,&y);
   tolua_pushnumber(tolua_S,(lua_Number)x);
   tolua_pushnumber(tolua_S,(lua_Number)y);
  }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_pixels'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: hline of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_hline00
static int tolua_px_ecl_Drawable_hline00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
  int w = ((int)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'hline'",NULL);
#endif
  {
   self->hline(*gs,x,y,w);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'hline'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: vline of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_vline00
static int tolua_px_ecl_Drawable_vline00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
  int h = ((int)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'vline'",NULL);
#endif
  {
   self->vline(*gs,x,y,h);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'vline'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: line of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_line00
static int tolua_px_ecl_Drawable_line00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,7,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x1 = ((int)  tolua_tonumber(tolua_S,3,0));
  int y1 = ((int)  tolua_tonumber(tolua_S,4,0));
  int x2 = ((int)  tolua_tonumber(tolua_S,5,0));
  int y2 = ((int)  tolua_tonumber(tolua_S,6,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'line'",NULL);
#endif
  {
   self->line(*gs,x1,y1,x2,y2);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'line'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: box of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_box00
static int tolua_px_ecl_Drawable_box00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Drawable",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const GS",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,7,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Drawable* self = (Drawable*)  tolua_tousertype(tolua_S,1,0);
  const GS* gs = ((const GS*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
  int w = ((int)  tolua_tonumber(tolua_S,5,0));
  int h = ((int)  tolua_tonumber(tolua_S,6,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'box'",NULL);
#endif
  {
   self->box(*gs,x,y,w,h);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'box'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: size of class  Drawable */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Drawable_size00
static int tolua_px_ecl_Drawable_size00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Drawable",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Drawable* self = (const Drawable*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'size'",NULL);
#endif
  {
   Rect tolua_ret = (Rect)  self->size();
   {
#ifdef __cplusplus
    void* tolua_obj = new Rect(tolua_ret);
    tolua_pushusertype_and_takeownership(tolua_S,tolua_obj,"Rect");
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(Rect));
    tolua_pushusertype_and_takeownership(tolua_S,tolua_obj,"Rect");
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'size'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  Surface */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Surface_delete00
static int tolua_px_ecl_Surface_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Surface",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Surface* self = (Surface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'",NULL);
#endif
  delete self;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: width of class  Surface */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Surface_width00
static int tolua_px_ecl_Surface_width00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Surface",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Surface* self = (Surface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'width'",NULL);
#endif
  {
   int tolua_ret = (int)  self->width();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'width'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: height of class  Surface */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Surface_height00
static int tolua_px_ecl_Surface_height00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Surface",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Surface* self = (Surface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'height'",NULL);
#endif
  {
   int tolua_ret = (int)  self->height();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'height'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: get_surface of class  Screen */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Screen_get_surface00
static int tolua_px_ecl_Screen_get_surface00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Screen",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Screen* self = (Screen*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'get_surface'",NULL);
#endif
  {
   Surface* tolua_ret = (Surface*)  self->get_surface();
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_surface'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: update_all of class  Screen */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Screen_update_all00
static int tolua_px_ecl_Screen_update_all00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Screen",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Screen* self = (Screen*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'update_all'",NULL);
#endif
  {
   self->update_all();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_all'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: update_rect of class  Screen */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Screen_update_rect00
static int tolua_px_ecl_Screen_update_rect00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Screen",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const Rect",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Screen* self = (Screen*)  tolua_tousertype(tolua_S,1,0);
  const Rect* r = ((const Rect*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'update_rect'",NULL);
#endif
  {
   self->update_rect(*r);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'update_rect'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: flush_updates of class  Screen */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Screen_flush_updates00
static int tolua_px_ecl_Screen_flush_updates00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Screen",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Screen* self = (Screen*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'flush_updates'",NULL);
#endif
  {
   self->flush_updates();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'flush_updates'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: set_caption of class  Screen */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Screen_set_caption00
static int tolua_px_ecl_Screen_set_caption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Screen",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Screen* self = (Screen*)  tolua_tousertype(tolua_S,1,0);
  const char* str = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'set_caption'",NULL);
#endif
  {
   self->set_caption(str);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'set_caption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  Font */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Font_delete00
static int tolua_px_ecl_Font_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Font",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Font* self = (Font*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'",NULL);
#endif
  delete self;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: get_lineskip of class  Font */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Font_get_lineskip00
static int tolua_px_ecl_Font_get_lineskip00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Font",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Font* self = (Font*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'get_lineskip'",NULL);
#endif
  {
   int tolua_ret = (int)  self->get_lineskip();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_lineskip'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: get_height of class  Font */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Font_get_height00
static int tolua_px_ecl_Font_get_height00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Font",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Font* self = (Font*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'get_height'",NULL);
#endif
  {
   int tolua_ret = (int)  self->get_height();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_height'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: get_width of class  Font */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Font_get_width00
static int tolua_px_ecl_Font_get_width00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Font",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Font* self = (Font*)  tolua_tousertype(tolua_S,1,0);
  const char* str = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'get_width'",NULL);
#endif
  {
   int tolua_ret = (int)  self->get_width(str);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'get_width'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: render of class  Font */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Font_render00
static int tolua_px_ecl_Font_render00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Font",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Font* self = (Font*)  tolua_tousertype(tolua_S,1,0);
  const char* str = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'render'",NULL);
#endif
  {
   Surface* tolua_ret = (Surface*)  self->render(str);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'render'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: render of class  Font */
#ifndef TOLUA_DISABLE_tolua_px_ecl_Font_render01
static int tolua_px_ecl_Font_render01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Font",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Surface",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isstring(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  Font* self = (Font*)  tolua_tousertype(tolua_S,1,0);
  Surface* s = ((Surface*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
  const char* str = ((const char*)  tolua_tostring(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'render'",NULL);
#endif
  {
   self->render(s,x,y,str);
  }
 }
 return 0;
tolua_lerror:
 return tolua_px_ecl_Font_render00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_px_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_module(tolua_S,"ecl",0);
  tolua_beginmodule(tolua_S,"ecl");
   #ifdef __cplusplus
   tolua_cclass(tolua_S,"Rect","Rect","",tolua_collect_Rect);
   #else
   tolua_cclass(tolua_S,"Rect","Rect","",NULL);
   #endif
   tolua_beginmodule(tolua_S,"Rect");
    tolua_variable(tolua_S,"x",tolua_get_Rect_x,tolua_set_Rect_x);
    tolua_variable(tolua_S,"y",tolua_get_Rect_y,tolua_set_Rect_y);
    tolua_variable(tolua_S,"w",tolua_get_Rect_w,tolua_set_Rect_w);
    tolua_variable(tolua_S,"h",tolua_get_Rect_h,tolua_set_Rect_h);
    tolua_function(tolua_S,"new",tolua_px_ecl_Rect_new00);
    tolua_function(tolua_S,"new_local",tolua_px_ecl_Rect_new00_local);
    tolua_function(tolua_S,".call",tolua_px_ecl_Rect_new00_local);
    tolua_function(tolua_S,"delete",tolua_px_ecl_Rect_delete00);
   tolua_endmodule(tolua_S);
   #ifdef __cplusplus
   tolua_cclass(tolua_S,"V2","V2","",tolua_collect_V2);
   #else
   tolua_cclass(tolua_S,"V2","V2","",NULL);
   #endif
   tolua_beginmodule(tolua_S,"V2");
    tolua_function(tolua_S,"new",tolua_px_ecl_V2_new00);
    tolua_function(tolua_S,"new_local",tolua_px_ecl_V2_new00_local);
    tolua_function(tolua_S,".call",tolua_px_ecl_V2_new00_local);
    tolua_function(tolua_S,"new",tolua_px_ecl_V2_new01);
    tolua_function(tolua_S,"new_local",tolua_px_ecl_V2_new01_local);
    tolua_function(tolua_S,".call",tolua_px_ecl_V2_new01_local);
    tolua_function(tolua_S,".geti",tolua_px_ecl_V2__geti00);
   tolua_endmodule(tolua_S);
   #ifdef __cplusplus
   tolua_cclass(tolua_S,"GS","GS","",tolua_collect_GS);
   #else
   tolua_cclass(tolua_S,"GS","GS","",NULL);
   #endif
   tolua_beginmodule(tolua_S,"GS");
    tolua_function(tolua_S,"new",tolua_px_ecl_GS_new00);
    tolua_function(tolua_S,"new_local",tolua_px_ecl_GS_new00_local);
    tolua_function(tolua_S,".call",tolua_px_ecl_GS_new00_local);
    tolua_function(tolua_S,"delete",tolua_px_ecl_GS_delete00);
    tolua_variable(tolua_S,"cliprect",tolua_get_GS_cliprect,tolua_set_GS_cliprect);
    tolua_variable(tolua_S,"pcolor",tolua_get_GS_unsigned_pcolor,tolua_set_GS_unsigned_pcolor);
   tolua_endmodule(tolua_S);
   #ifdef __cplusplus
   tolua_cclass(tolua_S,"Drawable","Drawable","",tolua_collect_Drawable);
   #else
   tolua_cclass(tolua_S,"Drawable","Drawable","",NULL);
   #endif
   tolua_beginmodule(tolua_S,"Drawable");
    tolua_function(tolua_S,"delete",tolua_px_ecl_Drawable_delete00);
    tolua_function(tolua_S,"map_color",tolua_px_ecl_Drawable_map_color00);
    tolua_function(tolua_S,"map_color",tolua_px_ecl_Drawable_map_color01);
    tolua_function(tolua_S,"blit",tolua_px_ecl_Drawable_blit00);
    tolua_function(tolua_S,"blit",tolua_px_ecl_Drawable_blit01);
    tolua_function(tolua_S,"get_pixel",tolua_px_ecl_Drawable_get_pixel00);
    tolua_function(tolua_S,"set_pixel",tolua_px_ecl_Drawable_set_pixel00);
    tolua_function(tolua_S,"set_pixels",tolua_px_ecl_Drawable_set_pixels00);
    tolua_function(tolua_S,"hline",tolua_px_ecl_Drawable_hline00);
    tolua_function(tolua_S,"vline",tolua_px_ecl_Drawable_vline00);
    tolua_function(tolua_S,"line",tolua_px_ecl_Drawable_line00);
    tolua_function(tolua_S,"box",tolua_px_ecl_Drawable_box00);
    tolua_function(tolua_S,"size",tolua_px_ecl_Drawable_size00);
   tolua_endmodule(tolua_S);
   #ifdef __cplusplus
   tolua_cclass(tolua_S,"Surface","Surface","Drawable",tolua_collect_Surface);
   #else
   tolua_cclass(tolua_S,"Surface","Surface","Drawable",NULL);
   #endif
   tolua_beginmodule(tolua_S,"Surface");
    tolua_function(tolua_S,"delete",tolua_px_ecl_Surface_delete00);
    tolua_function(tolua_S,"width",tolua_px_ecl_Surface_width00);
    tolua_function(tolua_S,"height",tolua_px_ecl_Surface_height00);
   tolua_endmodule(tolua_S);
   tolua_cclass(tolua_S,"Screen","Screen","",NULL);
   tolua_beginmodule(tolua_S,"Screen");
    tolua_function(tolua_S,"get_surface",tolua_px_ecl_Screen_get_surface00);
    tolua_function(tolua_S,"update_all",tolua_px_ecl_Screen_update_all00);
    tolua_function(tolua_S,"update_rect",tolua_px_ecl_Screen_update_rect00);
    tolua_function(tolua_S,"flush_updates",tolua_px_ecl_Screen_flush_updates00);
    tolua_function(tolua_S,"set_caption",tolua_px_ecl_Screen_set_caption00);
   tolua_endmodule(tolua_S);
   #ifdef __cplusplus
   tolua_cclass(tolua_S,"Font","Font","",tolua_collect_Font);
   #else
   tolua_cclass(tolua_S,"Font","Font","",NULL);
   #endif
   tolua_beginmodule(tolua_S,"Font");
    tolua_function(tolua_S,"delete",tolua_px_ecl_Font_delete00);
    tolua_function(tolua_S,"get_lineskip",tolua_px_ecl_Font_get_lineskip00);
    tolua_function(tolua_S,"get_height",tolua_px_ecl_Font_get_height00);
    tolua_function(tolua_S,"get_width",tolua_px_ecl_Font_get_width00);
    tolua_function(tolua_S,"render",tolua_px_ecl_Font_render00);
    tolua_function(tolua_S,"render",tolua_px_ecl_Font_render01);
   tolua_endmodule(tolua_S);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_px (lua_State* tolua_S) {
 return tolua_px_open(tolua_S);
};
#endif

