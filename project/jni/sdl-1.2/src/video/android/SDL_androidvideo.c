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

#include <jni.h>
#include <android/log.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_opengles.h"

#include "SDL_config.h"
#include "SDL_version.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "SDL_timer.h"
#include "SDL_android.h"
#include "SDL_syswm.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"


// The device screen dimensions to draw on
int SDL_ANDROID_sWindowWidth  = 0;
int SDL_ANDROID_sWindowHeight = 0;

int SDL_ANDROID_sRealWindowWidth  = 0;
int SDL_ANDROID_sRealWindowHeight = 0;

int SDL_ANDROID_ScreenKeep43Ratio = 0;

SDL_Rect SDL_ANDROID_ForceClearScreenRect[4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
int SDL_ANDROID_ForceClearScreenRectAmount = 0;

// Extremely wicked JNI environment to call Java functions from C code
static jclass JavaRendererClass = NULL;
static jobject JavaRenderer = NULL;
static jmethodID JavaSwapBuffers = NULL;
static jmethodID JavaShowScreenKeyboard = NULL;
static jmethodID JavaToggleScreenKeyboardWithoutTextInput = NULL;
static jmethodID JavaToggleInternalScreenKeyboard = NULL;
static jmethodID JavaHideScreenKeyboard = NULL;
static jmethodID JavaIsScreenKeyboardShown = NULL;
static jmethodID JavaSetScreenKeyboardHintMessage = NULL;
static jmethodID JavaStartAccelerometerGyroscope = NULL;
static jmethodID JavaGetClipboardText = NULL;
static jmethodID JavaSetClipboardText = NULL;
static jmethodID JavaGetAdvertisementParams = NULL;
static jmethodID JavaSetAdvertisementVisible = NULL;
static jmethodID JavaSetAdvertisementPosition = NULL;
static jmethodID JavaRequestNewAdvertisement = NULL;
static jmethodID JavaRequestCloudSave = NULL;
static jmethodID JavaRequestCloudLoad = NULL;
static jmethodID JavaRequestOpenExternalApp = NULL;
static jmethodID JavaRequestRestartMyself = NULL;
static jmethodID JavaRequestSetConfigOption = NULL;
static jmethodID JavaSetSystemMousePointerVisible = NULL;
static jmethodID JavaSetCapturedMousePosition = NULL;
static int glContextLost = 0;
static int showScreenKeyboardDeferred = 0;
static const char * showScreenKeyboardOldText = "";
int SDL_ANDROID_IsScreenKeyboardShownFlag = 0;
int SDL_ANDROID_TextInputFinished = 0;
int SDL_ANDROID_VideoLinearFilter = 0;
int SDL_ANDROID_VideoMultithreaded = 0;
int SDL_ANDROID_VideoForceSoftwareMode = 0;
int SDL_ANDROID_CompatibilityHacks = 0;
int SDL_ANDROID_BYTESPERPIXEL = 2;
int SDL_ANDROID_BITSPERPIXEL = 16;
int SDL_ANDROID_UseGles2 = 0;
int SDL_ANDROID_UseGles3 = 0;
int SDL_ANDROID_ShowMouseCursor = 0;
SDL_Rect SDL_ANDROID_VideoDebugRect;
SDL_Color SDL_ANDROID_VideoDebugRectColor;
SDL_Rect SDL_ANDROID_ScreenVisibleRect = {0, 0, 0, 0};

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

static inline JNIEnv *GetJavaEnv(void)
{
	JavaVM *vm = SDL_ANDROID_JavaVM();
	JNIEnv *ret = NULL;
	(*vm)->GetEnv(vm, (void **) &ret, JNI_VERSION_1_6);
	return ret;
}

int SDL_ANDROID_CallJavaSwapBuffers()
{
	JNIEnv *JavaEnv = GetJavaEnv();
	if( !glContextLost )
	{
		// Clear part of screen not used by SDL - on Android the screen contains garbage after each frame
#if SDL_VIDEO_OPENGL_ES_VERSION == 1
		if( SDL_ANDROID_ForceClearScreenRectAmount > 0 )
		{
			int i;

			glPushMatrix();
			glLoadIdentity();
			glOrthof( 0.0f, SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sRealWindowHeight, 0.0f, 0.0f, 1.0f );
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			glEnableClientState(GL_VERTEX_ARRAY);
			
			for( i = 0; i < SDL_ANDROID_ForceClearScreenRectAmount; i++ )
			{
				GLshort vertices[] = {	SDL_ANDROID_ForceClearScreenRect[i].x, SDL_ANDROID_ForceClearScreenRect[i].y,
										SDL_ANDROID_ForceClearScreenRect[i].x + SDL_ANDROID_ForceClearScreenRect[i].w, SDL_ANDROID_ForceClearScreenRect[i].y,
										SDL_ANDROID_ForceClearScreenRect[i].x + SDL_ANDROID_ForceClearScreenRect[i].w, SDL_ANDROID_ForceClearScreenRect[i].y + SDL_ANDROID_ForceClearScreenRect[i].h,
										SDL_ANDROID_ForceClearScreenRect[i].x, SDL_ANDROID_ForceClearScreenRect[i].y + SDL_ANDROID_ForceClearScreenRect[i].h };
				glVertexPointer(2, GL_SHORT, 0, vertices);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}

			glDisableClientState(GL_VERTEX_ARRAY);
			glPopMatrix();
		}
#endif
		SDL_ANDROID_drawTouchscreenKeyboard();
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
		(*JavaEnv)->PushLocalFrame(JavaEnv, 1);
		jstring s = (*JavaEnv)->NewStringUTF(JavaEnv, showScreenKeyboardOldText);
		showScreenKeyboardDeferred = 0;
		(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaShowScreenKeyboard, s, 0 );
		(*JavaEnv)->DeleteLocalRef( JavaEnv, s );
		(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
	}
	SDL_ANDROID_ProcessDeferredEvents();
	return 1;
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeResize) ( JNIEnv*  env, jobject  thiz, jint w, jint h, jint keepRatio )
{
	SDL_ANDROID_sRealWindowWidth = w;
	SDL_ANDROID_sRealWindowHeight = h;
	SDL_ANDROID_sWindowWidth = w;
	SDL_ANDROID_sWindowHeight = h;
	SDL_ANDROID_ScreenKeep43Ratio = keepRatio;
	if( SDL_ANDROID_ScreenKeep43Ratio )
		SDL_ANDROID_sWindowWidth = (SDL_ANDROID_sFakeWindowWidth * SDL_ANDROID_sRealWindowHeight) / SDL_ANDROID_sFakeWindowHeight;

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Physical screen resolution is %dx%d 43Ratio %d", w, h, keepRatio);
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

	SDL_ANDROID_VideoContextLost();
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeGlContextLostAsyncEvent) ( JNIEnv*  env, jobject thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context lost - sending SDL_ACTIVEEVENT");
	SDL_ANDROID_MainThreadPushAppActive(0);
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

int SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput(void)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaToggleScreenKeyboardWithoutTextInput );
	return 1;
}

int SDL_ANDROID_ToggleInternalScreenKeyboard(SDL_InternalKeyboard_t keyboard)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaToggleInternalScreenKeyboard, (jint)keyboard );
	return 1;
}

#if SDL_VERSION_ATLEAST(1,3,0)
#else
extern int SDL_Flip(SDL_Surface *screen);
extern SDL_Surface *SDL_GetVideoSurface(void);
#endif

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMotionEvent) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action, jint pointerId, jint force, jint radius );

void SDL_ANDROID_CallJavaShowScreenKeyboard(const char * oldText, char * outBuf, int outBufLen, int async)
{
	int i;
	JNIEnv *JavaEnv = GetJavaEnv();

	// Clear mouse button state, to avoid repeated clicks on the text field in some apps
	SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
	SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_RIGHT );
	SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_MIDDLE );
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Releasing all touch pointers: mouse at %4d %4d", SDL_ANDROID_currentMouseX, SDL_ANDROID_currentMouseY);
	JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMotionEvent) ( NULL, NULL, SDL_ANDROID_currentMouseX, SDL_ANDROID_currentMouseY, MOUSE_UP, 0, 0, 0 );
	for (i = 1; i < MAX_MULTITOUCH_POINTERS; i++)
	{
		JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMotionEvent) ( NULL, NULL, 0, 0, MOUSE_UP, i, 0, 0 );
	}

	SDL_ANDROID_TextInputFinished = 0;
	SDL_ANDROID_IsScreenKeyboardShownFlag = 1;
	if( !outBuf )
	{
		showScreenKeyboardDeferred = 1;
		showScreenKeyboardOldText = oldText;
		// Move mouse by 1 pixel to force screen update
		int x, y;
		SDL_GetMouseState( &x, &y );
		SDL_ANDROID_MainThreadPushMouseMotion(x > 0 ? x-1 : 0, y);
	}
	else
	{
		SDL_ANDROID_TextInputInit(outBuf, outBufLen);

		if( SDL_ANDROID_VideoMultithreaded )
		{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
			// Dirty hack: we may call (*JavaEnv)->CallVoidMethod(...) only from video thread
			showScreenKeyboardDeferred = 1;
			showScreenKeyboardOldText = oldText;
			SDL_Flip(SDL_GetVideoSurface());
#endif
		}
		else
		{
			(*JavaEnv)->PushLocalFrame(JavaEnv, 1);
			jstring s = (*JavaEnv)->NewStringUTF( JavaEnv, oldText );
			(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaShowScreenKeyboard, s, 0 );
			(*JavaEnv)->DeleteLocalRef( JavaEnv, s );
			(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
		}

		if( async )
			return;

		while( !SDL_ANDROID_TextInputFinished )
			SDL_Delay(100);
		SDL_ANDROID_TextInputFinished = 0;
		SDL_ANDROID_IsScreenKeyboardShownFlag = 0;
	}
}

void SDL_ANDROID_CallJavaHideScreenKeyboard()
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaHideScreenKeyboard );
}

int SDL_ANDROID_IsScreenKeyboardShown()
{
	return SDL_ANDROID_IsScreenKeyboardShownFlag || SDL_ANDROID_AsyncTextInputActive;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeScreenKeyboardShown) ( JNIEnv*  env, jobject thiz, jint shown )
{
	SDL_ANDROID_IsScreenKeyboardShownFlag = shown;
}

void SDL_ANDROID_CallJavaSetScreenKeyboardHintMessage(const char *hint)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->PushLocalFrame(JavaEnv, 1);
	jstring s = hint ? (*JavaEnv)->NewStringUTF(JavaEnv, hint) : NULL;
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaSetScreenKeyboardHintMessage, s );
	if( s )
		(*JavaEnv)->DeleteLocalRef( JavaEnv, s );
	(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
}

void SDL_ANDROID_CallJavaStartAccelerometerGyroscope(int start)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaStartAccelerometerGyroscope, (jint) start );
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoRenderer_nativeInitJavaCallbacks) ( JNIEnv*  env, jobject thiz )
{
	JNIEnv *JavaEnv = env;
	JavaRenderer = (*JavaEnv)->NewGlobalRef( JavaEnv, thiz );
	
	JavaRendererClass = (*JavaEnv)->GetObjectClass(JavaEnv, thiz);
	JavaSwapBuffers = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "swapBuffers", "()I");
	JavaShowScreenKeyboard = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "showScreenKeyboard", "(Ljava/lang/String;I)V");
	JavaToggleScreenKeyboardWithoutTextInput = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "showScreenKeyboardWithoutTextInputField", "()V");
	JavaToggleInternalScreenKeyboard = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "showInternalScreenKeyboard", "(I)V");
	JavaHideScreenKeyboard = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "hideScreenKeyboard", "()V");
	JavaIsScreenKeyboardShown = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "isScreenKeyboardShown", "()I");
	JavaSetScreenKeyboardHintMessage = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setScreenKeyboardHintMessage", "(Ljava/lang/String;)V");
	JavaStartAccelerometerGyroscope = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "startAccelerometerGyroscope", "(I)V");
	JavaGetClipboardText = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "getClipboardText", "()Ljava/lang/String;");
	JavaSetClipboardText = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setClipboardText", "(Ljava/lang/String;)V");

	JavaGetAdvertisementParams = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "getAdvertisementParams", "([I)V");
	JavaSetAdvertisementVisible = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setAdvertisementVisible", "(I)V");
	JavaSetAdvertisementPosition = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setAdvertisementPosition", "(II)V");
	JavaRequestNewAdvertisement = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "requestNewAdvertisement", "()V");

	JavaRequestCloudSave = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "cloudSave",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)Z");
	JavaRequestCloudLoad = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "cloudLoad",
													"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
	JavaRequestOpenExternalApp = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "openExternalApp", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	JavaRequestRestartMyself = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "restartMyself", "(Ljava/lang/String;)V");
	JavaRequestSetConfigOption = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setConfigOptionFromSDL", "(II)V");
	JavaSetSystemMousePointerVisible = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setSystemMousePointerVisible", "(I)V");
	JavaSetCapturedMousePosition = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "setCapturedMousePosition", "(II)V");

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
	return 0;
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
	return 0;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetVideoLinearFilter) (JNIEnv* env, jobject thiz)
{
	SDL_ANDROID_VideoLinearFilter = 1;
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
JAVA_EXPORT_NAME(Settings_nativeSetVideoDepth) (JNIEnv* env, jobject thiz, jint bpp, jint UseGles2, jint UseGles3)
{
	SDL_ANDROID_BITSPERPIXEL = bpp;
	SDL_ANDROID_BYTESPERPIXEL = SDL_ANDROID_BITSPERPIXEL / 8;
	SDL_ANDROID_UseGles2 = UseGles2;
	SDL_ANDROID_UseGles3 = UseGles3;
}

void SDLCALL SDL_ANDROID_GetClipboardText(char * buf, int len)
{
	char *c = SDL_GetClipboardText();
	strncpy(buf, c, len);
	buf[len-1] = 0;
	SDL_free(c);
}

int SDLCALL SDL_SetClipboardText(const char *text)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->PushLocalFrame(JavaEnv, 1);
	jstring s = (*JavaEnv)->NewStringUTF(JavaEnv, text);
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaSetClipboardText, s );
	if( s )
		(*JavaEnv)->DeleteLocalRef( JavaEnv, s );
	(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
	return 0;
}

