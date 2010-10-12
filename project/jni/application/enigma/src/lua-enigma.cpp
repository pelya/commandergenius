/*
** Lua binding: enigma
** Generated automatically by tolua++-1.0.92 on Sun Dec 13 15:12:49 2009.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_enigma_open (lua_State* tolua_S);

#include "server.hh"
#include "world.hh"
#include "lev/PersistentIndex.hh"
using namespace enigma;
using ecl::Surface;
using ecl::Font;
using namespace server;
using namespace lev;
using namespace world;
#include "video.hh"
using namespace video;
using ecl::Screen;
#include "sound.hh"
using namespace sound;

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"Stone");
 tolua_usertype(tolua_S,"Font");
 tolua_usertype(tolua_S,"Object");
 tolua_usertype(tolua_S,"Floor");
 tolua_usertype(tolua_S,"Item");
 tolua_usertype(tolua_S,"Screen");
 tolua_usertype(tolua_S,"Surface");
}

/* function: GetImage */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_GetImage00
static int tolua_enigma_enigma_GetImage00(lua_State* tolua_S)
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
   Surface* tolua_ret = (Surface*)  GetImage(name);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetImage */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_GetImage01
static int tolua_enigma_enigma_GetImage01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* extension = ((const char*)  tolua_tostring(tolua_S,2,0));
  {
   Surface* tolua_ret = (Surface*)  GetImage(name,extension);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
tolua_lerror:
 return tolua_enigma_enigma_GetImage00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* function: LoadImage */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_LoadImage00
static int tolua_enigma_enigma_LoadImage00(lua_State* tolua_S)
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
   Surface* tolua_ret = (Surface*)  LoadImage(name);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'LoadImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: RegisterImage */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_RegisterImage00
static int tolua_enigma_enigma_RegisterImage00(lua_State* tolua_S)
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
   Surface* tolua_ret = (Surface*)  RegisterImage(name,s);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Surface");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'RegisterImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetFont */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_GetFont00
static int tolua_enigma_enigma_GetFont00(lua_State* tolua_S)
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
   Font* tolua_ret = (Font*)  GetFont(name);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Font");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetFont'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: date */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_date00
static int tolua_enigma_enigma_date00(lua_State* tolua_S)
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
  const char* format = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   const char* tolua_ret = (const char*)  date(format);
   tolua_pushstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'date'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TwoPlayerGame */
#ifndef TOLUA_DISABLE_tolua_get_enigma_TwoPlayerGame
static int tolua_get_enigma_TwoPlayerGame(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)TwoPlayerGame);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TwoPlayerGame */
#ifndef TOLUA_DISABLE_tolua_set_enigma_TwoPlayerGame
static int tolua_set_enigma_TwoPlayerGame(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  TwoPlayerGame = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: SingleComputerGame */
#ifndef TOLUA_DISABLE_tolua_get_enigma_SingleComputerGame
static int tolua_get_enigma_SingleComputerGame(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)SingleComputerGame);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: SingleComputerGame */
#ifndef TOLUA_DISABLE_tolua_set_enigma_SingleComputerGame
static int tolua_set_enigma_SingleComputerGame(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SingleComputerGame = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ConserveLevel */
#ifndef TOLUA_DISABLE_tolua_get_enigma_ConserveLevel
static int tolua_get_enigma_ConserveLevel(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)ConserveLevel);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ConserveLevel */
#ifndef TOLUA_DISABLE_tolua_set_enigma_ConserveLevel
static int tolua_set_enigma_ConserveLevel(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  ConserveLevel = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AllowTogglePlayer */
#ifndef TOLUA_DISABLE_tolua_get_enigma_AllowTogglePlayer
static int tolua_get_enigma_AllowTogglePlayer(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)AllowTogglePlayer);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AllowTogglePlayer */
#ifndef TOLUA_DISABLE_tolua_set_enigma_AllowTogglePlayer
static int tolua_set_enigma_AllowTogglePlayer(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  AllowTogglePlayer = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowMoves */
#ifndef TOLUA_DISABLE_tolua_get_enigma_ShowMoves
static int tolua_get_enigma_ShowMoves(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)ShowMoves);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowMoves */
#ifndef TOLUA_DISABLE_tolua_set_enigma_ShowMoves
static int tolua_set_enigma_ShowMoves(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  ShowMoves = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Brittleness */
#ifndef TOLUA_DISABLE_tolua_get_enigma_Brittleness
static int tolua_get_enigma_Brittleness(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)Brittleness);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Brittleness */
#ifndef TOLUA_DISABLE_tolua_set_enigma_Brittleness
static int tolua_set_enigma_Brittleness(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  Brittleness = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: SlopeForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_SlopeForce
static int tolua_get_enigma_SlopeForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)SlopeForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: SlopeForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_SlopeForce
static int tolua_set_enigma_SlopeForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SlopeForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: FlatForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_FlatForce
static int tolua_get_enigma_FlatForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)FlatForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: FlatForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_FlatForce
static int tolua_set_enigma_FlatForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  FlatForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: FrictionFactor */
#ifndef TOLUA_DISABLE_tolua_get_enigma_FrictionFactor
static int tolua_get_enigma_FrictionFactor(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)FrictionFactor);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: FrictionFactor */
#ifndef TOLUA_DISABLE_tolua_set_enigma_FrictionFactor
static int tolua_set_enigma_FrictionFactor(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  FrictionFactor = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: IceFriction */
#ifndef TOLUA_DISABLE_tolua_get_enigma_IceFriction
static int tolua_get_enigma_IceFriction(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)IceFriction);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: IceFriction */
#ifndef TOLUA_DISABLE_tolua_set_enigma_IceFriction
static int tolua_set_enigma_IceFriction(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  IceFriction = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ElectricForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_ElectricForce
static int tolua_get_enigma_ElectricForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)ElectricForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ElectricForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_ElectricForce
static int tolua_set_enigma_ElectricForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  ElectricForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: BumperForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_BumperForce
static int tolua_get_enigma_BumperForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)BumperForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: BumperForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_BumperForce
static int tolua_set_enigma_BumperForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  BumperForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagnetForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_MagnetForce
static int tolua_get_enigma_MagnetForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)MagnetForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagnetForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_MagnetForce
static int tolua_set_enigma_MagnetForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  MagnetForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagnetRange */
#ifndef TOLUA_DISABLE_tolua_get_enigma_MagnetRange
static int tolua_get_enigma_MagnetRange(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)MagnetRange);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagnetRange */
#ifndef TOLUA_DISABLE_tolua_set_enigma_MagnetRange
static int tolua_set_enigma_MagnetRange(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  MagnetRange = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: WaterSinkSpeed */
#ifndef TOLUA_DISABLE_tolua_get_enigma_WaterSinkSpeed
static int tolua_get_enigma_WaterSinkSpeed(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)WaterSinkSpeed);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: WaterSinkSpeed */
#ifndef TOLUA_DISABLE_tolua_set_enigma_WaterSinkSpeed
static int tolua_set_enigma_WaterSinkSpeed(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  WaterSinkSpeed = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: SwampSinkSpeed */
#ifndef TOLUA_DISABLE_tolua_get_enigma_SwampSinkSpeed
static int tolua_get_enigma_SwampSinkSpeed(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)SwampSinkSpeed);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: SwampSinkSpeed */
#ifndef TOLUA_DISABLE_tolua_set_enigma_SwampSinkSpeed
static int tolua_set_enigma_SwampSinkSpeed(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SwampSinkSpeed = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: WormholeForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_WormholeForce
static int tolua_get_enigma_WormholeForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)WormholeForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: WormholeForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_WormholeForce
static int tolua_set_enigma_WormholeForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  WormholeForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: WormholeRange */
#ifndef TOLUA_DISABLE_tolua_get_enigma_WormholeRange
static int tolua_get_enigma_WormholeRange(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)WormholeRange);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: WormholeRange */
#ifndef TOLUA_DISABLE_tolua_set_enigma_WormholeRange
static int tolua_set_enigma_WormholeRange(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  WormholeRange = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: HoleForce */
#ifndef TOLUA_DISABLE_tolua_get_enigma_HoleForce
static int tolua_get_enigma_HoleForce(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)HoleForce);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: HoleForce */
#ifndef TOLUA_DISABLE_tolua_set_enigma_HoleForce
static int tolua_set_enigma_HoleForce(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  HoleForce = ((double)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: CreatingPreview */
#ifndef TOLUA_DISABLE_tolua_get_enigma_CreatingPreview
static int tolua_get_enigma_CreatingPreview(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)CreatingPreview);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CreatingPreview */
#ifndef TOLUA_DISABLE_tolua_set_enigma_CreatingPreview
static int tolua_set_enigma_CreatingPreview(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  CreatingPreview = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: AddLevelPack */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_AddLevelPack00
static int tolua_enigma_enigma_AddLevelPack00(lua_State* tolua_S)
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
  const char* init_file = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* name = ((const char*)  tolua_tostring(tolua_S,2,0));
  {
   AddLevelPack(init_file,name);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddLevelPack'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: AddZippedLevelPack */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_AddZippedLevelPack00
static int tolua_enigma_enigma_AddZippedLevelPack00(lua_State* tolua_S)
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
  const char* zipfile = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   AddZippedLevelPack(zipfile);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddZippedLevelPack'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetCompatibility */
#ifndef TOLUA_DISABLE_tolua_enigma_enigma_SetCompatibility00
static int tolua_enigma_enigma_SetCompatibility00(lua_State* tolua_S)
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
  const char* version = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   SetCompatibility(version);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetCompatibility'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: MakeObject */
#ifndef TOLUA_DISABLE_tolua_enigma_world_MakeObject00
static int tolua_enigma_world_MakeObject00(lua_State* tolua_S)
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
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   Object* tolua_ret = (Object*)  MakeObject(kind);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Object");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MakeObject'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: MakeFloor */
#ifndef TOLUA_DISABLE_tolua_enigma_world_MakeFloor00
static int tolua_enigma_world_MakeFloor00(lua_State* tolua_S)
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
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   Floor* tolua_ret = (Floor*)  MakeFloor(kind);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Floor");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MakeFloor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: MakeItem */
#ifndef TOLUA_DISABLE_tolua_enigma_world_MakeItem00
static int tolua_enigma_world_MakeItem00(lua_State* tolua_S)
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
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   Item* tolua_ret = (Item*)  MakeItem(kind);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Item");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MakeItem'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: MakeStone */
#ifndef TOLUA_DISABLE_tolua_enigma_world_MakeStone00
static int tolua_enigma_world_MakeStone00(lua_State* tolua_S)
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
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  {
   Stone* tolua_ret = (Stone*)  MakeStone(kind);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Stone");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'MakeStone'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetNamedObject */
#ifndef TOLUA_DISABLE_tolua_enigma_world_GetNamedObject00
static int tolua_enigma_world_GetNamedObject00(lua_State* tolua_S)
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
   Object* tolua_ret = (Object*)  GetNamedObject(name);
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Object");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetNamedObject'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Resize */
#ifndef TOLUA_DISABLE_tolua_enigma_world_Resize00
static int tolua_enigma_world_Resize00(lua_State* tolua_S)
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
   Resize(w,h);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Resize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineSimpleStone */
#ifndef TOLUA_DISABLE_tolua_enigma_world_DefineSimpleStone00
static int tolua_enigma_world_DefineSimpleStone00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* sound = ((const char*)  tolua_tostring(tolua_S,2,0));
  int hollow = ((int)  tolua_tonumber(tolua_S,3,0));
  int glass = ((int)  tolua_tonumber(tolua_S,4,0));
  {
   DefineSimpleStone(kind,sound,hollow,glass);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineSimpleStone'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineSimpleStoneMovable */
#ifndef TOLUA_DISABLE_tolua_enigma_world_DefineSimpleStoneMovable00
static int tolua_enigma_world_DefineSimpleStoneMovable00(lua_State* tolua_S)
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
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* sound = ((const char*)  tolua_tostring(tolua_S,2,0));
  int glass = ((int)  tolua_tonumber(tolua_S,3,0));
  {
   DefineSimpleStoneMovable(kind,sound,glass);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineSimpleStoneMovable'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineSimpleFloor */
#ifndef TOLUA_DISABLE_tolua_enigma_world_DefineSimpleFloor00
static int tolua_enigma_world_DefineSimpleFloor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,4,0,&tolua_err) ||
     !tolua_isstring(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* kind = ((const char*)  tolua_tostring(tolua_S,1,0));
  double friction = ((double)  tolua_tonumber(tolua_S,2,0));
  double mousefactor = ((double)  tolua_tonumber(tolua_S,3,0));
  bool burnable = ((bool)  tolua_toboolean(tolua_S,4,0));
  const char* firetransform = ((const char*)  tolua_tostring(tolua_S,5,0));
  {
   DefineSimpleFloor(kind,friction,mousefactor,burnable,firetransform);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineSimpleFloor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetScreen */
#ifndef TOLUA_DISABLE_tolua_enigma_video_GetScreen00
static int tolua_enigma_video_GetScreen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Screen* tolua_ret = (Screen*)  GetScreen();
   tolua_pushusertype(tolua_S,(void*)tolua_ret,"Screen");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetScreen'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: HideMouse */
#ifndef TOLUA_DISABLE_tolua_enigma_video_HideMouse00
static int tolua_enigma_video_HideMouse00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   HideMouse();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'HideMouse'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ShowMouse */
#ifndef TOLUA_DISABLE_tolua_enigma_video_ShowMouse00
static int tolua_enigma_video_ShowMouse00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   ShowMouse();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ShowMouse'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: DefineSoundEffect */
#ifndef TOLUA_DISABLE_tolua_enigma_sound_DefineSoundEffect00
static int tolua_enigma_sound_DefineSoundEffect00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,5,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,6,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,8,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,9,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,10,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,11,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,12,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,13,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,14,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  string table = ((string)  tolua_tocppstring(tolua_S,1,0));
  string name = ((string)  tolua_tocppstring(tolua_S,2,0));
  string filename = ((string)  tolua_tocppstring(tolua_S,3,0));
  double volume = ((double)  tolua_tonumber(tolua_S,4,0));
  bool loop = ((bool)  tolua_toboolean(tolua_S,5,0));
  bool global = ((bool)  tolua_toboolean(tolua_S,6,0));
  int priority = ((int)  tolua_tonumber(tolua_S,7,0));
  double damp_max = ((double)  tolua_tonumber(tolua_S,8,0));
  double damp_inc = ((double)  tolua_tonumber(tolua_S,9,0));
  double damp_mult = ((double)  tolua_tonumber(tolua_S,10,0));
  double damp_min = ((double)  tolua_tonumber(tolua_S,11,0));
  double damp_tick = ((double)  tolua_tonumber(tolua_S,12,0));
  string silence_string = ((string)  tolua_tocppstring(tolua_S,13,0));
  {
   DefineSoundEffect(table,name,filename,volume,loop,global,priority,damp_max,damp_inc,damp_mult,damp_min,damp_tick,silence_string);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'DefineSoundEffect'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetActiveSoundSet */
#ifndef TOLUA_DISABLE_tolua_enigma_sound_SetActiveSoundSet00
static int tolua_enigma_sound_SetActiveSoundSet00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  string table = ((string)  tolua_tocppstring(tolua_S,1,0));
  {
   SetActiveSoundSet(table);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetActiveSoundSet'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_enigma_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_module(tolua_S,"enigma",0);
  tolua_beginmodule(tolua_S,"enigma");
   tolua_constant(tolua_S,"NODIR",NODIR);
   tolua_constant(tolua_S,"NORTH",NORTH);
   tolua_constant(tolua_S,"EAST",EAST);
   tolua_constant(tolua_S,"SOUTH",SOUTH);
   tolua_constant(tolua_S,"WEST",WEST);
   tolua_function(tolua_S,"GetImage",tolua_enigma_enigma_GetImage00);
   tolua_function(tolua_S,"GetImage",tolua_enigma_enigma_GetImage01);
   tolua_function(tolua_S,"LoadImage",tolua_enigma_enigma_LoadImage00);
   tolua_function(tolua_S,"RegisterImage",tolua_enigma_enigma_RegisterImage00);
   tolua_function(tolua_S,"GetFont",tolua_enigma_enigma_GetFont00);
   tolua_function(tolua_S,"date",tolua_enigma_enigma_date00);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"enigma",1);
  tolua_beginmodule(tolua_S,"enigma");
   tolua_variable(tolua_S,"TwoPlayerGame",tolua_get_enigma_TwoPlayerGame,tolua_set_enigma_TwoPlayerGame);
   tolua_variable(tolua_S,"SingleComputerGame",tolua_get_enigma_SingleComputerGame,tolua_set_enigma_SingleComputerGame);
   tolua_variable(tolua_S,"ConserveLevel",tolua_get_enigma_ConserveLevel,tolua_set_enigma_ConserveLevel);
   tolua_variable(tolua_S,"AllowTogglePlayer",tolua_get_enigma_AllowTogglePlayer,tolua_set_enigma_AllowTogglePlayer);
   tolua_variable(tolua_S,"ShowMoves",tolua_get_enigma_ShowMoves,tolua_set_enigma_ShowMoves);
   tolua_variable(tolua_S,"Brittleness",tolua_get_enigma_Brittleness,tolua_set_enigma_Brittleness);
   tolua_variable(tolua_S,"SlopeForce",tolua_get_enigma_SlopeForce,tolua_set_enigma_SlopeForce);
   tolua_variable(tolua_S,"FlatForce",tolua_get_enigma_FlatForce,tolua_set_enigma_FlatForce);
   tolua_variable(tolua_S,"FrictionFactor",tolua_get_enigma_FrictionFactor,tolua_set_enigma_FrictionFactor);
   tolua_variable(tolua_S,"IceFriction",tolua_get_enigma_IceFriction,tolua_set_enigma_IceFriction);
   tolua_variable(tolua_S,"ElectricForce",tolua_get_enigma_ElectricForce,tolua_set_enigma_ElectricForce);
   tolua_variable(tolua_S,"BumperForce",tolua_get_enigma_BumperForce,tolua_set_enigma_BumperForce);
   tolua_variable(tolua_S,"MagnetForce",tolua_get_enigma_MagnetForce,tolua_set_enigma_MagnetForce);
   tolua_variable(tolua_S,"MagnetRange",tolua_get_enigma_MagnetRange,tolua_set_enigma_MagnetRange);
   tolua_variable(tolua_S,"WaterSinkSpeed",tolua_get_enigma_WaterSinkSpeed,tolua_set_enigma_WaterSinkSpeed);
   tolua_variable(tolua_S,"SwampSinkSpeed",tolua_get_enigma_SwampSinkSpeed,tolua_set_enigma_SwampSinkSpeed);
   tolua_variable(tolua_S,"WormholeForce",tolua_get_enigma_WormholeForce,tolua_set_enigma_WormholeForce);
   tolua_variable(tolua_S,"WormholeRange",tolua_get_enigma_WormholeRange,tolua_set_enigma_WormholeRange);
   tolua_variable(tolua_S,"HoleForce",tolua_get_enigma_HoleForce,tolua_set_enigma_HoleForce);
   tolua_variable(tolua_S,"CreatingPreview",tolua_get_enigma_CreatingPreview,tolua_set_enigma_CreatingPreview);
   tolua_function(tolua_S,"AddLevelPack",tolua_enigma_enigma_AddLevelPack00);
   tolua_function(tolua_S,"AddZippedLevelPack",tolua_enigma_enigma_AddZippedLevelPack00);
   tolua_function(tolua_S,"SetCompatibility",tolua_enigma_enigma_SetCompatibility00);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"world",0);
  tolua_beginmodule(tolua_S,"world");
   tolua_cclass(tolua_S,"Object","Object","",NULL);
   tolua_beginmodule(tolua_S,"Object");
   tolua_endmodule(tolua_S);
   tolua_cclass(tolua_S,"Floor","Floor","Object",NULL);
   tolua_beginmodule(tolua_S,"Floor");
   tolua_endmodule(tolua_S);
   tolua_cclass(tolua_S,"Item","Item","Object",NULL);
   tolua_beginmodule(tolua_S,"Item");
   tolua_endmodule(tolua_S);
   tolua_cclass(tolua_S,"Stone","Stone","Object",NULL);
   tolua_beginmodule(tolua_S,"Stone");
   tolua_endmodule(tolua_S);
   tolua_function(tolua_S,"MakeObject",tolua_enigma_world_MakeObject00);
   tolua_function(tolua_S,"MakeFloor",tolua_enigma_world_MakeFloor00);
   tolua_function(tolua_S,"MakeItem",tolua_enigma_world_MakeItem00);
   tolua_function(tolua_S,"MakeStone",tolua_enigma_world_MakeStone00);
   tolua_function(tolua_S,"GetNamedObject",tolua_enigma_world_GetNamedObject00);
   tolua_function(tolua_S,"Resize",tolua_enigma_world_Resize00);
   tolua_function(tolua_S,"DefineSimpleStone",tolua_enigma_world_DefineSimpleStone00);
   tolua_function(tolua_S,"DefineSimpleStoneMovable",tolua_enigma_world_DefineSimpleStoneMovable00);
   tolua_function(tolua_S,"DefineSimpleFloor",tolua_enigma_world_DefineSimpleFloor00);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"video",0);
  tolua_beginmodule(tolua_S,"video");
   tolua_function(tolua_S,"GetScreen",tolua_enigma_video_GetScreen00);
   tolua_function(tolua_S,"HideMouse",tolua_enigma_video_HideMouse00);
   tolua_function(tolua_S,"ShowMouse",tolua_enigma_video_ShowMouse00);
  tolua_endmodule(tolua_S);
  tolua_module(tolua_S,"sound",0);
  tolua_beginmodule(tolua_S,"sound");
   tolua_function(tolua_S,"DefineSoundEffect",tolua_enigma_sound_DefineSoundEffect00);
   tolua_function(tolua_S,"SetActiveSoundSet",tolua_enigma_sound_SetActiveSoundSet00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_enigma (lua_State* tolua_S) {
 return tolua_enigma_open(tolua_S);
};
#endif

