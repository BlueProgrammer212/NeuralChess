#include <iostream>
#include <vector>
#include <array>

constexpr std::uint8_t BOARD_SIZE = 8;
constexpr std::size_t NUM_OF_SQUARES = 128;

constexpr const char* ascii_pieces = ".KQBNRPkqbnrp";

enum Sides { WHITE, BLACK };

//The order depends on the Spritesheet.
enum Pieces : int {e, K, Q, B, N, R, P, k, q, b, n, r, p, o};

enum Squares : int {
    a8 = 0,   b8, c8, d8, e8, f8, g8, h8,
    a7 = 16,  b7, c7, d7, e7, f7, g7, h7,
    a6 = 32,  b6, c6, d6, e6, f6, g6, h6,
    a5 = 48,  b5, c5, d5, e5, f5, g5, h5,
    a4 = 64,  b4, c4, d4, e4, f4, g4, h4,
    a3 = 80,  b3, c3, d3, e3, f3, g3, h3,
    a2 = 96,  b2, c2, d2, e2, f2, g2, h2,
    a1 = 112, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum Castle : int {
    WHITE_SHORT_CASTLE  = 1 << 0,
    WHITE_LONG_CASTLE   = 1 << 1,
    BLACK_SHORT_CASTLE  = 1 << 2,
    BLACK_LONG_CASTLE   = 1 << 3
};