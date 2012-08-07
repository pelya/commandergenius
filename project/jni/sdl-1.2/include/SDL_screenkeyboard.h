/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifndef _SDL_screenkeyboard_h
#define _SDL_screenkeyboard_h

#include "SDL_stdinc.h"
#include "SDL_video.h"
#include "SDL_version.h"
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_scancode.h"
#else
#include "SDL_keysym.h"
#endif

/* On-screen keyboard exposed to the application, it's available on Android platform only */

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Button IDs */
enum { 

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD = 0, /* Joystick/D-Pad button */

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, /* Main (usually Fire) button */
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_1,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_2,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_3,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_4,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_5,

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT, /* Button to show screen keyboard */

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM
};

/* All functions return 0 on failure and 1 on success, contrary to other SDL API.
   On the other hand, those functions actually never fail, so you may skip error checking. */

/* Set on-screen button position, specify zero width to hide the button.
   All coordinates are in the actual screen dimensions, NOT what you are supplying to SDL_SetVideoMode(),
   use SDL_ListModes()[0] to determine the actual screen boundaries. */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos);
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos);

extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonKey(int buttonId,
#if SDL_VERSION_ATLEAST(1,3,0)
                                                                                 SDL_Keycode
#else
                                                                                 SDLKey
#endif
                                                                                             key);
/* Returns SDLK_UNKNOWN on failure */
extern DECLSPEC
#if SDL_VERSION_ATLEAST(1,3,0)
                SDL_Keycode
#else
                SDLKey
#endif
                            SDLCALL SDL_ANDROID_GetScreenKeyboardButtonKey(int buttonId);

/* Buttons 0 and 1 may have auto-fire state */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardAutoFireButtonsAmount(int nbuttons);
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardAutoFireButtonsAmount();

extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardShown(int shown);
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardShown();

extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardSize();

/* Show Android on-screen keyboard, and pass entered text back to application as SDL keypress events,
previousText is UTF-8 encoded, it may be NULL, only 256 first bytes will be used, and this call will not block */
extern DECLSPEC int SDLCALL SDL_ANDROID_ToggleScreenKeyboardTextInput(const char * previousText);

/* Show Android on-screen keyboard, and pass entered text back to application in a buffer,
using buffer contents as previous text (UTF-8 encoded), the buffer may be of any size -
this call will block until user typed all text. */
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardTextInput(char * textBuf, int textBufSize);

/* Whether user redefined on-screen keyboard layout via SDL menu, app should not enforce it's own layout in that case */
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardRedefinedByUser();

/* Show only the bare Android on-screen keyboard without any text input field */
extern DECLSPEC void SDLCALL SDL_ANDROID_CallJavaTogglePlainAndroidSoftKeyboardInput();

#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif
