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

#ifndef _SDL_android_h
#define _SDL_android_h

#include "SDL_video.h"
#include "SDL_screenkeyboard.h"
#include "SDL_audio.h"

#include "begin_code.h"

#if defined(__cplusplus)
struct JavaVM;
#else
struct JNIInvokeInterface;
typedef const struct JNIInvokeInterface* JavaVM;
#endif

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/*
Sets callbacks to be called when OS decides to put application to background, and restored to foreground.
*/
typedef void ( * SDL_ANDROID_ApplicationPutToBackgroundCallback_t ) (void);

extern DECLSPEC int SDLCALL SDL_ANDROID_SetApplicationPutToBackgroundCallback(
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appPutToBackground,
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appRestored );

/* Use these functions instead of setting volume to 0, that will save CPU and battery on device */
extern DECLSPEC int SDLCALL SDL_ANDROID_PauseAudioPlayback(void);
extern DECLSPEC int SDLCALL SDL_ANDROID_ResumeAudioPlayback(void);

/*
Get the advertisement size, position and visibility.
If the advertisement is not yet loaded, this function will return zero width and height,
so you'll need to account for that in your code, because you can never know
whether the user has no access to network, or if ad server is accessbile.
This function will return the coordinates in the physical screen pixels,
not in the "stretched" coordinates, which you get when you call SDL_SetVideoMode(640, 480, 0, 0);
The physical screen size is returned by SDL_ListModes(NULL, 0)[0].
*/
extern DECLSPEC int SDLCALL SDL_ANDROID_GetAdvertisementParams(int * visible, SDL_Rect * position);
/* Control the advertisement visibility */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetAdvertisementVisible(int visible);
/*
Control the advertisement  placement, you may use constants
ADVERTISEMENT_POSITION_RIGHT, ADVERTISEMENT_POSITION_BOTTOM, ADVERTISEMENT_POSITION_CENTER
to position the advertisement on the screen without needing to know the advertisment size
(which may be reported as zero, if the ad is still not loaded),
and to convert between "stretched" and physical coorinates.
*/
enum {
	ADVERTISEMENT_POSITION_LEFT = 0,
	ADVERTISEMENT_POSITION_TOP = 0,
	ADVERTISEMENT_POSITION_RIGHT = -1,
	ADVERTISEMENT_POSITION_BOTTOM = -1,
	ADVERTISEMENT_POSITION_CENTER = -2
};
extern DECLSPEC int SDLCALL SDL_ANDROID_SetAdvertisementPosition(int x, int y);

/* Request a new advertisement to be loaded */
extern DECLSPEC int SDLCALL SDL_ANDROID_RequestNewAdvertisement(void);


/** Exports for Java environment and Video object instance.
    To get JNIEnv pointer, use following code:
    JNIEnv *env = NULL;
    (*SDL_ANDROID_JavaVM())->GetEnv(SDL_ANDROID_JavaVM(), &env, JNI_VERSION_1_6); */
extern DECLSPEC JavaVM* SDL_ANDROID_JavaVM();

/*
Open audio recording device, it will use parameters freq, format, channels, size and callback,
and return internal buffer size on success, which you may ignore,
because your callback will always be called with buffer size you specified.
Returns 0 on failure (most probably hardware does not support requested audio rate).
SDL_Init(SDL_INIT_AUDIO) has to be done before calling this function.
*/
extern DECLSPEC int SDLCALL SDL_ANDROID_OpenAudioRecording(SDL_AudioSpec *spec);
/* Close audio recording device, SDL_Init(SDL_INIT_AUDIO) has to be done before calling this function. */
extern DECLSPEC void SDLCALL SDL_ANDROID_CloseAudioRecording(void);

/*
Save the file to the cloud, filename must be already present on disk.
This function will block, until user signs in to the cloud account, and presses Save button.
If saveId is NULL or empty, user will select the savegame from the dialog, if it's not empty -
do not show any dialog to the user, except foir sign-in, and write the savegame with specified ID.
dialogTitle may be NULL.
description and screenshotFile may be NULL. playedTimeMs is used for conflict resolution -
savegame with longer play time will get priority.
Use SDL_SaveBMP(SDL_GetVideoSurface(), "screenshot.bmp") to create the screenshot.
Returns 1 if save succeeded, 0 if user aborted sign-in, or there was no network available.
*/
extern DECLSPEC int SDLCALL SDL_ANDROID_CloudSave(const char *filename, const char *saveId, const char *dialogTitle,
													const char *description, const char *screenshotFile, uint64_t playedTimeMs);

/*
Load the specified file from the cloud.
This function will block, until user signs in to the cloud account, and selects a savegame.
The resulting savegame is written to the passed filename.
If saveId is NULL or empty, user will select the savegame from the dialog, if it's not empty -
do not show any dialog to the user, except foir sign-in, and read the savegame with specified ID.
dialogTitle may be NULL.
Returns 1 if load succeeded, 0 if user aborted sign-in, or there was no network available.
*/
extern DECLSPEC int SDLCALL SDL_ANDROID_CloudLoad(const char *filename, const char *saveId, const char *dialogTitle);

/* Open Android application, any parameter may be NULL */
extern DECLSPEC void SDLCALL SDL_ANDROID_OpenExternalApp(const char *package, const char *activity, const char *data);
/* Open an Android web browser, or Chrome, or Youtube for video links. Just a wrapper for SDL_ANDROID_OpenExternalApp(NULL, NULL, url) */
extern DECLSPEC void SDLCALL SDL_ANDROID_OpenExternalWebBrowser(const char *url);

/* Restart current app, you can pass a string to the new app instance, which can be retrieved with getenv("SDL_RESTART_PARAMS") */
extern DECLSPEC void SDLCALL SDL_ANDROID_RestartMyself(const char *restartParams);

enum {
	SDL_ANDROID_CONFIG_VIDEO_DEPTH_BPP = 0, /* Can be 16 or 24 for software video modes, OpenGL supports 16/24/32 */
	/* TODO: more options, see Globals.java */
};

/* Set SDL Android-specifc option, and save it to SDL config file, such as video depth or mouse emulation mode. Most options require restarting the app. */
extern DECLSPEC void SDLCALL SDL_ANDROID_SetConfigOption(int option, int value);

/* Change mouse emulation mode, pass -1 to any option to keep the current value, this does not change SDL config file.
   Currently only relativeMovement is processed, other options are ignored  */
extern DECLSPEC void SDLCALL SDL_ANDROID_SetMouseEmulationMode(
	int relativeMovement, int relativeMovementSpeed, int relativeMovementAcceleration,
	int leftClickMode, int leftClickKey, int leftClickTimeout,
	int rightClickMode, int rightClickKey, int rightClickTimeout,
	int moveMouseWithJoystick, int moveMouseWithJoystickSpeed, int moveMouseWithJoystickAcceleration,
	int moveMouseWithGyroscope, int moveMouseWithGyroscopeSpeed,
	int forceHardwareMouse, int showScreenUnderFinger,
	int fingerHover, int fingerHoverJitterFilter, int generateSubframeTouchEvents
);

extern DECLSPEC int SDLCALL SDL_ANDROID_GetMouseEmulationMode();

/* Control the System mouse pointer visibility */
extern DECLSPEC void SDLCALL SDL_ANDROID_SetSystemMousePointerVisible(int visible);

#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif
