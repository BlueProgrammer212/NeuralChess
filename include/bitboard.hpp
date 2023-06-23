#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <bitset>

namespace Bitboard
{
    constexpr std::uint8_t BOARD_SIZE = 7;
    constexpr int NUM_OF_SQUARES = 64;
    constexpr int MAX_SQUARES_TO_EDGE = Bitboard::BOARD_SIZE + 1;

    constexpr bool SHOULD_FLIP = false;

    // clang-format off

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

    //////////////CASTLING RIGHTS///////////////
    //For black, left-shift 2 for each constants.
    enum Castle : int 
    {
        SHORT_CASTLE  = 1 << 0, //0001
        LONG_CASTLE   = 1 << 1, //0010
    };
    ///////////////////////////////////////////

    // clang-format on

    // Least significant file mapping.
    inline int toSquareIndex(int file, int rank, bool should_flip = SHOULD_FLIP) noexcept
    {
        int square = (rank << 3) + file;
        return (should_flip ? square ^ 0b111000 : square);
    }

    // Least significant rank mapping.
    inline int toLSR(int file, int rank, bool should_flip = SHOULD_FLIP) noexcept
    {
        int lsr = (file << 3) + rank;
        return (should_flip ? lsr ^ 0b111000 : lsr);
    }

    inline bool isPawn(int type) noexcept
    {
        return type == Pieces::P || type == Pieces::p;
    }

    inline bool isKing(int type) noexcept
    {
        return type == Pieces::K || type == Pieces::k;
    }

    inline bool isKnight(int type) noexcept
    {
        return type == Pieces::N || type == Pieces::n;
    }

    //////////////SLIDING PIECES//////////////
    inline bool isBishop(int type) noexcept { return type == Pieces::B || type == Pieces::b; }

    inline bool isRook(int type) noexcept { return type == Pieces::R || type == Pieces::r; }

    inline bool isQueen(int type) noexcept { return type == Pieces::Q || type == Pieces::q; }

    inline bool isSlidingPiece(int type) noexcept
    {
        return isBishop(type) || isRook(type) || isQueen(type);
    }
    //////////////////////////////////////////

    // Convert little-endian file-rank mapping to big-endian file-rank and vice versa.
    inline int flipVertically(int square) { return square ^ 0x00038; }

    // Convert a little-endian position to a big-endian LSF and vice versa.
    inline int flipVertically(int file, int rank)
    {
        const int square = toSquareIndex(file, rank);
        return square ^ 0b111000;
    }

    // Least significant file to coordinates.
    [[nodiscard]] inline const SDL_Point squareToCoord(int square, bool should_flip = SHOULD_FLIP) noexcept
    {
        // clang-format off
        int final_square = (should_flip * flipVertically(square)) |
                           (!should_flip * square);

        return {final_square & BOARD_SIZE, final_square >> 3};
        // clang-format on
    }

    // Least significant rank to coordinates.
    [[nodiscard]] inline const SDL_Point lsrSquareToCoord(int lsr, int should_flip = SHOULD_FLIP) noexcept
    {
        int final_lsr = (should_flip ? flipVertically(lsr) : lsr);
        return {final_lsr >> 3, final_lsr & BOARD_SIZE};
    }

    inline int getColor(int type)
    {
        const bool is_black = type > 6;
        return (is_black * 0b01) | (!is_black * 0b10);
    }

    // Increment the ranks for the LSF. Add 8 to the LSF depending on how much ranks.
    inline int addRank(int square, int rank, bool should_flip = SHOULD_FLIP)
    {
        int final_square = (should_flip ? flipVertically(square) : square);
        const auto coords = squareToCoord(final_square);
        return toSquareIndex(coords.x, coords.y + rank);
    }

    // Check if the coordniate is an empty square
    // The type must be a 2 dimensional vector.
    template <typename T = SDL_Point>
    inline bool isCoordEmpty(const T &vector) noexcept
    {
        return vector.x & Bitboard::Squares::no_sq ||
               vector.y & Bitboard::Squares::no_sq;
    }
} // namespace Bitboard