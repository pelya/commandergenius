
/* 

 SDL_gfxPrimitives: graphics primitives for SDL

 LGPL (c) A. Schiffler
 
*/

#ifndef _SDL_gfxPrimitives_h
#define _SDL_gfxPrimitives_h

#include <math.h>
#ifndef M_PI
#define M_PI	3.141592654
#endif

#include "SDL.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* ----- Versioning */

#define SDL_GFXPRIMITIVES_MAJOR	1
#define SDL_GFXPRIMITIVES_MINOR	5

/* ----- Prototypes */

/* Note: all ___Color routines expect the color to be in format 0xRRGGBBAA */

/* Pixel */

    int pixelColor(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color);
    int pixelRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Horizontal line */

    int hlineColor(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
    int hlineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Vertical line */

    int vlineColor(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);
    int vlineRGBA(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Rectangle */

    int rectangleColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
    int rectangleRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
				   Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled rectangle (Box) */

    int boxColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
    int boxRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2,
			     Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Line */

    int lineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
    int lineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
			      Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA Line */
    int aalineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
    int aalineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
				Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Circle */

    int circleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
    int circleRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA Circle */

    int aacircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
    int aacircleRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
				  Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Circle */

    int filledCircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
    int filledCircleRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
				      Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Ellipse */

    int ellipseColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
    int ellipseRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
				 Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA Ellipse */

    int aaellipseColor(SDL_Surface * dst, Sint16 xc, Sint16 yc, Sint16 rx, Sint16 ry, Uint32 color);
    int aaellipseRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
				   Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Ellipse */

    int filledEllipseColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
    int filledEllipseRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
				       Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
/* Filled Pie */

    int filledpieColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad,
				    Sint16 start, Sint16 end, Uint32 color);
    int filledpieRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad,
				   Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Polygon */

    int polygonColor(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint32 color);
    int polygonRGBA(SDL_Surface * dst, Sint16 * vx, Sint16 * vy,
				 int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA-Polygon */

    int aapolygonColor(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint32 color);
    int aapolygonRGBA(SDL_Surface * dst, Sint16 * vx, Sint16 * vy,
				   int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Polygon */

    int filledPolygonColor(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, int color);
    int filledPolygonRGBA(SDL_Surface * dst, Sint16 * vx,
				       Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* 8x8 Characters/Strings */

    int characterColor(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint32 color);
    int characterRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    int stringColor(SDL_Surface * dst, Sint16 x, Sint16 y, char *c, Uint32 color);
    int stringRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, char *c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
};
#endif

#endif				/* _SDL_gfxPrimitives_h */
