#pragma once

#include <SDL2/SDL.h>
#include "bitboard.hpp"

constexpr unsigned int FPS = 60;
constexpr unsigned int FRAME_DELAY = FPS / 1000;

namespace Globals {
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* texture;

extern int BOX_WIDTH;
extern int BOX_HEIGHT;

extern SDL_Point last_move;
extern SDL_Point current_position;

extern int side;

extern std::vector<int> bitboard;
extern std::vector<int> pseudolegal_moves;

extern std::vector<SDL_Rect> quad_vector;

extern int en_passant;
extern int castling;
extern int selected_lsf;
} // namespace Globals