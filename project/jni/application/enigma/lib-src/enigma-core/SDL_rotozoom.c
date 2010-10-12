/*

  SDL_rotozoom.c - rotozoomer for 32bit or 8bit surfaces

  LGPL (c) A. Schiffler

  - Better 
*/

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SDL_rotozoom.h"

#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))

#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif

INLINE int clamp (int x, int xmin, int xmax)
{
    if (x < xmin)
        return xmin;
    else if (x > xmax)
        return xmax;
    return x;
}

INLINE double sinc(double x)
{
    if (x == 0)
        return 1;
    x *= M_PI;
    return sin(x)/x;
}

INLINE float lanczos2(float x)
{
/*     return sinc(x); */
  if (x < -2 || x > 2)
    return 0;
  else
    return sinc(x) * sinc(x/2);
/* sinc_table [(int)((x+4)*100. )] * sinc_table[(int)((x/2+4)*100.)]; */
}

INLINE float lanczos3(float x)
{
/*     return sinc(x); */
  if (x < -3 || x > 3)
    return 0;
  else
    return sinc(x) * sinc(x/3);
}

/* inline float lanczos4(float x) */
/* { */
/* /\*     return sinc(x); *\/ */
/*   if (x < -4 || x > 4) */
/*     return 0; */
/*   else */
/*     return sinc_table [(int)((x+4)*100. )] * sinc_table[(int)((x/4+4)*100.)]; */
/* } */

/* static void */
/* resample (Uint8 *s, int slen, int spitch, */
/*           Uint8 *d, int dlen, int dpitch) */
/* { */
/*     float f = (float) slen / dlen; */

/*     int i,j; */
/*     Uint8 *sp; */
/*     float r, g, b, a; */
/*     int support = 2; */

/*     for (i=0; i<dlen; ++i) { */
/*         float x = i*f; */
/*         float c = 0; */
/*         float dx = MAX(f, 1.0); */
/*         float center = (i+0.5)*f; /\* kernel center in the source image *\/ */
/*         int start = (int) MAX(0, center-support + 0.5); */
/*         int stop = (int) MIN(slen, center+support+0.5); */
/*         int n = stop-start; */
/*         r = g = b = a = 0; */
/*         sp = s + start*spitch; */
/*         for (j=0; j<n; ++j) { */
/*             float ff = lanczos2((j+start-center+0.5)/dx); */
/*             if (ff != 0) { */
/*                 r += ff * sp[0]; */
/*                 g += ff * sp[1]; */
/*                 b += ff * sp[2]; */
/*                 a += ff * sp[3]; */
/*                 c += ff; */
/*             } */
/*             sp += spitch; */
/*         } */
/*         r /= c; */
/*         g /= c; */
/*         b /= c; */
/*         a /= c; */
/*         d[0] = (Uint8) clamp (r, 0, 255);  */
/*         d[1] = (Uint8) clamp (g, 0, 255);  */
/*         d[2] = (Uint8) clamp (b, 0, 255);  */
/*         d[3] = (Uint8) clamp (a, 0, 255);  */
/*         d += dpitch; */
/*     } */
/* } */

typedef float (*FilterFunc)(float);

static void
resample_horiz (SDL_Surface *src, SDL_Surface *dst, FilterFunc filter, int support)
{
    int    x, y;
    int    i,j;
    Uint8 *sp, *dp;
    int    r, g, b, a;
    float  factor  = (float) src->w / dst->w;
    float  dx      = MAX(factor, 1.0);
    float *F       = (float*) malloc(sizeof(float)*(2*support+1));

    for (x=0; x<dst->w; ++x) 
    {
        float c      = 0;
        float center = (x+0.5)*factor; /* kernel center in the source image */
        int   start  = (int) MAX(0, center-support + 0.5);
        int   stop   = (int) MIN(src->w, center+support+0.5);
        int   n      = stop-start;

        for (j=0; j<n; ++j) {
            F[j] = filter((j+start-center+0.5)/dx);
            c += F[j];
        }
        if (c != 0 && c!=1)
            for (j=0; j<n; ++j)
                F[j] /= c;
        dp = (Uint8*)src->pixels + x*4;
        for (y=0; y<src->h; ++y) {
            sp = (Uint8*)src->pixels + y*src->pitch + start*4;
            r = g = b = a = 0;
            for (j=0; j<n; ++j) {
                float ff = F[j];
                r += ff * sp[0];
                g += ff * sp[1];
                b += ff * sp[2];
                a += ff * sp[3];
                sp += 4;
            }
            dp[0] = (Uint8) clamp (r, 0, 255); 
            dp[1] = (Uint8) clamp (g, 0, 255); 
            dp[2] = (Uint8) clamp (b, 0, 255); 
            dp[3] = (Uint8) clamp (a, 0, 255); 
            dp += src->pitch;
        }
    }
    free(F);
}

