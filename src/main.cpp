#include <iostream>

#include <SDL2/SDL.h>

#include "constants.hpp"

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static bool running = true;

int main()
{
	// TODO: parse args (screen size, board size, mine count)
	int sdl_status = SDL_Init(SDL_INIT_EVERYTHING);
	if(sdl_status < 0) {
		printf("Could not initialize SDL, ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;	
	}

	window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	if(!window) {
		std::cout << "Could not create window, ERROR:" << SDL_GetError() << "\n";
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_RenderClear(renderer);
	
	return EXIT_SUCCESS;
}