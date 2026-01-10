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

#include "SDL_config.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "SDL_timer.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_pixels.h"
#include "SDL_video-1.3.h"
#include "SDL_surface.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"

#include <jni.h>
#include <android/log.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memset()
#include <dlfcn.h>

#define _THIS	SDL_VideoDevice *this

#ifdef VIDEO_DEBUG
#define DEBUGOUT(...) __android_log_print(ANDROID_LOG_INFO, "libSDL", __VA_ARGS__)
#else
#define DEBUGOUT(...)
#endif

#include "SDL_opengles.h"

#ifdef USE_GL4ES
#include <GL/gl.h>
#endif

static int ANDROID_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **ANDROID_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *ANDROID_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int ANDROID_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void ANDROID_VideoQuit(_THIS);

static int ANDROID_AllocHWSurface(_THIS, SDL_Surface *surface);
static int ANDROID_LockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_FreeHWSurface(_THIS, SDL_Surface *surface);
static int ANDROID_FlipHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_GL_SwapBuffers(_THIS);
static void ANDROID_PumpEvents(_THIS);
static int ANDROID_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst);
static int ANDROID_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color);
static int ANDROID_SetHWColorKey(_THIS, SDL_Surface *surface, Uint32 key);
static int ANDROID_SetHWAlpha(_THIS, SDL_Surface *surface, Uint8 value);
static void* ANDROID_GL_GetProcAddress(_THIS, const char *proc);
static void ANDROID_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

// Multithreaded video support for speed optimization
static int ANDROID_VideoInitMT(_THIS, SDL_PixelFormat *vformat);
static SDL_Surface *ANDROID_SetVideoModeMT(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static void ANDROID_VideoQuitMT(_THIS);
static void ANDROID_UpdateRectsMT(_THIS, int numrects, SDL_Rect *rects);
static int ANDROID_FlipHWSurfaceMT(_THIS, SDL_Surface *surface);
static int ANDROID_ToggleFullScreen(_THIS, int fullscreen);
static Uint32 PixelFormatEnum = SDL_PIXELFORMAT_RGB565;
static Uint32 PixelFormatEnumAlpha = SDL_PIXELFORMAT_RGBA4444;
static Uint32 PixelFormatEnumColorkey = SDL_PIXELFORMAT_RGBA5551;


// Stubs to get rid of crashing in OpenGL mode
// The implementation dependent data for the window manager cursor
struct WMcursor {
    int unused ;
};

void ANDROID_FreeWMCursor(_THIS, WMcursor *cursor) {
    SDL_free (cursor);
    return;
}
WMcursor * ANDROID_CreateWMCursor(_THIS, Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y) {
    WMcursor * cursor;
    cursor = (WMcursor *) SDL_malloc (sizeof (WMcursor)) ;
    if (cursor == NULL) {
        SDL_OutOfMemory () ;
        return NULL ;
    }
    return cursor;
}
int ANDROID_ShowWMCursor(_THIS, WMcursor *cursor) {
    return 1;
}
void ANDROID_WarpWMCursor(_THIS, Uint16 x, Uint16 y)
{
	SDL_ANDROID_WarpMouse(x, y);
}
//void ANDROID_MoveWMCursor(_THIS, int x, int y) { }

int ANDROID_ToggleFullScreen(_THIS, int fullscreen)
{
	return 1;
}

enum { SDL_NUMMODES = 73 };
static SDL_Rect *SDL_modelist[SDL_NUMMODES+1];

//#define SDL_modelist		(this->hidden->SDL_modelist)

typedef struct SDL_Texture private_hwdata;

// Pointer to in-memory video surface
int SDL_ANDROID_sFakeWindowWidth = 640;
int SDL_ANDROID_sFakeWindowHeight = 480;
int sdl_opengl = 0;
static SDL_Window *SDL_VideoWindow = NULL;
SDL_Surface *SDL_CurrentVideoSurface = NULL;
static int HwSurfaceCount = 0;
static SDL_Surface ** HwSurfaceList = NULL;
void * glLibraryHandle = NULL;

static Uint32 SDL_VideoThreadID = 0;
int SDL_ANDROID_InsideVideoThread()
{
	return SDL_VideoThreadID == SDL_ThreadID();
}


/* ANDROID driver bootstrap functions */

static int ANDROID_Available(void)
{
	return 1;
}

static void ANDROID_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_VideoDevice *ANDROID_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, sizeof(SDL_VideoDevice));
	} else {
		SDL_OutOfMemory();
		return(0);
	}

	/* Set the function pointers */
	device->VideoInit = SDL_ANDROID_VideoMultithreaded ? ANDROID_VideoInitMT : ANDROID_VideoInit;
	device->ListModes = ANDROID_ListModes;
	device->SetVideoMode = SDL_ANDROID_VideoMultithreaded ? ANDROID_SetVideoModeMT : ANDROID_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = ANDROID_SetColors;
	device->UpdateRects = SDL_ANDROID_VideoMultithreaded ? ANDROID_UpdateRectsMT : ANDROID_UpdateRects;
	device->VideoQuit = SDL_ANDROID_VideoMultithreaded ? ANDROID_VideoQuitMT : ANDROID_VideoQuit;
	device->AllocHWSurface = ANDROID_AllocHWSurface;
	device->CheckHWBlit = ANDROID_CheckHWBlit;
	device->FillHWRect = ANDROID_FillHWRect;
	device->SetHWColorKey = ANDROID_SetHWColorKey;
	device->SetHWAlpha = ANDROID_SetHWAlpha;
	device->LockHWSurface = ANDROID_LockHWSurface;
	device->UnlockHWSurface = ANDROID_UnlockHWSurface;
	device->FlipHWSurface = SDL_ANDROID_VideoMultithreaded ? ANDROID_FlipHWSurfaceMT : ANDROID_FlipHWSurface;
	device->FreeHWSurface = ANDROID_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = ANDROID_InitOSKeymap;
	device->PumpEvents = ANDROID_PumpEvents;
	device->GL_SwapBuffers = ANDROID_GL_SwapBuffers;
	device->GL_GetProcAddress = ANDROID_GL_GetProcAddress;
	device->free = ANDROID_DeleteDevice;
	device->WarpWMCursor = ANDROID_WarpWMCursor;

	// Stubs
	device->FreeWMCursor = ANDROID_FreeWMCursor;
	device->CreateWMCursor = ANDROID_CreateWMCursor;
	device->ShowWMCursor = ANDROID_ShowWMCursor;
	device->ToggleFullScreen = ANDROID_ToggleFullScreen;

	device->handles_any_size = 1; // Any video mode is OK

	if ( SDL_ANDROID_UseGles3 )
	{
		glLibraryHandle = dlopen("libGLESv3.so", RTLD_LAZY | RTLD_GLOBAL);
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Loading libGLESv3.so: %p", glLibraryHandle);
	}
	else if ( SDL_ANDROID_UseGles2 )
	{
		glLibraryHandle = dlopen("libGLESv2.so", RTLD_LAZY | RTLD_GLOBAL);
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Loading libGLESv2.so: %p", glLibraryHandle);
	}
	else
	{
		glLibraryHandle = dlopen("libGLESv1_CM.so", RTLD_LAZY | RTLD_GLOBAL);
	}

	return device;
}

VideoBootStrap ANDROID_bootstrap = {
	"android", "SDL android video driver",
	ANDROID_Available, ANDROID_CreateDevice
};

