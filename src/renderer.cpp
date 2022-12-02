#include <iostream>
#include <filesystem>

#include "constants.hpp"
#include "renderer.hpp"
#include "globals.hpp"

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

SDL_Texture* RenderSurfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface)
{
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	if(texture == NULL) {
		std::cout << "Could create bomb texture, ERROR:" << SDL_GetError() << "\n";
		free_and_quit();
	}

	SDL_FreeSurface(surface);

	return texture;
}

SDL_Texture* load_and_render_image_to_texture(SDL_Renderer* renderer, const char* path)
{
	std::string cwd = std::filesystem::current_path().generic_string();
	SDL_Surface* image_surface = IMG_Load(cwd.append("/").append(path).c_str());

	if(image_surface == NULL) {
		std::cout << "Couldn't load image "<< path << ", ERROR: " << IMG_GetError() << "\n";
		free_and_quit();
	}

	return RenderSurfaceToTexture(renderer, image_surface);
}

SDL_Texture* render_colored_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);

	if(text_surface == NULL) {
		std::cout << "Couldn't render number text, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

	return RenderSurfaceToTexture(renderer, text_surface);
}