char * SDLCALL SDL_GetClipboardText(void)
{
	char *buf = NULL;
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->PushLocalFrame( JavaEnv, 1 );
	jstring s = (jstring) (*JavaEnv)->CallObjectMethod( JavaEnv, JavaRenderer, JavaGetClipboardText );
	if( s )
	{
		const char *c = (*JavaEnv)->GetStringUTFChars( JavaEnv, s, NULL );
		if( c )
		{
			int len = strlen(c);
			buf = SDL_malloc(len + 1);
			memcpy(buf, c, len + 1);
			(*JavaEnv)->ReleaseStringUTFChars( JavaEnv, s, c );
		}
		(*JavaEnv)->DeleteLocalRef( JavaEnv, s );
	}
	(*JavaEnv)->PopLocalFrame( JavaEnv, NULL );
	if (buf == NULL)
	{
		buf = SDL_malloc(1);
		buf[0] = 0;
	}
	return buf;
}

int SDLCALL SDL_HasClipboardText(void)
{
	char *c = SDL_GetClipboardText();
	int ret = c[0] != 0;
	SDL_free(c);
	return ret;
}

void JAVA_EXPORT_NAME(DemoRenderer_nativeClipboardChanged) ( JNIEnv* env, jobject thiz )
{
	if ( SDL_ProcessEvents[SDL_SYSWMEVENT] == SDL_ENABLE )
	{
		SDL_SysWMmsg wmmsg;
		SDL_VERSION(&wmmsg.version);
		wmmsg.type = SDL_SYSWM_ANDROID_CLIPBOARD_CHANGED;
		SDL_PrivateSysWMEvent(&wmmsg);
	}
}

int SDLCALL SDL_ANDROID_GetAdvertisementParams(int * visible, SDL_Rect * position)
{
	jint arr[5];
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->PushLocalFrame( JavaEnv, 1 );
	jintArray elemArr = (*JavaEnv)->NewIntArray(JavaEnv, 5);
	if (elemArr == NULL)
		return 0;
	(*JavaEnv)->SetIntArrayRegion(JavaEnv, elemArr, 0, 5, arr);
	(*JavaEnv)->CallVoidMethod(JavaEnv, JavaRenderer, JavaGetAdvertisementParams, elemArr);
	(*JavaEnv)->GetIntArrayRegion(JavaEnv, elemArr, 0, 5, arr);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, elemArr);
	(*JavaEnv)->PopLocalFrame( JavaEnv, NULL );
	if(visible)
		*visible = arr[0];
	if(position)
	{
		position->x = arr[1];
		position->y = arr[2];
		position->w = arr[3];
		position->h = arr[4];
	}
	return 1;
}
int SDLCALL SDL_ANDROID_SetAdvertisementVisible(int visible)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaSetAdvertisementVisible, (jint)visible );
	return 1;
}
int SDLCALL SDL_ANDROID_SetAdvertisementPosition(int left, int top)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaSetAdvertisementPosition, (jint)left, (jint)top );
	return 1;
}
int SDLCALL SDL_ANDROID_RequestNewAdvertisement(void)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaRequestNewAdvertisement );
	return 1;
}

int SDLCALL SDL_ANDROID_CloudSave(const char *filename, const char *saveId, const char *dialogTitle,
									const char *description, const char *screenshotFile, uint64_t playedTimeMs)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_ANDROID_CloudSave: played time %llu", playedTimeMs);
	if( !filename )
		return 0;
	if( !saveId )
		saveId = "";
	if( !dialogTitle )
		dialogTitle = "";
	if( !description )
		description = "";
	if( !screenshotFile )
		screenshotFile = "";
	(*JavaEnv)->PushLocalFrame(JavaEnv, 5);
	jstring s1 = (*JavaEnv)->NewStringUTF(JavaEnv, filename);
	jstring s2 = (*JavaEnv)->NewStringUTF(JavaEnv, saveId);
	jstring s3 = (*JavaEnv)->NewStringUTF(JavaEnv, dialogTitle);
	jstring s4 = (*JavaEnv)->NewStringUTF(JavaEnv, description);
	jstring s5 = (*JavaEnv)->NewStringUTF(JavaEnv, screenshotFile);
	int result = (*JavaEnv)->CallBooleanMethod( JavaEnv, JavaRenderer, JavaRequestCloudSave, s1, s2, s3, s4, s5, (jlong)playedTimeMs );
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s5);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s4);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s3);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s2);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s1);
	(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
	return result;
}

int SDLCALL SDL_ANDROID_CloudLoad(const char *filename, const char *saveId, const char *dialogTitle)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	if( !filename )
		return 0;
	if( !saveId )
		saveId = "";
	if( !dialogTitle )
		dialogTitle = "";
	(*JavaEnv)->PushLocalFrame(JavaEnv, 3);
	jstring s1 = (*JavaEnv)->NewStringUTF(JavaEnv, filename);
	jstring s2 = (*JavaEnv)->NewStringUTF(JavaEnv, saveId);
	jstring s3 = (*JavaEnv)->NewStringUTF(JavaEnv, dialogTitle);
	int result = (*JavaEnv)->CallBooleanMethod( JavaEnv, JavaRenderer, JavaRequestCloudLoad, s1, s2, s3 );
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s3);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s2);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s1);
	(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
	return result;
}

void SDLCALL SDL_ANDROID_OpenExternalApp(const char *package, const char *activity, const char *data)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->PushLocalFrame(JavaEnv, 3);
	jstring s1 = package ? (*JavaEnv)->NewStringUTF(JavaEnv, package) : (*JavaEnv)->NewStringUTF(JavaEnv, "");
	jstring s2 = activity ? (*JavaEnv)->NewStringUTF(JavaEnv, activity) : (*JavaEnv)->NewStringUTF(JavaEnv, "");
	jstring s3 = data ? (*JavaEnv)->NewStringUTF(JavaEnv, data) : (*JavaEnv)->NewStringUTF(JavaEnv, "");
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaRequestOpenExternalApp, s1, s2, s3 );
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s3);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s2);
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s1);
	(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
}

void SDLCALL SDL_ANDROID_RestartMyself(const char *restartParams)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->PushLocalFrame(JavaEnv, 1);
	jstring s1 = restartParams ? (*JavaEnv)->NewStringUTF(JavaEnv, restartParams) : (*JavaEnv)->NewStringUTF(JavaEnv, "");
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaRequestRestartMyself, s1 );
	(*JavaEnv)->DeleteLocalRef(JavaEnv, s1);
	(*JavaEnv)->PopLocalFrame(JavaEnv, NULL);
}

void SDLCALL SDL_ANDROID_SetConfigOption(int option, int value)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaRequestSetConfigOption, (jint)option, (jint)value );
}

void SDLCALL SDL_ANDROID_OpenExternalWebBrowser(const char *url)
{
	SDL_ANDROID_OpenExternalApp(NULL, NULL, url);
}

void SDLCALL SDL_ANDROID_SetSystemMousePointerVisible(int visible)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaSetSystemMousePointerVisible, (jint)visible );
}

void SDLCALL SDL_ANDROID_SetCapturedMousePosition(int x, int y)
{
	JNIEnv *JavaEnv = GetJavaEnv();
	(*JavaEnv)->CallVoidMethod( JavaEnv, JavaRenderer, JavaSetCapturedMousePosition, (jint)x, (jint)y );
}

// Dummy callback for SDL2 to satisfy linker
extern void SDL_Android_Init(JNIEnv* env, jclass cls);
void SDL_Android_Init(JNIEnv* env, jclass cls)
{
}