int ANDROID_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	int i;
	static SDL_PixelFormat alphaFormat;
	int bpp;
	static int alreadyInitialized = 0;
	if(alreadyInitialized)
		__android_log_print(ANDROID_LOG_WARN, "libSDL", "Application calls SDL_Init() multiple times, this is not supported yet!");
	alreadyInitialized = 1;

	/* Determine the screen depth (use default 16-bit depth) */
	/* we change this during the SDL_SetVideoMode implementation... */
	if( vformat ) {
		vformat->BitsPerPixel = SDL_ANDROID_BITSPERPIXEL;
		vformat->BytesPerPixel = SDL_ANDROID_BYTESPERPIXEL;
	}
	
	if( SDL_ANDROID_BITSPERPIXEL == 24 )
		PixelFormatEnum = SDL_PIXELFORMAT_RGB24;
	if( SDL_ANDROID_BITSPERPIXEL == 32 )
		PixelFormatEnum = SDL_PIXELFORMAT_ABGR8888;

	if( SDL_ANDROID_BITSPERPIXEL >= 24 )
	{
		PixelFormatEnumAlpha = SDL_PIXELFORMAT_ABGR8888;
		PixelFormatEnumColorkey = SDL_PIXELFORMAT_ABGR8888;
	}

	SDL_memset(&alphaFormat, 0, sizeof(alphaFormat));
	SDL_PixelFormatEnumToMasks( PixelFormatEnumAlpha, &bpp,
								&alphaFormat.Rmask, &alphaFormat.Gmask, 
								&alphaFormat.Bmask, &alphaFormat.Amask );
	alphaFormat.BitsPerPixel = SDL_ANDROID_BITSPERPIXEL;
	alphaFormat.BytesPerPixel = SDL_ANDROID_BYTESPERPIXEL;
	this->displayformatalphapixel = &alphaFormat;

	this->info.hw_available = 1;
	this->info.blit_hw = 1;
	this->info.blit_hw_CC = 1;
	this->info.blit_hw_A = 1;
	this->info.blit_fill = 1;
	this->info.video_mem = 128 * 1024; // Random value
	this->info.current_w = SDL_ANDROID_sWindowWidth;
	this->info.current_h = SDL_ANDROID_sWindowHeight;
	SDL_VideoThreadID = SDL_ThreadID();

	for ( i=0; i<SDL_NUMMODES; ++i ) {
		SDL_modelist[i] = SDL_malloc(sizeof(SDL_Rect));
		SDL_modelist[i]->x = SDL_modelist[i]->y = 0;
	}
	i = 0;
	/* Modes sorted largest to smallest */
	SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight;
	// These modes come first, because they preserve aspect ratio and keep pixels square
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 8 / 9; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 8 / 9; // 8 / 9 : 1920x1080 -> 1706x960
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 7 / 8; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 7 / 8; // 7 / 8 : 1920x1080 -> 1680x945
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 5 / 6; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 5 / 6; // 5 / 6 : 1920x1080 -> 1600x900
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 7 / 9; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 7 / 9; // 7 / 9 : 1920x1080 -> 1493x840
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 6 / 8; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 6 / 8; // 6 / 8 : 1920x1080 -> 1440x810
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 4 / 6; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 4 / 6; // 4 / 6 : 1920x1080 -> 1280x720
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 5 / 8; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 5 / 8; // 5 / 8 : 1920x1080 -> 1200x675
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 5 / 9; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 5 / 9; // 5 / 9 : 1920x1080 -> 1066x600
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 3 / 6; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 3 / 6; // 3 / 6 : 1920x1080 -> 960x540
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 4 / 9; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 4 / 9; // 4 / 9 : 1920x1080 -> 853x480
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 3 / 8; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 3 / 8; // 3 / 8 : 1920x1080 -> 720x405
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 2 / 6; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 2 / 6; // 2 / 6 : 1920x1080 -> 640x360
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 2 / 8; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 2 / 8; // 2 / 8 : 1920x1080 -> 480x270
	i++; SDL_modelist[i]->w = SDL_ANDROID_sWindowWidth * 2 / 9; SDL_modelist[i]->h = SDL_ANDROID_sWindowHeight * 2 / 9; // 2 / 9 : 1920x1080 -> 426x240
	// Other modes for compatibility with various emulators
	i++; SDL_modelist[i]->w = 800;  SDL_modelist[i]->h = 600; // Widely used on PC
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 480; // Widely used on PC
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 400; // Widely used on PC
	i++; SDL_modelist[i]->w = 320;  SDL_modelist[i]->h = 240; // For older games
	i++; SDL_modelist[i]->w = 320;  SDL_modelist[i]->h = 200; // For even older games
	i++; SDL_modelist[i]->w = 480;  SDL_modelist[i]->h = 320; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 800;  SDL_modelist[i]->h = 480; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 350; // For PrefClub app
	i++; SDL_modelist[i]->w = 320;  SDL_modelist[i]->h = 256; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 200; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 240; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 256; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 320;  SDL_modelist[i]->h = 262; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 262; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 320;  SDL_modelist[i]->h = 270; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 270; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 320;  SDL_modelist[i]->h = 216; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 216; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 272; // For VICE
	i++; SDL_modelist[i]->w = 854;  SDL_modelist[i]->h = 480; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 1280; SDL_modelist[i]->h = 720; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 1920; SDL_modelist[i]->h = 1080; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 1024; SDL_modelist[i]->h = 768; // Widely used on PC
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 512; // For P-UAE
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 524; // For P-UAE
	i++; SDL_modelist[i]->w = 640;  SDL_modelist[i]->h = 540; // For P-UAE
	i++; SDL_modelist[i]->w = 352;  SDL_modelist[i]->h = 200; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 352;  SDL_modelist[i]->h = 216; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 352;  SDL_modelist[i]->h = 240; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 352;  SDL_modelist[i]->h = 256; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 352;  SDL_modelist[i]->h = 262; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 352;  SDL_modelist[i]->h = 270; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 200; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 216; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 240; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 256; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 262; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 384;  SDL_modelist[i]->h = 270; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 704;  SDL_modelist[i]->h = 200; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 704;  SDL_modelist[i]->h = 216; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 704;  SDL_modelist[i]->h = 240; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 704;  SDL_modelist[i]->h = 256; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 704;  SDL_modelist[i]->h = 262; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 704;  SDL_modelist[i]->h = 270; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 768;  SDL_modelist[i]->h = 200; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 768;  SDL_modelist[i]->h = 216; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 768;  SDL_modelist[i]->h = 240; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 768;  SDL_modelist[i]->h = 256; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 768;  SDL_modelist[i]->h = 262; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 768;  SDL_modelist[i]->h = 270; // For UAE4ALL2
	i++; SDL_modelist[i]->w = 1280; SDL_modelist[i]->h = 960; // For UQM-HD
	i++; SDL_modelist[i]->w = 960;  SDL_modelist[i]->h = 600; // For ScummVM 3x-mode
	i++; SDL_modelist[i]->w = 960;  SDL_modelist[i]->h = 540; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 720;  SDL_modelist[i]->h = 400; // Virtual wide-screen mode
	i++; SDL_modelist[i]->w = 480;  SDL_modelist[i]->h = 272; // PSP
	i++; SDL_modelist[i]->w = 996;  SDL_modelist[i]->h = 560; // For OpenTTD
	i++; SDL_modelist[i]->w = 1280; SDL_modelist[i]->h = 800; // for Basilisk2
	i++; SDL_modelist[i]->w = 1366; SDL_modelist[i]->h = 768; // for Basilisk2
	i++; SDL_modelist[i] = NULL;
	// If you going to add another video mode, increase SDL_NUMMODES constant
	if(i != SDL_NUMMODES)
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "Please increase SDL_NUMMODES = %d at %s to match the number of actual video modes = %d", SDL_NUMMODES, __FILE__, i);
		abort();
	}

	SDL_VideoInit_1_3(NULL, 0);
	
	/* We're done! */
	return(0);
}

