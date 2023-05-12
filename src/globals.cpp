#include "globals.hpp"

using namespace Bitboard;

namespace Globals {
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

//Define the initial box sizes.
int BOX_WIDTH = 75;
int BOX_HEIGHT = 75;

//Initial Position of the Board.
std::vector<int> bitboard = {
    r, n, b, q, k, b, n, r,
    p, p, p, p, p, p, p, p,
    e, e, e, e, e, e, e, e,
    e, e, e, e, e, e, e, e,
    e, e, e, e, e, e, e, e,
    e, e, e, e, e, e, e, e,
    P, P, P, P, P, P, P, P,
    R, N, B, Q, K, B, N, R
};

std::vector<int> pseudolegal_moves = {};

std::vector<SDL_Rect> quad_vector = {};

int side = 0; 

SDL_Point last_move = SDL_Point{Squares::no_sq, Squares::no_sq};

//Define the En Passant Square Position.
int en_passant = Squares::no_sq;

//Castling Rights (1111 => 15)
int castling = WHITE_SHORT_CASTLE |
               WHITE_LONG_CASTLE  |
               BLACK_SHORT_CASTLE |
               BLACK_LONG_CASTLE;

SDL_Point current_position = SDL_Point{0, 0};

int selected_lsf = Squares::no_sq;
} // namespace Globals