static void
resample_vert (SDL_Surface *src, SDL_Surface *dst, FilterFunc filter, int support)
{
    int    x, y;
    int    i,j;
    Uint8 *sp, *dp;
    int    r, g, b, a;
    float  factor  = (float) src->h / dst->h;
    float  dx      = MAX(factor, 1.0);
    float *F       = (float*) malloc(sizeof(float)*(2*support+1));

    for (y=0; y<dst->h; ++y)
    {
        float c      = 0;
        float center = (y+0.5)*factor; /* kernel center in the source image */
        int   start  = (int) MAX(0, center-support + 0.5);
        int   stop   = (int) MIN(src->h, center+support+0.5);
        int   n      = stop-start;

        for (j=0; j<n; ++j) {
            F[j] = filter((j+start-center+0.5)/dx);
            c += F[j];
        }
        if (c != 0 && c!=1)
            for (j=0; j<n; ++j)
                F[j] /= c;
        dp = (Uint8*)dst->pixels + y*dst->pitch;
        for (x=0; x<dst->w; ++x) 
        {
            sp = (Uint8*)src->pixels + start*src->pitch + x*4;
            r = g = b = a = 0;
            for (j=0; j<n; ++j) {
                float ff = F[j];
                r += ff * sp[0];
                g += ff * sp[1];
                b += ff * sp[2];
                a += ff * sp[3];
                sp += src->pitch;
            }
            dp[0] = (Uint8) clamp (r, 0, 255); 
            dp[1] = (Uint8) clamp (g, 0, 255); 
            dp[2] = (Uint8) clamp (b, 0, 255); 
            dp[3] = (Uint8) clamp (a, 0, 255); 
            dp += 4;
/*            dp += src->pitch; */
        }
    }
    free(F);
}

/* FIXME: Can currently only shrink images!!!! */

static void zoomLanczos (SDL_Surface *src, SDL_Surface *dst)
{
#if 1
    resample_horiz(src, dst, lanczos2, 2);
    resample_vert(src, dst, lanczos2, 2);
#else
    resample_horiz(src, dst, lanczos3, 3);
    resample_vert(src, dst, lanczos3, 3);
#endif
}

/*

 32bit Zoomer with optional anti-aliasing by bilinear interpolation.

 Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.

*/

