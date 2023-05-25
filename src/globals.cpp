#include "globals.hpp"

using namespace Bitboard;

namespace Globals {
std::vector<SDL_Window*> window_set = {};
int window_count = 0;

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

std::vector<int> opponent_occupancy = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

std::bitset<Bitboard::NUM_OF_SQUARES> move_bitset;

std::vector<int> pseudolegal_moves = {};
std::vector<int> move_hints = {};

std::vector<SDL_Rect> quad_vector = {};

int side = 0; 
std::vector<int> castling_square = {-1, -1};

SDL_Point last_move = SDL_Point{Squares::no_sq, Squares::no_sq};

double recorded_time = 0.0;

SDL_Point linear_interpolant = {0, 0};
SDL_Point scaled_linear_interpolant = {0, 0};

double time = 0.0;

//Define the En Passant Square Position.
int en_passant = Squares::no_sq;

//Castling Rights (1111 => 15)
int castling = WHITE_SHORT_CASTLE |
               WHITE_LONG_CASTLE  |
               BLACK_SHORT_CASTLE |
               BLACK_LONG_CASTLE;

SDL_Point current_position = SDL_Point{0, 0};

int selected_lsf = Squares::no_sq;

void addWindow(const char* title, int width, int height) {
    window_set.push_back(SDL_CreateWindow(
        title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_FOCUS));

    if (window_set.at(window_count++) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create the window");
    }
}
} // namespace Globals