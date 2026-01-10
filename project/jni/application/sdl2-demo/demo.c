// SDL2 Demo by aurelien.esnard@u-bordeaux.fr

#include <SDL.h>
#include <SDL_image.h>  // required to load transparent texture from PNG
#include <SDL_ttf.h>    // required to use TTF fonts

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "model.h"

/* **************************************************************** */

#define FONT "arial.ttf"
#define FONTSIZE 36
#define BACKGROUND "background.png"
#define MARIO "mario.png"
#define BOMB "bomb.png"

/* **************************************************************** */

/* PUT YOUR VARIABLES HERE */
struct Env_t {
  SDL_Texture* background;
  SDL_Texture* mario;
  SDL_Texture* bomb;
  SDL_Texture* text;
  int bomb_x, bomb_y;
  int mario_x, mario_y;
};

/* **************************************************************** */

/* PUT YOUR CODE HERE TO INIT TEXTURES, ... */
Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[]) {
  Env* env = malloc(sizeof(struct Env_t));

  PRINT("Move the bomb with keyboard arrows and move Mario with mouse.\n");
  PRINT("Press ESC to quit. Enjoy this SDL2 sample!\n");

  /* get current window size */
  int w, h;
  SDL_GetWindowSize(win, &w, &h);

  /* init positions */
  env->bomb_x = w / 2;
  env->bomb_y = h / 4;
  env->mario_x = w / 2;
  env->mario_y = 3 * h / 4;

  /* init background texture from PNG image */
  env->background = IMG_LoadTexture(ren, BACKGROUND);
  if (!env->background) ERROR("IMG_LoadTexture: %s\n", BACKGROUND);

  /* init mario texture from PNG image */
  env->mario = IMG_LoadTexture(ren, MARIO);
  if (!env->mario) ERROR("IMG_LoadTexture: %s\n", MARIO);

  /* init bomb texture from PNG image */
  env->bomb = IMG_LoadTexture(ren, BOMB);
  if (!env->bomb) ERROR("IMG_LoadTexture: %s\n", BOMB);

  /* init text texture using Arial font */
  SDL_Color color = {0, 0, 255, 255}; /* blue color in RGBA */
  TTF_Font* font = TTF_OpenFont(FONT, FONTSIZE);
  if (!font) ERROR("TTF_OpenFont: %s\n", FONT);
  TTF_SetFontStyle(font, TTF_STYLE_BOLD);                                   // TTF_STYLE_ITALIC | TTF_STYLE_NORMAL
  SDL_Surface* surf = TTF_RenderText_Blended(font, "Hello World!", color);  // blended rendering for ultra nice text
  env->text = SDL_CreateTextureFromSurface(ren, surf);
  SDL_FreeSurface(surf);
  TTF_CloseFont(font);

  return env;
}

/* **************************************************************** */

/* PUT YOUR CODE HERE TO RENDER TEXTURES, ... */
void render(SDL_Window* win, SDL_Renderer* ren, Env* env) {
  SDL_Rect rect;

  /* get current window size */
  int w, h;
  SDL_GetWindowSize(win, &w, &h);

  /* render background texture */
  SDL_RenderCopy(ren, env->background, NULL, NULL); /* stretch it */

  /* render text texture */
  SDL_QueryTexture(env->text, NULL, NULL, &rect.w, &rect.h);
  rect.x = w / 2 - rect.w / 2;
  rect.y = h / 2 - rect.h / 2;
  SDL_RenderCopy(ren, env->text, NULL, &rect);

  /* render mario texture */
  SDL_QueryTexture(env->mario, NULL, NULL, &rect.w, &rect.h);
  rect.x = env->mario_x - rect.w / 2;
  rect.y = env->mario_y - rect.h / 2;
  SDL_RenderCopy(ren, env->mario, NULL, &rect);

  /* render bomb texture */
  SDL_QueryTexture(env->bomb, NULL, NULL, &rect.w, &rect.h);
  rect.x = env->bomb_x - rect.w / 2;
  rect.y = env->bomb_y - rect.h / 2;
  SDL_RenderCopy(ren, env->bomb, NULL, &rect);

  /* render a triangle with lines */
  SDL_SetRenderDrawColor(ren, 255, 0, 0, SDL_ALPHA_OPAQUE); /* red */
  SDL_RenderDrawLine(ren, 320, 200, 300, 240);
  SDL_RenderDrawLine(ren, 300, 240, 340, 240);
  SDL_RenderDrawLine(ren, 340, 240, 320, 200);

  /* render a circle with points */
  const float pi = 3.14159265358979323846264338327950288419716939937510;
  float step = 2 * pi / 300; /* smoothing */
  for (float theta = 0.0; theta <= 2 * pi; theta += step) {
    int x1 = 400 + 50 * cosf(theta) + 0.5;
    int y1 = 400 + 50 * sinf(theta) + 0.5;
    SDL_RenderDrawPoint(ren, x1, y1);
  }
}

/* **************************************************************** */

/* PUT YOUR CODE HERE TO PROCESS EVENTS */
bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e) {
  int w, h;
  SDL_GetWindowSize(win, &w, &h);

  /* generic events */
  if (e->type == SDL_QUIT) {
    return true;
  }
  /* Android events */
#ifdef __ANDROID__
  else if (e->type == SDL_FINGERDOWN) {
    env->mario_x = e->tfinger.x * w; /* tfinger.x, normalized in [0..1] */
    env->mario_y = e->tfinger.y * h; /* tfinger.y, normalized in [0..1] */
  }
  /* other events */
#else
  else if (e->type == SDL_MOUSEBUTTONDOWN) {
    SDL_Point mouse;
    SDL_GetMouseState(&mouse.x, &mouse.y);
    env->mario_x = mouse.x;
    env->mario_y = mouse.y;
  } else if (e->type == SDL_KEYDOWN) {
    switch (e->key.keysym.sym) {
      case SDLK_LEFT:
        env->bomb_x -= 10;
        break;
      case SDLK_RIGHT:
        env->bomb_x += 10;
        break;
      case SDLK_UP:
        env->bomb_y -= 10;
        break;
      case SDLK_DOWN:
        env->bomb_y += 10;
        break;
      case SDLK_ESCAPE:
        return true;
        break;
    }
  }
#endif

  return false; /* don't quit */
}

/* **************************************************************** */

/* PUT YOUR CODE HERE TO CLEAN MEMORY */
void clean(SDL_Window* win, SDL_Renderer* ren, Env* env) {
  SDL_DestroyTexture(env->background);
  SDL_DestroyTexture(env->mario);
  SDL_DestroyTexture(env->bomb);
  SDL_DestroyTexture(env->text);

  free(env);
}

/* **************************************************************** */