int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int smooth)
{
    if (!smooth) {
        int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey;
        tColorRGBA *sp, *csp, *dp;
        int sgap, dgap;
    	sx = (int) (65536.0 * (float) src->w / (float) dst->w);
	    sy = (int) (65536.0 * (float) src->h / (float) dst->h);

        /*
         * Allocate memory for row increments
         */
        if ((sax = (int *) malloc((dst->w + smooth) * sizeof(Uint32))) == NULL) {
            return (-1);
        }
        if ((say = (int *) malloc((dst->h + smooth) * sizeof(Uint32))) == NULL) {
            free(sax);
            return (-1);
        }

        /*
         * Precalculate row increments
         */
        csx = 0;
        csax = sax;
        for (x = 0; x < dst->w + smooth; x++) {
            csax[x] = csx;
            csx &= 0xffff;
            csx += sx;
            if ((csx >> 16) >= src->w)
                csx -= sx;
        }
        csy = 0;
        csay = say;
        for (y = 0; y < dst->h + smooth; y++) {
            csay[y] = csy;
            csy &= 0xffff;
            csy += sy;
            if ((csy >> 16) >= src->h)
                csy -= sy;
        }

        /*
         * Pointer setup
         */
        sp = csp = (tColorRGBA *) src->pixels;
        dp = (tColorRGBA *) dst->pixels;
        sgap = src->pitch - src->w * 4;
        dgap = dst->pitch - dst->w * 4;
    
        /*
         * Switch between interpolating and non-interpolating code
         */
        csay = say;
	    for (y = 0; y < dst->h; y++) {
	        sp = csp;
	        csax = sax;
	        for (x = 0; x < dst->w; x++, dp++) {
		        *dp = *sp;
		        csax++;
		        sp += (*csax >> 16);
	        }
	        csay++;
	        csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
	        dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
	    }

        free(sax);
        free(say);
        return (0);
    }
    else if (dst->w < src->w && dst->h <=src->h) {
        zoomLanczos (src, dst);
    }
    else
    {
        int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep;
        tColorRGBA *sp, *csp, *dp;
        int sgap, dgap;

        /*
         * Variable setup
         */
        if (smooth) {
            /*
             * For interpolation: assume source dimension is `smooth'
             * pixels smaller to avoid overflow on right and bottom edge.
             */
            smooth = 1;
            if (src->w > dst->w) // if the image gets shrinked -> use stronger blur :
                smooth = (int)((double)(src->w)/dst->w+0.5);

            sx = (int) (65536.0 * (float) (src->w) / (float) dst->w);
            sy = (int) (65536.0 * (float) (src->h) / (float) dst->h);
        } else {
            sx = (int) (65536.0 * (float) src->w / (float) dst->w);
            sy = (int) (65536.0 * (float) src->h / (float) dst->h);
        }

        /*
         * Allocate memory for row increments
         */
        if ((sax = (int *) malloc((dst->w + smooth) * sizeof(Uint32))) == NULL) {
            return (-1);
        }
        if ((say = (int *) malloc((dst->h + smooth) * sizeof(Uint32))) == NULL) {
            free(sax);
            return (-1);
        }

        /*
         * Precalculate row increments
         */
        csx = 0;
        csax = sax;
        int sum = 0;
        for (x = 0; x < dst->w + smooth; x++) {
            csax[x] = csx;
            csx &= 0xffff;
            csx += sx;
            sum += sx;
            if ((sum >> 16) >= src->w - smooth)
                csx -= sx;
        }
        csy = 0;
        csay = say;
        sum = 0;
        for (y = 0; y < dst->h + smooth; y++) {
            csay[y] = csy;
            csy &= 0xffff;
            csy += sy;
            sum += sy;
            if ((sum >> 16) >= src->h - smooth)
                csy -= sy;
        }

        /*
         * Pointer setup
         */
        sp = csp = (tColorRGBA *) src->pixels;
        dp = (tColorRGBA *) dst->pixels;
        sgap = src->pitch - src->w * 4;
        dgap = dst->pitch - dst->w * 4;
    
        /*
         * Switch between interpolating and non-interpolating code
         */
        csay = say;
        if (smooth == 1) {
            /* Interpolating Zoom */
            for (y = 0; y < dst->h; y++) {
                /* Setup color source pointers */
                tColorRGBA *c00, *c01, *c10, *c11;

                c00  = csp;
                c01  = csp+1;
                c10  = (tColorRGBA *) ((Uint8 *) csp + src->pitch);
                c11  = c10+1;
                csax = sax;
                for (x = 0; x < dst->w; x++, dp++) {

                    /* Interpolate colors */
                    ex = *csax & 0xffff;
                    ey = *csay & 0xffff;
                    t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
                    t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
                    dp->r = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
                    t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
                    dp->g = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
                    t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
                    dp->b = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
                    t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
                    dp->a = (((t2 - t1) * ey) >> 16) + t1;

                    /* Advance source pointers */
                    csax++;
                    sstep = (*csax >> 16);
                    c00 += sstep;
                    c01 += sstep;
                    c10 += sstep;
                    c11 += sstep;
                }
                csay++;
                csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
                dp  = (tColorRGBA *) ((Uint8 *) dp + dgap);
            }
        }
        else if (smooth > 1) { 
            // stronger blur for image shrinking
            for (y = 0; y < dst->h; y++) {
                tColorRGBA *c       = csp;
                int         smooth2 = smooth*smooth;
                csax                = sax;

                for (x = 0; x < dst->w; x++, dp++) {
                    int         xo, yo;
                    tColorRGBA *co    = c;
                    int         r     = 0, g = 0, b = 0, a = 0;

                    for (yo = 0; yo<smooth; ++yo) {
                        for (xo = 0; xo<smooth; ++xo) {
                            assert(co >= csp);
                            r += co->r;
                            g += co->g;
                            b += co->b;
                            a += co->a;
                            co++;
                        }
                        co  = (tColorRGBA *) ((Uint8 *) co + src->pitch);
                        co -= smooth;
                    }

                    dp->r = r/smooth2;
                    dp->g = g/smooth2;
                    dp->b = b/smooth2;
                    dp->a = a/smooth2;

                    csax++;
                    sstep  = (*csax >> 16);
                    c   += sstep;
                }
                csay++;
                csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
                dp  = (tColorRGBA *) ((Uint8 *) dp + dgap);
            }
        }
        else {                      /* Non-Interpolating Zoom */
            for (y = 0; y < dst->h; y++) {
                sp = csp;
                csax = sax;
                for (x = 0; x < dst->w; x++, dp++) {
                    *dp = *sp;
                    csax++;
                    sp += (*csax >> 16);
                }
                csay++;
                csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
                dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
            }

        }

        free(sax);
        free(say);
        return (0);
    }
    return 0;
}