SDL_Rect **ANDROID_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
	if(format->BitsPerPixel != SDL_ANDROID_BITSPERPIXEL)
		return NULL;
	return SDL_modelist;
}

SDL_Surface *ANDROID_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	SDL_PixelFormat format;
	int bpp1;
	
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_SetVideoMode(): application requested mode %dx%d OpenGL %d HW %d BPP %d", width, height, flags & SDL_OPENGL, flags & SDL_HWSURFACE, SDL_ANDROID_BITSPERPIXEL);
	if( ! SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return NULL;
	}

	if( SDL_ANDROID_VideoForceSoftwareMode )
	{
		if( flags & SDL_HWSURFACE )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_SetVideoMode(): application requested hardware video mode - forcing software video mode");
		if( flags & SDL_OPENGL )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: application requested OpenGL context - SDL will ignore this. Set SwVideoMode=n inside AndroidAppSettings.cfg to enable OpenGL inside SDL.");
		flags = (flags & SDL_FULLSCREEN) | (flags & SDL_DOUBLEBUF);
	}

	sdl_opengl = (flags & SDL_OPENGL) ? 1 : 0;

	SDL_ANDROID_sFakeWindowWidth = width;
	SDL_ANDROID_sFakeWindowHeight = height;

	current->flags = (flags & SDL_FULLSCREEN) | (flags & SDL_OPENGL) | SDL_DOUBLEBUF | (flags & SDL_HWSURFACE);
	current->w = width;
	current->h = height;
	current->pitch = SDL_ANDROID_sFakeWindowWidth * SDL_ANDROID_BYTESPERPIXEL;
	current->pixels = NULL;
	current->hwdata = NULL;

	HwSurfaceCount = 0;
	HwSurfaceList = NULL;
	DEBUGOUT("ANDROID_SetVideoMode() HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);
	
	if( ! sdl_opengl )
	{
		SDL_DisplayMode mode;
		SDL_RendererInfo SDL_VideoRendererInfo;
		SDL_Rect window;

		SDL_ANDROID_sWindowWidth = SDL_ANDROID_sRealWindowWidth;
		SDL_ANDROID_sWindowHeight = SDL_ANDROID_sRealWindowHeight;

		SDL_ANDROID_ForceClearScreenRectAmount = 0;
		if( SDL_ANDROID_ScreenKeep43Ratio )
		{
			if( (float)width / (float)height < (float)SDL_ANDROID_sWindowWidth / (float)SDL_ANDROID_sWindowHeight )
				SDL_ANDROID_sWindowWidth = (SDL_ANDROID_sFakeWindowWidth * SDL_ANDROID_sRealWindowHeight) / SDL_ANDROID_sFakeWindowHeight;
			else
				// Force 4:3 ratio, with black borders at the left/right,
				// this is needede for Uae4all2, which has 640x256 video mode,
				// and expects those 256 pixels to stretch 2x height like on a TV interlaced display.
				SDL_ANDROID_sWindowWidth = SDL_ANDROID_sWindowHeight * 4 / 3;

			SDL_ANDROID_ForceClearScreenRectAmount = 2;
		}

		window.x = (SDL_ANDROID_sRealWindowWidth - SDL_ANDROID_sWindowWidth) / 2;
		window.y = 0;
		window.w = width;
		window.h = height;

		SDL_ANDROID_ForceClearScreenRect[0].x = 0;
		SDL_ANDROID_ForceClearScreenRect[0].y = 0;
		SDL_ANDROID_ForceClearScreenRect[0].w = window.x;
		SDL_ANDROID_ForceClearScreenRect[0].h = SDL_ANDROID_sRealWindowHeight;
		SDL_ANDROID_ForceClearScreenRect[1].x = SDL_ANDROID_sRealWindowWidth - window.x;
		SDL_ANDROID_ForceClearScreenRect[1].y = 0;
		SDL_ANDROID_ForceClearScreenRect[1].w = window.x;
		SDL_ANDROID_ForceClearScreenRect[1].h = SDL_ANDROID_sRealWindowHeight;
		SDL_ANDROID_ForceClearScreenRect[2].x = window.x;
		SDL_ANDROID_ForceClearScreenRect[2].y = 0;
		SDL_ANDROID_ForceClearScreenRect[2].w = SDL_ANDROID_sRealWindowWidth - window.x * 2;
		SDL_ANDROID_ForceClearScreenRect[2].h = window.y;
		SDL_ANDROID_ForceClearScreenRect[3].x = window.x;
		SDL_ANDROID_ForceClearScreenRect[3].y = SDL_ANDROID_sRealWindowHeight - window.y;
		SDL_ANDROID_ForceClearScreenRect[3].w = SDL_ANDROID_sRealWindowWidth - window.x * 2;
		SDL_ANDROID_ForceClearScreenRect[3].h = window.y;

		SDL_SelectVideoDisplay(0);
		if (SDL_VideoWindow)
			SDL_DestroyWindow(SDL_VideoWindow);
		SDL_VideoWindow = SDL_CreateWindow("", window.x, window.y, window.w, window.h, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);

		SDL_memset(&mode, 0, sizeof(mode));
		mode.format = PixelFormatEnum;
		SDL_SetWindowDisplayMode(SDL_VideoWindow, &mode);
		
		if (SDL_CreateRenderer(SDL_VideoWindow, -1, 0) < 0) {
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_SetVideoMode(): Error creating renderer");
			return NULL;
		}
		SDL_GetRendererInfo(&SDL_VideoRendererInfo);
		
		current->hwdata = NULL;
		if( ! (flags & SDL_HWSURFACE) )
		{
			current->pixels = SDL_malloc(width * height * SDL_ANDROID_BYTESPERPIXEL);
			if ( ! current->pixels ) {
				__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate buffer for requested mode");
				SDL_SetError("Couldn't allocate buffer for requested mode");
				return(NULL);
			}
			SDL_memset(current->pixels, 0, width * height * SDL_ANDROID_BYTESPERPIXEL);
			current->hwdata = (struct private_hwdata *)SDL_CreateTexture(PixelFormatEnum, SDL_TEXTUREACCESS_STATIC, width, height);
			if( !current->hwdata ) {
				__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate texture for SDL_CurrentVideoSurface");
				SDL_free(current->pixels);
				current->pixels = NULL;
				SDL_OutOfMemory();
				return(NULL);
			}
			if( SDL_ANDROID_VideoLinearFilter )
				SDL_SetTextureScaleMode((SDL_Texture *)current->hwdata, SDL_SCALEMODE_SLOW);

			// Register main video texture to be recreated when needed
			HwSurfaceCount++;
			HwSurfaceList = SDL_realloc( HwSurfaceList, HwSurfaceCount * sizeof(SDL_Surface *) );
			HwSurfaceList[HwSurfaceCount-1] = current;
			DEBUGOUT("ANDROID_SetVideoMode() HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);
		}
#if SDL_VIDEO_OPENGL_ES_VERSION == 1
		glViewport(0, 0, SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sRealWindowHeight);
		glOrthof(0, SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sRealWindowHeight, 0, 0, 1);
#endif
	}

	/* Allocate the new pixel format for the screen */
    SDL_memset(&format, 0, sizeof(format));
	SDL_PixelFormatEnumToMasks( PixelFormatEnum, &bpp1,
								&format.Rmask, &format.Gmask,
								&format.Bmask, &format.Amask );
	format.BitsPerPixel = bpp1;
	format.BytesPerPixel = SDL_ANDROID_BYTESPERPIXEL;

	if ( ! SDL_ReallocFormat(current, SDL_ANDROID_BITSPERPIXEL, format.Rmask, format.Gmask, format.Bmask, format.Amask) ) {
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate new pixel format for requested mode");
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	/* Set up the new mode framebuffer */
	SDL_CurrentVideoSurface = current;

	SDL_ANDROID_UpdateScreenUnderFingerRect(0,0);
	SDL_ANDROID_ShowScreenUnderFingerRect.w = SDL_ANDROID_ShowScreenUnderFingerRect.h = 0;
	SDL_ANDROID_SetHoverDeadzone();
	SDL_ANDROID_currentMouseX = SDL_ANDROID_sFakeWindowWidth / 2;
	SDL_ANDROID_currentMouseY = SDL_ANDROID_sFakeWindowHeight / 2;

	/* We're done */
	return(current);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void ANDROID_VideoQuit(_THIS)
{
	// TODO: this function crashes SDL
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Calling VideoQuit()");
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
	}

	if( ! sdl_opengl )
	{
		DEBUGOUT("ANDROID_VideoQuit() in HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);
		HwSurfaceCount = 0;
		if(HwSurfaceList)
			SDL_free(HwSurfaceList);
		HwSurfaceList = NULL;
		DEBUGOUT("ANDROID_VideoQuit() out HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);

		if( SDL_CurrentVideoSurface )
		{
			if( SDL_CurrentVideoSurface->hwdata )
				SDL_DestroyTexture((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata);
			if( SDL_CurrentVideoSurface->pixels )
				SDL_free(SDL_CurrentVideoSurface->pixels);
			SDL_CurrentVideoSurface->pixels = NULL;
		}
		SDL_CurrentVideoSurface = NULL;
		if(SDL_VideoWindow)
			SDL_DestroyWindow(SDL_VideoWindow);
		SDL_VideoWindow = NULL;
	}

	SDL_ANDROID_sFakeWindowWidth = 0;
	SDL_ANDROID_sFakeWindowWidth = 0;

	int i;
	
	/* Free video mode lists */
	for ( i=0; i<SDL_NUMMODES; ++i ) {
		if ( SDL_modelist[i] != NULL ) {
			SDL_free(SDL_modelist[i]);
			SDL_modelist[i] = NULL;
		}
	}
}

void ANDROID_PumpEvents(_THIS)
{
	SDL_ANDROID_PumpEvents();
}

static int ANDROID_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}

	if ( ! (surface->w && surface->h) )
		return -1;

	if( SDL_ANDROID_VideoForceSoftwareMode )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_SetVideoMode(): ignoring application attempt to allocate HW surface");
		return -1;
	}

	DEBUGOUT("ANDROID_AllocHWSurface() surface %p w %d h %d", surface, surface->w, surface->h);
	Uint32 format = PixelFormatEnumColorkey; // 1-bit alpha for color key, every surface will have colorkey so it's easier for us
	if( surface->format->Amask )
	{
		SDL_PixelFormat format1;
		int bpp;
		format = PixelFormatEnumAlpha;
		SDL_memset(&format1, 0, sizeof(format1));
		SDL_PixelFormatEnumToMasks( format, &bpp,
									&format1.Rmask, &format1.Gmask,
									&format1.Bmask, &format1.Amask );
		if( surface->format->BitsPerPixel != bpp ||
			surface->format->Rmask != format1.Rmask ||
			surface->format->Gmask != format1.Gmask ||
			surface->format->Bmask != format1.Bmask ||
			surface->format->Amask != format1.Amask )
			return(-1); // Do not allow alpha-surfaces with format other than RGBA4444 (it will be pain to lock/copy them)
	}
	else
	{
		// HW-accel surface should be RGB565
		if( !( SDL_CurrentVideoSurface->format->BitsPerPixel == surface->format->BitsPerPixel &&
			SDL_CurrentVideoSurface->format->Rmask == surface->format->Rmask &&
			SDL_CurrentVideoSurface->format->Gmask == surface->format->Gmask &&
			SDL_CurrentVideoSurface->format->Bmask == surface->format->Bmask &&
			SDL_CurrentVideoSurface->format->Amask == surface->format->Amask ) )
			return(-1);
	}

	surface->pitch = surface->w * surface->format->BytesPerPixel;
	surface->pixels = SDL_malloc(surface->h * surface->w * surface->format->BytesPerPixel);
	if ( surface->pixels == NULL ) {
		SDL_OutOfMemory();
		return(-1);
	}
	SDL_memset(surface->pixels, 0, surface->h*surface->pitch);

	surface->hwdata = (struct private_hwdata *)SDL_CreateTexture(format, SDL_TEXTUREACCESS_STATIC, surface->w, surface->h);
	if( !surface->hwdata ) {
		SDL_free(surface->pixels);
		surface->pixels = NULL;
		SDL_OutOfMemory();
		return(-1);
	}

	if( SDL_ANDROID_VideoLinearFilter )
		SDL_SetTextureScaleMode((SDL_Texture *)surface->hwdata, SDL_SCALEMODE_SLOW);

	if( surface->format->Amask )
	{
		SDL_SetTextureAlphaMod((struct SDL_Texture *)surface->hwdata, SDL_ALPHA_OPAQUE);
		SDL_SetTextureBlendMode((struct SDL_Texture *)surface->hwdata, SDL_BLENDMODE_BLEND);
	}
	
	surface->flags |= SDL_HWSURFACE | SDL_HWACCEL;
	
	HwSurfaceCount++;
	DEBUGOUT("ANDROID_AllocHWSurface() in HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);
	HwSurfaceList = SDL_realloc( HwSurfaceList, HwSurfaceCount * sizeof(SDL_Surface *) );
	DEBUGOUT("ANDROID_AllocHWSurface() out HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);

	HwSurfaceList[HwSurfaceCount-1] = surface;
	
	return 0;
}

static void ANDROID_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	int i;

	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return;
	}

	if( !surface->hwdata )
		return;
	SDL_DestroyTexture((struct SDL_Texture *)surface->hwdata);

	DEBUGOUT("ANDROID_FreeHWSurface() surface %p w %d h %d in HwSurfaceCount %d HwSurfaceList %p", surface, surface->w, surface->h, HwSurfaceCount, HwSurfaceList);

	for( i = 0; i < HwSurfaceCount; i++ )
	{
		if( HwSurfaceList[i] == surface )
		{
			HwSurfaceCount--;
			memmove(HwSurfaceList + i, HwSurfaceList + i + 1, sizeof(SDL_Surface *) * (HwSurfaceCount - i) );
			HwSurfaceList = SDL_realloc( HwSurfaceList, HwSurfaceCount * sizeof(SDL_Surface *) );
			i = -1;
			DEBUGOUT("ANDROID_FreeHWSurface() in HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);
			break;
		}
	}
	if( i != -1 )
		SDL_SetError("ANDROID_FreeHWSurface: cannot find freed HW surface in HwSurfaceList array");
}

static int ANDROID_LockHWSurface(_THIS, SDL_Surface *surface)
{
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}

	if( surface == SDL_CurrentVideoSurface )
	{
		// Copy pixels from pixelbuffer to video surface - this is slow!
		Uint16 * row = NULL;
		int fakeH = SDL_ANDROID_sFakeWindowHeight, fakeW = SDL_ANDROID_sFakeWindowWidth;
		int realH = SDL_ANDROID_sWindowHeight, realW = SDL_ANDROID_sWindowWidth;
		int x, y;
		if( ! SDL_CurrentVideoSurface->pixels )
		{
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			SDL_CurrentVideoSurface->pixels = SDL_malloc(SDL_ANDROID_sFakeWindowWidth * SDL_ANDROID_sFakeWindowHeight * SDL_ANDROID_BYTESPERPIXEL);
			if ( ! SDL_CurrentVideoSurface->pixels ) {
				__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate buffer for SDL_CurrentVideoSurface");
				SDL_SetError("Couldn't allocate buffer for SDL_CurrentVideoSurface");
				return(-1);
			}
		}
		if( ! SDL_CurrentVideoSurface->hwdata )
		{
			SDL_CurrentVideoSurface->hwdata = (struct private_hwdata *)SDL_CreateTexture(PixelFormatEnum, SDL_TEXTUREACCESS_STATIC, SDL_ANDROID_sFakeWindowWidth, SDL_ANDROID_sFakeWindowHeight);
			if( !SDL_CurrentVideoSurface->hwdata ) {
				__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate texture for SDL_CurrentVideoSurface");
				SDL_OutOfMemory();
				return(-1);
			}
			if( SDL_ANDROID_VideoLinearFilter )
				SDL_SetTextureScaleMode((SDL_Texture *)SDL_CurrentVideoSurface->hwdata, SDL_SCALEMODE_SLOW);
			// Register main video texture to be recreated when needed
			HwSurfaceCount++;
			HwSurfaceList = SDL_realloc( HwSurfaceList, HwSurfaceCount * sizeof(SDL_Surface *) );
			HwSurfaceList[HwSurfaceCount-1] = SDL_CurrentVideoSurface;
			DEBUGOUT("ANDROID_SetVideoMode() HwSurfaceCount %d HwSurfaceList %p", HwSurfaceCount, HwSurfaceList);
		}

		row = SDL_stack_alloc(Uint16, SDL_ANDROID_sWindowWidth);

		for(y=0; y<fakeH; y++)
		{
			// TODO: support 24bpp and 32bpp
			glReadPixels(0, realH - 1 - (realH * y / fakeH),
							realW, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, row);
			for(x=0; x<fakeW; x++)
				((Uint16 *)SDL_CurrentVideoSurface->pixels)[ fakeW * y + x ] = row[ x * fakeW / realW ];
		}
		
		SDL_stack_free(row);
	}

	if( !surface->hwdata )
		return(-1);

	// Extra check not necessary
	/*
	if( SDL_CurrentVideoSurface->format->BitsPerPixel == surface->format->BitsPerPixel &&
		SDL_CurrentVideoSurface->format->Rmask == surface->format->Rmask &&
		SDL_CurrentVideoSurface->format->Gmask == surface->format->Gmask &&
		SDL_CurrentVideoSurface->format->Bmask == surface->format->Bmask &&
		SDL_CurrentVideoSurface->format->Amask == surface->format->Amask )
		return(0);

	if( this->displayformatalphapixel->BitsPerPixel == surface->format->BitsPerPixel &&
		this->displayformatalphapixel->Rmask == surface->format->Rmask &&
		this->displayformatalphapixel->Gmask == surface->format->Gmask &&
		this->displayformatalphapixel->Bmask == surface->format->Bmask &&
		this->displayformatalphapixel->Amask == surface->format->Amask )
		return(0);
	return(-1);
	*/
	
	return(0);
}

static void ANDROID_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	SDL_PixelFormat format;
	Uint32 hwformat = PixelFormatEnumColorkey;
	int bpp;
	SDL_Surface * converted = NULL;

	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return;
	}

	if( !surface->hwdata )
		return;
	
	if( surface->format->Amask )
		hwformat = PixelFormatEnumAlpha;
		
	if( surface == SDL_CurrentVideoSurface ) // Special case
		hwformat = PixelFormatEnum;
	
		/* Allocate the new pixel format for the screen */
    SDL_memset(&format, 0, sizeof(format));
	SDL_PixelFormatEnumToMasks( hwformat, &bpp,
								&format.Rmask, &format.Gmask,
								&format.Bmask, &format.Amask );
	format.BytesPerPixel = SDL_ANDROID_BYTESPERPIXEL;
	format.BitsPerPixel = bpp;
	
	// TODO: support 24bpp and 32bpp
	if( format.BitsPerPixel == surface->format->BitsPerPixel &&
		format.Rmask == surface->format->Rmask &&
		format.Gmask == surface->format->Gmask &&
		format.Bmask == surface->format->Bmask &&
		format.Amask == surface->format->Amask )
	{
		converted = surface; // No need for conversion
	}
	else
	{
		Uint16 x, y;

		converted = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, format.BitsPerPixel,
											format.Rmask, format.Gmask, format.Bmask, format.Amask);
		if( !converted ) {
			SDL_OutOfMemory();
			return;
		}

		#define CONVERT_RGB565_RGBA5551( pixel ) (0x1 | ( (pixel & 0xFFC0) | ( (pixel & 0x1F) << 1 ) ))
		
		if( surface->flags & SDL_SRCCOLORKEY )
		{
			DEBUGOUT("ANDROID_UnlockHWSurface() CONVERT_RGB565_RGBA5551 + colorkey");
			for( y = 0; y < surface->h; y++ )
			{
				Uint16* src = (Uint16 *)( surface->pixels + surface->pitch * y );
				Uint16* dst = (Uint16 *)( converted->pixels + converted->pitch * y );
				Uint16 w = surface->w;
				Uint16 key = surface->format->colorkey;
				Uint16 pixel;
				for( x = 0; x < w; x++, src++, dst++ )
				{
					pixel = *src;
					*dst = (pixel == key) ? 0 : CONVERT_RGB565_RGBA5551( pixel );
				}
			}
		}
		else
		{
			DEBUGOUT("ANDROID_UnlockHWSurface() CONVERT_RGB565_RGBA5551");
			for( y = 0; y < surface->h; y++ )
			{
				Uint16* src = (Uint16 *)( surface->pixels + surface->pitch * y );
				Uint16* dst = (Uint16 *)( converted->pixels + converted->pitch * y );
				Uint16 w = surface->w;
				Uint16 pixel;
				for( x = 0; x < w; x++, src++, dst++ )
				{
					pixel = *src;
					*dst = CONVERT_RGB565_RGBA5551( pixel );
				}
			}
		}
	}

	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = surface->w;
	rect.h = surface->h;
	SDL_UpdateTexture((struct SDL_Texture *)surface->hwdata, &rect, converted->pixels, converted->pitch);

	if( surface == SDL_CurrentVideoSurface ) // Special case
		SDL_RenderCopy((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata, NULL, NULL);
	
	if( converted != surface )
		SDL_FreeSurface(converted);
}

