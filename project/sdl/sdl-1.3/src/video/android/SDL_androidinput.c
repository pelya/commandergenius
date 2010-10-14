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
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"

#include "SDL_version.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"

SDLKey SDL_android_keymap[KEYCODE_LAST+1];


static int isTrackballUsed = 0;
static int isMouseUsed = 0;
static int isJoystickUsed = 0;
static int isMultitouchUsed = 0;
static SDL_Joystick *CurrentJoysticks[MAX_MULTITOUCH_POINTERS+1] = {NULL};
static int TrackballDampening = 0; // in milliseconds
static int lastTrackballAction = 0;

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouse) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action, jint pointerId, jint force, jint radius )
{
	if(pointerId < 0)
		pointerId = 0;
	if(pointerId > MAX_MULTITOUCH_POINTERS)
		pointerId = MAX_MULTITOUCH_POINTERS;

	if( SDL_ANDROID_isTouchscreenKeyboardUsed && SDL_ANDROID_processTouchscreenKeyboard(x, y, action, pointerId) )
		return;

#if SDL_VIDEO_RENDER_RESIZE
	// Translate mouse coordinates

#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_Window * window = SDL_GetFocusWindow();
	if( window && window->renderer->window ) {
		x = x * window->w / window->display->desktop_mode.w;
		y = y * window->h / window->display->desktop_mode.h;
	}
#else
	x = x * SDL_ANDROID_sFakeWindowWidth / SDL_ANDROID_sWindowWidth;
	y = y * SDL_ANDROID_sFakeWindowHeight / SDL_ANDROID_sWindowHeight;
#endif

#endif

	if( isMultitouchUsed )
	{
		if( CurrentJoysticks[pointerId] )
		{
			SDL_PrivateJoystickAxis(CurrentJoysticks[pointerId+1], 0, x);
			SDL_PrivateJoystickAxis(CurrentJoysticks[pointerId+1], 1, y);
			SDL_PrivateJoystickAxis(CurrentJoysticks[pointerId+1], 2, force);
			SDL_PrivateJoystickAxis(CurrentJoysticks[pointerId+1], 3, radius);
			if( action == MOUSE_DOWN )
				SDL_PrivateJoystickButton(CurrentJoysticks[pointerId+1], 0, SDL_PRESSED);
			if( action == MOUSE_UP )
				SDL_PrivateJoystickButton(CurrentJoysticks[pointerId+1], 0, SDL_RELEASED);
		}
	}
	if( !isMouseUsed && !SDL_ANDROID_isTouchscreenKeyboardUsed )
	{
		SDL_keysym keysym;
		if( action != MOUSE_MOVE )
			SDL_SendKeyboardKey( action == MOUSE_DOWN ? SDL_PRESSED : SDL_RELEASED, GetKeysym(SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_0)) ,&keysym) );
		return;
	}

	if( !isMouseUsed )
		return;

	if( action == MOUSE_DOWN || action == MOUSE_UP )
	{
#if SDL_VERSION_ATLEAST(1,3,0)
		SDL_SendMouseMotion(NULL, 0, x, y);
		SDL_SendMouseButton(NULL, (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, 1 );
#else
		SDL_PrivateMouseMotion(0, 0, x, y);
		SDL_PrivateMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, 1, x, y );
#endif
	}
	if( action == MOUSE_MOVE )
#if SDL_VERSION_ATLEAST(1,3,0)
		SDL_SendMouseMotion(NULL, 0, x, y);
#else
		SDL_PrivateMouseMotion(0, 0, x, y);
#endif
}

static int processAndroidTrackball(int key, int action);

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey) ( JNIEnv*  env, jobject thiz, jint key, jint action )
{
	if( isTrackballUsed )
		if( processAndroidTrackball(key, action) )
			return;
	SDL_keysym keysym;
	SDL_SendKeyboardKey( action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key ,&keysym) );
}


