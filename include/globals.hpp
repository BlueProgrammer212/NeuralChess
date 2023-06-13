#pragma once

#include <SDL2/SDL.h>
#include "bitboard.hpp"
#include "audio_manager.hpp"
#include <memory>

constexpr unsigned int FPS = 60;
constexpr unsigned int FRAME_DELAY = FPS / 1000;

enum GameState : int
{
    OPENING = 1 << 0,
    MIDDLEGAME = 1 << 1,
    ENDGAME = 1 << 2,

    DRAW = 1 << 3,
    CHECKMATE = 1 << 4,
};

struct Ply
{
    // x -> Old square
    // y -> Target square
    SDL_Point move;

    bool is_capture{false};
    int old_piece_in_dest{Bitboard::Squares::no_sq};
    int moved_piece{Bitboard::Pieces::e};

    bool old_move_bit{false};
    bool old_move_bit_in_dest{false};
};

struct LegalMove
{
    int x{Bitboard::Squares::no_sq};
    int y{Bitboard::Squares::no_sq};

    bool is_pawn_promotion{false};

    // The pawn will be promoted to a queen by default.
    int will_promote_to{Bitboard::Pieces::Q};
};

struct PerftData
{
    int num_of_positions = 0;
    int num_of_captures = 0;
    int num_of_checks = 0;
    int num_of_en_passant = 0;
    int num_of_castles = 0;
    int num_of_promotions = 0;
};

namespace Globals
{
    extern SDL_Window *window;
    extern SDL_Renderer *renderer;
    extern SDL_Texture *texture;

    extern SDL_Point current_position;

    // Preferences
    extern bool display_legal_move_hints;

    extern int side;
    extern int en_passant;
    extern int castling;
    extern unsigned int promotion_lsfs;

    extern int selected_lsf;

    extern int game_state;

    extern int BOX_WIDTH;
    extern int BOX_HEIGHT;

    extern int black_eval;

    // Bilinear interpolation
    extern double time;
    extern double elapsed_time;

    extern SDL_Point linear_interpolant;
    extern SDL_Point scaled_linear_interpolant;

    extern std::vector<int> bitboard;

    extern std::vector<LegalMove> legal_moves;
    extern std::vector<LegalMove> move_hints;

    // This is array of LSF for controlled squares.
    extern std::vector<SDL_Point> opponent_occupancy;

    extern std::vector<int> opponent_pseudolegal_moves;
    extern std::vector<int> max_squares;
    extern std::bitset<64U> move_bitset;

    extern std::vector<Ply> ply_array;

    extern std::vector<SDL_Rect> quad_vector;

    extern bool is_in_check;
    extern bool should_show_promotion_dialog;

    extern int lsf_of_king_in_check;

    extern SDL_Point castling_square;

    // This is useful for the 50-move rule.
    extern int halfmove_clock;

    extern int move_delay;

    extern int current_move;

    extern std::shared_ptr<AudioManager> audio_manager;

    //void createWindow(const char *title, int width, int height);

    extern int window_count;

    // Interactability
    extern bool is_mouse_down;
    extern SDL_Point mouse_coord;
} // namespace Globals