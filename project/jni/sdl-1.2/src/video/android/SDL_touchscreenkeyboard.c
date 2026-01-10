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
#include <netinet/in.h>

#define GL_GLEXT_PROTOTYPES 1

#include "SDL_opengles.h"

#include "SDL_config.h"

#include "SDL_version.h"

#include "SDL_screenkeyboard.h"
#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"
#include "atan2i.h"

// TODO: this code is a HUGE MESS

enum {
	MAX_JOYSTICKS = 3,
	MAX_BUTTONS = SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM,  // Max amount of custom buttons
	MAX_BUTTONS_AUTOFIRE = 2,
	BUTTON_TEXT_INPUT = SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT,
	BUTTON_ARROWS = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD
};

int SDL_ANDROID_isTouchscreenKeyboardUsed = 0;
static short touchscreenKeyboardTheme = 0;
static short touchscreenKeyboardShown = 1;
static SDL_Rect hiddenButtons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM];
static short buttonsize = 1;
static short buttonDrawSize = 1;
static float transparency = 128.0f/255.0f;
static int preventButtonOverlap = 0;

static SDL_Rect arrows[MAX_JOYSTICKS], arrowsExtended[MAX_JOYSTICKS], buttons[MAX_BUTTONS];
static SDL_Rect arrowsDraw[MAX_JOYSTICKS], buttonsDraw[MAX_BUTTONS];
static SDLKey buttonKeysyms[MAX_BUTTONS] = {
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_0)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_1)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_2)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_3)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_4)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_5)),
SDLK_UNKNOWN, // Text input
SDLK_UNKNOWN, // Joystick 0
SDLK_UNKNOWN, // Joystick 1
SDLK_UNKNOWN, // Joystick 2
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_6)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_7)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_8)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_9)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_10)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_11)),
};

enum { ARROW_LEFT = 1, ARROW_RIGHT = 2, ARROW_UP = 4, ARROW_DOWN = 8 };
static short oldArrows = 0;

static Sint8 pointerInButtonRect[MAX_BUTTONS];
static Sint8 buttonsGenerateSdlEvents[MAX_BUTTONS];
static Sint8 buttonsStayPressedAfterTouch[MAX_BUTTONS];

typedef struct
{
    GLuint id;
    GLfloat w;
    GLfloat h;
} GLTexture_t;

static GLTexture_t arrowImages[12];
static GLTexture_t buttonAutoFireImages[MAX_BUTTONS_AUTOFIRE*2]; // These are not used anymore
static GLTexture_t buttonImages[MAX_BUTTONS*2];
static GLTexture_t mousePointer;
enum { MOUSE_POINTER_W = 32, MOUSE_POINTER_H = 32, MOUSE_POINTER_X = 5, MOUSE_POINTER_Y = 7 }; // X and Y are offsets of the pointer tip

static int themeType = 0;
static int joystickTouchPoints[MAX_JOYSTICKS*2];
static int floatingScreenJoystick = 0;

int SDL_ANDROID_AsyncTextInputActive = 0;

static inline int InsideRect(const SDL_Rect * r, int x, int y)
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}

// Find the intersection of a line and a rectangle,
// where on of the line points and the center of rectangle
// are both at the coordinate [0,0].
// It returns the remaining line segment outside of the rectangle.
// Do not check for border condition, we check that the line point
// is outside of the rectangle in another function.
static inline void LineAndRectangleIntersection(
		int lx, int ly, // Line point, that is outside of rectangle
		int rw, int rh, // Rectangle dimensions
		int *x, int *y)
{
	if( abs(lx) * rh > abs(ly) * rw )
	{
		rw /= 2;
		// Intersection at the left side
		if( lx < -rw )
			*x = lx + rw; // lx is negative
		else //if( lx > rw ) // At the right side
			*x = lx - rw; // lx is positive
		*y = *x * ly / lx;
	}
	else
	{
		rh /= 2;
		// At the top
		if( ly < -rh )
			*y = ly + rh; // ly is negative
		else //if( ly > rh ) // At the right side
			*y = ly - rh; // ly is positive
		*x = *y * lx / ly;
	}
}

static struct ScreenKbGlState_t
{
	GLboolean texture2d;
	GLuint texunitId;
	GLuint clientTexunitId;
	GLuint textureId;
	GLfloat color[4];
	GLint texEnvMode;
	GLboolean blend;
	GLenum blend1, blend2;
	GLint texFilter1, texFilter2;
	GLboolean colorArray;
}
oldGlState;

static inline void beginDrawingTex()
{
#if SDL_VIDEO_OPENGL_ES_VERSION == 1

#ifndef SDL_TOUCHSCREEN_KEYBOARD_SAVE_RESTORE_OPENGL_STATE
	// Make the video somehow work on emulator
	oldGlState.texture2d = GL_TRUE;
	oldGlState.texunitId = GL_TEXTURE0;
	oldGlState.clientTexunitId = GL_TEXTURE0;
	oldGlState.textureId = 0;
	oldGlState.texEnvMode = GL_MODULATE;
	oldGlState.blend = GL_TRUE;
	oldGlState.blend1 = GL_SRC_ALPHA;
	oldGlState.blend2 = GL_ONE_MINUS_SRC_ALPHA;
	oldGlState.colorArray = GL_FALSE;
#else
	// Save OpenGL state
	// This code does not work on 1.6 emulator, and on some older devices
	// However GLES 1.1 spec defines all theese values, so it's a device fault for not implementing them
	oldGlState.texture2d = glIsEnabled(GL_TEXTURE_2D);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &oldGlState.texunitId);
#endif

	/*
	glActiveTexture(GL_TEXTURE1);
	glClientActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	*/

	glActiveTexture(GL_TEXTURE0);

#ifdef SDL_TOUCHSCREEN_KEYBOARD_SAVE_RESTORE_OPENGL_STATE
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldGlState.textureId);
	glGetFloatv(GL_CURRENT_COLOR, &(oldGlState.color[0]));
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &oldGlState.texEnvMode);
	oldGlState.blend = glIsEnabled(GL_BLEND);
	glGetIntegerv(GL_BLEND_SRC, &oldGlState.blend1);
	glGetIntegerv(GL_BLEND_DST, &oldGlState.blend2);
	//glGetBooleanv(GL_COLOR_ARRAY, &oldGlState.colorArray);
	// It's very unlikely that some app will use GL_TEXTURE_CROP_RECT_OES, so just skip it