static void updateOrientation ( float accX, float accY, float accZ );

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AccelerometerReader_nativeAccelerometer) ( JNIEnv*  env, jobject  thiz, jfloat accPosX, jfloat accPosY, jfloat accPosZ )
{
	// Calculate two angles from three coordinates - TODO: this is faulty!
	//float accX = atan2f(-accPosX, sqrtf(accPosY*accPosY+accPosZ*accPosZ) * ( accPosY > 0 ? 1.0f : -1.0f ) ) * M_1_PI * 180.0f;
	//float accY = atan2f(accPosZ, accPosY) * M_1_PI;
	float normal = sqrt(accPosX*accPosX+accPosY*accPosY+accPosZ*accPosZ);
	if(normal <= 0.0000001f)
		normal = 1.0f;
	
	updateOrientation (accPosX/normal, accPosY/normal, 0.0f);
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AccelerometerReader_nativeOrientation) ( JNIEnv*  env, jobject  thiz, jfloat accX, jfloat accY, jfloat accZ )
{
	updateOrientation (accX, accY, accZ);
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTrackballUsed) ( JNIEnv*  env, jobject thiz)
{
	isTrackballUsed = 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetMouseUsed) ( JNIEnv*  env, jobject thiz)
{
	isMouseUsed = 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetJoystickUsed) ( JNIEnv*  env, jobject thiz)
{
	isJoystickUsed = 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetMultitouchUsed) ( JNIEnv*  env, jobject thiz)
{
	isMultitouchUsed = 1;
}

