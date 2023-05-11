#include "Globals.hpp"

namespace Globals {
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

int BOX_WIDTH = 600 / BOARD_SIZE;
int BOX_HEIGHT = 600 / BOARD_SIZE;

//Initial Position of the Board.
std::array<int, NUM_OF_SQUARES> bitboard = {
    r, n, b, q, k, b, n, r,     o, o, o, o, o, o, o, o,
    p, p, p, p, p, p, p, p,     o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,     o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,     o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,     o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,     o, o, o, o, o, o, o, o,
    P, P, P, P, P, P, P, P,     o, o, o, o, o, o, o, o,
    R, N, B, Q, K, B, N, R,     o, o, o, o, o, o, o, o
};

//By default it's white to move.
int side = white; 

//Define the En Passant Square Position.
int en_passant = no_sq;

//Castling Rights (1111 => 15)
int castling = WHITE_SHORT_CASTLE |
               WHITE_LONG_CASTLE  |
               BLACK_SHORT_CASTLE |
               BLACK_LONG_CASTLE;

} // namespace Globals