// We're only blitting HW surface to screen, no other options provided (and if you need them your app designed wrong)
int ANDROID_HWBlit(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}

	if( dst != SDL_CurrentVideoSurface || (! src->hwdata) )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_HWBlit(): blitting SW");
		return(src->map->sw_blit(src, srcrect, dst, dstrect));
	}
	if( src == SDL_CurrentVideoSurface )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_HWBlit(): reading from screen surface not supported");
		return(-1);
	}
	
	return SDL_RenderCopy((struct SDL_Texture *)src->hwdata, srcrect, dstrect);
};

static int ANDROID_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst)
{
	// This part is ignored by SDL (though it should not be)
	/*
	if( dst != SDL_CurrentVideoSurface || ! src->hwdata )
		return(-1);
	*/
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_CheckHWBlit()");
	src->map->hw_blit = ANDROID_HWBlit;
	src->flags |= SDL_HWACCEL;
	return(0);
};

static int ANDROID_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *rect, Uint32 color)
{
	Uint8 r, g, b, a;

	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}

	if( dst != SDL_CurrentVideoSurface )
	{
		// TODO: hack
		current_video->info.blit_fill = 0;
		SDL_FillRect( dst, rect, color );
		current_video->info.blit_fill = 1;
		return(0);
	}
	SDL_GetRGBA(color, dst->format, &r, &g, &b, &a);
	SDL_SetRenderDrawColor( r, g, b, a );
	return SDL_RenderFillRect(rect);
};