#endif

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableClientState(GL_COLOR_ARRAY);
	//static const GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_ALPHA_TEST);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
}

static inline void endDrawingTex()
{
	// Restore OpenGL state
#if SDL_VIDEO_OPENGL_ES_VERSION == 1
	glBindTexture(GL_TEXTURE_2D, oldGlState.textureId);
	if( oldGlState.blend == GL_FALSE )
		glDisable(GL_BLEND);
	glBlendFunc(oldGlState.blend1, oldGlState.blend2);
	glActiveTexture(oldGlState.texunitId);

	if( oldGlState.texture2d == GL_FALSE )
		glDisable(GL_TEXTURE_2D);
	glColor4f(oldGlState.color[0], oldGlState.color[1], oldGlState.color[2], oldGlState.color[3]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, oldGlState.texEnvMode);
	if( oldGlState.colorArray )
		glEnableClientState(GL_COLOR_ARRAY);
#endif
}

static inline void drawCharTexFlip(GLTexture_t * tex, SDL_Rect * src, SDL_Rect * dest, int flipX, int flipY, float r, float g, float b, float a)
{
#if SDL_VIDEO_OPENGL_ES_VERSION == 1
	GLint cropRect[4];

	if( !dest->h || !dest->w )
		return;

	glBindTexture(GL_TEXTURE_2D, tex->id);

	glColor4f(r, g, b, a);

	if(src)
	{
		cropRect[0] = src->x;
		cropRect[1] = tex->h - src->y;
		cropRect[2] = src->w;
		cropRect[3] = -src->h;
	}
	else
	{
		cropRect[0] = 0;
		cropRect[1] = tex->h;
		cropRect[2] = tex->w;
		cropRect[3] = -tex->h;
	}
	if(flipX)
	{
		cropRect[0] += cropRect[2];
		cropRect[2] = -cropRect[2];
	}
	if(flipY)
	{
		cropRect[1] += cropRect[3];
		cropRect[3] = -cropRect[3];
	}
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
	glDrawTexiOES(dest->x + SDL_ANDROID_ScreenVisibleRect.x, SDL_ANDROID_sRealWindowHeight - dest->y - dest->h - SDL_ANDROID_ScreenVisibleRect.y, 0, dest->w, dest->h);
#endif
}

static inline void drawCharTex(GLTexture_t * tex, SDL_Rect * src, SDL_Rect * dest, float r, float g, float b, float a)
{
	drawCharTexFlip(tex, src, dest, 0, 0, r, g, b, a);
}

static void drawTouchscreenKeyboardLegacy()
{
	int i;
	float blendFactor;

	if( SDL_ANDROID_joysticksAmount >= 1 )
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
	else if( arrowImages[8].id == 0 ) // No diagonal arrow images
	{
		blendFactor =		( SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] ? 4 : 0 ) +
							( SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] ? 4 : 0 ) +
							( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] ? 4 : 0 ) +
							( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] ? 4 : 0 );
		if (blendFactor >= 8)
			blendFactor = 7;
		if( blendFactor == 0 )
			drawCharTex( &arrowImages[0], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else
		{
			if( SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] )
				drawCharTex( &arrowImages[1], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency * 4 / blendFactor );
			if( SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] )
				drawCharTex( &arrowImages[2], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency * 4 / blendFactor );
			if( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] )
				drawCharTex( &arrowImages[3], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency * 4 / blendFactor );
			if( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] )
				drawCharTex( &arrowImages[4], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency * 4 / blendFactor );
		}
	}
	else  // Diagonal arrow images present
	{
		if( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] && SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] )
			drawCharTex( &arrowImages[5], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] && SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] )
			drawCharTex( &arrowImages[6], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] && SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] )
			drawCharTex( &arrowImages[7], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] && SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] )
			drawCharTex( &arrowImages[8], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] )
			drawCharTex( &arrowImages[1], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] )
			drawCharTex( &arrowImages[2], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] )
			drawCharTex( &arrowImages[3], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else if( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] )
			drawCharTex( &arrowImages[4], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
		else
			drawCharTex( &arrowImages[0], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
	}

	if( SDL_ANDROID_joysticksAmount >= 2 )
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[1], 1.0f, 1.0f, 1.0f, transparency );
	if( SDL_ANDROID_joysticksAmount >= 3 )
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[2], 1.0f, 1.0f, 1.0f, transparency );

	for( i = 0; i < MAX_BUTTONS; i++ )
	{
		if( ! buttons[i].h || ! buttons[i].w )
			continue;

		drawCharTex( &buttonImages[ SDL_GetKeyboardState(NULL)[buttonKeysyms[i]] ? (i * 2 + 1) : (i * 2) ],
					NULL, &buttonsDraw[i], 1.0f, 1.0f, 1.0f, transparency );
	}
}

static void drawTouchscreenKeyboardSun()
{
	int i;

	for( i = 0; i < SDL_ANDROID_joysticksAmount || (i == 0 && arrowsDraw[0].w > 0); i++ )
	{
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[i], 1.0f, 1.0f, 1.0f, transparency );
		if(pointerInButtonRect[BUTTON_ARROWS+i] != -1)
		{
			SDL_Rect touch = arrowsDraw[i];
			touch.w /= 2;
			touch.h /= 2;
			touch.x = joystickTouchPoints[0+i*2] - touch.w / 2;
			touch.y = joystickTouchPoints[1+i*2] - touch.h / 2;
			drawCharTex( &arrowImages[0], NULL, &touch, 1.0f, 1.0f, 1.0f, transparency );
		}
	}

	for( i = 0; i < MAX_BUTTONS; i++ )
	{
		int pressed = SDL_GetKeyboardState(NULL)[buttonKeysyms[i]];
		int flip = (i >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_2 && i <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5) ||
					(i >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_8 && i <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_11);

		if( ! buttons[i].h || ! buttons[i].w )
			continue;

		drawCharTexFlip( &buttonImages[ pressed ? (i * 2 + 1) : (i * 2) ],
						NULL, &buttonsDraw[i], (flip && pressed), 0, 1.0f, 1.0f, 1.0f, transparency );
	}
}

