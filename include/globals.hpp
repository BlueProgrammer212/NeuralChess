#pragma once

#include <SDL2/SDL.h>
#include "bitboard.hpp"

constexpr unsigned int FPS = 60;
constexpr unsigned int FRAME_DELAY = FPS / 1000;

namespace Globals
{
    extern std::vector<SDL_Window *> window_set;
    extern SDL_Renderer *renderer;
    extern SDL_Texture *texture;

    extern int BOX_WIDTH;
    extern int BOX_HEIGHT;

    extern SDL_Point last_move;
    extern SDL_Point current_position;

    extern int side;
    extern double time;

    extern std::vector<int> bitboard;

    extern std::vector<int> pseudolegal_moves;
    extern std::vector<int> move_hints;

    extern std::bitset<64> move_bitset;

    extern std::vector<SDL_Rect> quad_vector;

    extern double recorded_time;
    extern SDL_Point linear_interpolant;
    extern SDL_Point scaled_linear_interpolant;

    extern int en_passant;
    extern int castling;
    extern int selected_lsf;

    extern std::vector<int> opponent_occupancy;

    extern std::vector<int> castling_square;
    extern std::vector<int> max_squares;

    void addWindow(const char *title, int width, int height);
    extern int window_count;
} // namespace Globals