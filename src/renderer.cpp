#include <SDL2/SDL.h>

#include "renderer.hpp"

void RenderFilledRectWithColor(SDL_Renderer* renderer, const SDL_Rect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint8_t o_r, o_g, o_b, o_a = 0; 
    SDL_GetRenderDrawColor(renderer, &o_r, &o_g, &o_b, &o_a);
    
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, rect);
    SDL_SetRenderDrawColor(renderer, o_r, o_g, o_b, o_a);
}

void RenderRectWithColor(SDL_Renderer* renderer, const SDL_Rect* rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint8_t o_r, o_g, o_b, o_a = 0; 
    SDL_GetRenderDrawColor(renderer, &o_r, &o_g, &o_b, &o_a);
    
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawRect(renderer, rect);
    SDL_SetRenderDrawColor(renderer, o_r, o_g, o_b, o_a);
}