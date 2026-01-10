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
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, /* Main (usually Fire) button */
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_1,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_2,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_3,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_4,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_5,

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT, /* Button to show screen keyboard */

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, /* Joystick/D-Pad button */
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2, /* Second joystick button */
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3, /* Third joystick button */

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_6, /* Has the same graphics as the first 6 keys */
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_7,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_8,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_9,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_10,
	SDL_ANDROID_SCREENKEYBOARD_BUTTON_11,

	SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM
};

/* All functions return 0 on failure and 1 on success, contrary to other SDL API.
   On the other hand, those functions actually never fail, so you may skip error checking. */

/* Set on-screen button position, specify zero width to hide the button.
   All coordinates are in the actual screen dimensions, NOT what you are supplying to SDL_SetVideoMode(),
   use SDL_ListModes(NULL, 0)[0] to determine the actual screen boundaries. */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos);
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos);

/* Set button image position - it can be smaller than the button area */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonImagePos(int buttonId, SDL_Rect * pos);

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

/* Hide all on-screen buttons (not the QWERTY text input) */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardShown(int shown);
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardShown(void);
/* Hide individual buttons - this will just put button width and height to 0, but it will save previous button size */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonShown(int buttonId, int shown);
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardButtonShown(int buttonId);
/* Get the button size modifier, as configured by user with SDL startup menu */
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardSize(void);

/* Set a particular button to pass a mouse/multitouch events down to the application, by default all buttons block touch events */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonGenerateTouchEvents(int buttonId, int generateEvents);

/* Prevent a button from sharing touch events with other buttons, if they overlap */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardPreventButtonOverlap(int prevent);

/* Configure a button to stay pressed after touch, and un-press after second touch, to emulate Ctrl/Alt/Shift keys  */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonStayPressedAfterTouch(int buttonId, int stayPressed);

/* Enable or disable floating joystick, the joystick is hidden after you disable floating joystick,
   you should set it's coordinates with SDL_ANDROID_SetScreenKeyboardButtonPos(). */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardFloatingJoystick(int enabled);

/* Set screen keyboard transparency, 255 or SDL_ALPHA_OPAQUE is non-transparent, 0 or SDL_ALPHA_TRANSPARENT is transparent */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardTransparency(int alpha);

/* Show Android QWERTY keyboard, and pass entered text back to application as SDL keypress events,
   previousText is UTF-8 encoded, it may be NULL, only 256 first bytes will be used, and this call will not block */
extern DECLSPEC int SDLCALL SDL_ANDROID_ToggleScreenKeyboardTextInput(const char * previousText);

/* Show only the bare Android QWERTY keyboard without any text input field, so it won't cover the screen */
extern DECLSPEC int SDLCALL SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput(void);

typedef enum
{
	SDL_KEYBOARD_QWERTY = 1,
	SDL_KEYBOARD_COMMODORE = 2,
	SDL_KEYBOARD_AMIGA = 3,
	SDL_KEYBOARD_ATARI800 = 4,
} SDL_InternalKeyboard_t;
/* Show internal keyboard, built into SDL */
extern DECLSPEC int SDLCALL SDL_ANDROID_ToggleInternalScreenKeyboard(SDL_InternalKeyboard_t keyboard);

/* Show Android QWERTY keyboard, and pass entered text back to application in a buffer,
   using buffer contents as previous text (UTF-8 encoded), the buffer may be of any size.
   This function will block until user typed all text. */
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardTextInput(char * textBuf, int textBufSize);

typedef enum
{
	SDL_ANDROID_TEXTINPUT_ASYNC_IN_PROGRESS = 0,
	SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED = 1,
} SDL_AndroidTextInputAsyncStatus_t;

/* Show Android QWERTY keyboard, and pass entered text back to application in a buffer,
   using buffer contents as previous text (UTF-8 encoded), the buffer may be of any size.
   This function will return immediately with return status SDL_ANDROID_TEXTINPUT_ASYNC_IN_PROGRESS,
   and will change the contents of the buffer in another thread. You then should call this function
   with the same parameters, until it will return status SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED,
   and only after this you can access the contents of the buffer. */
extern DECLSPEC SDL_AndroidTextInputAsyncStatus_t SDLCALL SDL_ANDROID_GetScreenKeyboardTextInputAsync(char * textBuf, int textBufSize);