static int ANDROID_SetHWColorKey(_THIS, SDL_Surface *surface, Uint32 key)
{
	SDL_PixelFormat format;
	SDL_Surface * converted = NULL;

	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}
	
	if( !surface->hwdata )
		return(-1);
	if( surface->format->Amask )
		return(-1);

	surface->flags |= SDL_SRCCOLORKEY;

	ANDROID_UnlockHWSurface(this, surface); // Convert surface using colorkey

	SDL_SetTextureBlendMode((struct SDL_Texture *)surface->hwdata, SDL_BLENDMODE_BLEND);

	return 0;
};

static int ANDROID_SetHWAlpha(_THIS, SDL_Surface *surface, Uint8 value)
{
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}

	if( !surface->hwdata )
		return(-1);

	surface->flags |= SDL_SRCALPHA;

	if( value == SDL_ALPHA_OPAQUE && ! (surface->flags & SDL_SRCCOLORKEY) )
		SDL_SetTextureBlendMode((struct SDL_Texture *)surface->hwdata, SDL_BLENDMODE_NONE);
	else
		SDL_SetTextureBlendMode((struct SDL_Texture *)surface->hwdata, SDL_BLENDMODE_BLEND);
	
	return SDL_SetTextureAlphaMod((struct SDL_Texture *)surface->hwdata, value);
};

