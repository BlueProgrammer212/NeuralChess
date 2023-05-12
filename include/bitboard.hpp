#pragma once

#include <iostream>
#include <vector>
#include <array>

namespace Bitboard {
constexpr std::uint8_t BOARD_SIZE = 7;
constexpr std::size_t NUM_OF_SQUARES = 64;

constexpr const char* ascii_pieces = ".KQBNRPkqbnrp";

enum Sides { 
    BLACK = 1 << 0, 
    WHITE = 1 << 1 
};

//The order depends on the Spritesheet.
enum Pieces : int {e, K, Q, B, N, R, P, k, q, b, n, r, p};

//Little-endian file-rank mapping enumeration.
enum Squares : int 
{
    a1, a2, a3, a4, a5, a6, a7, a8,
    b1, b2, b3, b4, b5, b6, b7, b8,
    c1, c2, c3, c4, c5, c6, c7, c8,
    d1, d2, d3, d4, d5, d6, d7, d8,
    e1, e2, e3, e4, e5, e6, e7, e8,
    f1, f2, f3, f4, f5, f6, f7, f8,
    g1, g2, g3, g4, g5, g6, g7, g8,
    h1, h2, h3, h4, h5, h6, h7, h8, no_sq
};

enum Castle : int 
{
    WHITE_SHORT_CASTLE  = 1 << 0,
    WHITE_LONG_CASTLE   = 1 << 1,
    BLACK_SHORT_CASTLE  = 1 << 2,
    BLACK_LONG_CASTLE   = 1 << 3
};

//Least significant file mapping.
inline int toLSF(int file, int rank) {
    return (rank << 3) + file;
}

//Least significant rank mapping.
inline int toLSR(int file, int rank) {
    return (file << 3) + rank;
}

//Least significant file to coordinates.
inline SDL_Point lsfToCoord(int lsf) {
    return SDL_Point{lsf & BOARD_SIZE, lsf >> 3};
}

//Least significant rank to coordinates.
inline SDL_Point lsrToCoord(int lsr) {
    return SDL_Point{lsr & BOARD_SIZE, lsr >> 3};
}

inline int convertEndianess(int file, int rank) {
    int new_file = file ^ BOARD_SIZE;
    int new_rank = rank ^ 56;

    int target_square = toLSF(new_file, new_rank);

    return target_square;
}

inline int convertEndianess(int lsf) {
    const auto coords = lsfToCoord(lsf);
    return toLSF(coords.x ^ BOARD_SIZE, coords.y ^ 56);
}

inline int getColor(int type) {
    return (type > 6 ? Sides::BLACK : Sides::WHITE);
}
}