/*

 8bit Zoomer without smoothing.

 Zoomes 8bit palette/Y 'src' surface to 'dst' surface.

*/

int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst)
{
    Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
    Uint8 *sp, *dp, *csp;
    int dgap;

    /*
     * Variable setup
     */
    sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
    sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);

    /*
     * Allocate memory for row increments
     */
    if ((sax = (Uint32 *) malloc(dst->w * sizeof(Uint32))) == NULL) {
    	return (-1);
    }
    if ((say = (Uint32 *) malloc(dst->h * sizeof(Uint32))) == NULL) {
	    if (sax != NULL) {
	        free(sax);
	    }
	    return (-1);
    }

    /*
     * Precalculate row increments
     */
    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
	    csx += sx;
	    *csax = (csx >> 16);
	    csx &= 0xffff;
	    csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
	    csy += sy;
	    *csay = (csy >> 16);
	    csy &= 0xffff;
	    csay++;
    }

    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
	    csx += (*csax);
	    csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
	    csy += (*csay);
	    csay++;
    }

    /*
     * Pointer setup
     */
    sp = csp = (Uint8 *) src->pixels;
    dp = (Uint8 *) dst->pixels;
    dgap = dst->pitch - dst->w;

    /*
     * Draw
     */
    csay = say;
    for (y = 0; y < dst->h; y++) {
	    csax = sax;
	    sp = csp;
	    for (x = 0; x < dst->w; x++) {
	        /*
	        * Draw
	        */
	        *dp = *sp;
	        /*
	        * Advance source pointers
	        */
	        sp += (*csax);
	        csax++;
	        /*
	        * Advance destination pointer
	        */
	        dp++;
	    }
	    /*
	    * Advance source pointer (for row)
	    */
	    csp += ((*csay) * src->pitch);
	    csay++;
	    /*
	    * Advance destination pointers
	    */
	    dp += dgap;
    }

    /*
     * Remove temp arrays
     */
    free(sax);
    free(say);

    return (0);
}

/*

 32bit Rotozoomer with optional anti-aliasing by bilinear interpolation.

 Rotates and zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.

*/

void transformSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos, int smooth)
{
    int x, y, t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
    tColorRGBA c00, c01, c10, c11;
    tColorRGBA *pc, *sp, *spb;
    int gap;

    /*
     * Variable setup
     */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = dst->pixels;
    gap = dst->pitch - dst->w * 4;

    /*
     * Switch between interpolating and non-interpolating code
     */
    if (smooth) {
	for (y = 0; y < dst->h; y++) {
	    dy = cy - y;
	    sdx = (ax + (isin * dy)) + xd;
	    sdy = (ay - (icos * dy)) + yd;
	    for (x = 0; x < dst->w; x++) {
		dx = (sdx >> 16);
		dy = (sdy >> 16);
		if ((dx >= -1) && (dy >= -1) && (dx < src->w) && (dy < src->h)) {
		    if ((dx >= 0) && (dy >= 0) && (dx < sw) && (dy < sh)) {
			sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			sp += 1;
			c01 = *sp;
			sp = (tColorRGBA *) ((Uint8 *) sp + src->pitch);
			sp -= 1;
			c10 = *sp;
			sp += 1;
			c11 = *sp;
		    } else if ((dx == sw) && (dy == sh)) {
			sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if ((dx == -1) && (dy == -1)) {
			sp = (tColorRGBA *) (src->pixels);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if ((dx == -1) && (dy == sh)) {
			sp = (tColorRGBA *) (src->pixels);
			sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if ((dx == sw) && (dy == -1)) {
			sp = (tColorRGBA *) (src->pixels);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    } else if (dx == -1) {
			sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			sp = (tColorRGBA *) ((Uint8 *) sp + src->pitch);
			c11 = *sp;
		    } else if (dy == -1) {
			sp = (tColorRGBA *) (src->pixels);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			c10 = *sp;
			sp += 1;
			c11 = *sp;
		    } else if (dx == sw) {
			sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			c01 = *sp;
			sp = (tColorRGBA *) ((Uint8 *) sp + src->pitch);
			c10 = *sp;
			c11 = *sp;
		    } else if (dy == sh) {
			sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
			sp += dx;
			c00 = *sp;
			sp += 1;
			c01 = *sp;
			c10 = *sp;
			c11 = *sp;
		    }
		    /*
		     * Interpolate colors
		     */
		    ex = (sdx & 0xffff);
		    ey = (sdy & 0xffff);
		    t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
		    t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
		    pc->r = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
		    t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
		    pc->g = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
		    t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
		    pc->b = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
		    t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
		    pc->a = (((t2 - t1) * ey) >> 16) + t1;
		}
		sdx += icos;
		sdy += isin;
		pc++;
	    }
	    pc = (tColorRGBA *) ((Uint8 *) pc + gap);
	}
    } else {
	for (y = 0; y < dst->h; y++) {
	    dy = cy - y;
	    sdx = (ax + (isin * dy)) + xd;
	    sdy = (ay - (icos * dy)) + yd;
	    for (x = 0; x < dst->w; x++) {
		dx = (short) (sdx >> 16);
		dy = (short) (sdy >> 16);
		if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
		    sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy);
		    sp += dx;
		    *pc = *sp;
		}
		sdx += icos;
		sdy += isin;
		pc++;
	    }
	    pc = (tColorRGBA *) ((Uint8 *) pc + gap);
	}
    }
}

/*

 8bit Rotozoomer without smoothing

 Rotates and zoomes 8bit palette/Y 'src' surface to 'dst' surface.

*/

void transformSurfaceY(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos)
{
    int x, y, dx, dy, xd, yd, sdx, sdy, ax, ay, sw, sh;
    tColorY *pc, *sp;
    int gap;

    /*
     * Variable setup
     */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = dst->pixels;
    gap = dst->pitch - dst->w;
    /*
     * Clear surface to colorkey
     */
    memset(pc, (unsigned char) (src->format->colorkey & 0xff), dst->pitch * dst->h);
    /*
     * Iterate through destination surface
     */
    for (y = 0; y < dst->h; y++) {
	dy = cy - y;
	sdx = (ax + (isin * dy)) + xd;
	sdy = (ay - (icos * dy)) + yd;
	for (x = 0; x < dst->w; x++) {
	    dx = (short) (sdx >> 16);
	    dy = (short) (sdy >> 16);
	    if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
		sp = (tColorY *) (src->pixels);
		sp += (src->pitch * dy + dx);
		*pc = *sp;
	    }
	    sdx += icos;
	    sdy += isin;
	    pc++;
	}
	pc += gap;
    }
}

/*

 rotozoomSurface()

 Rotates and zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
 'angle' is the rotation in degrees. 'zoom' a scaling factor. If 'smooth' is 1
 then the destination 32bit surface is anti-aliased. If the surface is not 8bit
 or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

*/

#define VALUE_LIMIT	0.001


/* Local rotozoom-size function with trig result return */

void rotozoomSurfaceSizeTrig(int width, int height, double angle, double zoom, int *dstwidth, int *dstheight,
			     double *canglezoom, double *sanglezoom)
{
    double x, y, cx, cy, sx, sy;
    double radangle;
    int dstwidthhalf, dstheighthalf;

    /*
     * Determine destination width and height by rotating a centered source box
     */
    radangle = angle * (M_PI / 180.0);
    *sanglezoom = sin(radangle);
    *canglezoom = cos(radangle);
    *sanglezoom *= zoom;
    *canglezoom *= zoom;
    x = width / 2;
    y = height / 2;
    cx = *canglezoom * x;
    cy = *canglezoom * y;
    sx = *sanglezoom * x;
    sy = *sanglezoom * y;
    dstwidthhalf = MAX((int)
		       ceil(MAX(MAX(MAX(fabs(cx + sy), fabs(cx - sy)), fabs(-cx + sy)), fabs(-cx - sy))), 1);
    dstheighthalf = MAX((int)
			ceil(MAX(MAX(MAX(fabs(sx + cy), fabs(sx - cy)), fabs(-sx + cy)), fabs(-sx - cy))), 1);
    *dstwidth = 2 * dstwidthhalf;
    *dstheight = 2 * dstheighthalf;
}


