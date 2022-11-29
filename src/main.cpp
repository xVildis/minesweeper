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

bool initialize_sdl()
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

void handle_input()
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

				// TODO: set emoji at the top
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				{
					//Get mouse position
					int x, y;
					int button = SDL_GetMouseState( &x, &y );

					if(button == SDL_BUTTON_LEFT) {
						printf("left button pressed!\n");
					}
				}

				default:
					break;
			}
		}
		
}

constexpr char numbers[] = "12345678";

SDL_Texture* render_number_texture(TTF_Font* font)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, numbers, {0, 0, 0});

	if(text_surface == NULL) {
		std::cout << "Couldn't render number text, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

	return RenderSurfaceToTexture(g_renderer, text_surface);
}

SDL_Texture* load_bomb_texture()
{
	std::string path = std::filesystem::current_path().generic_string();
	SDL_Surface* bomb_surface = IMG_Load(path.append("/assets/bomb.png").c_str());

	if(bomb_surface == NULL) {
		std::cout << "Couldn't load bomb image, ERROR: " << IMG_GetError() << "\n";
		free_and_quit();
	}

	return RenderSurfaceToTexture(g_renderer, bomb_surface);
}

int main(int argc, char* argv[])
{
	// TODO: parse args (screen size, board size, mine count)
	if(!initialize_sdl())
		free_and_quit();

	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	//TODO: variable font pt size?
	TTF_Font* test_font = TTF_OpenFont("assets/joystix.monospace-regular.ttf", 24);

	if(!test_font) {
		std::cout << "Couldn't load font, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

	SDL_Texture* number_texture = render_number_texture(test_font);

	int font_w, font_h = 0;
	if(TTF_SizeText(test_font, numbers, &font_w, &font_h) < 0) {
		std::cout << "Couldn't calculate font size ERROR:" << TTF_GetError() << "\n";
		free_and_quit();
	}

	printf("font size: %i, %i\n", font_h, font_w);

	Minesweeper game(30, 16, 99);

	int minimum_h = (AREA_START * 4) + (game.height * 5) + game.height * tile_h;
	int minimum_w = (AREA_START * 4) + (game.width * 5) + game.width * tile_w;

	SDL_SetWindowMinimumSize(g_window, minimum_w, minimum_h);

	while (g_running)
	{
		handle_input();
		SDL_RenderClear(g_renderer);
		
		for(int column = 1; column <= game.height; column++) 
		{
			for(int row = 1; row <= game.width; row++) 
			{
				TileData tile = game.Tilemap[column][row];
				const SDL_Rect bound_rect = {
					.x = row * tile_w + (row * GLOBAL_SCALE),
					.y = column * tile_h + (column * GLOBAL_SCALE),
					.w = tile_w,
					.h = tile_h
				};
				
				const SDL_Rect font_rect = {
					// dont know why i need to add tile number of pixels as offset
					.x = (font_w / 8) * (tile - 1) + tile,
					.y = 0,
					.w = (font_w / 10),
					.h = font_h
				};
				SDL_RenderCopy(g_renderer, number_texture, &font_rect, &bound_rect);

				RenderRectWithColor(g_renderer, &bound_rect, 0, 0, 0);
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