#include <iostream>

#include <SDL2/SDL_ttf.h>

#include "constants.hpp"
#include "font.hpp"

TTF_Font* load_font(const char* font_name, int font_size)
{
    TTF_Font* font = TTF_OpenFont(font_name, font_size);

	if(!font) {
		std::cout << "Couldn't load font, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

	return font;
}