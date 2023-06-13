#include "globals.hpp"

using namespace Bitboard;

namespace Globals {
SDL_Window* window;
int window_count = 0;

SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

//Define the initial box sizes.
int BOX_WIDTH = 75;
int BOX_HEIGHT = 75;

bool is_in_check = false;
bool should_show_promotion_dialog = false;
bool display_legal_move_hints = true;

bool is_mouse_down = false;

int lsf_of_king_in_check = Bitboard::Squares::no_sq;

//This contains the pieces and the LSF.
//TODO: Consider using a 64-bit unsigned integer instead to represent
//the pieces. To distinguish pieces, we can use 14 bits.

std::vector<int> bitboard = {
    r, n, b, q, k, b, n, r,
    p, p, p, p, p, p, p, p,
    e, e, e, e, e, e, e, e,
    e, e, e, e, e, e, e, e,
    e, e, e, e, e, e, e, e,
    e, e, e, e, e, e, e, e,
    P, P, P, P, P, P, P, P,
    R, N, B, Q, K, B, N, R,
};

std::vector<SDL_Point> opponent_occupancy = {};

constexpr std::size_t TOTAL_SQUARES = 64U;

std::bitset<TOTAL_SQUARES> move_bitset;

//x - Target LSF.
//y - Old LSF where the piece is located.
std::vector<LegalMove> legal_moves = {};
std::vector<LegalMove> move_hints = {};

std::vector<SDL_Rect> quad_vector = {};

int side = 0;

//x -> King side castling square.
//y -> Queen side castling square
SDL_Point castling_square = {Squares::no_sq, Squares::no_sq};

std::vector<int> max_squares = {};

//Keep track of old moves to generate old moves.
std::vector<Ply> ply_array = {};

int last_piece_to_move = -1;

double elapsed_time = 0.0;

SDL_Point linear_interpolant = {0, 0};
SDL_Point scaled_linear_interpolant = {0, 0};

double time = 0.0;

//Define the En Passant Square Position.
int en_passant = Squares::no_sq;

//Castling Rights
int castling = 0;

int game_state = GameState::OPENING;

SDL_Point current_position = SDL_Point{0, 0};

int selected_lsf = Squares::no_sq;

int halfmove_clock = 0;

int move_delay = 0;

int black_eval = 30;

int current_move = 0;

unsigned int promotion_lsf = 0;

std::shared_ptr<AudioManager> audio_manager = std::make_shared<AudioManager>();

SDL_Point mouse_coord = {0, 0};
}  // namespace Globals