static void drawTouchscreenKeyboardDualShock()
{
	int i;

	for( i = 0; i < SDL_ANDROID_joysticksAmount || (i == 0 && arrowsDraw[0].w > 0); i++ )
	{
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[i], 1.0f, 1.0f, 1.0f, transparency );
		if(pointerInButtonRect[BUTTON_ARROWS+i] != -1)
		{
			SDL_Rect touch = arrowsDraw[i];
			touch.w /= 1.5;
			touch.h /= 1.5;
			touch.x = joystickTouchPoints[0+i*2] - touch.w / 2;
			touch.y = joystickTouchPoints[1+i*2] - touch.h / 2;
			drawCharTex( &arrowImages[6], NULL, &touch, 1.0f, 1.0f, 1.0f, transparency );
		}
		else
		{
			SDL_Rect touch = arrowsDraw[i];
			touch.w /= 1.5;
			touch.h /= 1.5;
			touch.x = arrowsDraw[i].x + touch.w / 4;
			touch.y = arrowsDraw[i].y + touch.h / 4;
			drawCharTex( &arrowImages[6], NULL, &touch, 1.0f, 1.0f, 1.0f, transparency );
		}
	}

	for( i = 0; i < MAX_BUTTONS; i++ )
	{
		int pressed = SDL_GetKeyboardState(NULL)[buttonKeysyms[i]];
		int flip = (i >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_2 && i <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5) ||
					(i >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_8 && i <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_11);

		if( ! buttons[i].h || ! buttons[i].w )
			continue;

		drawCharTexFlip( &buttonImages[ pressed ? (i * 2 + 1) : (i * 2) ],
						NULL, &buttonsDraw[i], (flip && pressed), 0, 1.0f, 1.0f, 1.0f, transparency );
	}
}

int SDL_ANDROID_drawTouchscreenKeyboard()
{
	if( !SDL_ANDROID_isTouchscreenKeyboardUsed || !touchscreenKeyboardShown )
		return 0;

	beginDrawingTex();

	if(themeType==1)
		drawTouchscreenKeyboardSun();
	else if(themeType==2)
		drawTouchscreenKeyboardDualShock();
	else
		drawTouchscreenKeyboardLegacy();

	endDrawingTex();

	return 1;
};

static inline int ArrowKeysPressed(int x, int y)
{
	int ret = 0, dx, dy;
	dx = x - arrows[0].x - arrows[0].w / 2;
	dy = y - arrows[0].y - arrows[0].h / 2;

	// Small deadzone at the center
	if( abs(dx) < arrows[0].w / 20 && abs(dy) < arrows[0].h / 20 )
		return ret;

	// Single arrow key pressed
	if( abs(dy / 2) >= abs(dx) )
	{
		if( dy < 0 )
			ret |= ARROW_UP;
		else
			ret |= ARROW_DOWN;
	}
	else
	if( abs(dx / 2) >= abs(dy) )
	{
		if( dx > 0 )
			ret |= ARROW_RIGHT;
		else
			ret |= ARROW_LEFT;
	}
	else // Two arrow keys pressed
	{
		if( dx > 0 )
			ret |= ARROW_RIGHT;
		else
			ret |= ARROW_LEFT;

		if( dy < 0 )
			ret |= ARROW_UP;
		else
			ret |= ARROW_DOWN;
	}
	return ret;
}