static void ANDROID_FlipHWSurfaceInternal(int numrects, SDL_Rect *rects)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_FlipHWSurface()");
	if( SDL_CurrentVideoSurface->hwdata && SDL_CurrentVideoSurface->pixels && ! ( SDL_CurrentVideoSurface->flags & SDL_HWSURFACE ) )
	{
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = SDL_CurrentVideoSurface->w;
		rect.h = SDL_CurrentVideoSurface->h;
		if(numrects == 0)
			SDL_UpdateTexture((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata, &rect, SDL_CurrentVideoSurface->pixels, SDL_CurrentVideoSurface->pitch);
		else
		{
			int i;
			for(i = 0; i < numrects; i++)
			{
				//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_UpdateTexture: rect %d: %04d:%04d:%04d:%04d", i, rects[i].x, rects[i].y, rects[i].w, rects[i].h);
				SDL_UpdateTexture((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata, &rects[i],
					SDL_CurrentVideoSurface->pixels + rects[i].y * SDL_CurrentVideoSurface->pitch +
					rects[i].x * SDL_CurrentVideoSurface->format->BytesPerPixel,
					SDL_CurrentVideoSurface->pitch);
			}
		}

		if( !SDL_ANDROID_SystemBarAndKeyboardShown )
			SDL_RenderCopy((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata, &rect, &rect);
		else
		{
			int x, y;
			SDL_Rect dstrect;
			SDL_GetMouseState(&x, &y); // Follow mouse pointer location
			rect.w = SDL_ANDROID_ScreenVisibleRect.w * SDL_ANDROID_sFakeWindowWidth / SDL_ANDROID_sRealWindowWidth;
			rect.h = SDL_ANDROID_ScreenVisibleRect.h * SDL_ANDROID_sFakeWindowHeight / SDL_ANDROID_sRealWindowHeight;
			rect.x = x - rect.w / 2;
			if( rect.x < 0 )
				rect.x = 0;
			if( rect.x + rect.w > SDL_ANDROID_sFakeWindowWidth )
				rect.x = SDL_ANDROID_sFakeWindowWidth - rect.w;
			rect.y = y - rect.h / 2;
			if( rect.y < 0 )
				rect.y = 0;
			if( rect.y + rect.h > SDL_ANDROID_sFakeWindowHeight )
				rect.y = SDL_ANDROID_sFakeWindowHeight - rect.h;
			dstrect.w = rect.w;
			dstrect.h = rect.h;
			dstrect.x = SDL_ANDROID_ScreenVisibleRect.x * SDL_ANDROID_sFakeWindowWidth / SDL_ANDROID_sRealWindowWidth;
			dstrect.y = SDL_ANDROID_ScreenVisibleRect.y * SDL_ANDROID_sFakeWindowHeight / SDL_ANDROID_sRealWindowHeight;
			//__android_log_print(ANDROID_LOG_INFO, "SDL", "SDL_Flip: %04d:%04d:%04d:%04d -> %04d:%04d:%04d:%04d vis %04d:%04d:%04d:%04d", rect.x, rect.y, rect.w, rect.h, dstrect.x, dstrect.y, dstrect.w, dstrect.h, SDL_ANDROID_ScreenVisibleRect.x, SDL_ANDROID_ScreenVisibleRect.y, SDL_ANDROID_ScreenVisibleRect.w, SDL_ANDROID_ScreenVisibleRect.h);
			SDL_RenderCopy((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata, &rect, &dstrect);
		}

		if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_MAGNIFIER )
		{
			SDL_Rect dstrect = SDL_ANDROID_ShowScreenUnderFingerRect;
			rect = SDL_ANDROID_ShowScreenUnderFingerRectSrc;
			SDL_RenderCopy((struct SDL_Texture *)SDL_CurrentVideoSurface->hwdata, &rect, &dstrect);
			int buttons = SDL_GetMouseState(NULL, NULL);
			// Do it old-fashioned way with direct GL calls
#if SDL_VIDEO_OPENGL_ES_VERSION == 1
			glPushMatrix();
			glLoadIdentity();
			glOrthof( 0.0f, SDL_ANDROID_sFakeWindowWidth, SDL_ANDROID_sFakeWindowHeight, 0.0f, 0.0f, 1.0f );
			glEnableClientState(GL_VERTEX_ARRAY);
			glColor4f(0.0f, buttons & SDL_BUTTON_RMASK ? 1.0f : 0.0f, buttons & SDL_BUTTON_LMASK ? 1.0f : 0.0f, 1.0f);
			GLshort vertices[] = {	dstrect.x, dstrect.y,
									dstrect.x + dstrect.w, dstrect.y,
									dstrect.x + dstrect.w, dstrect.y + dstrect.h,
									dstrect.x, dstrect.y + dstrect.h };
			glVertexPointer(2, GL_SHORT, 0, vertices);
			glDrawArrays(GL_LINE_LOOP, 0, 4);
			glDisableClientState(GL_VERTEX_ARRAY);
			glPopMatrix();
#endif
		}
#ifdef VIDEO_DEBUG
		if( SDL_ANDROID_VideoDebugRect.w > 0 )
		{
			SDL_Rect frame = SDL_ANDROID_VideoDebugRect;
			glPushMatrix();
			glLoadIdentity();
			glOrthof( 0.0f, SDL_ANDROID_sFakeWindowWidth, SDL_ANDROID_sFakeWindowHeight, 0.0f, 0.0f, 1.0f );
			glEnableClientState(GL_VERTEX_ARRAY);
			glColor4f(SDL_ANDROID_VideoDebugRectColor.r / 255.0f, SDL_ANDROID_VideoDebugRectColor.g / 255.0f, SDL_ANDROID_VideoDebugRectColor.b / 255.0f, 1.0f);
			GLshort vertices[] = {	frame.x, frame.y,
									frame.x + frame.w, frame.y,
									frame.x + frame.w, frame.y + frame.h,
									frame.x, frame.y + frame.h };
			glVertexPointer(2, GL_SHORT, 0, vertices);
			glDrawArrays(GL_LINE_LOOP, 0, 4);
			glDisableClientState(GL_VERTEX_ARRAY);
			glPopMatrix();
		}
#endif
		if(SDL_ANDROID_ShowMouseCursor)
		{
			if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_NONE || SDL_ANDROID_ShowScreenUnderFinger == ZOOM_MAGNIFIER )
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				x = x * SDL_ANDROID_sRealWindowWidth / SDL_ANDROID_sFakeWindowWidth;
				y = y * SDL_ANDROID_sRealWindowHeight / SDL_ANDROID_sFakeWindowHeight;
				SDL_ANDROID_DrawMouseCursor( x, y, 0, 1.0f );
			}
			if( SDL_ANDROID_ShowScreenUnderFinger != ZOOM_NONE &&
				SDL_ANDROID_ShowScreenUnderFingerRectSrc.w > 0 &&
				SDL_ANDROID_ShowScreenUnderFingerRectSrc.h > 0 )
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				x = SDL_ANDROID_ShowScreenUnderFingerRect.x +
					( x - SDL_ANDROID_ShowScreenUnderFingerRectSrc.x ) *
					SDL_ANDROID_ShowScreenUnderFingerRect.w / SDL_ANDROID_ShowScreenUnderFingerRectSrc.w;
				y = SDL_ANDROID_ShowScreenUnderFingerRect.y +
					( y - SDL_ANDROID_ShowScreenUnderFingerRectSrc.y ) *
					SDL_ANDROID_ShowScreenUnderFingerRect.h / SDL_ANDROID_ShowScreenUnderFingerRectSrc.h;
				x = x * SDL_ANDROID_sRealWindowWidth / SDL_ANDROID_sFakeWindowWidth;
				y = y * SDL_ANDROID_sRealWindowHeight / SDL_ANDROID_sFakeWindowHeight;
				SDL_ANDROID_DrawMouseCursor( x, y, 16, 1.0f );
			}
		}
	}
};