void ANDROID_InitOSKeymap()
{
  int i;
  SDLKey * keymap = SDL_android_keymap;

#if (SDL_VERSION_ATLEAST(1,3,0))
  SDLKey defaultKeymap[SDL_NUM_SCANCODES];
  SDL_GetDefaultKeymap(defaultKeymap);
  SDL_SetKeymap(0, defaultKeymap, SDL_NUM_SCANCODES);
#endif

  // TODO: keys are mapped rather randomly

  for (i=0; i<SDL_arraysize(SDL_android_keymap); ++i)
    SDL_android_keymap[i] = SDL_KEY(UNKNOWN);

  keymap[KEYCODE_UNKNOWN] = SDL_KEY(UNKNOWN);

  keymap[KEYCODE_BACK] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_5));

  // TODO: make this configurable
  keymap[KEYCODE_MENU] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_4));

  keymap[KEYCODE_SEARCH] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_1));
  keymap[KEYCODE_CALL] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_1));
  keymap[KEYCODE_DPAD_CENTER] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_1));

  //keymap[KEYCODE_CALL] = SDL_KEY(RCTRL);
  //keymap[KEYCODE_DPAD_CENTER] = SDL_KEY(LALT);
  
  keymap[KEYCODE_VOLUME_UP] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_2));
  keymap[KEYCODE_VOLUME_DOWN] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_3));
  
  keymap[KEYCODE_HOME] = SDL_KEY(HOME); // Cannot be used in application

  keymap[KEYCODE_CAMERA] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_6));

  keymap[KEYCODE_0] = SDL_KEY(0);
  keymap[KEYCODE_1] = SDL_KEY(1);
  keymap[KEYCODE_2] = SDL_KEY(2);
  keymap[KEYCODE_3] = SDL_KEY(3);
  keymap[KEYCODE_4] = SDL_KEY(4);
  keymap[KEYCODE_5] = SDL_KEY(5);
  keymap[KEYCODE_6] = SDL_KEY(6);
  keymap[KEYCODE_7] = SDL_KEY(7);
  keymap[KEYCODE_8] = SDL_KEY(8);
  keymap[KEYCODE_9] = SDL_KEY(9);
  keymap[KEYCODE_STAR] = SDL_KEY(KP_DIVIDE);
  keymap[KEYCODE_POUND] = SDL_KEY(KP_MULTIPLY);

  keymap[KEYCODE_DPAD_UP] = SDL_KEY(UP);
  keymap[KEYCODE_DPAD_DOWN] = SDL_KEY(DOWN);
  keymap[KEYCODE_DPAD_LEFT] = SDL_KEY(LEFT);
  keymap[KEYCODE_DPAD_RIGHT] = SDL_KEY(RIGHT);

  keymap[KEYCODE_SOFT_LEFT] = SDL_KEY(KP_4);
  keymap[KEYCODE_SOFT_RIGHT] = SDL_KEY(KP_6);
  keymap[KEYCODE_ENTER] = SDL_KEY(RETURN); //SDL_KEY(KP_ENTER);


  keymap[KEYCODE_CLEAR] = SDL_KEY(BACKSPACE);
  keymap[KEYCODE_A] = SDL_KEY(A);
  keymap[KEYCODE_B] = SDL_KEY(B);
  keymap[KEYCODE_C] = SDL_KEY(C);
  keymap[KEYCODE_D] = SDL_KEY(D);
  keymap[KEYCODE_E] = SDL_KEY(E);
  keymap[KEYCODE_F] = SDL_KEY(F);
  keymap[KEYCODE_G] = SDL_KEY(G);
  keymap[KEYCODE_H] = SDL_KEY(H);
  keymap[KEYCODE_I] = SDL_KEY(I);
  keymap[KEYCODE_J] = SDL_KEY(J);
  keymap[KEYCODE_K] = SDL_KEY(K);
  keymap[KEYCODE_L] = SDL_KEY(L);
  keymap[KEYCODE_M] = SDL_KEY(M);
  keymap[KEYCODE_N] = SDL_KEY(N);
  keymap[KEYCODE_O] = SDL_KEY(O);
  keymap[KEYCODE_P] = SDL_KEY(P);
  keymap[KEYCODE_Q] = SDL_KEY(Q);
  keymap[KEYCODE_R] = SDL_KEY(R);
  keymap[KEYCODE_S] = SDL_KEY(S);
  keymap[KEYCODE_T] = SDL_KEY(T);
  keymap[KEYCODE_U] = SDL_KEY(U);
  keymap[KEYCODE_V] = SDL_KEY(V);
  keymap[KEYCODE_W] = SDL_KEY(W);
  keymap[KEYCODE_X] = SDL_KEY(X);
  keymap[KEYCODE_Y] = SDL_KEY(Y);
  keymap[KEYCODE_Z] = SDL_KEY(Z);
  keymap[KEYCODE_COMMA] = SDL_KEY(COMMA);
  keymap[KEYCODE_PERIOD] = SDL_KEY(PERIOD);
  keymap[KEYCODE_TAB] = SDL_KEY(TAB);
  keymap[KEYCODE_SPACE] = SDL_KEY(SPACE);
  keymap[KEYCODE_DEL] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_8));
  keymap[KEYCODE_GRAVE] = SDL_KEY(GRAVE);
  keymap[KEYCODE_MINUS] = SDL_KEY(KP_MINUS);
  keymap[KEYCODE_PLUS] = SDL_KEY(KP_PLUS);
  keymap[KEYCODE_EQUALS] = SDL_KEY(EQUALS);
  keymap[KEYCODE_LEFT_BRACKET] = SDL_KEY(LEFTBRACKET);
  keymap[KEYCODE_RIGHT_BRACKET] = SDL_KEY(RIGHTBRACKET);
  keymap[KEYCODE_BACKSLASH] = SDL_KEY(BACKSLASH);
  keymap[KEYCODE_SEMICOLON] = SDL_KEY(SEMICOLON);
  keymap[KEYCODE_APOSTROPHE] = SDL_KEY(APOSTROPHE);
  keymap[KEYCODE_SLASH] = SDL_KEY(SLASH);
  keymap[KEYCODE_AT] = SDL_KEY(KP_PERIOD);

  keymap[KEYCODE_MEDIA_PLAY_PAUSE] = SDL_KEY(KP_2);
  keymap[KEYCODE_MEDIA_STOP] = SDL_KEY(HELP);
  keymap[KEYCODE_MEDIA_NEXT] = SDL_KEY(KP_8);
  keymap[KEYCODE_MEDIA_PREVIOUS] = SDL_KEY(KP_5);
  keymap[KEYCODE_MEDIA_REWIND] = SDL_KEY(KP_1);
  keymap[KEYCODE_MEDIA_FAST_FORWARD] = SDL_KEY(KP_3);
  keymap[KEYCODE_MUTE] = SDL_KEY(KP_0);

  keymap[KEYCODE_SYM] = SDL_KEY(LGUI);
  keymap[KEYCODE_NUM] = SDL_KEY(NUMLOCKCLEAR);

  keymap[KEYCODE_ALT_LEFT] = SDL_KEY(KP_7);
  keymap[KEYCODE_ALT_RIGHT] = SDL_KEY(KP_9);

  keymap[KEYCODE_SHIFT_LEFT] = SDL_KEY(F1);
  keymap[KEYCODE_SHIFT_RIGHT] = SDL_KEY(F2);

  keymap[KEYCODE_EXPLORER] = SDL_KEY(F3);
  keymap[KEYCODE_ENVELOPE] = SDL_KEY(F4);

  keymap[KEYCODE_HEADSETHOOK] = SDL_KEY(F5);
  keymap[KEYCODE_FOCUS] = SDL_KEY(F6);
  keymap[KEYCODE_NOTIFICATION] = SDL_KEY(F7);

  // Cannot be received by application, OS internal
  keymap[KEYCODE_ENDCALL] = SDL_KEY(LSHIFT);
  keymap[KEYCODE_POWER] = SDL_KEY(RALT);

}

