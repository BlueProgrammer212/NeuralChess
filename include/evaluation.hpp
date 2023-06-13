#include <iostream>
#include <vector>

#include "globals.hpp"
#include "bitboard.hpp"

enum MaterialValue {
    PAWN = 1,
    
    //Minor Pieces
    BISHOP = 3,
    KNIGHT = 3,

    //Major Pieces
    ROOK = 5,
    QUEEN = 9,

    KING = INT32_MAX
};

namespace Evaluation {
    extern int pawn_count;
    extern int bishop_count;
    extern int knight_count;
    extern int rook_count;
    extern int queen_count;

    //Depth = 0
    int countMaterial();
}