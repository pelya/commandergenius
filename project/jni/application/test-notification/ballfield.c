/*
 * "Ballfield"
 *
 *   (C) David Olofson <david@olofson.net>, 2002, 2003
 *
 * This software is released under the terms of the GPL.
 *
 * Contact author for permission if you want to use this
 * software, or work derived from it, under other terms.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>
#include <wchar.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_screenkeyboard.h>
#include <SDL/SDL_android.h>

#define fprintf(X, ...) __android_log_print(ANDROID_LOG_INFO, "Ballfield", __VA_ARGS__)
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "Ballfield", __VA_ARGS__)


/*----------------------------------------------------------
	Definitions...
----------------------------------------------------------*/

#define	SCREEN_W	640
#define	SCREEN_H	540


#define	BALLS	300

#define	COLORS	2

typedef struct
{
	Sint32	x, y, z;	/* Position */
	Uint32	c;		/* Color */
} point_t;


/*
 * Ballfield
 */
typedef struct
{
	point_t		points[BALLS];
	SDL_Rect	*frames;
	SDL_Surface	*gfx[COLORS];
	int		use_alpha;
} ballfield_t;


/*
 * Size of the biggest ball image in pixels
 *
 * Balls are scaled down and *packed*, one pixel
 * smaller for each frame down to 1x1. The actual
 * image width is (obviously...) the same as the
 * width of the first frame.
 */
#define	BALL_W	32
#define	BALL_H	32




/*----------------------------------------------------------
	General tool functions
----------------------------------------------------------*/

/*
 * Bump areas of low and high alpha to 0% or 100%
 * respectively, just in case the graphics contains
 * "alpha noise".
 */
SDL_Surface *clean_alpha(SDL_Surface *s)
{
	SDL_Surface *work;
	SDL_Rect r;
	Uint32 *pixels;
	int pp;
	int x, y;
	
	work = SDL_CreateRGBSurface(SDL_SWSURFACE, s->w, s->h,
			32, 0xff000000, 0x00ff0000, 0x0000ff00,
			0x000000ff);
	if(!work)
		return NULL;

	r.x = r.y = 0;
	r.w = s->w;
	r.h = s->h;
	if(SDL_BlitSurface(s, &r, work, NULL) < 0)
	{
		SDL_FreeSurface(work);
		return NULL;
	}

	SDL_LockSurface(work);
	pixels = (Uint32 *)work->pixels;
	pp = work->pitch / sizeof(Uint32);
	for(y = 0; y < work->h; ++y)
		for(x = 0; x < work->w; ++x)
		{
			Uint32 pix = pixels[y*pp + x];
			switch((pix & 0xff) >> 4)
			{
			  case 0:
				pix = 0x00000000;
			  	break;
			  default:
			  	break;
			  case 15:
				pix |= 0xff;
			  	break;
			}
			pixels[y*pp + x] = pix;
		}
	SDL_UnlockSurface(work);

	return work;
}


/*
 * Load and convert an antialiazed, zoomed set of sprites.
 */
SDL_Surface *load_zoomed(const char *name, int alpha)
{
	SDL_Surface *sprites;
	SDL_Surface *temp = IMG_Load(name);
	if(!temp)
		return NULL;

	/*
	sprites = temp;
	SDL_SetAlpha(sprites, 0, 255);
	temp = clean_alpha(sprites);
	SDL_FreeSurface(sprites);
	*/
	if(!temp)
	{
		fprintf(stderr, "Could not clean alpha!\n");
		return NULL;
	}

	if(alpha)
	{
		SDL_SetAlpha(temp, 0, SDL_ALPHA_OPAQUE);
		sprites = SDL_DisplayFormatAlpha(temp);
	}
	else
	{
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY,
				SDL_MapRGB(temp->format, 0, 0, 0));
		sprites = SDL_DisplayFormat(temp);
	}
	SDL_FreeSurface(temp);

	return sprites;
}


