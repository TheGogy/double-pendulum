#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* Constants */
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800
#define TRAIL_SIZE 1024

/* Acceleration due to gravity (m/s^2)
 * https://nssdc.gsfc.nasa.gov/planetary/
 * Uncomment one of these
 * */
// #define G 274.0    // Sun gravity
// #define G 3.70     // Mercury gravity
// #define G 8.87     // Venus gravity
#define G 9.78     // Earth
// #define G 3.73     // Mars 
// #define G 23.12    // Jupiter
// #define G 8.96     // Saturn
// #define G 8.69     // Uranus
// #define G 11.00    // Neptune
// #define G 0.62     // Pluto

// #define G 1.625    // Moon 


#define DT 0.01   // Time diff

typedef struct Color {
  int r;
  int g;
  int b;
  int a;
} Color;

typedef struct Body {
  long double l;
  long double m;
  long double t;
  long double w;
  Color color;
} Body;

typedef struct Trail {
  int idx;
  int n_elements;
  Color color;
  SDL_FPoint points[TRAIL_SIZE];
} Trail;

void appendToTrail(Trail *t, SDL_FPoint pt) {
  t->points[t->idx++] = pt;
  t->idx %= TRAIL_SIZE;
  t->n_elements = MIN(t->n_elements + 1, TRAIL_SIZE);
}

long double getPotential(Body *a, Body *b) {
  long double y1 = -a->l * cosl(a->t);
  long double y2 = y1 - b->l * cosl(b->t);

  return a->m * G * y1 + b->m * G * y2;
}

long double getKinetic(Body *a, Body *b) {
  long double av2 = pow(a->l * a->w, 2);
  long double bv2 = pow(b->l * b->w, 2);

  long double k1 = 0.5 * a->m * av2;
  long double k2 =
      0.5 * b->m *
      (av2 + bv2 + 2 * a->l * b->l * a->w * b->w * cosl(a->t - b->t));

  return k1 + k2;
}

void lagrange(Body *a, Body *b, long double *k, long double *y) {

  long double b_a = (b->l / a->l);
  long double a_b = (a->l / b->l);
  long double total_mass = (a->m + b->m);

  long double accel_1 = b_a * (b->m / total_mass) * cosl(y[0] - y[1]);
  long double accel_2 = a_b * cosl(y[0] - y[1]);

  long double force_1 =
      -b_a * (b->m / total_mass) * (y[3] * y[3]) * sinl(y[0] - y[1]) -
      (G / a->l) * sinl(y[0]);
  long double force_2 =
      a_b * (a->w * a->w) * sinl(y[0] - y[1]) - (G / b->l) * sinl(y[1]);

  long double g1 = (force_1 - accel_1 * force_2) / (1 - accel_1 * accel_2);
  long double g2 = (force_2 - accel_2 * force_1) / (1 - accel_1 * accel_2);

  k[0] = y[2];
  k[1] = y[3];
  k[2] = g1;
  k[3] = g2;
}

void updatePositions(Body *a, Body *b) {
  long double y[4] = {a->t, b->t, a->w, b->w};
  long double k1[4], k2[4], k3[4], k4[4];
  long double tmp[4];

  lagrange(a, b, k1, y);

  tmp[0] = y[0] + DT * k1[0] / 2;
  tmp[1] = y[1] + DT * k1[1] / 2;
  tmp[2] = y[2] + DT * k1[2] / 2;
  tmp[3] = y[3] + DT * k1[3] / 2;
  lagrange(a, b, k2, tmp);

  tmp[0] = y[0] + DT * k2[0] / 2;
  tmp[1] = y[1] + DT * k2[1] / 2;
  tmp[2] = y[2] + DT * k2[2] / 2;
  tmp[3] = y[3] + DT * k2[3] / 2;
  lagrange(a, b, k3, tmp);

  tmp[0] = y[0] + DT * k3[0];
  tmp[1] = y[1] + DT * k3[1];
  tmp[2] = y[2] + DT * k3[2];
  tmp[3] = y[3] + DT * k3[3];
  lagrange(a, b, k4, tmp);

  a->t += 1.0 / 6.0 * DT * (k1[0] + 2.0 * k2[0] + 2.0 * k3[0] + k4[0]);
  b->t += 1.0 / 6.0 * DT * (k1[1] + 2.0 * k2[1] + 2.0 * k3[1] + k4[1]);
  a->w += 1.0 / 6.0 * DT * (k1[2] + 2.0 * k2[2] + 2.0 * k3[2] + k4[2]);
  b->w += 1.0 / 6.0 * DT * (k1[3] + 2.0 * k2[3] + 2.0 * k3[3] + k4[3]);
}

void draw(SDL_Renderer *renderer, Body *a, Body *b, Trail *t) {

  int size = 0.8 * MIN(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  long double total_len = a->l + b->l;

  long double length_a = size * (a->l / total_len);
  long double length_b = size * (b->l / total_len);

  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2;

  int ax = cx + (int)length_a * sinl(a->t);
  int ay = cy + (int)length_a * cosl(a->t);

  int bx = ax + (int)length_b * sinl(b->t);
  int by = ay + (int)length_b * cosl(b->t);

  // Handle trail
  SDL_FPoint tip = {.x = bx, .y = by};
  appendToTrail(t, tip);

  /* First segment */
  SDL_SetRenderDrawColor(renderer, a->color.r, a->color.g, a->color.b,
                         a->color.a);
  SDL_RenderDrawLine(renderer, cx, cy, ax, ay);

  /* Second segment */
  SDL_SetRenderDrawColor(renderer, b->color.r, b->color.g, b->color.b,
                         b->color.a);
  SDL_RenderDrawLine(renderer, ax, ay, bx, by);

  /* Trail */
  SDL_SetRenderDrawColor(renderer, t->color.r, t->color.g, t->color.b,
                         t->color.a);
  SDL_RenderDrawPointsF(renderer, t->points, t->n_elements);
}

int main() {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  window = SDL_CreateWindow("Double Pendulum", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);
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

  Body a1 = {.l = 1.0,
             .m = 1.0,
             .t = 1.8,
             .w = 0.0,
             .color = {.r = 243, .g = 139, .b = 168, .a = 255}};

  Body b1 = {.l = 1.0,
             .m = 1.0,
             .t = 1.0,
             .w = 0.0,
             .color = {.r = 166, .g = 227, .b = 161, .a = 255}};

  Trail t1 = {.n_elements = 0,
              .idx = 0,
              .color = {.r = 203, .g = 166, .b = 247, .a = 255}};

  // Body a2 = {.l = 1.0,
  //            .m = 1.0,
  //            .t = 1.81,
  //            .w = 0.0,
  //            .color = {.r = 243, .g = 139, .b = 168, .a = 255}};
  //
  // Body b2 = {.l = 1.0,
  //            .m = 1.0,
  //            .t = 1.0,
  //            .w = 0.0,
  //            .color = {.r = 243, .g = 139, .b = 168, .a = 255}};
  //
  // Trail t2 = {.n_elements = 0,
  //             .idx = 0,
  //             .color = {.r = 243, .g = 139, .b = 168, .a = 255}};
  //
  int i = 0;
  SDL_Event event;
  int quit = 0;

  while (!quit) {
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }
    }

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 17, 17, 27, 255);
    SDL_RenderClear(renderer);

    updatePositions(&a1, &b1);
    // updatePositions(&a2, &b2);
    draw(renderer, &a1, &b1, &t1);
    // draw(renderer, &a2, &b2, &t2);

    i++;
    // Update the screen
    SDL_RenderPresent(renderer);
    SDL_Delay(10);
  }

  // Cleanup
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
