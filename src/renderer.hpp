#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

void render_filled_rect(SDL_Renderer* renderer, const SDL_Rect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a = SDL_ALPHA_OPAQUE);
void render_rect_with_color(SDL_Renderer* renderer, const SDL_Rect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a = SDL_ALPHA_OPAQUE);

SDL_Texture* render_surface_to_texture(SDL_Renderer* renderer, SDL_Surface* surface);
SDL_Texture* load_and_render_image_to_texture(SDL_Renderer* renderer, const char* path);
SDL_Texture* render_colored_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color = {0,0,0,0});
