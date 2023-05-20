#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <bitset>

namespace Bitboard {
constexpr std::uint8_t BOARD_SIZE = 7;
constexpr std::size_t NUM_OF_SQUARES = 64;

constexpr bool SHOULD_FLIP = false;

//This is used for the FEN parser and the PGN notation.
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
inline int toLSF(int file, int rank, bool should_flip = SHOULD_FLIP) noexcept
{ 
    int lsf = (rank << 3) + file;
    return (should_flip ? lsf ^ 0b111000 : lsf);
}

//Least significant rank mapping.
inline int toLSR(int file, int rank, bool should_flip = SHOULD_FLIP) noexcept
{ 
    int lsr = (file << 3) + rank;
    return (should_flip ? lsr ^ 0b111000 : lsr); 
}

//Check if a piece is a pawn, regardless of their color.
inline bool isPawn(int type) { return type == Pieces::P || type == Pieces::p; }

inline bool isSlidingPiece(int type) { return type == Pieces::B || type == Pieces::b; }

//Convert little-endian file-rank mapping to big-endian file-rank and vice versa.
inline int flipVertically(int lsf) { return lsf ^ 0x00038; }

//Convert a little-endian position to a big-endian LSF and vice versa.
inline int flipVertically(int file, int rank)
{
    const int lsf = toLSF(file, rank); 
    return lsf ^ 0b111000;
}

//Least significant file to coordinates.
[[nodiscard]] inline const SDL_Point lsfToCoord(int lsf, bool should_flip = SHOULD_FLIP) noexcept 
{ 
    int final_lsf = (should_flip ? flipVertically(lsf) : lsf);
    return {final_lsf & BOARD_SIZE, final_lsf >> 3}; 
}

//Least significant rank to coordinates.
[[nodiscard]] inline const SDL_Point lsrToCoord(int lsr, int should_flip = SHOULD_FLIP) noexcept
{
    int final_lsr = (should_flip ? flipVertically(lsr) : lsr);
    return {final_lsr >> 3, final_lsr & BOARD_SIZE}; 
}

inline int getColor(int type) {
    return (type > 6 ? Sides::BLACK : Sides::WHITE);
}

inline int addRank(int lsf, int rank, bool should_flip = SHOULD_FLIP) {
    int final_lsf = (should_flip ? flipVertically(lsf) : lsf);
    const auto coords = lsfToCoord(final_lsf);
    return toLSF(coords.x, coords.y + rank);
}
}