/* Publically available rotozoom-size function */

void rotozoomSurfaceSize(int width, int height, double angle, double zoom, int *dstwidth, int *dstheight)
{
    double dummy_sanglezoom, dummy_canglezoom;

    rotozoomSurfaceSizeTrig(width, height, angle, zoom, dstwidth, dstheight, &dummy_sanglezoom, &dummy_canglezoom);
}


/* Publically available rotozoom function */

SDL_Surface *rotozoomSurface(SDL_Surface * src, double angle, double zoom, int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    double zoominv;
    double sanglezoom, canglezoom, sanglezoominv, canglezoominv;
    int dstwidthhalf, dstwidth, dstheighthalf, dstheight;
    double x, y, cx, cy;
    int is32bit;
    int i, src_converted;

    /*
     * Sanity check
     */
    if (src == NULL)
	return (NULL);

    /*
     * Determine if source surface is 32bit or 8bit
     */
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
	/*
	 * Use source surface 'as is'
	 */
	rz_src = src;
	src_converted = 0;
    } else {
	/*
	 * New source surface is 32bit with a defined RGBA ordering
	 */
	rz_src =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_BlitSurface(src, NULL, rz_src, NULL);
	src_converted = 1;
	is32bit = 1;
    }

    /*
     * Sanity check zoom factor
     */
    if (zoom < VALUE_LIMIT) {
	zoom = VALUE_LIMIT;
    }
    zoominv = 65536.0 / (zoom * zoom);

    /*
     * Check if we have a rotozoom or just a zoom
     */
    if (fabs(angle) > VALUE_LIMIT) {

	/*
	 * Angle!=0: full rotozoom
	 */
	/*
	 * -----------------------
	 */

	/* Determine target size */
	rotozoomSurfaceSizeTrig(rz_src->w, rz_src->h, angle, zoom, &dstwidth, &dstheight, &canglezoom, &sanglezoom);

	/*
	 * Calculate target factors from sin/cos and zoom
	 */
	sanglezoominv = sanglezoom;
	canglezoominv = canglezoom;
	sanglezoominv *= zoominv;
	canglezoominv *= zoominv;

	/* Calculate half size */
	dstwidthhalf = dstwidth / 2;
	dstheighthalf = dstheight / 2;

	/*
	 * Alloc space to completely contain the rotated surface
	 */
	rz_dst = NULL;
	if (is32bit) {
	    /*
	     * Target surface is 32bit with source RGBA/ABGR ordering
	     */
	    rz_dst =
		SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
				     rz_src->format->Rmask, rz_src->format->Gmask,
				     rz_src->format->Bmask, rz_src->format->Amask);
	} else {
	    /*
	     * Target surface is 8bit
	     */
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
	}

	/*
	 * Lock source surface
	 */
	SDL_LockSurface(rz_src);
	/*
	 * Check which kind of surface we have
	 */
	if (is32bit) {
	    /*
	     * Call the 32bit transformation routine to do the rotation (using alpha)
	     */
	    transformSurfaceRGBA(rz_src, rz_dst, dstwidthhalf, dstheighthalf,
				 (int) (sanglezoominv), (int) (canglezoominv), smooth);
	    /*
	     * Turn on source-alpha support
	     */
	    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
	} else {
	    /*
	     * Copy palette and colorkey info
	     */
	    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
		rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	    }
	    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	    /*
	     * Call the 8bit transformation routine to do the rotation
	     */
	    transformSurfaceY(rz_src, rz_dst, dstwidthhalf, dstheighthalf,
			      (int) (sanglezoominv), (int) (canglezoominv));
	    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
	}
	/*
	 * Unlock source surface
	 */
	SDL_UnlockSurface(rz_src);

    } else {

	/*
	 * Angle=0: Just a zoom
	 */
	/*
	 * --------------------
	 */

	/*
	 * Calculate target size
	 */
	zoomSurfaceSize(rz_src->w, rz_src->h, zoom, zoom, &dstwidth, &dstheight);

	/*
	 * Alloc space to completely contain the zoomed surface
	 */
	rz_dst = NULL;
	if (is32bit) {
	    /*
	     * Target surface is 32bit with source RGBA/ABGR ordering
	     */
	    rz_dst =
		SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
				     rz_src->format->Rmask, rz_src->format->Gmask,
				     rz_src->format->Bmask, rz_src->format->Amask);
	} else {
	    /*
	     * Target surface is 8bit
	     */
	    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
	}

	/*
	 * Lock source surface
	 */
	SDL_LockSurface(rz_src);
	/*
	 * Check which kind of surface we have
	 */
	if (is32bit) {
	    /*
	     * Call the 32bit transformation routine to do the zooming (using alpha)
	     */
	    zoomSurfaceRGBA(rz_src, rz_dst, smooth);
	    /*
	     * Turn on source-alpha support
	     */
	    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
	} else {
	    /*
	     * Copy palette and colorkey info
	     */
	    for (i = 0; i < rz_src->format->palette->ncolors; i++) {
		rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	    }
	    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	    /*
	     * Call the 8bit transformation routine to do the zooming
	     */
	    zoomSurfaceY(rz_src, rz_dst);
	    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
	}
	/*
	 * Unlock source surface
	 */
	SDL_UnlockSurface(rz_src);
    }

    /*
     * Cleanup temp surface
     */
    if (src_converted) {
	SDL_FreeSurface(rz_src);
    }

    /*
     * Return destination surface
     */
    return (rz_dst);
}

