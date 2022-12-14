#include <iostream>
#include <vector>
#include <random>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "constants.hpp"
#include "globals.hpp"
#include "renderer.hpp"
#include "font.hpp"

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;

static bool g_running = true;

enum TileData
{
    TILE_EMPTY = 0,
    TILE_1,
    TILE_2,
    TILE_3,
    TILE_4,
    TILE_5,
    TILE_6,
    TILE_7,
    TILE_8,
    TILE_BOMB,
};

struct Tile {
	TileData data = TILE_EMPTY;
	bool open = false;
	bool flagged = false;
};

class Minesweeper {
	int bombcount;
public:
	int width, height;

	std::vector<std::vector<Tile>> tilemap;

	Minesweeper(int width, int height, int bombcount) 
		: width(width), height(height)
	{
		this->tilemap = std::vector<std::vector<Tile>>( 1 + height + 1, std::vector<Tile>(1 + width + 1, Tile() ) );

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

			Tile& tile = this->tilemap[r_height][r_width];

			if(tile.data != TILE_BOMB) {
				tile.data = TILE_BOMB;
				placed_bombs++;
			}
		}
		
		for(int i = 1; i <= height; i++) {
			for(int j = 1; j <= width; j++) {
				if(this->tilemap[i][j].data == TILE_BOMB)
					continue;

				const std::vector<Tile> neighbors = {
					this->tilemap[i - 1][j - 1], this->tilemap[i - 1][j], this->tilemap[i - 1][j + 1],
					this->tilemap[i][j - 1],   /*this->Tilemap[i][j],*/   this->tilemap[i][j + 1],
					this->tilemap[i + 1][j - 1], this->tilemap[i + 1][j], this->tilemap[i + 1][j + 1]
				};

				int tile_number = 0;
				for(const Tile& neighbor : neighbors) {
					if(neighbor.data == TILE_BOMB)
						tile_number++;
				}

				this->tilemap[i][j].data = (TileData)tile_number;
			}
		}
	}

	void open_tile(int row, int col)
	{
		Tile* tile = &this->tilemap[row][col];
		if(!tile->flagged) {
			tile->open = true;
		}
	}

	void flag_tile(int row, int col)
	{
		if(row > height) return;
		if(col > width) return;

		Tile* tile = &this->tilemap[row][col];
		if(!tile->open) {
			tile->flagged = !tile->flagged;
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

bool rmb_isdown;
bool rmb_wasdown;

bool lmb_isdown;
bool lmb_wasdown;

bool pixel_to_tile(Minesweeper* game, int x, int y, int* row, int* column)
{
	int bound_x = AREA_START + (game->width  * TILE_WIDTH)  + game->width; 
	int bound_y = AREA_START + (game->height * TILE_HEIGHT) + game->height;

	printf("x: %i, y: %i\n", x, y);

	if (x < AREA_START || y < AREA_START ||
		x > bound_x || y > bound_y) 
	{
		return false;
	}

	*row =    (y - (y % (TILE_HEIGHT + 1))) / (TILE_HEIGHT + 1);
	*column = (x - (x % (TILE_WIDTH  + 1))) / (TILE_WIDTH  + 1);

	return true;
}

void handle_input(Minesweeper* &game)
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
			{
				SDL_MouseButtonEvent mouse_event = event.button;
				int x = mouse_event.x, y = mouse_event.y;
				int button = mouse_event.button;

				switch (button) 
				{
					case SDL_BUTTON_LEFT:
					{
						int row = 0, col = 0;
						if(pixel_to_tile(game, x, y, &row, &col)) {
							game->open_tile(row, col);
						}
						break;
					}
					case SDL_BUTTON_RIGHT:
					{
						int row = 0, col = 0;
						if(pixel_to_tile(game, x, y, &row, &col)) {
							game->flag_tile(row, col);
						}
						break;
					}
					case SDL_BUTTON_MIDDLE: {
						printf("before: %p\n", game);
						delete game;
						printf("deleted: %p\n", game);
						game = new Minesweeper(30, 16, 99);
						printf("after: %p\n", game);
						break;
					}

					default:
						break;
				}
				
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				break;
			}

			default:
				break;
		}
	}
		
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// TODO: parse args (screen size, board size, mine count)
	if(!initialize_sdl())
		free_and_quit();

	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

#ifdef __unix__
	if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
    {
        std::cout << "SDL can not disable compositor bypass!" << std::endl;
        free_and_quit();
    }
