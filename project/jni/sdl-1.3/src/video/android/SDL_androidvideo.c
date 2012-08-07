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

#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"
#include "SDL_version.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "jniwrapperstuff.h"


// The device screen dimensions to draw on
int SDL_ANDROID_sWindowWidth  = 0;
int SDL_ANDROID_sWindowHeight = 0;

int SDL_ANDROID_sRealWindowWidth  = 0;
int SDL_ANDROID_sRealWindowHeight = 0;

SDL_Rect SDL_ANDROID_ForceClearScreenRect = { 0, 0, 0, 0 };

// Extremely wicked JNI environment to call Java functions from C code
static JNIEnv* JavaEnv = NULL;
static jclass JavaRendererClass = NULL;
static jobject JavaRenderer = NULL;
static jmethodID JavaSwapBuffers = NULL;
static jmethodID JavaShowScreenKeyboard = NULL;
static jmethodID JavaTogglePlainAndroidSoftKeyboardInput = NULL;
static int glContextLost = 0;
static int showScreenKeyboardDeferred = 0;
static const char * showScreenKeyboardOldText = "";
static int showScreenKeyboardSendBackspace = 0;
int SDL_ANDROID_SmoothVideo = 0;
int SDL_ANDROID_VideoMultithreaded = 0;
int SDL_ANDROID_VideoForceSoftwareMode = 0;
int SDL_ANDROID_CompatibilityHacks = 0;
int SDL_ANDROID_BYTESPERPIXEL = 2;
int SDL_ANDROID_BITSPERPIXEL = 16;
int SDL_ANDROID_UseGles2 = 0;
int SDL_ANDROID_ShowMouseCursor = 0;

static void appPutToBackgroundCallbackDefault(void)
{
	SDL_ANDROID_PauseAudioPlayback();
}
static void appRestoredCallbackDefault(void)
{
	SDL_ANDROID_ResumeAudioPlayback();
}

static SDL_ANDROID_ApplicationPutToBackgroundCallback_t appPutToBackgroundCallback = appPutToBackgroundCallbackDefault;
static SDL_ANDROID_ApplicationPutToBackgroundCallback_t appRestoredCallback = appRestoredCallbackDefault;
static SDL_ANDROID_ApplicationPutToBackgroundCallback_t openALPutToBackgroundCallback = NULL;
static SDL_ANDROID_ApplicationPutToBackgroundCallback_t openALRestoredCallback = NULL;