static float dx = 0.04, dy = 0.1, dz = 0.1; // For accelerometer

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetAccelerometerSensitivity) ( JNIEnv*  env, jobject thiz, jint value)
{
	dx = 0.04; dy = 0.08; dz = 0.08; // Fast sensitivity
	if( value == 1 ) // Medium sensitivity
	{
		dx = 0.1; dy = 0.15; dz = 0.15;
	}
	if( value == 2 ) // Slow sensitivity
	{
		dx = 0.2; dy = 0.25; dz = 0.25;
	}
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTrackballDampening) ( JNIEnv*  env, jobject thiz, jint value)
{
	TrackballDampening = (value * 200);
}

void updateOrientation ( float accX, float accY, float accZ )
{
	SDL_keysym keysym;
	// TODO: ask user for accelerometer precision from Java

	static float midX = 0, midY = 0, midZ = 0;
	static int pressLeft = 0, pressRight = 0, pressUp = 0, pressDown = 0, pressR = 0, pressL = 0;
	
	midX = 0.0f; // Do not remember old value for phone tilt, it feels weird
	
	if( isJoystickUsed && CurrentJoysticks[0] ) // TODO: mutex for that stuff?
	{
		SDL_PrivateJoystickAxis(CurrentJoysticks[0], 0, (Sint16)(fmin(32767, fmax(-32768, accX * 400.0))));
		SDL_PrivateJoystickAxis(CurrentJoysticks[0], 1, (Sint16)(fmin(32767, fmax(-32768, accY * 400.0))));
		SDL_PrivateJoystickAxis(CurrentJoysticks[0], 2, (Sint16)(fmin(32767, fmax(-32768, accZ * 400.0))));
	}

	if(isJoystickUsed)
		return;

	
	if( accX < midX - dx )
	{
		if( !pressLeft )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press left, acc %f mid %f d %f", accX, midX, dx);
			pressLeft = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_LEFT, &keysym) );
		}
	}
	else
	{
		if( pressLeft )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release left, acc %f mid %f d %f", accX, midX, dx);
			pressLeft = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT, &keysym) );
		}
	}
	if( accX < midX - dx*2 )
		midX = accX + dx*2;

	if( accX > midX + dx )
	{
		if( !pressRight )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press right, acc %f mid %f d %f", accX, midX, dx);
			pressRight = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_RIGHT, &keysym) );
		}
	}
	else
	{
		if( pressRight )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release right, acc %f mid %f d %f", accX, midX, dx);
			pressRight = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT, &keysym) );
		}
	}
	if( accX > midX + dx*2 )
		midX = accX - dx*2;

	if( accY < midY - dy )
	{
		if( !pressUp )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press up, acc %f mid %f d %f", accY, midY, dy);
			pressUp = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_DOWN, &keysym) );
		}
	}
	else
	{
		if( pressUp )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release up, acc %f mid %f d %f", accY, midY, dy);
			pressUp = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN, &keysym) );
		}
	}
	if( accY < midY - dy*2 )
		midY = accY + dy*2;

	if( accY > midY + dy )
	{
		if( !pressDown )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press down, acc %f mid %f d %f", accY, midY, dy);
			pressDown = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_UP, &keysym) );
		}
	}
	else
	{
		if( pressDown )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release down, acc %f mid %f d %f", accY, midY, dy);
			pressDown = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP, &keysym) );
		}
	}
	if( accY > midY + dy*2 )
		midY = accY - dy*2;

	if( accZ < midZ - dz )
	{
		if( !pressL )
		{
			pressL = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_ALT_LEFT, &keysym) );
		}
	}
	else
	{
		if( pressL )
		{
			pressL = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_ALT_LEFT, &keysym) );
		}
	}
	if( accZ < midZ - dz*2 )
		midZ = accZ + dz*2;

	if( accZ > midZ + dz )
	{
		if( !pressR )
		{
			pressR = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_ALT_RIGHT, &keysym) );
		}
	}
	else
	{
		if( pressR )
		{
			pressR = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_ALT_RIGHT, &keysym) );
		}
	}
	if( accZ > midZ + dz*2 )
		midZ = accZ - dz*2;

}

