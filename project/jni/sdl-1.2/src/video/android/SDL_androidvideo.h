/*
Simple DirectMedia Layer
Copyright (C) 2009-2014 Sergii Pylypenko

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
/*
This source code is distibuted under ZLIB license, however when compiling with SDL 1.2,
which is licensed under LGPL, the resulting library, and all it's source code,
falls under "stronger" LGPL terms, so is this file.
If you compile this code with SDL 1.3 or newer, or use in some other way, the license stays ZLIB.
*/

#ifndef _SDL_androidvideo_h
#define _SDL_androidvideo_h

#include "SDL_version.h"
#include "SDL_config.h"
#include "SDL_video.h"
#include "SDL_joystick.h"
#include "SDL_events.h"

//#define VIDEO_DEBUG 1

enum ScreenZoom { ZOOM_NONE = 0, ZOOM_MAGNIFIER = 1 };

extern int SDL_ANDROID_sWindowWidth;
extern int SDL_ANDROID_sWindowHeight;
extern int SDL_ANDROID_sRealWindowWidth;
extern int SDL_ANDROID_sRealWindowHeight;
extern int SDL_ANDROID_sFakeWindowWidth; // SDL 1.2 only
extern int SDL_ANDROID_sFakeWindowHeight; // SDL 1.2 only
extern int SDL_ANDROID_ScreenKeep43Ratio;
extern int SDL_ANDROID_TouchscreenCalibrationWidth;
extern int SDL_ANDROID_TouchscreenCalibrationHeight;
extern int SDL_ANDROID_TouchscreenCalibrationX;
extern int SDL_ANDROID_TouchscreenCalibrationY;
extern int SDL_ANDROID_VideoLinearFilter;
extern int SDL_ANDROID_VideoMultithreaded;
extern int SDL_ANDROID_VideoForceSoftwareMode;
extern int SDL_ANDROID_CompatibilityHacks;
extern int SDL_ANDROID_ShowMouseCursor;
extern int SDL_ANDROID_UseGles2;
extern int SDL_ANDROID_UseGles3;
extern int SDL_ANDROID_BYTESPERPIXEL;
extern int SDL_ANDROID_BITSPERPIXEL;
extern void SDL_ANDROID_TextInputInit(char * buffer, int len);
extern int SDL_ANDROID_TextInputFinished;
extern SDL_Surface *SDL_CurrentVideoSurface;
extern SDL_Rect SDL_ANDROID_ForceClearScreenRect[];
extern int SDL_ANDROID_ForceClearScreenRectAmount;
extern int SDL_ANDROID_ShowScreenUnderFinger;
extern SDL_Rect SDL_ANDROID_ShowScreenUnderFingerRect, SDL_ANDROID_ShowScreenUnderFingerRectSrc;
extern int SDL_ANDROID_CallJavaSwapBuffers();
extern void SDL_ANDROID_CallJavaShowScreenKeyboard(const char * oldText, char * outBuf, int outBufLen, int async);
extern void SDL_ANDROID_CallJavaHideScreenKeyboard();
extern void SDL_ANDROID_CallJavaSetScreenKeyboardHintMessage(const char *hint);
extern int SDL_ANDROID_IsScreenKeyboardShown();
extern int SDL_ANDROID_IsScreenKeyboardShownFlag;
extern int SDL_ANDROID_AsyncTextInputActive;
extern int SDL_ANDROID_drawTouchscreenKeyboard();
extern void SDL_ANDROID_VideoContextLost();
extern void SDL_ANDROID_VideoContextRecreated();
extern void SDL_ANDROID_processMoveMouseWithKeyboard();
extern int SDL_ANDROID_InsideVideoThread();
extern void SDL_ANDROID_initFakeStdout();
extern SDL_VideoDevice *ANDROID_CreateDevice_1_3(int devindex);
extern void SDL_ANDROID_ProcessDeferredEvents();
extern void SDL_ANDROID_WarpMouse(int x, int y);
extern void SDL_ANDROID_SetCapturedMousePosition(int x, int y);
extern void SDL_ANDROID_DrawMouseCursor(int x, int y, int size, float alpha);
extern void SDL_ANDROID_DrawMouseCursorIfNeeded();
extern void SDL_ANDROID_CallJavaTogglePlainAndroidSoftKeyboardInput();
extern void SDL_ANDROID_CallJavaStartAccelerometerGyroscope(int start);
extern SDL_Rect SDL_ANDROID_ScreenVisibleRect;
extern SDL_Rect SDL_ANDROID_VideoDebugRect;
extern SDL_Color SDL_ANDROID_VideoDebugRectColor;

#if SDL_VERSION_ATLEAST(1,3,0)
extern SDL_Window * ANDROID_CurrentWindow;
#endif

// Exports from SDL_androidinput.c - SDL_androidinput.h is too encumbered
enum { MAX_MULTITOUCH_POINTERS = 16 };
extern void ANDROID_InitOSKeymap();
extern int SDL_ANDROID_joysticksAmount;
// Events have to be sent only from main thread from PumpEvents(), so we'll buffer them here
extern void SDL_ANDROID_PumpEvents();
extern void SDL_ANDROID_SetHoverDeadzone();

#endif /* _SDL_androidvideo_h */
