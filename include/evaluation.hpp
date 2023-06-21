#include <iostream>
#include <vector>
#include <numeric>
#include <array>

#include "globals.hpp"
#include "bitboard.hpp"
#include "move.hpp"

enum MaterialValue
{
    PAWN = 100,

    // Minor Pieces
    KNIGHT = 300,
    BISHOP = 330,

    // Major Pieces
    ROOK = 500,
    QUEEN = 900,

    KING = 20000
};

namespace Evaluation
{
    constexpr int PAWN_CAPTURE_PENALTY = 350;

    int evaluateFactors();

    int getPieceValue(int type);
}