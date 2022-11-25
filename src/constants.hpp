#pragma once

constexpr static int SCREEN_WIDTH = 800;
constexpr static int SCREEN_HEIGHT = 600;

constexpr static int GLOBAL_SCALE = 3;

constexpr int AREA_WIDTH  = 10;
constexpr int AREA_HEIGHT = 10;
constexpr int AREA_START = 15;
constexpr int tile_w = 10 * GLOBAL_SCALE;
constexpr int tile_h = 10 * GLOBAL_SCALE;

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