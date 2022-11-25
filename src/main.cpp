#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <filesystem>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "constants.hpp"
#include "renderer.hpp"

static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;

static bool g_running = true;

class Minesweeper {
public:
	int width, height;
	int bombcount;

	std::vector<std::vector<TileData>> Tilemap;

	Minesweeper(int width, int height, int bombcount) 
		: width(width), height(height)
	{
		this->Tilemap = std::vector<std::vector<TileData>>( 1 + height + 1, std::vector<TileData>(1 + width + 1, TILE_EMPTY));

		if(bombcount > width * height) {
			this->bombcount = width * height;
		} else {
			this->bombcount = bombcount;
		}
		
		std::random_device dev;
		std::mt19937 rng(dev());

		std::uniform_int_distribution<std::mt19937::result_type> random_width(1,  width);
		std::uniform_int_distribution<std::mt19937::result_type> random_height(1, height);

		int placed_bombs = 0;
		while(placed_bombs < bombcount)
		{
			int r_width  = random_width(rng);
			int r_height = random_height(rng);

			assert(r_width <= width);
			assert(r_height <= height);

			TileData& tile = this->Tilemap[r_height][r_width];

			if(tile != TILE_BOMB) {
				tile = TILE_BOMB;
				placed_bombs++;
			}
		}

		for(int i = 1; i <= height; i++) {
			for(int j = 1; j <= width; j++) {
				if(this->Tilemap[i][j] == TILE_BOMB)
					continue;

				const std::vector<TileData> neighbors = {
					this->Tilemap[i - 1][j - 1], this->Tilemap[i - 1][j], this->Tilemap[i - 1][j + 1],
					this->Tilemap[i][j - 1],   /*this->Tilemap[i][j],*/   this->Tilemap[i][j + 1],
					this->Tilemap[i + 1][j - 1], this->Tilemap[i + 1][j], this->Tilemap[i + 1][j + 1]
				};

				int tile_number = 0;
				for(const TileData& neighbor : neighbors) {
					if(neighbor == TILE_BOMB)
						tile_number++;
				}

				this->Tilemap[i][j] = (TileData)tile_number;
			}
		}
	}

	void print_minefield()
	{
		for(int i = 0; i <= this->height + 1; i++) {
			for(int j = 0; j <= this->width + 1; j++) {
				if(this->Tilemap[i][j] == TILE_BOMB) {
					printf("\033[41mB\033[0m ");
				} else if(this->Tilemap[i][j] == TILE_EMPTY) {
					printf("  ");
				} else {
					printf("%i ", this->Tilemap[i][j]);
				}
			}
			printf("\n");
		}
	}
};

void _GLIBCXX_NORETURN free_and_quit()
{
	IMG_Quit();
	TTF_Quit();

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();

	exit(EXIT_FAILURE);
}

bool InitializeSDL()
{
	int sdl_status = SDL_Init(SDL_INIT_EVERYTHING);
	if(sdl_status < 0) {
		printf("Could not initialize SDL, ERROR: %s\n", SDL_GetError());
		return false;
	}

	g_window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);

	if(g_window == NULL) {
		std::cout << "Could not create window, ERROR:" << SDL_GetError() << "\n";
		return false;
	}

	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	if(g_renderer == NULL) {
		std::cout << "Could not create renderer, ERROR:" << SDL_GetError() << "\n";
		return false;
	}	

	if(TTF_Init() < 0) {
		std::cout << "Could not initialize SDL_TTF, ERROR: " << TTF_GetError() << "\n";
		return false;
	}

	if(IMG_Init(IMG_INIT_PNG) < 0) {
		std::cout << "Could not initialize SDL_IMG, ERROR: " << IMG_GetError() << "\n";
		return false;
	}

	return true;
}