unsigned SDL_ANDROID_processTouchscreenKeyboard(int x, int y, int action, int pointerId)
{
	int i, j;
	unsigned processed = 0;
	int joyAmount = SDL_ANDROID_joysticksAmount;
	if( joyAmount == 0 && (arrows[0].w > 0 || floatingScreenJoystick) )
		joyAmount = 1;
	
	if( !touchscreenKeyboardShown )
	{
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( pointerInButtonRect[i] != -1 )
			{
				pointerInButtonRect[i] = -1;
				if( buttonKeysyms[i] != SDLK_UNKNOWN )
				{
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, buttonKeysyms[i], 0 );
				}
			}
		}
		for( j = 0; j < joyAmount; j++ )
		{
			if( pointerInButtonRect[BUTTON_ARROWS+j] != -1 )
			{
				pointerInButtonRect[BUTTON_ARROWS+j] = -1;
				if( SDL_ANDROID_joysticksAmount > 0 )
				{
					int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
					SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, 0 );
					SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, 0 );
				}
				else
				{
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
					oldArrows = 0;
				}
			}
		}
		return 0;
	}
	
	if( action == MOUSE_DOWN )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "touch %03dx%03d ptr %d action %d", x, y, pointerId, action);
		int processOtherButtons = 1;

		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( InsideRect( &buttons[i], x, y) )
			{
				processed |= 1<<i;
				if( pointerInButtonRect[i] == -1 )
				{
					pointerInButtonRect[i] = pointerId;
					if( i == BUTTON_TEXT_INPUT )
					{
						SDL_ANDROID_ToggleScreenKeyboardTextInput(NULL);
					}
					else if( buttonKeysyms[i] != SDLK_UNKNOWN )
					{
						if( buttonsStayPressedAfterTouch[i] )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_GetKeyboardState(NULL)[buttonKeysyms[i]] == 0 ? SDL_PRESSED : SDL_RELEASED, buttonKeysyms[i], 0 );
						else
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, buttonKeysyms[i], 0 );
					}
					if( preventButtonOverlap )
					{
						processOtherButtons = 0;
						break;
					}
				}
			}
		}

		for( j = 0; j < joyAmount && processOtherButtons; j++ )
		{
			if( InsideRect( &arrows[j], x, y ) )
			{
				processed |= 1<<(BUTTON_ARROWS+j);
				if( pointerInButtonRect[BUTTON_ARROWS+j] == -1 )
				{
					pointerInButtonRect[BUTTON_ARROWS+j] = pointerId;
					joystickTouchPoints[0+j*2] = x;
					joystickTouchPoints[1+j*2] = y;
					if( SDL_ANDROID_joysticksAmount > 0 )
					{
						int xx = (x - arrows[j].x - arrows[j].w / 2) * 65534 / arrows[j].w;
						if( xx == 0 ) // Do not allow (0,0) coordinate, when the user touches the joystick - this indicates 'finger up' in OpenArena
							xx = 1;   // Yeah, maybe I should not include app-specific hacks into the library
						int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, xx );
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, (y - arrows[j].y - arrows[j].h / 2) * 65534 / arrows[j].h );
					}
					else
					{
						i = ArrowKeysPressed(x, y);
						if( i & ARROW_UP )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(UP), 0 );
						if( i & ARROW_DOWN )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(DOWN), 0 );
						if( i & ARROW_LEFT )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(LEFT), 0 );
						if( i & ARROW_RIGHT )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(RIGHT), 0 );
						oldArrows = i;
					}
				}
				if( preventButtonOverlap )
				{
					processOtherButtons = 0;
					break;
				}
			}
		}

		if( floatingScreenJoystick && !processed && pointerInButtonRect[BUTTON_ARROWS] == -1 )
		{
			// Center joystick under finger
			SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, 1);
			arrows[0].x = x - arrows[0].w / 2;
			arrows[0].y = y - arrows[0].h / 2;
			arrowsDraw[0] = arrowsExtended[0] = arrows[0];
			processed |= 1<<BUTTON_ARROWS;
			pointerInButtonRect[BUTTON_ARROWS] = pointerId;
			joystickTouchPoints[0] = x;
			joystickTouchPoints[1] = y;
			if( SDL_ANDROID_joysticksAmount > 0 )
			{
				SDL_ANDROID_MainThreadPushJoystickAxis( 0, 0, 1 ); // Non-zero joystick coordinate
				SDL_ANDROID_MainThreadPushJoystickAxis( 0, 1, 0 );
			}
		}
	}
	else
	if( action == MOUSE_UP )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "touch %03dx%03d ptr %d action %d", x, y, pointerId, action);
		for( j = 0; j < joyAmount; j++ )
		{
			if( pointerInButtonRect[BUTTON_ARROWS+j] == pointerId )
			{
				processed |= 1<<(BUTTON_ARROWS+j);
				pointerInButtonRect[BUTTON_ARROWS+j] = -1;
				if( SDL_ANDROID_joysticksAmount > 0 )
				{
					int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
					SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, 0 );
					SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, 0 );
				}
				else
				{
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
					oldArrows = 0;
				}
				if( floatingScreenJoystick && j == 0 )
					SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, 0);
			}
		}
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( pointerInButtonRect[i] == pointerId )
			{
				processed |= 1<<i;
				pointerInButtonRect[i] = -1;
				if( i != BUTTON_TEXT_INPUT && !buttonsStayPressedAfterTouch[i] && buttonKeysyms[i] != SDLK_UNKNOWN )
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, buttonKeysyms[i], 0 );
			}
		}
	}
	else
	if( action == MOUSE_MOVE )
	{
		// Process cases when pointer enters button area (it won't send keypress twice if button already pressed)
		int processOtherButtons = 1;
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( pointerInButtonRect[i] == pointerId )
			{
				if (buttonsGenerateSdlEvents[i] || preventButtonOverlap)
				{
					processOtherButtons = 0;
					break;
				}
			}
		}
		if( processOtherButtons )
		{
			processed |= SDL_ANDROID_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
		}
		
		// Process cases when pointer leaves button area
		// TODO: huge code size, split it or somehow make it more readable
		for( j = 0; j < joyAmount; j++ )
		{
			if( pointerInButtonRect[BUTTON_ARROWS+j] == pointerId )
			{
				processed |= 1<<(BUTTON_ARROWS+j);
				/*
				if( floatingScreenJoystick && j == 0 && ! InsideRect( &arrows[j], x, y ) )
				{
					int xx = 0, yy = 0;
					// A finger moved outside the joystick - move the joystick back under the finger
					LineAndRectangleIntersection(x - (arrows[0].x + arrows[0].w / 2), y - (arrows[0].y + arrows[0].h / 2), arrows[0].w, arrows[0].h, &xx, &yy);
					arrows[0].x += xx;
					arrows[0].y += yy;
					arrowsDraw[0] = arrowsExtended[0] = arrows[0];
				}
				*/
				if( ! InsideRect( &arrowsExtended[j], x, y ) && ! floatingScreenJoystick )
				{
					pointerInButtonRect[BUTTON_ARROWS+j] = -1;
					if( SDL_ANDROID_joysticksAmount > 0 )
					{
						int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, 0 );
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, 0 );
					}
					else
					{
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
						oldArrows = 0;
					}
				}
				else
				{
					joystickTouchPoints[0+j*2] = x;
					joystickTouchPoints[1+j*2] = y;
					if( SDL_ANDROID_joysticksAmount > 0 )
					{
						int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
						int xx = (x - arrows[j].x - arrows[j].w / 2) * 65534 / arrows[j].w;
						if( xx == 0 ) // Do not allow (0,0) coordinate, when the user touches the joystick - this indicates 'finger up' in OpenArena
							xx = 1;   // Yeah, maybe I should not include app-specific hacks into the library
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, xx );
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, (y - arrows[j].y - arrows[j].h / 2) * 65534 / arrows[j].h );
					}
					else
					{
						i = ArrowKeysPressed(x, y);
						if( i != oldArrows )
						{
							if( oldArrows & ARROW_UP && ! (i & ARROW_UP) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
							if( oldArrows & ARROW_DOWN && ! (i & ARROW_DOWN) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
							if( oldArrows & ARROW_LEFT && ! (i & ARROW_LEFT) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
							if( oldArrows & ARROW_RIGHT && ! (i & ARROW_RIGHT) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
							if( i & ARROW_UP )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(UP), 0 );
							if( i & ARROW_DOWN )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(DOWN), 0 );
							if( i & ARROW_LEFT )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(LEFT), 0 );
							if( i & ARROW_RIGHT )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(RIGHT), 0 );
						}
						oldArrows = i;
					}
				}
			}
		}
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( pointerInButtonRect[i] == pointerId )
			{
				processed |= 1<<i;
				if( ! InsideRect( &buttons[i], x, y ) && ! buttonsGenerateSdlEvents[i] )
				{
					pointerInButtonRect[i] = -1;
					if( i != BUTTON_TEXT_INPUT && buttonKeysyms[i] != SDLK_UNKNOWN )
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, buttonKeysyms[i], 0 );
				}
			}
		}
	}

	for( i = 0; i <= MAX_BUTTONS ; i++ )
		if( ( processed & (1<<i) ) && buttonsGenerateSdlEvents[i] )
			processed |= TOUCHSCREEN_KEYBOARD_PASS_EVENT_DOWN_TO_SDL;
	return processed;
};