/*

 zoomSurface()

 Zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
 'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is 1
 then the destination 32bit surface is anti-aliased. If the surface is not 8bit
 or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

*/

#define VALUE_LIMIT	0.001

void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
{
    /*
     * Sanity check zoom factors
     */
    if (zoomx < VALUE_LIMIT) {
	zoomx = VALUE_LIMIT;
    }
    if (zoomy < VALUE_LIMIT) {
	zoomy = VALUE_LIMIT;
    }

    /*
     * Calculate target size
     */
    *dstwidth = (int) ((double) width * zoomx);
    *dstheight = (int) ((double) height * zoomy);
    if (*dstwidth < 1) {
	*dstwidth = 1;
    }
    if (*dstheight < 1) {
	*dstheight = 1;
    }
}

SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth)
{
    SDL_Surface *rz_src;
    SDL_Surface *rz_dst;
    int dstwidth, dstheight;
    int is32bit;
    int i, src_converted;

    /*
     * Sanity check
     */
    if (src == NULL)
	return (NULL);

    /*
     * Determine if source surface is 32bit or 8bit
     */
    is32bit = (src->format->BitsPerPixel == 32);
    if ((is32bit) || (src->format->BitsPerPixel == 8)) {
	/*
	 * Use source surface 'as is'
	 */
	rz_src = src;
	src_converted = 0;
    } else {
	/*
	 * New source surface is 32bit with a defined RGBA ordering
	 */
	rz_src =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_BlitSurface(src, NULL, rz_src, NULL);
	src_converted = 1;
	is32bit = 1;
    }

    /* Get size if target */
    zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);

    /*
     * Alloc space to completely contain the zoomed surface
     */
    rz_dst = NULL;
    if (is32bit) {
	/*
	 * Target surface is 32bit with source RGBA/ABGR ordering
	 */
	rz_dst =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
				 rz_src->format->Rmask, rz_src->format->Gmask,
				 rz_src->format->Bmask, rz_src->format->Amask);
    } else {
	/*
	 * Target surface is 8bit
	 */
	rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
    }

    /*
     * Lock source surface
     */
    SDL_LockSurface(rz_src);
    /*
     * Check which kind of surface we have
     */
    if (is32bit) {
	/*
	 * Call the 32bit transformation routine to do the zooming (using alpha)
	 */
	zoomSurfaceRGBA(rz_src, rz_dst, smooth);
	/*
	 * Turn on source-alpha support
	 */
	SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
    } else {
	/*
	 * Copy palette and colorkey info
	 */
	for (i = 0; i < rz_src->format->palette->ncolors; i++) {
	    rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
	}
	rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
	/*
	 * Call the 8bit transformation routine to do the zooming
	 */
	zoomSurfaceY(rz_src, rz_dst);
	SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey);
    }
    /*
     * Unlock source surface
     */
    SDL_UnlockSurface(rz_src);

    /*
     * Cleanup temp surface
     */
    if (src_converted) {
	SDL_FreeSurface(rz_src);
    }

    /*
     * Return destination surface
     */
    return (rz_dst);
}