int main(int argc, char* argv[])
{
	// TODO: parse args (screen size, board size, mine count)
	if(!InitializeSDL())
		free_and_quit();

	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	std::string path = std::filesystem::current_path().generic_string();

	//TODO: variable font pt size?
	TTF_Font* test_font = TTF_OpenFont("assets/joystix.monospace-regular.ttf", 24);

	if(!test_font) {
		std::cout << "Couldn't load font, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

	SDL_Surface* bomb_surface = IMG_Load(path.append("/assets/bomb.png").c_str());

	if(bomb_surface == NULL) {
		std::cout << "Couldn't load bomb image, ERROR: " << IMG_GetError() << "\n";
		free_and_quit();
	}

	SDL_Texture* bomb_texture = SDL_CreateTextureFromSurface(g_renderer, bomb_surface);

	if(bomb_texture == NULL) {
		std::cout << "Could create bomb texture, ERROR:" << SDL_GetError() << "\n";
		free_and_quit();
	}

	SDL_FreeSurface(bomb_surface);

	const char* numbers = "12345678";

	SDL_Surface* text_surface = TTF_RenderText_Solid(test_font, numbers, {0, 0, 0});

	if(text_surface == NULL) {
		std::cout << "Couldn't render number text, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(g_renderer, text_surface);

	if(text_texture == NULL) {
		std::cout << "Could create font texture, ERROR:" << SDL_GetError() << "\n";
		free_and_quit();
	}

	int font_w, font_h = 0;
	if(TTF_SizeText(test_font, numbers, &font_w, &font_h) < 0) {
		std::cout << "Couldn't calculate font size ERROR:" << TTF_GetError() << "\n";
		free_and_quit();
	}

	printf("font size: %i, %i\n", font_h, font_w);

	SDL_FreeSurface(text_surface);

	Minesweeper game(30, 16, 99);

	int minimum_h = (AREA_START * 4) + (game.height * 5) + game.height * tile_h;
	int minimum_w = (AREA_START * 4) + (game.width * 5) + game.width * tile_w;

	SDL_SetWindowMinimumSize(g_window, minimum_w, minimum_h);

	while (g_running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				g_running = false;
			}

			switch(event.type)
			{
				case SDL_KEYDOWN:
				{
					SDL_KeyboardEvent keyevent = event.key;
					if(keyevent.keysym.sym == SDLK_ESCAPE)
					{
						g_running = false;
						break;
					}
			
					break;
				}

				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				{
					//Get mouse position
					int x, y;
					int button = SDL_GetMouseState( &x, &y );

					if(button == SDL_BUTTON_LEFT) {
						
					}
				}

				default:
					break;
			}
		}
		SDL_RenderClear(g_renderer);
		
		for(int i = 1; i <= game.height; i++) 
		{
			for(int j = 1; j <= game.width; j++) 
			{
				int x_offset = j * 5;
				int y_offset = i * 5;
				const SDL_Rect bound_rect = {
					.x = j * tile_w + x_offset,
					.y = i * tile_h + y_offset,
					.w = tile_w,
					.h = tile_h
				};

				TileData tile = game.Tilemap[i][j];
				
				if(tile != TILE_EMPTY && tile != TILE_BOMB) {

					const SDL_Rect number_rect = {
						.x = 0,
						.y = 0,
						.w = (font_w + 1) / 8,
						.h = font_h,
					};

					const SDL_Rect text_rect = {
						.x = bound_rect.x,
						.y = bound_rect.y,
						.w = (font_w + 1) / 8,
						.h = font_h,
					};
					
					SDL_SetRenderDrawColor(g_renderer, 0,0,0, SDL_ALPHA_OPAQUE);
					SDL_RenderCopy(g_renderer, text_texture, &number_rect, &text_rect);
					SDL_SetRenderDrawColor(g_renderer, 255,255,255, SDL_ALPHA_OPAQUE);
				} else if(tile == TILE_BOMB) {

					const SDL_Rect bomb_rect = {
						.x = bound_rect.x + 3,
						.y = bound_rect.y + 3,
						.w = 8 * GLOBAL_SCALE,
						.h = 8 * GLOBAL_SCALE,
					};

					SDL_SetRenderDrawColor(g_renderer, 0,0,0, SDL_ALPHA_OPAQUE);
					SDL_RenderCopy(g_renderer, bomb_texture, NULL, &bomb_rect);
					SDL_SetRenderDrawColor(g_renderer, 255,255,255, SDL_ALPHA_OPAQUE);
				}
				RenderRectWithColor(g_renderer, &bound_rect, 0, 0, 0);
				//RenderFilledRectWithColor(g_renderer, &tile_rect, 127, 127, 127, SDL_ALPHA_OPAQUE);
			}
		}

		SDL_RenderPresent(g_renderer);
		SDL_Delay(14);
	}

	IMG_Quit();

	TTF_CloseFont(test_font); 
	TTF_Quit();

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	
	return EXIT_SUCCESS;
}