void shrinkButtonRect(SDL_Rect s, SDL_Rect * d)
{
	int i;

	if( !buttonDrawSize )
	{
		memcpy(d, &s, sizeof(s));
		return;
	}

	d->w = s.w * 2 / (buttonDrawSize+3);
	d->h = s.h * 2 / (buttonDrawSize+3);
	d->x = s.x + s.w / 2 - d->w / 2;
	d->y = s.y + s.h / 2 - d->h / 2;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboard) ( JNIEnv* env, jobject thiz,
		jint size, jint drawsize, jint theme, jint _transparency, jint _floatingScreenJoystick, jint buttonAmount )
{
	int i, ii;
	int nbuttons1row, nbuttons2row;
	int _nbuttons = MAX_BUTTONS;
	SDL_Rect * r;

	// TODO: screenRatio is not used yet
	enum { STANDARD_PHONE_SCREEN_HEIGHT = 70 }; // And by "standard phone", I mean my own.
	float screenRatio = getenv("DISPLAY_HEIGHT_MM") ? atoi(getenv("DISPLAY_HEIGHT_MM")) / STANDARD_PHONE_SCREEN_HEIGHT : 1.0f;
	if( screenRatio < STANDARD_PHONE_SCREEN_HEIGHT )
		screenRatio = STANDARD_PHONE_SCREEN_HEIGHT;

	touchscreenKeyboardTheme = theme;
	// TODO: works for horizontal screen orientation only!
	buttonsize = size;
	buttonDrawSize = drawsize;
	switch(_transparency)
	{
		case 0: transparency = 32.0f/255.0f; break;
		case 1: transparency = 64.0f/255.0f; break;
		case 2: transparency = 128.0f/255.0f; break;
		case 3: transparency = 192.0f/255.0f; break;
		case 4: transparency = 255.0f/255.0f; break;
		default: transparency = 192.0f/255.0f; break;
	}

	// Screen height fits three buttons at max size
	int buttonSizePixels = SDL_ANDROID_sRealWindowHeight * (8 - size) / 8 / 3;
	if (buttonAmount <= 4) // Screen height fits two buttons at max size
		buttonSizePixels = SDL_ANDROID_sRealWindowHeight * (8 - size) / 8 * 3 / 8; // BIGGER BUTTONS ARE BETTER BUTTONS

	// Arrows to the lower-left part of screen
	arrows[0].w = buttonSizePixels * 2; // JOYSTICK SIZE XXL
	arrows[0].h = arrows[0].w;
	// Move to the screen edge
	arrows[0].x = 0;
	arrows[0].y = SDL_ANDROID_sRealWindowHeight - arrows[0].h;
	if (size <= 2)
		arrows[0].y = SDL_ANDROID_sRealWindowHeight - arrows[0].h * 3 / 4;

	arrowsExtended[0].w = arrows[0].w * 2;
	arrowsExtended[0].h = arrows[0].h * 2;
	arrowsExtended[0].x = arrows[0].x + arrows[0].w / 2 - arrowsExtended[0].w / 2;
	arrowsExtended[0].y = arrows[0].y + arrows[0].h / 2 - arrowsExtended[0].h / 2;

	arrows[1].w = arrows[0].w;
	arrows[1].h = arrows[0].h;
	arrows[1].x = SDL_ANDROID_sRealWindowWidth - arrows[1].w;
	arrows[1].y = arrows[0].y;

	arrowsExtended[1].w = arrows[1].w * 2;
	arrowsExtended[1].h = arrows[1].h * 2;
	arrowsExtended[1].x = arrows[1].x + arrows[1].w / 2 - arrowsExtended[1].w / 2;
	arrowsExtended[1].y = arrows[1].y + arrows[1].h / 2 - arrowsExtended[1].h / 2;

	arrows[2].w = arrows[1].w;
	arrows[2].h = arrows[1].h;
	arrows[2].x = arrows[1].x;
	arrows[2].y = arrows[1].y - arrows[1].h;

	arrowsExtended[2].w = arrows[2].w * 2;
	arrowsExtended[2].h = arrows[2].h * 2;
	arrowsExtended[2].x = arrows[2].x + arrows[2].w / 2 - arrowsExtended[2].w / 2;
	arrowsExtended[2].y = arrows[2].y + arrows[2].h / 2 - arrowsExtended[2].h / 2;

	// Buttons to the lower-right in 2 rows
	for(i = 0; i < 3; i++)
	for(ii = 0; ii < 2; ii++)
	{
		// Custom button ordering
		int iii = ii + i * 2;
		buttons[iii].w = buttonSizePixels;
		buttons[iii].h = buttons[iii].w;
		// Move to the screen edge
		buttons[iii].x = SDL_ANDROID_sRealWindowWidth - buttons[iii].w * (ii + 1);
		buttons[iii].y = SDL_ANDROID_sRealWindowHeight - buttons[iii].h * (i + 1);
		// Second set of buttons to the left
		buttons[iii + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6].x = buttons[iii].x - buttonSizePixels * 2;
		buttons[iii + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6].y = buttons[iii].y;
		buttons[iii + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6].w = buttons[iii].w;
		buttons[iii + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6].h = buttons[iii].h;
	}
	if( SDL_ANDROID_joysticksAmount >= 2 )
	{
		// Move all buttons to center, 5-th and 6-th button will be misplaced, but we don't care much about that.
		ii = SDL_ANDROID_sRealWindowWidth / 2 - buttons[0].w;
		for(i = 0; i < 6; i++)
			buttons[i].x -= ii;
	}
	buttons[6].x = 0;
	buttons[6].y = 0;
	buttons[6].w = SDL_ANDROID_sRealWindowHeight/10;
	buttons[6].h = SDL_ANDROID_sRealWindowHeight/10;

	for( i = 0; i < sizeof(pointerInButtonRect)/sizeof(pointerInButtonRect[0]); i++ )
		pointerInButtonRect[i] = -1;
	for( i = 0; i < MAX_JOYSTICKS; i++ )
		shrinkButtonRect(arrows[i], &arrowsDraw[i]);
	for( i = 0; i < MAX_BUTTONS; i++ )
		shrinkButtonRect(buttons[i], &buttonsDraw[i]);
	for( i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++ )
		SDL_ANDROID_GetScreenKeyboardButtonPos(i, &hiddenButtons[i]);

	floatingScreenJoystick = _floatingScreenJoystick;
	if( floatingScreenJoystick )
	{
		arrowsExtended[0] = arrows[0] = arrowsDraw[0];
		SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, 0);
	}
};

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetTouchscreenKeyboardUsed) ( JNIEnv*  env, jobject thiz)
{
	SDL_ANDROID_isTouchscreenKeyboardUsed = 1;
}