int SDL_ANDROID_CallJavaSwapBuffers()
{
	if( !glContextLost )
	{
		SDL_ANDROID_drawTouchscreenKeyboard();
	}
	
	// Clear part of screen not used by SDL - on Android the screen contains garbage after each frame
	if( SDL_ANDROID_ForceClearScreenRect.w != 0 && SDL_ANDROID_ForceClearScreenRect.h != 0 )
	{
		glPushMatrix();
		glLoadIdentity();
		glOrthox( 0, (SDL_ANDROID_sRealWindowWidth) * 0x10000, SDL_ANDROID_sRealWindowHeight * 0x10000, 0, 0, 1 * 0x10000 );
		glColor4x(0, 0, 0, 0x10000);
		glEnableClientState(GL_VERTEX_ARRAY);
		
		GLshort vertices[] = {	SDL_ANDROID_ForceClearScreenRect.x, SDL_ANDROID_ForceClearScreenRect.y,
								SDL_ANDROID_ForceClearScreenRect.x + SDL_ANDROID_ForceClearScreenRect.w, SDL_ANDROID_ForceClearScreenRect.y,
								SDL_ANDROID_ForceClearScreenRect.x + SDL_ANDROID_ForceClearScreenRect.w, SDL_ANDROID_ForceClearScreenRect.y + SDL_ANDROID_ForceClearScreenRect.h,
								SDL_ANDROID_ForceClearScreenRect.x, SDL_ANDROID_ForceClearScreenRect.y + SDL_ANDROID_ForceClearScreenRect.h };
		glVertexPointer(2, GL_SHORT, 0, vertices);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}

	if( ! (*JavaEnv)->CallIntMethod( JavaEnv, JavaRenderer, JavaSwapBuffers ) )
		return 0;
	if( glContextLost )
	{
		glContextLost = 0;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context recreated, refreshing textures");
		SDL_ANDROID_VideoContextRecreated();
		appRestoredCallback();
		if(openALRestoredCallback)
			openALRestoredCallback();
	}
	if( showScreenKeyboardDeferred )
	{
		showScreenKeyboardDeferred = 0;
		(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaShowScreenKeyboard, (*JavaEnv)->NewStringUTF(JavaEnv, showScreenKeyboardOldText), showScreenKeyboardSendBackspace );
	}
	SDL_ANDROID_ProcessDeferredEvents();
	return 1;
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeResize) ( JNIEnv*  env, jobject  thiz, jint w, jint h, jint keepRatio )
{
	if( SDL_ANDROID_sWindowWidth == 0 )
	{
		SDL_ANDROID_sRealWindowWidth = w;
		SDL_ANDROID_sRealWindowHeight = h;
#if SDL_VERSION_ATLEAST(1,3,0)
		// Not supported in SDL 1.3
#else
		if( keepRatio )
		{
			// TODO: tweak that parameters when app calls SetVideoMode(), not here - app may request something else than 640x480, it's okay for most apps though
			SDL_ANDROID_sWindowWidth  = (SDL_ANDROID_sFakeWindowWidth*h)/SDL_ANDROID_sFakeWindowHeight;
			SDL_ANDROID_sWindowHeight = h;
			SDL_ANDROID_ForceClearScreenRect.x = SDL_ANDROID_sWindowWidth;
			SDL_ANDROID_ForceClearScreenRect.y = 0;
			SDL_ANDROID_ForceClearScreenRect.w = w - SDL_ANDROID_sWindowWidth;
			SDL_ANDROID_ForceClearScreenRect.h = h;

			if(SDL_ANDROID_sWindowWidth >= w) 
			{
				SDL_ANDROID_sWindowWidth  = w;
				SDL_ANDROID_sWindowHeight = (SDL_ANDROID_sFakeWindowHeight*w)/SDL_ANDROID_sFakeWindowWidth;
				SDL_ANDROID_ForceClearScreenRect.x = 0;
				SDL_ANDROID_ForceClearScreenRect.y = SDL_ANDROID_sWindowHeight;
				SDL_ANDROID_ForceClearScreenRect.w = w;
				SDL_ANDROID_ForceClearScreenRect.h = SDL_ANDROID_sWindowHeight - h; // OpenGL vertical coord is inverted
			}
		}
		else
#endif
		{
			SDL_ANDROID_ForceClearScreenRect.w = 0;
			SDL_ANDROID_ForceClearScreenRect.h = 0;
			SDL_ANDROID_ForceClearScreenRect.x = 0;
			SDL_ANDROID_ForceClearScreenRect.y = 0;
			SDL_ANDROID_sWindowWidth = w;
			SDL_ANDROID_sWindowHeight = h;
		}
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Physical screen resolution is %dx%d, virtual screen %dx%d", w, h, SDL_ANDROID_sWindowWidth, SDL_ANDROID_sWindowHeight );
		SDL_ANDROID_TouchscreenCalibrationWidth = SDL_ANDROID_sWindowWidth;
		SDL_ANDROID_TouchscreenCalibrationHeight = SDL_ANDROID_sWindowHeight;
	}
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeDone) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendQuit();
#else
	SDL_PrivateQuit();
#endif
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeGlContextLost) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context lost, waiting for new OpenGL context");
	glContextLost = 1;
	appPutToBackgroundCallback();
	if(openALPutToBackgroundCallback)
		openALPutToBackgroundCallback();

#if SDL_VERSION_ATLEAST(1,3,0)
	//if( ANDROID_CurrentWindow )
	//	SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
#else
	SDL_PrivateAppActive(0, SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS);
#endif

	SDL_ANDROID_VideoContextLost();
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeGlContextRecreated) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context recreated, sending SDL_ACTIVEEVENT");
#if SDL_VERSION_ATLEAST(1,3,0)
	//if( ANDROID_CurrentWindow )
	//	SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_RESTORED, 0, 0);
#else
	SDL_PrivateAppActive(1, SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS);
#endif
}

