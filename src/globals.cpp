#include "globals.hpp"
#include "zobrist_hashing.hpp"
#include "interface.hpp"

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

int square_of_king_in_check = Bitboard::Squares::no_sq;

//This contains the pieces and the square.
//TODO: Consider using a 64-bit unsigned integer instead to represent
//the pieces. To distinguish pieces, we can use 14 bits.

// clang-format off

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

// clang-format on

std::vector<SDL_Point> opponent_occupancy = {};

constexpr std::size_t TOTAL_SQUARES = 64U;

std::bitset<TOTAL_SQUARES> move_bitset;
std::vector<std::tuple<int, int, int>> move_squares = {};

//x - Target square.
//y - Old square where the piece is located.
std::vector<LegalMove> legal_moves = {};
std::vector<LegalMove> move_hints = {};

std::vector<SDL_Rect> quad_vector = {};

std::vector<std::uint64_t> position_history = {};

int side = 0;

//x -> King side castling square.
//y -> Queen side castling square
SDL_Point castling_square = {Squares::no_sq, Squares::no_sq};

std::vector<int> max_squares = {};

std::vector<std::vector<int>> precomputed_max_squares_to_edge = {};

//Keep track of old moves to generate old moves.
std::vector<Ply> ply_array = {};

int last_piece_to_move = -1;

double elapsed_time = 0.0;

SDL_Point linear_interpolant = {0, 0};
SDL_Point scaled_linear_interpolant = {0, 0};

double time = 0.0;

//Define the En Passant Square Position.
int en_passant = Squares::no_sq;
int en_passant_legal_move_index = -1;

bool show_legal_moves = false;

//Castling Rights
int castling = 0;

int game_state = GameState::OPENING;

SDL_Point current_position = SDL_Point{0, 0};

int selected_square = Squares::no_sq;
int halfmove_clock = 0;
int move_delay = 0;

int black_eval = 0;
int white_eval = 0;

int current_move = 0;

//LSB to 2nd LSB: Color of the pawn.
//3rd LSB: Promote to Queen.
//4th LSB: Promote to Bishop.
//5th LSB: Promote to Knight
//6th LSB: Promote to Rook.
unsigned int promotion_squares = 0;

std::shared_ptr<AudioManager> audio_manager = std::make_shared<AudioManager>();
std::shared_ptr<ZobristHashing> zobrist_hashing = std::make_shared<ZobristHashing>();
std::shared_ptr<Interface> interface_handler = std::make_shared<Interface>();

SDL_Point mouse_coord = {0, 0};

}  // namespace Globals