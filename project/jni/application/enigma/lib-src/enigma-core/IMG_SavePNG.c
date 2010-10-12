#include "SDL.h"
#include "IMG_SavePNG.h"
#include "png.h"
#include <stdlib.h>
#include <setjmp.h>

#define IMG_SetError(a) SDL_SetError(a)

/* Save a PNG type image to an SDL datasource */
static void png_write_data(png_structp ctx, png_bytep area, png_size_t size)
{
	SDL_RWops *src;
	
	src = (SDL_RWops *)png_get_io_ptr(ctx);
	SDL_RWwrite(src, area, size, 1);
}

static void png_io_flush(png_structp ctx)
{
	SDL_RWops *src;
	
	src = (SDL_RWops *)png_get_io_ptr(ctx);
	/* how do I flush src? */
}

static int png_colortype_from_surface(SDL_Surface *surface)
{
	int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */
	
	if (surface->format->palette)
		colortype |= PNG_COLOR_MASK_PALETTE;
	else if (surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;
	
	return colortype;
}

static void png_user_warn(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: warning: %s\n", str);
}

static void png_user_error(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: error: %s\n", str);
}



int IMG_SavePNG_RW(SDL_Surface *face, SDL_RWops *src) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const int rmask = 0x00ff0000;
	const int gmask = 0x0000ff00;
	const int bmask = 0x000000ff;
	const int amask = 0x00000000;
#else
    const int rmask = 0x000000ff;
	const int gmask = 0x0000ff00;
	const int bmask = 0x00ff0000;
	const int amask = 0x00000000;
#endif

	int          result  = -1;
	SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, face->w, face->h, 24,
                                                rmask, gmask, bmask, amask);

    if (surface) {
		png_structp png_ptr;

        SDL_BlitSurface(face, NULL, surface, NULL);
        SDL_LockSurface(surface);

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_user_error, png_user_warn);
        if (!png_ptr) {
            IMG_SetError("Couldn't allocate memory for PNG file");
        }
        else {
            /* Allocate/initialize the image information data.  REQUIRED */
            png_infop info_ptr = png_create_info_struct(png_ptr);
            if (!info_ptr) {
                IMG_SetError("Couldn't create image information for PNG file");
            }
            else {
                png_bytep *row_pointers = 0;

                /* Set error handling. */
                if (setjmp(png_ptr->jmpbuf)) {
                    IMG_SetError("Error writing the PNG file");
                }
                else {
					int colortype;
                    png_set_write_fn(png_ptr, src, png_write_data, png_io_flush);
                    /* Set the image information here.  Width and height are up to 2^31,
                     * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
                     * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
                     * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
                     * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
                     * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
                     * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
                     */
                    colortype = png_colortype_from_surface(surface);
                    png_set_IHDR(png_ptr, info_ptr, surface->w, surface->h, 8,
                                 colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
                    /* Write the file header information.  REQUIRED */
                    png_write_info(png_ptr, info_ptr);
	
                    /* pack pixels into bytes */
                    png_set_packing(png_ptr);

                    /* Create the array of pointers to image data */
                    row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*surface->h);
                    if (!row_pointers) {
                        IMG_SetError("Couldn't allocate PNG row pointers");
                    }
                    else {
                        int i;
                        for (i = 0; i < surface->h; i++)
                            row_pointers[i] = (png_bytep)(Uint8 *)surface->pixels + i*surface->pitch;

                        /* write out the entire image data in one call */
                        png_write_image(png_ptr, row_pointers);
                        png_write_end(png_ptr, info_ptr);
                        result = 0; /* success! */
                    }
                }
                if (row_pointers)
                    free(row_pointers);
                
                png_destroy_info_struct(png_ptr, &info_ptr);
            }
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        }

        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
    }

    return result;
}

int IMG_SavePNG(SDL_Surface *surface, const char *file)
{
    SDL_RWops *out = SDL_RWFromFile(file, "wb");
    int ret;
    if(!out)
		return -1;
    ret = IMG_SavePNG_RW(surface, out);
    SDL_RWclose(out);
    return ret;
}