static int ANDROID_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return -1;
	}

	if(!SDL_CurrentVideoSurface)
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s without main video surface!", __PRETTY_FUNCTION__);
		return -1;
	}

	ANDROID_FlipHWSurfaceInternal(0, NULL);

	SDL_ANDROID_CallJavaSwapBuffers();

	return(0);
}

static void ANDROID_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return;
	}

	if(!SDL_CurrentVideoSurface)
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s without main video surface!", __PRETTY_FUNCTION__);
		return;
	}

#ifdef SDL_COMPATIBILITY_HACKS_PROPER_USADE_OF_SDL_UPDATERECTS
	ANDROID_FlipHWSurfaceInternal(numrects, rects); // Fails for fheroes2, I'll add a compatibility option later.
#else
	ANDROID_FlipHWSurfaceInternal(0, NULL);
#endif

	SDL_ANDROID_CallJavaSwapBuffers();
}

void ANDROID_GL_SwapBuffers(_THIS)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_GL_SwapBuffers");
	// TeeWorlds use hacky hack to draw from separate thread, however it works surprisingly well
	/*
	if( !SDL_ANDROID_InsideVideoThread() )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Error: calling %s not from the main thread!", __PRETTY_FUNCTION__);
		return;
	}
	*/

	SDL_ANDROID_CallJavaSwapBuffers();
};

int ANDROID_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	return(1);
}

void SDL_ANDROID_VideoContextLost()
{
	if( ! sdl_opengl )
	{
		int i;
		for( i = 0; i < HwSurfaceCount; i++ )
		{
			SDL_DestroyTexture((struct SDL_Texture *)HwSurfaceList[i]->hwdata);
			HwSurfaceList[i]->hwdata = NULL;
		}
	}
};

extern DECLSPEC int SDL_ANDROID_ScreenKeyboardUpdateToNewVideoMode(int oldx, int oldy, int newx, int newy);

void SDL_ANDROID_VideoContextRecreated()
{
	int i;
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Sending SDL_VIDEORESIZE event %dx%d", SDL_ANDROID_sFakeWindowWidth, SDL_ANDROID_sFakeWindowHeight);
	//SDL_PrivateResize(SDL_ANDROID_sFakeWindowWidth, SDL_ANDROID_sFakeWindowHeight);
	if ( SDL_ProcessEvents[SDL_VIDEORESIZE] == SDL_ENABLE ) {
		SDL_Event event;
		event.type = SDL_VIDEORESIZE;
		event.resize.w = SDL_ANDROID_sFakeWindowWidth;
		event.resize.h = SDL_ANDROID_sFakeWindowHeight;
		if ( (SDL_EventOK == NULL) || (*SDL_EventOK)(&event) ) {
			SDL_PushEvent(&event);
		}
	}

	SDL_ANDROID_ScreenKeyboardUpdateToNewVideoMode(SDL_modelist[0]->w, SDL_modelist[0]->h, SDL_ANDROID_sWindowWidth, SDL_ANDROID_sWindowHeight);

	SDL_modelist[0]->w = SDL_ANDROID_sWindowWidth;
	SDL_modelist[0]->h = SDL_ANDROID_sWindowHeight;

	if( ! sdl_opengl )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Setting display dimensions to %dx%d", SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sRealWindowHeight);
		SDL_PrivateAndroidSetDesktopMode(SDL_VideoWindow, SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sRealWindowHeight);
		SDL_SelectRenderer(SDL_VideoWindow); // Re-apply glOrtho() and blend modes
		// Re-apply our custom 4:3 screen aspect ratio
#if SDL_VIDEO_OPENGL_ES_VERSION == 1
		glViewport(0, 0, SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sRealWindowHeight);
		glOrthof(0, SDL_ANDROID_sRealWindowWidth, SDL_ANDROID_sWindowHeight, 0, 0, 1);
#endif
		for( i = 0; i < HwSurfaceCount; i++ )
		{
			// Allocate HW texture
			Uint32 format = PixelFormatEnumColorkey; // 1-bit alpha for color key, every surface will have colorkey so it's easier for us
			int flags = HwSurfaceList[i]->flags;
			if( HwSurfaceList[i]->format->Amask )
				format = PixelFormatEnumAlpha;
			if( HwSurfaceList[i] == SDL_CurrentVideoSurface )
				format = PixelFormatEnum;
			HwSurfaceList[i]->hwdata = (struct private_hwdata *)SDL_CreateTexture(format, SDL_TEXTUREACCESS_STATIC, HwSurfaceList[i]->w, HwSurfaceList[i]->h);
			if( !HwSurfaceList[i]->hwdata )
			{
				SDL_OutOfMemory();
				return;
			}
			if( SDL_ANDROID_VideoLinearFilter )
				SDL_SetTextureScaleMode((SDL_Texture *)HwSurfaceList[i]->hwdata, SDL_SCALEMODE_SLOW);
			if (flags & SDL_SRCALPHA)
			{
				int alpha = HwSurfaceList[i]->format->alpha;
				ANDROID_SetHWAlpha(NULL, HwSurfaceList[i], alpha);
				ANDROID_UnlockHWSurface(NULL, HwSurfaceList[i]); // Re-fill texture with graphics
			}
			else if (flags & SDL_SRCCOLORKEY)
			{
				int colorkey = HwSurfaceList[i]->format->colorkey;
				ANDROID_SetHWColorKey(NULL, HwSurfaceList[i], colorkey);
			}
			else
			{
				ANDROID_UnlockHWSurface(NULL, HwSurfaceList[i]); // Re-fill texture with graphics
			}
		}
		SDL_ANDROID_CallJavaSwapBuffers(); // Swap buffers once to force screen redraw
	}
};