void SDL_ANDROID_DrawMouseCursor(int x, int y, int size, float alpha)
{
	SDL_Rect r;
	// I've failed with size calculations, so leaving it as-is
	r.x = x - MOUSE_POINTER_X;
	r.y = y - MOUSE_POINTER_Y;
	r.w = MOUSE_POINTER_W;
	r.h = MOUSE_POINTER_H;
	beginDrawingTex();
	drawCharTex( &mousePointer, NULL, &r, 1.0f, 1.0f, 1.0f, alpha );
	endDrawingTex();
}

static int
power_of_2(int input)
{
    int value = 1;

    while (value < input) {
        value <<= 1;
    }
    return value;
}

static int setupScreenKeyboardButtonTexture( GLTexture_t * data, Uint8 * charBuf )
{
	int w, h, format, bpp;
	int texture_w, texture_h;

	memcpy(&w, charBuf, sizeof(int));
	memcpy(&h, charBuf + sizeof(int), sizeof(int));
	memcpy(&format, charBuf + 2*sizeof(int), sizeof(int));
	w = ntohl(w);
	h = ntohl(h);
	format = ntohl(format);
	bpp = 2;
	if(format == 2)
		bpp = 4;

	texture_w = power_of_2(w);
	texture_h = power_of_2(h);
	data->w = w;
	data->h = h;

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &data->id);
	glBindTexture(GL_TEXTURE_2D, data->id);
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "On-screen keyboard generated OpenGL texture ID %d", data->id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA,
					bpp == 4 ? GL_UNSIGNED_BYTE : (format ? GL_UNSIGNED_SHORT_4_4_4_4 : GL_UNSIGNED_SHORT_5_5_5_1), NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
						bpp == 4 ? GL_UNSIGNED_BYTE : (format ? GL_UNSIGNED_SHORT_4_4_4_4 : GL_UNSIGNED_SHORT_5_5_5_1),
						charBuf + 3*sizeof(int) );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if( SDL_ANDROID_VideoLinearFilter )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glDisable(GL_TEXTURE_2D);

	return 3*sizeof(int) + w * h * bpp;
}

static int setupScreenKeyboardButtonLegacy( int buttonID, Uint8 * charBuf )
{
	GLTexture_t * data = NULL;

	if( buttonID < 5 )
		data = &(arrowImages[buttonID]);
	else if( buttonID < 9 )
		data = &(buttonAutoFireImages[buttonID-5]);
	else if( buttonID < 23 )
		data = &(buttonImages[buttonID-9]);
	else if( buttonID == 23 )
		data = &mousePointer;
	else if( buttonID < 28 )
		data = &(arrowImages[buttonID - 24 + 5]); // Diagonal arrows
	else // Error, array too big
		return 12; // Return value bigger than zero to iterate it

	for( int i = SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 * 2; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 * 2; i++ )
		buttonImages[i + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6 * 2] = buttonImages[i];

	return setupScreenKeyboardButtonTexture(data, charBuf);
}

static int setupScreenKeyboardButtonSun( int buttonID, Uint8 * charBuf )
{
	GLTexture_t * data = NULL;
	int i, ret;

	if( buttonID == 0 )
		data = &(arrowImages[0]);
	if( buttonID >= 1 && buttonID <= 4 )
		data = &(buttonImages[buttonID-1]);
	if( buttonID >= 5 && buttonID <= 8 )
		data = &(buttonImages[4+(buttonID-5)*2]);
	if( buttonID == 9 )
		data = &mousePointer;
	if( buttonID == 10 )
		data = &(arrowImages[10]);
	if( buttonID == 11 )
		data = &(arrowImages[11]);
	else if( buttonID > 11 ) // Error, array too big
		return 12; // Return value bigger than zero to iterate it

	ret = setupScreenKeyboardButtonTexture(data, charBuf);

	for( i = 1; i <= 4; i++ )
		arrowImages[i] = arrowImages[0];

	for( i = 2; i < MAX_BUTTONS; i++ )
		buttonImages[i * 2 + 1] = buttonImages[i * 2];

	for( i = 0; i < MAX_BUTTONS_AUTOFIRE*2; i++ )
		buttonAutoFireImages[i] = arrowImages[0];

	buttonImages[BUTTON_TEXT_INPUT*2] = buttonImages[10];
	buttonImages[BUTTON_TEXT_INPUT*2+1] = buttonImages[10];

	for( int i = SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 * 2; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 * 2; i++ )
		buttonImages[i + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6 * 2] = buttonImages[i];

	return ret;
}

static int setupScreenKeyboardButtonDualShock( int buttonID, Uint8 * charBuf )
{
	GLTexture_t * data = NULL;
	int i, ret;

	if( buttonID == 0 )
		data = &(arrowImages[0]);
	if( buttonID >= 1 && buttonID <= 4 )
		data = &(buttonImages[buttonID-1]);
	if( buttonID >= 5 && buttonID <= 8 )
		data = &(buttonImages[4+(buttonID-5)*2]);
	if( buttonID == 9 )
		data = &mousePointer;
	if( buttonID == 10 )
		data = &(arrowImages[5]);
	if( buttonID == 11 )
		data = &(arrowImages[6]);
	else if( buttonID > 11 ) // Error, array too big
		return 12; // Return value bigger than zero to iterate it

	ret = setupScreenKeyboardButtonTexture(data, charBuf);

	for( i = 1; i <=4; i++ )
		arrowImages[i] = arrowImages[0];
	
	for( i = 2; i < MAX_BUTTONS; i++ )
		buttonImages[i * 2 + 1] = buttonImages[i * 2];

	for( i = 0; i < MAX_BUTTONS_AUTOFIRE*2; i++ )
		buttonAutoFireImages[i] = arrowImages[0];

	buttonImages[BUTTON_TEXT_INPUT*2] = arrowImages[5];
	buttonImages[BUTTON_TEXT_INPUT*2+1] = arrowImages[5];

	for( int i = SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 * 2; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 * 2; i++ )
		buttonImages[i + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6 * 2] = buttonImages[i];

	return ret;
}

