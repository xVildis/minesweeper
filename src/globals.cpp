#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "globals.hpp"

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;

void _GLIBCXX_NORETURN free_and_quit()
{
	IMG_Quit();
	TTF_Quit();

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();

	exit(EXIT_FAILURE);
}