#endif
	//TODO: variable font pt size?
	TTF_Font* test_font = load_font("assets/joystix.monospace-regular.ttf", 24);

	SDL_Texture* number_textures[] = {
		render_colored_text(g_renderer, test_font, "1"), render_colored_text(g_renderer, test_font, "2"), render_colored_text(g_renderer, test_font, "3"),
		render_colored_text(g_renderer, test_font, "4"), render_colored_text(g_renderer, test_font, "5"), render_colored_text(g_renderer, test_font, "6"),
		render_colored_text(g_renderer, test_font, "7"), render_colored_text(g_renderer, test_font, "8")
	};

	SDL_Texture* bomb_texture = load_and_render_image_to_texture(g_renderer, "assets/bomb.png");
	SDL_Texture* flag_texture = load_and_render_image_to_texture(g_renderer, "assets/flag.png");

	Minesweeper* game = new Minesweeper(30, 16, 99);

	const int minimum_h = (AREA_START * 3) + game->height * TILE_HEIGHT;
	const int minimum_w = (AREA_START * 3) + game->width  * TILE_WIDTH;
	SDL_SetWindowMinimumSize(g_window, minimum_w, minimum_h);

	int w, h = 0;
	SDL_QueryTexture(number_textures[0], NULL, NULL, &w, &h);

	const static uint64_t freq = SDL_GetPerformanceFrequency();

	while (g_running)
	{
		handle_input(game);
		uint64_t start = SDL_GetPerformanceCounter();
		SDL_RenderClear(g_renderer);
/*
		for(int i = 0; auto* texture : number_textures) {
			const SDL_Rect debug_rect = {
				.x = AREA_START + (i * w),
				.y = AREA_START * 2 + (TILE_HEIGHT * game.height),
				.w = w,
				.h = h
			};
			SDL_RenderCopy(g_renderer, texture, NULL, &debug_rect);
			i++;
		}
*/
		for(int column = 1; column <= game->height; column++) 
		{
			for(int row = 1; row <= game->width; row++) 
			{
				const int xpos = AREA_START + (row    - 1) * TILE_WIDTH  + (row    - 1);
				const int ypos = AREA_START + (column - 1) * TILE_HEIGHT + (column - 1);
				const Tile& tile = game->tilemap[column][row];
				const SDL_Rect bound_rect = {
					.x = xpos,
					.y = ypos,
					.w = TILE_WIDTH,
					.h = TILE_HEIGHT
				};
				if(tile.open) {
					if(tile.data != TILE_BOMB && tile.data != TILE_EMPTY) {
						const SDL_Rect number_rect = {
							.x = xpos + 4,
							.y = ypos,
							.w = w, 
							.h = h
						};
						SDL_RenderCopy(g_renderer, number_textures[tile.data - 1], NULL, &number_rect);
					} else if(tile.data == TILE_BOMB) {
						const SDL_Rect bomb_rect = {
							.x = bound_rect.x + TILE_SPACER,
							.y = bound_rect.y + TILE_SPACER,
							.w = TILE_WIDTH -  (TILE_SPACER * 2),
							.h = TILE_HEIGHT - (TILE_SPACER * 2)
						};
						SDL_RenderCopy(g_renderer, bomb_texture, NULL, &bomb_rect);
					}
				}
				else 
				{
					RenderFilledRectWithColor(g_renderer, &bound_rect, 127, 127, 127);
					if(tile.flagged) {
						const SDL_Rect flag_rect = {
							.x = bound_rect.x + TILE_SPACER,
							.y = bound_rect.y + TILE_SPACER,
							.w = TILE_WIDTH  - (TILE_SPACER * 2),
							.h = TILE_HEIGHT - (TILE_SPACER * 2)
						};
						SDL_RenderCopy(g_renderer, flag_texture, NULL, &flag_rect);
					}
				}

				RenderRectWithColor(g_renderer, &bound_rect, 0, 0, 0);
			}
		}

		SDL_RenderPresent(g_renderer);
		const uint64_t end = SDL_GetPerformanceCounter();
		uint64_t elapsed_ticks = end - start;

		//https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps#using-qpc-in-native-code
		elapsed_ticks *= 1000000;

		const uint64_t elapsed_microseconds = elapsed_ticks / freq;
		const double frametime = elapsed_microseconds / 1000.f;

		SDL_Delay(16);
/*
		if(frametime < 1000 / 60) {
			SDL_Delay(1000 / 60 - frametime);
		}
*/
	}

	IMG_Quit();

	TTF_CloseFont(test_font); 
	TTF_Quit();

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	
	return EXIT_SUCCESS;
}