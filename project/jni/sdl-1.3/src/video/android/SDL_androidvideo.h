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
#ifndef _SDL_androidvideo_h
#define _SDL_androidvideo_h

#include "SDL_version.h"
#include "SDL_config.h"
#include "SDL_video.h"
#include "SDL_joystick.h"
#include "SDL_events.h"

enum ScreenZoom { ZOOM_NONE = 0, ZOOM_MAGNIFIER = 1, ZOOM_SCREEN_TRANSFORM = 2, ZOOM_FULLSCREEN_MAGNIFIER = 3 };

extern int SDL_ANDROID_sWindowWidth;
extern int SDL_ANDROID_sWindowHeight;
extern int SDL_ANDROID_sRealWindowWidth;
extern int SDL_ANDROID_sRealWindowHeight;
extern int SDL_ANDROID_sFakeWindowWidth; // SDL 1.2 only
extern int SDL_ANDROID_sFakeWindowHeight; // SDL 1.2 only
extern int SDL_ANDROID_TouchscreenCalibrationWidth;
extern int SDL_ANDROID_TouchscreenCalibrationHeight;
extern int SDL_ANDROID_TouchscreenCalibrationX;
extern int SDL_ANDROID_TouchscreenCalibrationY;
extern int SDL_ANDROID_SmoothVideo;
extern int SDL_ANDROID_VideoMultithreaded;
extern int SDL_ANDROID_VideoForceSoftwareMode;
extern int SDL_ANDROID_CompatibilityHacks;
extern int SDL_ANDROID_ShowMouseCursor;
extern int SDL_ANDROID_UseGles2;
extern int SDL_ANDROID_BYTESPERPIXEL;
extern int SDL_ANDROID_BITSPERPIXEL;
extern void SDL_ANDROID_TextInputInit(char * buffer, int len);
extern void SDL_ANDROID_TextInputFinished();
extern SDL_Surface *SDL_CurrentVideoSurface;
extern SDL_Rect SDL_ANDROID_ForceClearScreenRect;
extern int SDL_ANDROID_ShowScreenUnderFinger;
extern SDL_Rect SDL_ANDROID_ShowScreenUnderFingerRect, SDL_ANDROID_ShowScreenUnderFingerRectSrc;
extern int SDL_ANDROID_CallJavaSwapBuffers();
extern void SDL_ANDROID_CallJavaShowScreenKeyboard(const char * oldText, char * outBuf, int outBufLen);
extern int SDL_ANDROID_drawTouchscreenKeyboard();
extern void SDL_ANDROID_VideoContextLost();
extern void SDL_ANDROID_VideoContextRecreated();
extern void SDL_ANDROID_processAndroidTrackballDampening();
extern void SDL_ANDROID_processMoveMouseWithKeyboard();
extern int SDL_ANDROID_InsideVideoThread();
extern void SDL_ANDROID_initFakeStdout();
extern SDL_VideoDevice *ANDROID_CreateDevice_1_3(int devindex);
extern void SDL_ANDROID_ProcessDeferredEvents();
extern void SDL_ANDROID_WarpMouse(int x, int y);
extern void SDL_ANDROID_DrawMouseCursor(int x, int y, int size, int alpha);
extern void SDL_ANDROID_DrawMouseCursorIfNeeded();
extern void SDL_ANDROID_CallJavaTogglePlainAndroidSoftKeyboardInput();


#if SDL_VERSION_ATLEAST(1,3,0)
extern SDL_Window * ANDROID_CurrentWindow;
#endif

// Exports from SDL_androidinput.c - SDL_androidinput.h is too encumbered
enum { MAX_MULTITOUCH_POINTERS = 16 };
extern void ANDROID_InitOSKeymap();
extern int SDL_ANDROID_isJoystickUsed;
// Events have to be sent only from main thread from PumpEvents(), so we'll buffer them here
extern void SDL_ANDROID_PumpEvents();


#endif /* _SDL_androidvideo_h */