void print_num(SDL_Surface *dst, SDL_Surface *font, int x, int y, float value)
{
	char buf[16];
	int val = (int)(value * 10.0);
	int pos, p = 0;
	SDL_Rect from;

	/* Sign */
	if(val < 0)
	{
		buf[p++] = 10;
		val = -val;
	}

	/* Integer part */
	pos = 10000000;
	while(pos > 1)
	{
		int num = val / pos;
		val -= num * pos;
		pos /= 10;
		if(p || num)
			buf[p++] = num;
	}

	/* Decimals */
	if(val / pos)
	{
		buf[p++] = 11;
		while(pos > 0)
		{
			int num = val / pos;
			val -= num * pos;
			pos /= 10;
			buf[p++] = num;
		}
	}

	/* Render! */
	from.y = 0;
	from.w = 7;
	from.h = 10;
	for(pos = 0; pos < p; ++pos)
	{
		SDL_Rect to;
		to.x = x + pos * 7;
		to.y = y;
		from.x = buf[pos] * 7;
		SDL_BlitSurface(font, &from, dst, &to);
	}
}

void print_num_hex(SDL_Surface *dst, SDL_Surface *font, int x, int y, unsigned val)
{
	char buf[8];
	int pos, p = 0;
	SDL_Rect from;
	
	//val = htonl(val); // Big-endian

	/* Render! */
	from.y = 0;
	from.w = 7;
	from.h = 10;
	for(pos = 0; pos < 8; ++pos)
	{
		SDL_Rect to;
		to.x = 8 * 7 - (x + pos * 7); // Little-endian number wrapped backwards
		to.y = y;
		from.x = ( ( val >> (pos * 4) ) & 0xf ) * 7;
		SDL_BlitSurface(font, &from, dst, &to);
	}
}

/*----------------------------------------------------------
	ballfield_t functions
----------------------------------------------------------*/

ballfield_t *ballfield_init(void)
{
	int i;
	ballfield_t *bf = (ballfield_t *)calloc(sizeof(ballfield_t), 1);
	if(!bf)
		return NULL;
	for(i = 0; i < BALLS; ++i)
	{
		bf->points[i].x = rand() % 0x20000;
		bf->points[i].y = rand() % 0x20000;
		bf->points[i].z = 0x20000 * i / BALLS;
		if(rand() % 100 > 80)
			bf->points[i].c = 1;
		else
			bf->points[i].c = 0;
	}
	return bf;
}


void ballfield_free(ballfield_t *bf)
{
	int i;
	for(i = 0; i < COLORS; ++i)
		SDL_FreeSurface(bf->gfx[i]);
}


static int ballfield_init_frames(ballfield_t *bf)
{
	int i, j;
	/*
	 * Set up source rects for all frames
	 */
	bf->frames = (SDL_Rect *)calloc(sizeof(SDL_Rect), bf->gfx[0]->w);
	if(!bf->frames)
	{
		fprintf(stderr, "No memory for frame rects!\n");
		return -1;
	}
	for(j = 0, i = 0; i < bf->gfx[0]->w; ++i)
	{
		bf->frames[i].x = 0;
		bf->frames[i].y = j;
		bf->frames[i].w = bf->gfx[0]->w - i;
		bf->frames[i].h = bf->gfx[0]->w - i;
		j += bf->gfx[0]->w - i;
	}
	return 0;
}


int ballfield_load_gfx(ballfield_t *bf, const char *name, unsigned int color)
{
	if(color >= COLORS)
		return -1;

	bf->gfx[color] = load_zoomed(name, bf->use_alpha);
	if(!bf->gfx[color])
		return -2;

	if(!bf->frames)
		return ballfield_init_frames(bf);

	return 0;
}


void ballfield_move(ballfield_t *bf, Sint32 dx, Sint32 dy, Sint32 dz)
{
	int i;
	for(i = 0; i < BALLS; ++i)
	{
		bf->points[i].x += dx;
		bf->points[i].x &= 0x1ffff;
		bf->points[i].y += dy;
		bf->points[i].y &= 0x1ffff;
		bf->points[i].z += dz;
		bf->points[i].z &= 0x1ffff;
	}
}


void ballfield_render(ballfield_t *bf, SDL_Surface *screen)
{
	int i, j, z;

	/* 
	 * Find the ball with the highest Z.
	 */
	z = 0;
	j = 0;
	for(i = 0; i < BALLS; ++i)
	{
		if(bf->points[i].z > z)
		{
			j = i;
			z = bf->points[i].z;
		}
	}

	/* 
	 * Render all balls in back->front order.
	 */
	for(i = 0; i < BALLS; ++i)
	{
		SDL_Rect r;
		int f;
		z = bf->points[j].z;
		z += 50;
		f = ((bf->frames[0].w << 12) + 100000) / z;
		f = bf->frames[0].w - f;
		if(f < 0)
			f = 0;
		else if(f > bf->frames[0].w - 1)
			f = bf->frames[0].w - 1;
		z >>= 7;
		z += 1;
		r.x = (bf->points[j].x - 0x10000) / z;
		r.y = (bf->points[j].y - 0x10000) / z;
		r.x += (screen->w - bf->frames[f].w) >> 1;
		r.y += (screen->h - bf->frames[f].h) >> 1;
		SDL_BlitSurface(bf->gfx[bf->points[j].c],
			&bf->frames[f], screen, &r);
		if(--j < 0)
			j = BALLS - 1;
	}
}