void SDL_ANDROID_CallJavaTogglePlainAndroidSoftKeyboardInput()
{
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaTogglePlainAndroidSoftKeyboardInput );
}

volatile static textInputFinished = 0;
void SDL_ANDROID_TextInputFinished()
{
	textInputFinished = 1;
};

#if SDL_VERSION_ATLEAST(1,3,0)
#else
extern int SDL_Flip(SDL_Surface *screen);
extern SDL_Surface *SDL_GetVideoSurface(void);
#endif

void SDL_ANDROID_CallJavaShowScreenKeyboard(const char * oldText, char * outBuf, int outBufLen)
{
	if( !outBuf )
	{
		showScreenKeyboardDeferred = 1;
		showScreenKeyboardOldText = oldText;
		showScreenKeyboardSendBackspace = 1;
		// Move mouse by 1 pixel to force screen update
		int x, y;
		SDL_GetMouseState( &x, &y );
		SDL_ANDROID_MainThreadPushMouseMotion(x > 0 ? x-1 : 0, y);
	}
	else
	{
		textInputFinished = 0;
		SDL_ANDROID_TextInputInit(outBuf, outBufLen);

		if( SDL_ANDROID_VideoMultithreaded )
		{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
			// Dirty hack: we may call (*JavaEnv)->CallVoidMethod(...) only from video thread
			showScreenKeyboardDeferred = 1;
			showScreenKeyboardOldText = oldText;
			showScreenKeyboardSendBackspace = 0;
			SDL_Flip(SDL_GetVideoSurface());
#endif
		}
		else
			(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaShowScreenKeyboard, (*JavaEnv)->NewStringUTF(JavaEnv, oldText), 0 );

		while( !textInputFinished )
			SDL_Delay(100);
		textInputFinished = 0;
	}
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeInitJavaCallbacks) ( JNIEnv*  env, jobject thiz )
{
	JavaEnv = env;
	JavaRenderer = (*JavaEnv)->NewGlobalRef( JavaEnv, thiz );
	
	JavaRendererClass = (*JavaEnv)->GetObjectClass(JavaEnv, thiz);
	JavaSwapBuffers = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "swapBuffers", "()I");
	JavaShowScreenKeyboard = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "showScreenKeyboard", "(Ljava/lang/String;I)V");
	JavaTogglePlainAndroidSoftKeyboardInput = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "togglePlainAndroidSoftKeyboardInput", "()V");
	
	ANDROID_InitOSKeymap();
}

int SDL_ANDROID_SetApplicationPutToBackgroundCallback(
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appPutToBackground,
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appRestored )
{
	appPutToBackgroundCallback = appPutToBackgroundCallbackDefault;
	appRestoredCallback = appRestoredCallbackDefault;
	
	if( appPutToBackground )
		appPutToBackgroundCallback = appPutToBackground;

	if( appRestoredCallback )
		appRestoredCallback = appRestored;
}

extern int SDL_ANDROID_SetOpenALPutToBackgroundCallback(
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t PutToBackground,
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t Restored );

int SDL_ANDROID_SetOpenALPutToBackgroundCallback(
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t PutToBackground,
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t Restored )
{
	openALPutToBackgroundCallback = PutToBackground;
	openALRestoredCallback = Restored;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetSmoothVideo) (JNIEnv* env, jobject thiz)
{
	SDL_ANDROID_SmoothVideo = 1;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetVideoMultithreaded) (JNIEnv* env, jobject thiz)
{
	SDL_ANDROID_VideoMultithreaded = 1;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetVideoForceSoftwareMode) (JNIEnv* env, jobject thiz)
{
	SDL_ANDROID_VideoForceSoftwareMode = 1;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetCompatibilityHacks) (JNIEnv* env, jobject thiz)
{
	SDL_ANDROID_CompatibilityHacks = 1;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetVideoDepth) (JNIEnv* env, jobject thiz, jint bpp, jint UseGles2)
{
	SDL_ANDROID_BITSPERPIXEL = bpp;
	SDL_ANDROID_BYTESPERPIXEL = SDL_ANDROID_BITSPERPIXEL / 8;
	SDL_ANDROID_UseGles2 = UseGles2;
}