static void* ANDROID_GL_GetProcAddress(_THIS, const char *proc)
{
#ifdef USE_GL4ES
	void * func = glXGetProcAddress(proc);
#else
	void * func = dlsym(glLibraryHandle, proc);
#endif
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_GL_GetProcAddress(\"%s\"): %p", proc, func);
	return func;
};

// Multithreaded video - this will free up some CPU time while GPU renders video inside SDL_Flip()
enum videoThreadCmd_t { CMD_INIT, CMD_SETVIDEOMODE, CMD_QUIT, CMD_UPDATERECTS, CMD_FLIP };
typedef struct
{
	SDL_mutex * mutex;
	SDL_cond * cond;
	SDL_cond * cond2;
	int execute;
	int threadReady;
	enum videoThreadCmd_t cmd;
	SDL_VideoDevice *_this;
	SDL_PixelFormat *vformat;
	SDL_Surface *current;
	int width;
	int height;
	int bpp;
	Uint32 flags;
	int numrects;
	SDL_Rect *rects;
	
	int retcode;
	SDL_Surface * retcode2;
} videoThread_t;
static videoThread_t videoThread;

extern void SDL_ANDROID_MultiThreadedVideoLoopInit();
extern void SDL_ANDROID_MultiThreadedVideoLoop();

void SDL_ANDROID_MultiThreadedVideoLoopInit()
{
	videoThread.mutex = SDL_CreateMutex();
	videoThread.cond = SDL_CreateCond();
	videoThread.cond2 = SDL_CreateCond();
	videoThread.execute = 0;
}

void SDL_ANDROID_MultiThreadedVideoLoop()
{
	int lastUpdate = SDL_GetTicks(); // For compat mode
	int nextUpdateDelay = 100; // For compat mode
	while(1)
	{
		int signalNeeded = 0;
		int swapBuffersNeeded = 0;
		int ret;
		int currentTime;
		SDL_mutexP(videoThread.mutex);
		videoThread.threadReady = 1;
		SDL_CondSignal(videoThread.cond2);
		ret = SDL_CondWaitTimeout(videoThread.cond, videoThread.mutex, SDL_ANDROID_CompatibilityHacks ? nextUpdateDelay : 1000);
		if( videoThread.execute )
		{
			videoThread.threadReady = 0;
			switch( videoThread.cmd )
			{
				case CMD_INIT:
					videoThread.retcode = ANDROID_VideoInit(videoThread._this, videoThread.vformat);
					break;
				case CMD_SETVIDEOMODE:
					videoThread.retcode2 = ANDROID_SetVideoMode(videoThread._this, videoThread.current,
							videoThread.width, videoThread.height, videoThread.bpp, videoThread.flags);
					break;
				case CMD_QUIT:
					ANDROID_VideoQuit(videoThread._this);
					break;
				case CMD_UPDATERECTS:
					if( SDL_ANDROID_CompatibilityHacks ) // DIRTY HACK for MilkyTracker - DO NOT update screen when application requests that, update 50 ms later
					{
						if( nextUpdateDelay >= 100 )
							nextUpdateDelay = 50;
						else
							nextUpdateDelay = lastUpdate + 50 - (int)SDL_GetTicks();
					}
					else
					{
						ANDROID_FlipHWSurfaceInternal(videoThread.numrects, videoThread.rects);
						swapBuffersNeeded = 1;
					}
					break;
				case CMD_FLIP:
					if( SDL_ANDROID_CompatibilityHacks ) // DIRTY HACK for MilkyTracker - DO NOT update screen when application requests that, update 50 ms later
					{
						if( nextUpdateDelay >= 100 )
							nextUpdateDelay = 50;
						else
							nextUpdateDelay = lastUpdate + 50 - (int)SDL_GetTicks();
					}
					else
					{
						ANDROID_FlipHWSurfaceInternal(0, NULL);
						swapBuffersNeeded = 1;
					}
					break;
			}
			videoThread.execute = 0;
			signalNeeded = 1;
		}
		if( SDL_ANDROID_CompatibilityHacks && SDL_CurrentVideoSurface &&
				( ret == SDL_MUTEX_TIMEDOUT || nextUpdateDelay <= 0 ) )
		{
			ANDROID_FlipHWSurfaceInternal(0, NULL);
			swapBuffersNeeded = 1;
			lastUpdate = SDL_GetTicks();
			nextUpdateDelay = 100;
		}
		SDL_mutexV(videoThread.mutex);
		if( signalNeeded )
			SDL_CondSignal(videoThread.cond2);
		if( swapBuffersNeeded )
		{
			SDL_ANDROID_CallJavaSwapBuffers();
		}
	}
}

int ANDROID_VideoInitMT(_THIS, SDL_PixelFormat *vformat)
{
	SDL_mutexP(videoThread.mutex);
	while( ! videoThread.threadReady )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	videoThread.cmd = CMD_INIT;
	videoThread._this = this;
	videoThread.vformat = vformat;
	videoThread.execute = 1;
	SDL_CondSignal(videoThread.cond);
	while( videoThread.execute )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	int ret = videoThread.retcode;
	SDL_mutexV(videoThread.mutex);
	return ret;
}

SDL_Surface *ANDROID_SetVideoModeMT(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags)
{
	if( flags & SDL_OPENGL || (flags & SDL_HWSURFACE && !SDL_ANDROID_VideoForceSoftwareMode) )
	{
		return NULL;
	}
	SDL_mutexP(videoThread.mutex);
	while( ! videoThread.threadReady )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	videoThread.cmd = CMD_SETVIDEOMODE;
	videoThread._this = this;
	videoThread.current = current;
	videoThread.width = width;
	videoThread.height = height;
	videoThread.bpp = bpp;
	videoThread.flags = flags;
	videoThread.execute = 1;
	SDL_CondSignal(videoThread.cond);
	while( videoThread.execute )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	SDL_Surface * ret = videoThread.retcode2;
	SDL_mutexV(videoThread.mutex);
	return ret;
}

void ANDROID_VideoQuitMT(_THIS)
{
	SDL_mutexP(videoThread.mutex);
	while( ! videoThread.threadReady )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	videoThread.cmd = CMD_QUIT;
	videoThread._this = this;
	videoThread.execute = 1;
	SDL_CondSignal(videoThread.cond);
	while( videoThread.execute )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	SDL_mutexV(videoThread.mutex);
}

void ANDROID_UpdateRectsMT(_THIS, int numrects, SDL_Rect *rects)
{
	SDL_mutexP(videoThread.mutex);
	while( ! videoThread.threadReady )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	videoThread.cmd = CMD_UPDATERECTS;
	videoThread._this = this;
	videoThread.numrects = numrects;
	videoThread.rects = rects;
	videoThread.execute = 1;
	SDL_CondSignal(videoThread.cond);
	while( videoThread.execute )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	SDL_mutexV(videoThread.mutex);
}

int ANDROID_FlipHWSurfaceMT(_THIS, SDL_Surface *surface)
{
	SDL_mutexP(videoThread.mutex);
	while( ! videoThread.threadReady )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	videoThread.cmd = CMD_FLIP;
	videoThread._this = this;
	videoThread.execute = 1;
	SDL_CondSignal(videoThread.cond);
	while( videoThread.execute )
		SDL_CondWaitTimeout(videoThread.cond2, videoThread.mutex, 1000);
	SDL_mutexV(videoThread.mutex);
	return 0;
}
