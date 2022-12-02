#include <iostream>
#include <vector>
#include <random>
#include <filesystem>

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
};

class Minesweeper {
public:
	int width, height;
	int bombcount;

	std::vector<std::vector<Tile>> Tilemap;

	Minesweeper(int width, int height, int bombcount) 
		: width(width), height(height)
	{
		this->Tilemap = std::vector<std::vector<Tile>>( 1 + height + 1, std::vector<Tile>(1 + width + 1, Tile() ) );

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

			Tile& tile = this->Tilemap[r_height][r_width];

			if(tile.data != TILE_BOMB) {
				tile.data = TILE_BOMB;
				placed_bombs++;
			}
		}

		for(int i = 1; i <= height; i++) {
			for(int j = 1; j <= width; j++) {
				if(this->Tilemap[i][j].data == TILE_BOMB)
					continue;

				const std::vector<Tile> neighbors = {
					this->Tilemap[i - 1][j - 1], this->Tilemap[i - 1][j], this->Tilemap[i - 1][j + 1],
					this->Tilemap[i][j - 1],   /*this->Tilemap[i][j],*/   this->Tilemap[i][j + 1],
					this->Tilemap[i + 1][j - 1], this->Tilemap[i + 1][j], this->Tilemap[i + 1][j + 1]
				};

				int tile_number = 0;
				for(const Tile& neighbor : neighbors) {
					if(neighbor.data == TILE_BOMB)
						tile_number++;
				}

				this->Tilemap[i][j].data = (TileData)tile_number;
			}
		}
	}

	void print_minefield()
	{
		for(int i = 0; i <= this->height + 1; i++) {
			for(int j = 0; j <= this->width + 1; j++) {
				if(this->Tilemap[i][j].data == TILE_BOMB) {
					printf("\033[41mB\033[0m ");
				} else if(this->Tilemap[i][j].data == TILE_EMPTY) {
					printf("  ");
				} else {
					printf("%i ", this->Tilemap[i][j].data);
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

bool rmb_isdown;
bool rmb_wasdown;

bool lmb_isdown;
bool lmb_wasdown;

bool pixel_to_tile(int x, int y)
{
	
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
			{
				SDL_MouseButtonEvent mouse_event = event.button;
				int x, y;
				int button = SDL_GetMouseState( &x, &y );
				if(button & SDL_BUTTON_LMASK) {
					lmb_isdown = true;
					if(!lmb_wasdown) {
						lmb_wasdown = true;
						printf("lmb down\n");
					}
				}
				if(button & SDL_BUTTON_RMASK) {
					rmb_isdown = true;
					if(!rmb_wasdown) {
						rmb_wasdown = true;
						printf("rmb down\n");
					}
				}
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				SDL_MouseButtonEvent mouse_event = event.button;
				int x, y;
				int button = SDL_GetMouseState( &x, &y );
				if(!(button & SDL_BUTTON_LMASK) && lmb_wasdown) {
					lmb_isdown = false;
					lmb_wasdown = false;
					printf("lmb up\n");
				}
				if(!(button & SDL_BUTTON_RMASK) && rmb_wasdown) {
					rmb_isdown = false;
					rmb_wasdown = false;
					printf("rmb up\n");
				}
				break;
			}

			default:
				break;
		}
	}
		
}

SDL_Texture* render_number_texture(TTF_Font* font, const char* text, SDL_Color color = {0,0,0})
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);

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
		render_number_texture(test_font, "1"), render_number_texture(test_font, "2"), render_number_texture(test_font, "3"),
		render_number_texture(test_font, "4"), render_number_texture(test_font, "5"), render_number_texture(test_font, "6"),
		render_number_texture(test_font, "7"), render_number_texture(test_font, "8")
	};

	SDL_Texture* bomb_texture = load_bomb_texture();

	Minesweeper game(30, 16, 99);

	const int minimum_h = (AREA_START * 4) + game.height * tile_h;
	const int minimum_w = (AREA_START * 4) + game.width  * tile_w;
	SDL_SetWindowMinimumSize(g_window, minimum_w, minimum_h);

	printf("min height %i\n", minimum_h);

	while (g_running)
	{
		handle_input();
		SDL_RenderClear(g_renderer);

		for(int i = 0; auto* texture : number_textures) {
			int w, h = 0;
			SDL_QueryTexture(texture, NULL, NULL, &w, &h);
			//printf("w %i, h %i\n", w, h);
			const SDL_Rect debug_rect = {
				.x = AREA_START + (i * w),
				.y = 520,
				.w = 20,
				.h = 29
			};

			SDL_RenderCopy(g_renderer, texture, NULL, &debug_rect);
			i++;
		}

		for(int column = 1; column <= game.height; column++) 
		{
			for(int row = 1; row <= game.width; row++) 
			{
				const int xpos = AREA_START + (row    - 1) * tile_w + (row    - 1);
				const int ypos = AREA_START + (column - 1) * tile_h + (column - 1);
				Tile& tile = game.Tilemap[column][row];
				const SDL_Rect bound_rect = {
					.x = xpos,
					.y = ypos,
					.w = tile_w,
					.h = tile_h
				};
				if(tile.open) {
					if(tile.data != TILE_BOMB && tile.data != TILE_EMPTY) {
						const SDL_Rect number_rect = {
							.x = xpos + 4,
							.y = ypos,
							.w = 20, 
							.h = 29
						};
						SDL_RenderCopy(g_renderer, number_textures[tile.data - 1], NULL, &number_rect);
					} else if(tile.data == TILE_BOMB) {
						const SDL_Rect bomb_rect = {
							.x = bound_rect.x + 3,
							.y = bound_rect.y + 3,
							.w = tile_w - 6,
							.h = tile_h - 6
						};
						SDL_RenderCopy(g_renderer, bomb_texture, NULL, &bomb_rect);
					}
				} else {
					RenderFilledRectWithColor(g_renderer, &bound_rect, 127, 127, 127);
				}

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