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

namespace Globals
{
    extern std::vector<SDL_Window *> window_set;
    extern SDL_Renderer *renderer;
    extern SDL_Texture *texture;

    extern SDL_Point last_move;
    extern SDL_Point last_ply;

    extern SDL_Point current_position;

    extern int side;
    extern int en_passant;
    extern int castling;
    extern int selected_lsf;

    extern int last_piece_to_move;
    extern int game_state;

    extern int BOX_WIDTH;
    extern int BOX_HEIGHT;

    extern int black_eval;

    //Bilinear interpolation
    extern double time;
    extern double elapsed_time;

    extern SDL_Point linear_interpolant;
    extern SDL_Point scaled_linear_interpolant;

    extern std::vector<int> bitboard;

    extern std::vector<SDL_Point> legal_moves;
    extern std::vector<SDL_Point> move_hints;
    
    //This is array of LSF for controlled squares.
    extern std::vector<SDL_Point> opponent_occupancy;

    extern std::vector<int> opponent_pseudolegal_moves;
    extern std::vector<int> max_squares;
    extern std::bitset<64U> move_bitset;

    extern std::vector<SDL_Rect> quad_vector;

    extern bool is_in_check;
    extern bool should_show_promotion_dialog;

    extern int lsf_of_king_in_check;

    extern SDL_Point castling_square;

    // This is useful for the 50-move rule.
    extern int halfmove_clock;

    extern int move_delay;

    extern std::shared_ptr<AudioManager> audio_manager;

    void createWindow(const char *title, int width, int height);

    extern int window_count;
} // namespace Globals