/*----------------------------------------------------------
	Other rendering functions
----------------------------------------------------------*/

/*
 * Draw tiled background image with offset.
 */
void tiled_back(SDL_Surface *back, SDL_Surface *screen, int xo, int yo)
{
	/*
	int x, y;
	SDL_Rect r;
	if(xo < 0)
		xo += back->w*(-xo/back->w + 1);
	if(yo < 0)
		yo += back->h*(-yo/back->h + 1);
	xo %= back->w;
	yo %= back->h;
	for(y = -yo; y < screen->h; y += back->h)
		for(x = -xo; x < screen->w; x += back->w)
		{
			r.x = x;
			r.y = y;
			SDL_BlitSurface(back, NULL, screen, &r);
		}
	*/
	SDL_Rect r;
	xo %= back->w/8;
	yo %= back->h/8;
	r.x = xo - back->w/2 + screen->w/2;
	r.y = yo - back->h/2 + screen->h/2;
	r.w = back->w;
	r.h = back->h;
	SDL_BlitSurface(back, NULL, screen, &r);
}

/*----------------------------------------------------------
	main()
----------------------------------------------------------*/

int main(int argc, char* argv[])
{
	ballfield_t	*balls;
	SDL_Surface	*screen;
	SDL_Surface	*temp_image;
	SDL_Surface	*back, *logo, *font, *font_hex;
	SDL_Event	event;
	int		bpp = 16,
			flags = SDL_HWSURFACE,
			alpha = 1;
	int		x_offs = 0, y_offs = 0;
	long		tick,
			last_tick,
			last_avg_tick;
	double		t = 0;
	float		dt;
	int		i;
	float		fps = 0.0;
	int		fps_count = 0;
	int		fps_start = 0;
	float		x_speed, y_speed, z_speed;
	enum { MAX_POINTERS = 16 };
	// some random colors
	int colors[MAX_POINTERS] = { 0xaaaaaa, 0xffffff, 0x888888, 0xcccccc, 0x666666, 0x999999, 0xdddddd, 0xeeeeee, 0xaaaaaa, 0xffffff, 0x888888, 0xcccccc, 0x666666, 0x999999, 0xdddddd, 0xeeeeee };
	struct TouchPointer_t { int x; int y; int pressure; int pressed; } touchPointers[MAX_POINTERS];
	int accel[5], screenjoy[4], gamepads[4][8];
	SDL_Surface	*mouse[4];
	int screenKeyboardShown = 0;
	char asyncTextInputBuf[256];


	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_Joystick * joysticks[6];
	for( i = 0; i < 6; i++ )
		joysticks[i] = SDL_JoystickOpen(i);

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
	if(!screen)
	{
		fprintf(stderr, "Failed to open screen!\n");
		exit(-1);
	}

	SDL_WM_SetCaption("Ballfield", "Ballfield");
	if(flags & SDL_FULLSCREEN)
		SDL_ShowCursor(0);

	balls = ballfield_init();
	if(!balls)
	{
		fprintf(stderr, "Failed to create ballfield!\n");
		exit(-1);
	}

	/*
	 * Load and prepare balls...
	 */
	balls->use_alpha = alpha;
	if( ballfield_load_gfx(balls, "blueball.png", 0)
				||
			ballfield_load_gfx(balls, "redball.png", 1) )
	{
		fprintf(stderr, "Could not load balls!\n");
		exit(-1);
	}

	/*
	 * Load background image
	 */
	temp_image = IMG_Load(argc > 1 ? argv[1] : "sun.gif");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load background!\n");
		exit(-1);
	}
	back = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	/*
	 * Load logo
	 */
	temp_image = SDL_LoadBMP("logo.bmp");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load logo!\n");
		exit(-1);
	}
	SDL_SetColorKey(temp_image, SDL_SRCCOLORKEY,
			SDL_MapRGB(temp_image->format, 255, 0, 255));
	logo = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	/*
	 * Load font
	 */
	temp_image = SDL_LoadBMP("font7x10.bmp");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load font!\n");
		exit(-1);
	}
	SDL_SetColorKey(temp_image, SDL_SRCCOLORKEY,
			SDL_MapRGB(temp_image->format, 255, 0, 255));
	font = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	temp_image = SDL_LoadBMP("font7x10-hex.bmp");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load hex font!\n");
		exit(-1);
	}
	SDL_SetColorKey(temp_image, SDL_SRCCOLORKEY,
			SDL_MapRGB(temp_image->format, 255, 0, 255));
	font_hex = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	for(i = 0; i < 4; i++)
	{
		char name[32];
		sprintf(name, "mouse%d.png", i);
		temp_image = IMG_Load(name);
		if(!temp_image)
		{
			fprintf(stderr, "Could not load %s!\n", name);
			exit(-1);
		}
		//mouse[i] = SDL_DisplayFormat(temp_image);
		//SDL_FreeSurface(temp_image);
		mouse[i] = temp_image; // Keep alpha
	}

	last_avg_tick = last_tick = SDL_GetTicks();
	
	memset(touchPointers, 0, sizeof(touchPointers));
	memset(accel, 0, sizeof(accel));
	memset(screenjoy, 0, sizeof(screenjoy));
	memset(gamepads, 0, sizeof(gamepads));

	__android_log_print(ANDROID_LOG_INFO, "Ballfield", "sizeof(int) %d long %d long long %d size_t %d", sizeof(int), sizeof(long), sizeof(long long), sizeof(size_t));
	/*
	wchar_t ss[256];
	const wchar_t *ss2 = L"String 2 ЕНГ ---";
	swprintf(ss, 256, L"String ЙЦУК --- %ls", ss2);
	char ss3[512] = "";
	char ss4[512] = "";
	for(i = 0; i < wcslen(ss); i++)
	{
		char tmp[16];
		sprintf(tmp, "%04X ", (int)ss[i]);
		strcat(ss3, tmp);
		sprintf(ss4, "%ls", ss);
	}
	__android_log_print(ANDROID_LOG_VERBOSE, "Ballfield", "swprintf: len %d data %s: %s", wcslen(ss), ss3, ss4);
	*/

	__android_log_print(ANDROID_LOG_VERBOSE, "Ballfield", "On-screen buttons:");
	for(i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++)
	{
		SDL_Rect r;
		SDL_ANDROID_GetScreenKeyboardButtonPos(i, &r);
		__android_log_print(ANDROID_LOG_VERBOSE, "Ballfield", "{ %d, %d, %d, %d },", r.x, r.y, r.x+r.h, r.y+r.w);
	}
	//SDL_ANDROID_SetScreenKeyboardButtonGenerateTouchEvents(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, 1);
	//SDL_ANDROID_SetScreenKeyboardButtonGenerateTouchEvents(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, 1);

	while(1)
	{
		SDL_Rect r;

		/* Timing */
		tick = SDL_GetTicks();
		dt = (tick - last_tick) * 0.001f;
		last_tick = tick;
		
		if( bpp == 32 )
			SDL_FillRect(screen, NULL, 0); // Clear alpha channel

		/* Background image */
		tiled_back(back, screen, x_offs>>11, y_offs>>11);

		/* Ballfield */
		ballfield_render(balls, screen);

		/* Logo */
		r.x = 2;
		r.y = 2;
		SDL_BlitSurface(logo, NULL, screen, &r);

		/* FPS counter */
		if(tick > fps_start + 1000)
		{
			fps = (float)fps_count * 1000.0 / (tick - fps_start);
			fps_count = 0;
			fps_start = tick;
		}

		print_num(screen, font, screen->w-37, screen->h-12, fps);
		++fps_count;

		for(i=0; i<MAX_POINTERS; i++)
		{
			if( !touchPointers[i].pressed )
				continue;
			r.x = touchPointers[i].x;
			r.y = touchPointers[i].y;
			r.w = 50;// + touchPointers[i].pressure / 5;
			r.h = 50;// + touchPointers[i].pressure / 5;
			r.x -= r.w/2;
			r.y -= r.h/2;
			SDL_FillRect(screen, &r, colors[i]);
		}
		int joyInput[][3] = {	
			{accel[0], accel[1], 10},
			{accel[2], accel[3], 10 + abs(accel[4]) * 100 / 32767},
			{screenjoy[0], screenjoy[1], 10},
			{screenjoy[2], screenjoy[3], 10},
			{gamepads[0][0], gamepads[0][1], 10 + gamepads[0][4] * 100 / 32767},
			{gamepads[0][2], gamepads[0][3], 10 + gamepads[0][5] * 100 / 32767},
			{gamepads[0][6], gamepads[0][7], 10},
			{gamepads[1][0], gamepads[1][1], 10 + gamepads[1][4] * 100 / 32767},
			{gamepads[1][2], gamepads[1][3], 10 + gamepads[1][5] * 100 / 32767},
			{gamepads[1][6], gamepads[1][7], 10},
			{gamepads[2][0], gamepads[2][1], 10 + gamepads[2][4] * 100 / 32767},
			{gamepads[2][2], gamepads[2][3], 10 + gamepads[2][5] * 100 / 32767},
			{gamepads[2][6], gamepads[2][7], 10},
			{gamepads[3][0], gamepads[3][1], 10 + gamepads[3][4] * 100 / 32767},
			{gamepads[3][2], gamepads[3][3], 10 + gamepads[3][5] * 100 / 32767},
			{gamepads[3][6], gamepads[3][7], 10},
		};
		for( i = 0; i < 15; i++ )
		{
			r.w = joyInput[i][2];
			r.h = joyInput[i][2];
			r.x = SCREEN_W/2 + joyInput[i][0] * SCREEN_H / 65536 - r.w/2;
			r.y = SCREEN_H/2 + joyInput[i][1] * SCREEN_H / 65536 - r.w/2;
			//__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Joy input %d: %d %d %d", i, joyInput[i][0], joyInput[i][1], joyInput[i][2] );
			SDL_FillRect(screen, &r, i * 123);
		}

		int mx, my;
		int b = SDL_GetMouseState(&mx, &my);
		//__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Mouse: %04d %04d buttons %d", mx, my, b);
		int cursorIdx = 0;
		if( b & SDL_BUTTON_LMASK )
			cursorIdx |= 1;
		if( b & SDL_BUTTON_RMASK )
			cursorIdx |= 2;
		r.x = mx;
		r.y = my;
		r.w = mouse[cursorIdx]->w;
		r.h = mouse[cursorIdx]->h;
		r.x -= r.w/2;
		r.y -= r.h/2;
		SDL_BlitSurface(mouse[cursorIdx], NULL, screen, &r);

		SDL_Flip(SDL_GetVideoSurface());
		SDL_Event evt;
		while( SDL_PollEvent(&evt) )
		{
			if(evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN)
			{
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL key event: evt %s state %s key %4d %12s scancode %4d mod %2d unicode %d", evt.type == SDL_KEYUP ? "UP  " : "DOWN" , evt.key.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)evt.key.keysym.sym, SDL_GetKeyName(evt.key.keysym.sym), (int)evt.key.keysym.scancode, (int)evt.key.keysym.mod, (int)evt.key.keysym.unicode);
				if(evt.key.keysym.sym == SDLK_ESCAPE)
					return 0;
				if( evt.key.state == SDL_RELEASED )
				{
					if(evt.key.keysym.sym == SDLK_0)
					{
						SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, 1);
						//SDL_ANDROID_SetMouseEmulationMode(0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
					}
					if(evt.key.keysym.sym == SDLK_1)
					{
						SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, 0);
						//SDL_ANDROID_SetMouseEmulationMode(1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
						//*((int*) 0x1000) = 1000; // When you need to test your debugger
					}
					if(evt.key.keysym.sym == SDLK_2)
					{
						__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Async text input started");
						strcpy(asyncTextInputBuf, "async");
						SDL_ANDROID_GetScreenKeyboardTextInputAsync(asyncTextInputBuf, sizeof(asyncTextInputBuf));
					}
					if(evt.key.keysym.sym == SDLK_3)
						SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, 0);
					if(evt.key.keysym.sym == SDLK_4)
						SDL_ToggleScreenKeyboard(NULL);
					if(evt.key.keysym.sym == SDLK_5)
						SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput();
				}
			}
			if(evt.type == SDL_MOUSEBUTTONUP || evt.type == SDL_MOUSEBUTTONDOWN)
			{
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL mouse button event: evt %s state %s button %d coords %d:%d", evt.type == SDL_MOUSEBUTTONUP ? "UP  " : "DOWN" , evt.button.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)evt.button.button, (int)evt.button.x, (int)evt.button.y);
				if (evt.type == SDL_MOUSEBUTTONDOWN && evt.button.button == SDL_BUTTON_WHEELDOWN)
				{
					y_offs += 10000;
				}
				if (evt.type == SDL_MOUSEBUTTONDOWN && evt.button.button == SDL_BUTTON_WHEELUP)
				{
					y_offs -= 10000;
				}
				if (evt.type == SDL_MOUSEBUTTONDOWN && evt.button.button == SDL_BUTTON_X1)
				{
					x_offs += 10000;
				}
				if (evt.type == SDL_MOUSEBUTTONDOWN && evt.button.button == SDL_BUTTON_X2)
				{
					x_offs -= 10000;
				}
			}
			if(evt.type == SDL_VIDEORESIZE)
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL resize event: %d x %d", evt.resize.w, evt.resize.h);
			if(evt.type == SDL_ACTIVEEVENT)
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "======= SDL active event: gain %d state %d", evt.active.gain, evt.active.state);
			// Android-specific events - accelerometer, multitoush, and on-screen joystick
			if( evt.type == SDL_JOYAXISMOTION )
			{
				if(evt.jaxis.which == 0) // Multitouch and on-screen joysticks
				{
					if(evt.jaxis.axis < 4)
						screenjoy[evt.jaxis.axis] = evt.jaxis.value;
					else
						touchPointers[evt.jaxis.axis - 4].pressure = evt.jaxis.value;
				}
				if(evt.jaxis.which == 1)
				{
					accel[evt.jaxis.axis] = evt.jaxis.value; // accelerometer and gyroscope
				}
				if(evt.jaxis.which >= 2)
				{
					// Each gamepad has 8 axes - two joystick hats, two triggers, and Ouya touchpad
					gamepads[evt.jaxis.which - 2][evt.jaxis.axis] = evt.jaxis.value;
				}
			}
			if( evt.type == SDL_JOYBUTTONDOWN || evt.type == SDL_JOYBUTTONUP )
			{
				if(evt.jbutton.which == 0) // Multitouch and on-screen joystick
					touchPointers[evt.jbutton.button].pressed = (evt.jbutton.state == SDL_PRESSED);
			}
			if( evt.type == SDL_JOYBALLMOTION )
			{
				if(evt.jball.which == 0) // Multitouch and on-screen joystick
				{
					touchPointers[evt.jball.ball].x = evt.jball.xrel;
					touchPointers[evt.jball.ball].y = evt.jball.yrel;
				}
			}
		}
		if( screenKeyboardShown != SDL_IsScreenKeyboardShown(NULL) )
		{
			__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Screen keyboard shown: %d -> %d", screenKeyboardShown, SDL_IsScreenKeyboardShown(NULL));
			screenKeyboardShown = SDL_IsScreenKeyboardShown(NULL);
		}
		if( SDL_IsScreenKeyboardShown(NULL) )
		{
			if(SDL_ANDROID_GetScreenKeyboardTextInputAsync(asyncTextInputBuf, sizeof(asyncTextInputBuf)) == SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED)
			{
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Async text input: %s", asyncTextInputBuf);
			}
		}

		/* Animate */
		x_speed = 500.0 * sin(t * 0.37);
		y_speed = 500.0 * sin(t * 0.53);
		z_speed = 400.0 * sin(t * 0.21);
		if( SDL_GetKeyState(NULL)[SDLK_LEFT] || SDL_GetKeyState(NULL)[SDLK_a] )
			x_speed -= 100000 * dt;
		if( SDL_GetKeyState(NULL)[SDLK_RIGHT] || SDL_GetKeyState(NULL)[SDLK_d] )
			x_speed += 100000 * dt;
		if( SDL_GetKeyState(NULL)[SDLK_UP] || SDL_GetKeyState(NULL)[SDLK_w] )
			y_speed -= 100000 * dt;
		if( SDL_GetKeyState(NULL)[SDLK_DOWN] || SDL_GetKeyState(NULL)[SDLK_s] )
			y_speed += 100000 * dt;

		ballfield_move(balls, x_speed, y_speed, z_speed);
		x_offs -= x_speed;
		y_offs -= y_speed;

		t += dt;
	}

	ballfield_free(balls);
	SDL_FreeSurface(back);
	SDL_FreeSurface(logo);
	SDL_FreeSurface(font);
	return 0;
}
