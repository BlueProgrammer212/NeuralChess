#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <array>

#include "globals.hpp"
#include "bitboard.hpp"
#include "move.hpp"

enum MaterialValue : int
{
    PAWN = 100,

    // Minor Pieces
    KNIGHT = 300,
    BISHOP = 320,

    // Major Pieces
    ROOK = 500,
    QUEEN = 900,

    KING = 20000
};

namespace Evaluation
{
    constexpr int PAWN_CAPTURE_PENALTY = 350;
    constexpr int LOSING_CASTLING_RIGHTS_PENALTY = 350;

    struct Factors
    {
        int material_white = 0;
        int material_black = 0;

        int doubled_pawn_structure_white = 0;
        int doubled_pawn_structure_black = 0;

        int blocked_pawns_white = 0;
        int blocked_pawns_black = 0;

        int piece_square_table_white = 0;
        int piece_square_table_black = 0;
    };

    const int evaluateFactors();

    const int getPieceValue(const int type);
    const std::array<int, 64> &getPieceSquareTable(const int type);

    int getSquareValue(const int side, int square, const int type);

    extern std::array<std::array<int, 64>, 7> PIECE_SQUARE_TABLES;
    extern int endgame_weight;
}