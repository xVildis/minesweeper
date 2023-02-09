#include <iostream>
#include <vector>
#include <random>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "globals.hpp"
#include "renderer.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

constexpr int GLOBAL_SCALE = 3;

constexpr int TILE_WIDTH  = 10 * GLOBAL_SCALE;
constexpr int TILE_HEIGHT = 10 * GLOBAL_SCALE;

constexpr int OUTSIDE_PADDING = 30;
constexpr int INSIDE_TILE_PADDING = 3;

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
			printf("opening (%i,%i)\n", row, col);
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

	void flag_tile(int row, int col) {
		if (this->dead) return;

		if(row > height) return;
		if(col > width) return;

		Tile* tile = &this->tilemap[row][col];
		if(!tile->open) {
			tile->flagged = !tile->flagged;
		}
	}

	bool pixel_to_tile(int x, int y, int* row, int* column) {
		int bound_x = OUTSIDE_PADDING + (this->width  * TILE_WIDTH)  + this->width;
		int bound_y = OUTSIDE_PADDING + (this->height * TILE_HEIGHT) + this->height;

		if (x < OUTSIDE_PADDING || y < OUTSIDE_PADDING ||
			x > bound_x || y > bound_y) 
		{
			return false;
		}

		*row 	= (y - (y % (TILE_HEIGHT + 1))) / (TILE_HEIGHT + 1);
		*column = (x - (x % (TILE_WIDTH  + 1))) / (TILE_WIDTH  + 1);

		return true;
	}
};

struct number_texture
{
	SDL_Texture* tex;
	int w, h;
};

struct game_context 
{
	Minesweeper* game;
	SDL_Texture* bomb;
	SDL_Texture* flag;
	number_texture numbers[8];
};

bool initialize_sdl()
{
	int sdl_status = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
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
					case SDL_BUTTON_RIGHT:
					{
						int row = 0, col = 0;
						if(game->pixel_to_tile(x, y, &row, &col)) {
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
				SDL_MouseButtonEvent mouse_event = event.button;
				int x = mouse_event.x, y = mouse_event.y;
				int button = mouse_event.button;

				switch (button) 
				{
					case SDL_BUTTON_LEFT:
					{
						int row = 0, col = 0;
						if(game->pixel_to_tile(x, y, &row, &col)) {
							game->open_tile(row, col);
						}
						break;
					}

					default:
						break;
				}
				
				break;
			}

			default:
				break;
		}
	}
		
} 

void game_loop(void* ctx)
{
	game_context* context = (game_context*)ctx;
	Minesweeper* &game = context->game;

	handle_input(game);
	SDL_RenderClear(g_renderer);

	// HACK: starting at index 1 due the hack when generating bombs
	for(int column = 1; column <= game->height; column++) 
	{
		for(int row = 1; row <= game->width; row++) 
		{
			const int tile_xpos = (row    - 1) * TILE_WIDTH  + OUTSIDE_PADDING + (row    - 1);
			const int tile_ypos = (column - 1) * TILE_HEIGHT + OUTSIDE_PADDING + (column - 1);

			Tile& tile = game->tilemap[column][row];

			int tile_number = tile.data - 1;
			
			const SDL_Rect bound_rect = {
				.x = tile_xpos,
				.y = tile_ypos,
				.w = TILE_WIDTH,
				.h = TILE_HEIGHT
			};
			
			if(tile.open) {
				if(tile.data != TILE_BOMB && tile.data != TILE_EMPTY) {
					int tile_width = context->numbers[tile_number].w;
					int tile_height = context->numbers[tile_number].h;

					const SDL_Rect number_rect = {
						.x = tile_xpos + (bound_rect.w - tile_width) / 2,
						.y = tile_ypos + (bound_rect.h - tile_height) / 2,
						.w = tile_width, 
						.h = tile_height
					};
					SDL_RenderCopy(g_renderer, context->numbers[tile_number].tex, NULL, &number_rect);
				} else if(tile.data == TILE_BOMB) {
					const SDL_Rect bomb_rect = {
						.x = bound_rect.x + INSIDE_TILE_PADDING,
						.y = bound_rect.y + INSIDE_TILE_PADDING,
						.w = TILE_WIDTH  - (INSIDE_TILE_PADDING * 2),
						.h = TILE_HEIGHT - (INSIDE_TILE_PADDING * 2)
					};
					SDL_RenderCopy(g_renderer, context->bomb, NULL, &bomb_rect);
				}
			} 
			else if (game->dead && tile.data == TILE_BOMB) 
			{
				const SDL_Rect bomb_rect = {
					.x = bound_rect.x + INSIDE_TILE_PADDING,
					.y = bound_rect.y + INSIDE_TILE_PADDING,
					.w = TILE_WIDTH  - (INSIDE_TILE_PADDING * 2),
					.h = TILE_HEIGHT - (INSIDE_TILE_PADDING * 2)
				};
				SDL_RenderCopy(g_renderer, context->bomb, NULL, &bomb_rect);
			}
			else 
			{
				render_filled_rect(g_renderer, &bound_rect, 127, 127, 127);
				if(tile.flagged) {
					const SDL_Rect flag_rect = {
						.x = bound_rect.x + INSIDE_TILE_PADDING,
						.y = bound_rect.y + INSIDE_TILE_PADDING,
						.w = TILE_WIDTH  - (INSIDE_TILE_PADDING * 2),
						.h = TILE_HEIGHT - (INSIDE_TILE_PADDING * 2)
					};
					SDL_RenderCopy(g_renderer, context->flag, NULL, &flag_rect);
				}
			}

			render_rect_with_color(g_renderer, &bound_rect, 0, 0, 0);
		}
	}

	SDL_RenderPresent(g_renderer);
}

int main( [[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	if(!initialize_sdl())
		free_and_quit();

	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	//TODO: variable font pt size?
	TTF_Font* test_font = TTF_OpenFont("assets/Rubik-Medium.ttf", 28);

	if(!test_font) {
		std::cout << "Couldn't load font, ERROR: " << TTF_GetError() << "\n";
		free_and_quit();
	}

	game_context context = {};

	context.bomb = load_and_render_image_to_texture(g_renderer, "assets/bomb.png");
	context.flag = load_and_render_image_to_texture(g_renderer, "assets/flag.png");

	constexpr SDL_Color number_colors[] = {
		{0,0,255,255}, {0,128,0,255}, {255,0,0,255},
		{0,0,128,255}, {128,0,0,255}, {0,128,128,255},
		{0,0,0,255},   {128,128,128,255}
	};

	for (int i = 0; i <= 7; i++) {
		char text[2];
		snprintf(text, sizeof(text),"%d", i+1);
		SDL_Texture* texture = render_colored_text(g_renderer, test_font, text, number_colors[i]);
		int tex_w = 0, tex_h = 0;
		SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);

		context.numbers[i] = {
			.tex = texture,
			.w = tex_w,
			.h = tex_h
		};
	}

	context.game = new Minesweeper(30, 16, 99);

	// calculate minimum window dimensions
	// TODO: change these on new game
	int minimum_w = context.game->width  * TILE_WIDTH  + context.game->width  + 2 * OUTSIDE_PADDING;
	int minimum_h = context.game->height * TILE_HEIGHT + context.game->height + 2 * OUTSIDE_PADDING;
	SDL_SetWindowMinimumSize(g_window, minimum_w, minimum_h);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(game_loop, (void*)&context, 0, true);
#else
	const uint64_t freq = SDL_GetPerformanceFrequency();
	while (g_running)
	{
		uint64_t start = SDL_GetPerformanceCounter();

		game_loop(&context);

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
#endif

	IMG_Quit();

	TTF_CloseFont(test_font); 
	TTF_Quit();

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	
	return EXIT_SUCCESS;
}