static int setupScreenKeyboardButton( int buttonID, Uint8 * charBuf, int count )
{
	if( count == 24 || count == 28)
	{
		themeType = 0;
		return setupScreenKeyboardButtonLegacy(buttonID, charBuf);
	}
	else if( count == 10)
	{
		themeType = 1;
		return setupScreenKeyboardButtonSun(buttonID, charBuf);
	}
	else if( count == 12)
	{
		themeType = 2;
		return setupScreenKeyboardButtonDualShock(buttonID, charBuf);
	}
	else
	{
		__android_log_print(ANDROID_LOG_FATAL, "libSDL", "On-screen keyboard buton img count = %d, should be 10 or 24 or 28", count);
		return 12; // Return value bigger than zero to iterate it
	}
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboardButtons) ( JNIEnv*  env, jobject thiz, jbyteArray charBufJava )
{
	jboolean isCopy = JNI_TRUE;
	int len = (*env)->GetArrayLength(env, charBufJava);
	Uint8 * charBuf = (Uint8 *) (*env)->GetByteArrayElements(env, charBufJava, &isCopy);
	int but, pos, count;
	memcpy(&count, charBuf, sizeof(int));
	count = ntohl(count);
	
	for( but = 0, pos = sizeof(int); pos < len; but ++ )
		pos += setupScreenKeyboardButton( but, charBuf + pos, count );
	
	(*env)->ReleaseByteArrayElements(env, charBufJava, (jbyte *)charBuf, 0);
}

JNIEXPORT jint JNICALL
JAVA_EXPORT_NAME(Settings_nativeGetKeymapKeyScreenKb) (JNIEnv* env, jobject thiz, jint keynum)
{
	if( keynum >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 && keynum <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 )
	{
		return SDL_ANDROID_GetScreenKeyboardButtonKey(keynum);
	}

	if( keynum >= 6 && keynum <= 11 )
	{
		return SDL_ANDROID_GetScreenKeyboardButtonKey(keynum - 6 + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6);
	}

	return SDL_KEY(UNKNOWN);
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetKeymapKeyScreenKb) (JNIEnv* env, jobject thiz, jint keynum, jint key)
{
	if( keynum >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 && keynum <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 )
	{
		SDL_ANDROID_SetScreenKeyboardButtonKey(keynum, key);
	}

	if( keynum >= 6 && keynum <= 11 )
	{
		SDL_ANDROID_SetScreenKeyboardButtonKey(keynum - 6 + SDL_ANDROID_SCREENKEYBOARD_BUTTON_6, key);
	}
}

static int convertJavaKeyIdToC(int keynum)
{
	// Why didn't I use consistent IDs between Java and C code?
	int key = -1;
	if( keynum == 0 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD;
	if( keynum == 1 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT;
	if( keynum - 2 >= 0 && keynum - 2 <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 )
		key = keynum - 2 + SDL_ANDROID_SCREENKEYBOARD_BUTTON_0;
	if( keynum >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2 && keynum < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		key = keynum; // This one is consistent by chance
	return key;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetScreenKbKeyUsed) (JNIEnv*  env, jobject thiz, jint keynum, jint used)
{
	SDL_Rect rect = {0, 0, 0, 0};
	int key = convertJavaKeyIdToC(keynum);

	if( key >= 0 && !used )
		SDL_ANDROID_SetScreenKeyboardButtonPos(key, &rect);
}


int SDL_ANDROID_SetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM || ! pos )
		return 0;
	
	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
	{
		int i = buttonId - SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD;
		arrows[i] = *pos;
		arrowsExtended[i].w = arrows[i].w * 2;
		arrowsExtended[i].h = arrows[i].h * 2;
		arrowsExtended[i].x = arrows[i].x + arrows[i].w / 2 - arrowsExtended[i].w / 2;
		arrowsExtended[i].y = arrows[i].y + arrows[i].h / 2 - arrowsExtended[i].h / 2;
		shrinkButtonRect(arrows[i], &arrowsDraw[i]);
	}
	else
	{
		int i = buttonId;
		buttons[i] = *pos;
		shrinkButtonRect(buttons[i], &buttonsDraw[i]);
	}
	return 1;
};

int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonImagePos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM || ! pos )
		return 0;

	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
		arrowsDraw[buttonId - SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD] = *pos;
	else
		buttonsDraw[buttonId] = *pos;

	return 1;
}

int SDL_ANDROID_GetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM || ! pos )
		return 0;
	
	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
	{
		*pos = arrows[buttonId - SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD];
	}
	else
	{
		*pos = buttons[buttonId];
	}
	return 1;
};

int SDL_ANDROID_SetScreenKeyboardButtonKey(int buttonId, SDLKey key)
{
	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 )
	{
		buttonKeysyms[buttonId] = key;
		return 1;
	}
	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_6 && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_11 )
	{
		buttonKeysyms[buttonId] = key;
		return 1;
	}
	return 0;
};

SDLKey SDL_ANDROID_GetScreenKeyboardButtonKey(int buttonId)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return SDLK_UNKNOWN;
	return buttonKeysyms[buttonId];
};

int SDL_ANDROID_SetScreenKeyboardShown(int shown)
{
	touchscreenKeyboardShown = shown;
	return 0;
};

int SDL_ANDROID_GetScreenKeyboardShown(void)
{
	return touchscreenKeyboardShown;
};

