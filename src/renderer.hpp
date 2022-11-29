#pragma once
#include <SDL2/SDL.h>

void RenderFilledRectWithColor(SDL_Renderer* renderer, const SDL_Rect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a = SDL_ALPHA_OPAQUE);
void RenderRectWithColor(SDL_Renderer* renderer, const SDL_Rect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a = SDL_ALPHA_OPAQUE);
SDL_Texture* RenderSurfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface);