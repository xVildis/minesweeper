#include <iostream>
#include <vector>
#include <random>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "globals.hpp"
#include "renderer.hpp"

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

constexpr int GLOBAL_SCALE = 3;

constexpr int TILE_WIDTH  = 10 * GLOBAL_SCALE;
constexpr int TILE_HEIGHT = 10 * GLOBAL_SCALE;

constexpr int OUTSIDE_PADDING = 30;
constexpr int TILE_NUMBER_SPACER = 3;

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
	bool dead = false;

	std::vector<std::vector<Tile>> tilemap;

	Minesweeper(int width, int height, int bombcount) : bombcount(bombcount), width(width), height(height)
	{
		// HACK: adding 1 tile to each side to prevent OOB
		this->tilemap = std::vector<std::vector<Tile>>( 1 + height + 1, std::vector<Tile>(1 + width + 1, Tile() ) );

		// cap bombcount to number of tiles
		if(this->bombcount > width * height) {
			this->bombcount = width * height;
		}

		std::random_device dev;
		std::mt19937 rng(dev());

		// HACK: skip index 0 to prevent OOB
		std::uniform_int_distribution<std::mt19937::result_type> random_width(1,  width);
		std::uniform_int_distribution<std::mt19937::result_type> random_height(1, height);

		// this is not optimal as random will probably generate the same number
		// which makes this loop take longer than it should
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

				// collect tile neighbors for calculating numbers
				const Tile neighbors[] = {
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
		if (this->dead) return;

		Tile* tile = &this->tilemap[row][col];
		if(!tile->flagged && !tile->open) {
			tile->open = true;

			// open neighboring empty tiles
			if (tile->data == TILE_EMPTY) {
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						if (i == 0 && j == 0) continue;
						if ( (row + i < 1 || col + j < 1) || (row + i > height || col + j > width)) continue;
						
						open_tile(row + i, col + j);
					}
				}
			}
			else if (tile->data == TILE_BOMB) {
				this->dead = true;
			}
		}
	}

	void flag_tile(int row, int col)
	{
		if (this->dead) return;

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
		std::cout << "Could not initialize SDL, ERROR: " << SDL_GetError() << "\n";
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
	int bound_x = OUTSIDE_PADDING + (game->width  * TILE_WIDTH)  + game->width;
	int bound_y = OUTSIDE_PADDING + (game->height * TILE_HEIGHT) + game->height;

	if (x < OUTSIDE_PADDING || y < OUTSIDE_PADDING ||
		x > bound_x || y > bound_y) 
	{
		return false;
	}

	*row 	= (y - (y % (TILE_HEIGHT + 1))) / (TILE_HEIGHT + 1);
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
						delete game;
						game = new Minesweeper(30, 16, 99);
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
	if(!initialize_sdl())
		free_and_quit();

	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	//TODO: variable font pt size?
	TTF_Font* test_font = TTF_OpenFont("assets/joystix.monospace-regular.ttf", 24);

	if(!test_font) {
		std::cout << "Couldn't load font, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}
	
	// rendering each number separately because all of them have different dimensions when rendered together
	SDL_Texture* number_textures[] = {
		render_colored_text(g_renderer, test_font, "1", {0,0,255,255}), render_colored_text(g_renderer, test_font, "2", {0,128,0,255}), render_colored_text(g_renderer, test_font, "3", {255,0,0,255}),
		render_colored_text(g_renderer, test_font, "4", {0,0,128,255}), render_colored_text(g_renderer, test_font, "5", {128,0,0,255}), render_colored_text(g_renderer, test_font, "6", {100, 255, 255,255}),
		render_colored_text(g_renderer, test_font, "7", {0,0,0,255}),   render_colored_text(g_renderer, test_font, "8", {80,80,80,255})
	};

	// load and render necessary textures
	SDL_Texture* bomb_texture = load_and_render_image_to_texture(g_renderer, "assets/bomb.png");
	SDL_Texture* flag_texture = load_and_render_image_to_texture(g_renderer, "assets/flag.png");

	Minesweeper* game = new Minesweeper(30, 16, 99);

	// calculate minimum window dimensions
	// TODO: change these on new game
	int minimum_w = game->width  * TILE_WIDTH  + game->width  + 2 * OUTSIDE_PADDING;
	int minimum_h = game->height * TILE_HEIGHT + game->height + 2 * OUTSIDE_PADDING;
	SDL_SetWindowMinimumSize(g_window, minimum_w, minimum_h);

	int char_width, char_height = 0;
	SDL_QueryTexture(number_textures[0], NULL, NULL, &char_width, &char_height);

	const static uint64_t freq = SDL_GetPerformanceFrequency();

	while (g_running)
	{
		handle_input(game);
		uint64_t start = SDL_GetPerformanceCounter();
		SDL_RenderClear(g_renderer);

		// HACK: starting at index 1 due the hack when generating bombs
		for(int column = 1; column <= game->height; column++) 
		{
			for(int row = 1; row <= game->width; row++) 
			{
				const int tile_xpos = (row    - 1) * TILE_WIDTH  + OUTSIDE_PADDING + (row    - 1);
				const int tile_ypos = (column - 1) * TILE_HEIGHT + OUTSIDE_PADDING + (column - 1);

				Tile& tile = game->tilemap[column][row];
				
				const SDL_Rect bound_rect = {
					.x = tile_xpos,
					.y = tile_ypos,
					.w = TILE_WIDTH,
					.h = TILE_HEIGHT
				};
				
				if(tile.open) {
					if(tile.data != TILE_BOMB && tile.data != TILE_EMPTY) {
						const SDL_Rect number_rect = {
							.x = tile_xpos + TILE_NUMBER_SPACER + 1,
							.y = tile_ypos,
							.w = char_width, 
							.h = char_height
						};
						SDL_RenderCopy(g_renderer, number_textures[tile.data - 1], NULL, &number_rect);
					} else if(tile.data == TILE_BOMB) {
						const SDL_Rect bomb_rect = {
							.x = bound_rect.x + TILE_NUMBER_SPACER,
							.y = bound_rect.y + TILE_NUMBER_SPACER,
							.w = TILE_WIDTH  - (TILE_NUMBER_SPACER * 2),
							.h = TILE_HEIGHT - (TILE_NUMBER_SPACER * 2)
						};
						SDL_RenderCopy(g_renderer, bomb_texture, NULL, &bomb_rect);
					}
				} 
				else if (game->dead && tile.data == TILE_BOMB) 
				{
					const SDL_Rect bomb_rect = {
						.x = bound_rect.x + TILE_NUMBER_SPACER,
						.y = bound_rect.y + TILE_NUMBER_SPACER,
						.w = TILE_WIDTH  - (TILE_NUMBER_SPACER * 2),
						.h = TILE_HEIGHT - (TILE_NUMBER_SPACER * 2)
					};
					SDL_RenderCopy(g_renderer, bomb_texture, NULL, &bomb_rect);
				}
				else 
				{
					render_filled_rect(g_renderer, &bound_rect, 127, 127, 127);
					if(tile.flagged) {
						const SDL_Rect flag_rect = {
							.x = bound_rect.x + TILE_NUMBER_SPACER,
							.y = bound_rect.y + TILE_NUMBER_SPACER,
							.w = TILE_WIDTH  - (TILE_NUMBER_SPACER * 2),
							.h = TILE_HEIGHT - (TILE_NUMBER_SPACER * 2)
						};
						SDL_RenderCopy(g_renderer, flag_texture, NULL, &flag_rect);
					}
				}

				render_rect_with_color(g_renderer, &bound_rect, 0, 0, 0);
			}
		}

		SDL_RenderPresent(g_renderer);
		const uint64_t end = SDL_GetPerformanceCounter();
		uint64_t elapsed_ticks = end - start;

		//https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps#using-qpc-in-native-code
		elapsed_ticks *= 1000000;

		const uint64_t elapsed_microseconds = elapsed_ticks / freq;
		const double frametime = elapsed_microseconds / 1000.f;
		
		// cap framerate at 60
		if(frametime < 1000.f / 60.f) {
			SDL_Delay(1000.f / 60.f - frametime);
		}
	}

	IMG_Quit();

	TTF_CloseFont(test_font); 
	TTF_Quit();

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	
	return EXIT_SUCCESS;
}