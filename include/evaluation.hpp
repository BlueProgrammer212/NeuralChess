#include <iostream>
#include <vector>

#include "globals.hpp"
#include "bitboard.hpp"

enum MaterialValue {
    PAWN = 100,
    
    //Minor Pieces
    BISHOP = 300,
    KNIGHT = 300,

    //Major Pieces
    ROOK = 500,
    QUEEN = 900,

    KING = INT_MAX
};

namespace Evaluation {
    int evaluateMaterial();
}