/* Whether user redefined on-screen keyboard layout via SDL menu, app should not enforce it's own layout in that case */
extern DECLSPEC int SDLCALL SDL_ANDROID_GetScreenKeyboardRedefinedByUser(void);

/* Set hint message for the QWERTY text input field, it may be multi-line, set NULL to reset hint to default */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardHintMesage(const char * hint);

/* API compatible to SDL2, it's a wrapper to the SDL_ANDROID_ToggleScreenKeyboardTextInput(""), it does not block.
   These functions control native Android QWERTY keyboard, not the overlay buttons */
extern DECLSPEC int SDLCALL SDL_HasScreenKeyboardSupport(void *unused);

extern DECLSPEC int SDLCALL SDL_ShowScreenKeyboard(void *unused);

extern DECLSPEC int SDLCALL SDL_HideScreenKeyboard(void *unused);

extern DECLSPEC int SDLCALL SDL_ToggleScreenKeyboard(void *unused);

extern DECLSPEC int SDLCALL SDL_IsScreenKeyboardShown(void *unused);

enum
{
	SDL_ANDROID_MAX_GAMEPADS      = 4, /* Maximum amount of gamepads supported */
	SDL_ANDROID_FIRST_GAMEPAD_ID  = 2, /* Joystick ID for SDL_JoystickOpen() and for SDL_Event event.jaxis.which */
	SDL_ANDROID_SECOND_GAMEPAD_ID = 3, /* Joystick ID for SDL_JoystickOpen() and for SDL_Event event.jaxis.which */
	SDL_ANDROID_THIRD_GAMEPAD_ID  = 4, /* Joystick ID for SDL_JoystickOpen() and for SDL_Event event.jaxis.which */
	SDL_ANDROID_FOURTH_GAMEPAD_ID = 5, /* Joystick ID for SDL_JoystickOpen() and for SDL_Event event.jaxis.which */
};

/* Remap SDL keycodes returned by gamepad buttons.
   Pass the SDLK_ constants, or 0 to leave old value.
   On OUYA: O = A, U = X, Y = Y, A = B.
   GamepadId is from 0 to 3, up to SDL_ANDROID_MAX_GAMEPADS
   Analog thumb joysticks will send keycodes instead of joystick events
   only if SDL_JoystickOpen() was NOT called for joystick IDs from 2 to 5. */
extern DECLSPEC void SDLCALL SDL_ANDROID_SetIndividualGamepadKeymap(int GamepadId,
	int A, int B, int X, int Y, int L1, int R1, int L2, int R2, int LThumb, int RThumb,
	int Start, int Select, int Up, int Down, int Left, int Right,
	int LThumbUp, int LThumbDown, int LThumbLeft, int LThumbRight,
	int RThumbUp, int RThumbDown, int RThumbLeft, int RThumbRight);

/* Deprecated API, will change keymap for all gamepads */
extern DECLSPEC void SDLCALL SDL_ANDROID_SetGamepadKeymap(int A, int B, int X, int Y, int L1, int R1, int L2, int R2, int LThumb, int RThumb);

/* Set SDL keycode for hardware Android key. Android keycodes are defined here:
http://developer.android.com/reference/android/view/KeyEvent.html#KEYCODE_0
KEYCODE_VOLUME_UP = 24, KEYCODE_VOLUME_DOWN = 25, KEYCODE_BACK = 4 */
extern DECLSPEC void SDLCALL SDL_ANDROID_SetAndroidKeycode(int Android_Key, int Sdl_Key);

/* Copy contents of Android clipboard into supplied buffer */
extern DECLSPEC void SDLCALL SDL_ANDROID_GetClipboardText(char * buf, int len);

/* Copy of SDL2 API */

/**
 * \brief Put UTF-8 text into the clipboard
 *
 * \sa SDL_GetClipboardText()
 */

extern DECLSPEC int SDLCALL SDL_SetClipboardText(const char *text);

/**
 * \brief Get UTF-8 text from the clipboard, which must be freed with SDL_free()
 *
 * \sa SDL_SetClipboardText()
 */
extern DECLSPEC char * SDLCALL SDL_GetClipboardText(void);

/**
 * \brief Returns a flag indicating whether the clipboard exists and contains a text string that is non-empty
 *
 * \sa SDL_GetClipboardText()
 */
extern DECLSPEC int SDLCALL SDL_HasClipboardText(void);

#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif
