#pragma once

#include <SDL2/SDL.h>
#include "bitboard.hpp"

constexpr unsigned int FPS = 60;
constexpr unsigned int FRAME_DELAY = FPS / 1000;

namespace Globals {
extern std::vector<SDL_Window*> window_set;
extern SDL_Renderer* renderer;
extern SDL_Texture* texture;

extern int BOX_WIDTH;
extern int BOX_HEIGHT;

extern SDL_Point last_move;
extern SDL_Point current_position;

extern int side;

extern std::vector<int> bitboard;

extern std::vector<int> pseudolegal_moves;
extern std::vector<int> move_hints;

extern std::bitset<Bitboard::NUM_OF_SQUARES> move_bitset;

extern std::vector<SDL_Rect> quad_vector;

extern int en_passant;
extern int castling;
extern int selected_lsf;

void addWindow(const char* title, int width, int height);
extern int window_count;
} // namespace Globals