int SDL_ANDROID_SetScreenKeyboardButtonShown(int buttonId, int shown)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return 0;

	//__android_log_print(ANDROID_LOG_INFO, "libsdl", "SDL_ANDROID_SetScreenKeyboardButtonShown: button %d shown %d", buttonId, shown);
	if( !shown && SDL_ANDROID_GetScreenKeyboardButtonShown(buttonId) )
	{
		SDL_Rect pos = { 0, 0, 0, 0 };
		SDL_ANDROID_GetScreenKeyboardButtonPos(buttonId, &hiddenButtons[buttonId]);
		SDL_ANDROID_SetScreenKeyboardButtonPos(buttonId, &pos);
	}
	if( shown && !SDL_ANDROID_GetScreenKeyboardButtonShown(buttonId) )
		SDL_ANDROID_SetScreenKeyboardButtonPos(buttonId, &hiddenButtons[buttonId]);
	return 1;
};

int SDL_ANDROID_GetScreenKeyboardButtonShown(int buttonId)
{
	SDL_Rect pos;
	if( !SDL_ANDROID_GetScreenKeyboardButtonPos(buttonId, &pos) )
		return 0;
	return pos.h > 0 && pos.w > 0;
};

int SDL_ANDROID_GetScreenKeyboardSize(void)
{
	return buttonsize;
};

int SDL_ANDROID_ToggleScreenKeyboardTextInput(const char * previousText)
{
	static char textIn[255];
	if( previousText == NULL )
		previousText = "";
	strncpy(textIn, previousText, sizeof(textIn));
	textIn[sizeof(textIn)-1] = 0;
	SDL_ANDROID_CallJavaShowScreenKeyboard(textIn, NULL, 0, 0);
	return 1;
};

int SDLCALL SDL_ANDROID_GetScreenKeyboardTextInput(char * textBuf, int textBufSize)
{
	SDL_ANDROID_CallJavaShowScreenKeyboard(textBuf, textBuf, textBufSize, 0);
	return 1;
};

SDL_AndroidTextInputAsyncStatus_t SDLCALL SDL_ANDROID_GetScreenKeyboardTextInputAsync(char * textBuf, int textBufSize)
{
	if( SDL_ANDROID_TextInputFinished )
	{
		SDL_ANDROID_TextInputFinished = 0;
		SDL_ANDROID_AsyncTextInputActive = 0;
		return SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED;
	}
	if( !SDL_ANDROID_IsScreenKeyboardShownFlag )
	{
		SDL_ANDROID_AsyncTextInputActive = 1;
		SDL_ANDROID_CallJavaShowScreenKeyboard(textBuf, textBuf, textBufSize, 1);
	}
	return SDL_ANDROID_TEXTINPUT_ASYNC_IN_PROGRESS;
}

int SDLCALL SDL_HasScreenKeyboardSupport(void *unused)
{
	return 1;
}

// SDL2 compatibility
int SDLCALL SDL_ShowScreenKeyboard(void *unused)
{
	return SDL_ANDROID_ToggleScreenKeyboardTextInput(NULL);
}

int SDLCALL SDL_HideScreenKeyboard(void *unused)
{
	SDL_ANDROID_CallJavaHideScreenKeyboard();
	return 1;
}

int SDLCALL SDL_IsScreenKeyboardShown(void *unused)
{
	return SDL_ANDROID_IsScreenKeyboardShown();
}

int SDLCALL SDL_ToggleScreenKeyboard(void *unused)
{
	if( SDL_IsScreenKeyboardShown(NULL) )
		return SDL_HideScreenKeyboard(NULL);
	else
		return SDL_ShowScreenKeyboard(NULL);
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonGenerateTouchEvents(int buttonId, int generateEvents)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return 0;
	buttonsGenerateSdlEvents[buttonId] = generateEvents;
	return 1;
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardPreventButtonOverlap(int prevent)
{
	preventButtonOverlap = prevent;
	return 1;
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonStayPressedAfterTouch(int buttonId, int stayPressed)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return 0;
	buttonsStayPressedAfterTouch[buttonId] = stayPressed;
	return 1;
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardTransparency(int alpha)
{
	transparency = (float)alpha / 255.0f;
	return 0;
}

static int ScreenKbRedefinedByUser = 0;

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetScreenKbKeyLayout) (JNIEnv* env, jobject thiz, jint keynum, jint x1, jint y1, jint x2, jint y2)
{
	SDL_Rect rect = {x1, y1, x2-x1, y2-y1};
	int key = convertJavaKeyIdToC(keynum);

	if( key >= 0 )
	{
		ScreenKbRedefinedByUser = 1;
		SDL_ANDROID_SetScreenKeyboardButtonPos(key, &rect);
	}
}

JNIEXPORT jint JNICALL
JAVA_EXPORT_NAME(Settings_nativeGetScreenKeyboardButtonLayout) ( JNIEnv* env, jobject thiz, jint keynum, jint coord )
{
	SDL_Rect rect = {0, 0, 0, 0};
	int key = convertJavaKeyIdToC(keynum);

	if( key < 0 )
		return 0;

	SDL_ANDROID_GetScreenKeyboardButtonPos(key, &rect);
	if( coord == 0 )
		return rect.x;
	if( coord == 1 )
		return rect.y;
	if( coord == 2 )
		return rect.x + rect.w;
	if( coord == 3 )
		return rect.y + rect.h;

	return 0;
}

int SDL_ANDROID_GetScreenKeyboardRedefinedByUser()
{
	return ScreenKbRedefinedByUser;
}

int SDL_ANDROID_SetScreenKeyboardHintMesage(const char * hint)
{
	SDL_ANDROID_CallJavaSetScreenKeyboardHintMessage(hint);
	return 1;
}

extern DECLSPEC int SDLCALL SDL_ANDROID_SetScreenKeyboardFloatingJoystick(int enabled)
{
	floatingScreenJoystick = enabled;
	SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, 0);
	return 1;
}

extern DECLSPEC int SDL_ANDROID_ScreenKeyboardUpdateToNewVideoMode(int oldx, int oldy, int newx, int newy)
{
	int i;
	for( i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++ )
	{
		SDL_Rect pos, pos2;
		SDL_ANDROID_GetScreenKeyboardButtonPos(i, &pos);
		pos2.x = pos.x * newx / oldx;
		pos2.y = pos.y * newy / oldy;
		pos2.w = (pos.x + pos.w) * newx / oldx - pos2.x;
		pos2.h = (pos.y + pos.h) * newy / oldy - pos2.y;
		SDL_ANDROID_SetScreenKeyboardButtonPos(i, &pos2);
	}
	return 0;
}
