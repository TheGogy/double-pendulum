#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define SCREEN_WIDTH 1000 
#define SCREEN_HEIGHT 800

/* Constants */
#define G 10      // Gravitational constant
#define dt 1.0    // Time diff

typedef struct Color {
  int r;
  int g;
  int b;
  int a;
} Color;

typedef struct Body {
  float l;
  float m;
  float t;
  float w;
  Color color;
} Body;



void updatePositions(Body *a, Body *b) {

}

void draw(SDL_Renderer *renderer, Body *a, Body *b) {
  
  int size = 0.8 * MIN(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  float total_len = a->l + b->l;

  float length_a = size * (a->l / total_len);
  float length_b = size * (b->l / total_len);

  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2;
  
  int ax = cx + (int)length_a * sin(a->t);
  int ay = cy + (int)length_a * cos(a->t);

  int bx = ax + (int)length_b * sin(b->t);
  int by = ay + (int)length_b * cos(b->t);

  SDL_SetRenderDrawColor(renderer, a->color.r, a->color.g, a->color.b, a->color.a);
  SDL_RenderDrawLine(renderer, cx, cy, ax, ay); 
  SDL_SetRenderDrawColor(renderer, b->color.r, b->color.g, b->color.b, b->color.a);
  SDL_RenderDrawLine(renderer, cx, cy, bx, by); 
}

int main() {
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  window = SDL_CreateWindow("Three Body Problem", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create renderer
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Event event;
  int quit = 0;

  Uint32 totalFrameTicks = 0;
	Uint32 totalFrames = 0;

  Body a = {
    .l = 1.0,
    .m = 1.0,
    .t = 0.5,
    .w = 0.0,
    .color = {.r = 243, .g = 139, .b = 168, .a = 255}
  };

  Body b = {
    .l = 1.0,
    .m = 1.0,
    .t = 0.5,
    .w = 0.0,
    .color = {.r = 166, .g = 227, .b = 161, .a = 255}
  };

  int i = 0;

  while (!quit) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }
    }

    totalFrames++;
		Uint32 startTicks = SDL_GetTicks();
		Uint64 startPerf = SDL_GetPerformanceCounter();

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 17, 17, 27, 255);
    SDL_RenderClear(renderer);

    updatePositions(&a, &b);
    draw(renderer, &a, &b);

    Uint32 endTicks = SDL_GetTicks();
		Uint64 endPerf = SDL_GetPerformanceCounter();
		Uint64 framePerf = endPerf - startPerf;
		totalFrameTicks += endTicks - startTicks;

    if (!(i % 10000)) {
      printf("Average fps: %f || Perf: %"PRIu64"\n", 1000.0f / ((float)totalFrameTicks/totalFrames), framePerf);
    }

    i++;
    // Update the screen
    SDL_RenderPresent(renderer);
  }

  // Cleanup
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