static int leftPressed = 0, rightPressed = 0, upPressed = 0, downPressed = 0;

int processAndroidTrackball(int key, int action)
{
	SDL_keysym keysym;
	
	if( ! action && (
		key == KEYCODE_DPAD_UP ||
		key == KEYCODE_DPAD_DOWN ||
		key == KEYCODE_DPAD_LEFT ||
		key == KEYCODE_DPAD_RIGHT ) )
		return 1;
	lastTrackballAction = SDL_GetTicks();

	if( key == KEYCODE_DPAD_UP )
	{
		if( downPressed )
		{
			downPressed = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN ,&keysym) );
			return 1;
		}
		if( !upPressed )
		{
			upPressed = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		else
		{
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(key ,&keysym) );
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_DOWN )
	{
		if( upPressed )
		{
			upPressed = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP ,&keysym) );
			return 1;
		}
		if( !upPressed )
		{
			downPressed = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		else
		{
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(key ,&keysym) );
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_LEFT )
	{
		if( rightPressed )
		{
			rightPressed = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT ,&keysym) );
			return 1;
		}
		if( !leftPressed )
		{
			leftPressed = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		else
		{
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(key ,&keysym) );
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_RIGHT )
	{
		if( leftPressed )
		{
			leftPressed = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT ,&keysym) );
			return 1;
		}
		if( !rightPressed )
		{
			rightPressed = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		else
		{
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(key ,&keysym) );
			SDL_SendKeyboardKey( SDL_PRESSED, TranslateKey(key ,&keysym) );
		}
		return 1;
	}

	return 0;
}

void SDL_ANDROID_processAndroidTrackballDampening()
{
	SDL_keysym keysym;
	if( !TrackballDampening )
		return;
	if( SDL_GetTicks() - lastTrackballAction > TrackballDampening )
	{
		if( upPressed )
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP ,&keysym) );
		if( downPressed )
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN ,&keysym) );
		if( leftPressed )
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT ,&keysym) );
		if( rightPressed )
			SDL_SendKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT ,&keysym) );
		upPressed = 0;
		downPressed = 0;
		leftPressed = 0;
		rightPressed = 0;
	}
}

int SDL_SYS_JoystickInit(void)
{
	SDL_numjoysticks = MAX_MULTITOUCH_POINTERS+1;
	return(SDL_numjoysticks);
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	if(index)
		return("Android multitouch");
	return("Android accelerometer/orientation sensor");
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	joystick->nbuttons = 0; // Ignored
	joystick->nhats = 0;
	joystick->nballs = 0;
	if( joystick->index == 0 )
		joystick->naxes = 3;
	else
	{
		joystick->naxes = 4;
		joystick->nbuttons = 1;
	}
	CurrentJoysticks[joystick->index] = joystick;
	return(0);
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	return;
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	CurrentJoysticks[joystick->index] = NULL;
	return;
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	int i;
	for(i=0; i<MAX_MULTITOUCH_POINTERS+1; i++)
		CurrentJoysticks[i] = NULL;
	return;
}
