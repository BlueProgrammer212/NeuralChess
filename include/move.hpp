#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>
#include <array>
#include <string>
#include <utility>
#include <numeric>

#include "globals.hpp"

struct ImaginaryMove
{
    int old_piece = Bitboard::Squares::no_sq;
    int captured_piece = Bitboard::Squares::no_sq;
    bool old_move_bit = false;
    bool old_move_bit_of_dest = false;

    bool is_castling = false;
    bool is_en_passant = false;

    int en_passant_capture_square = Bitboard::Squares::no_sq;
    int en_passant_capture_piece_type = Bitboard::Pieces::e;

    int team = Bitboard::Sides::WHITE;
    int old_half_move_clock = 0;
};

namespace MoveGenerator
{
    const std::vector<int> OFFSETS = {
        // Rook (R, L, B, T)
        1, -1, 8, -8,
        // Bishop (SE, SW, NE, NW)
        9, 7, -7, -9,
        // Knights (North)
        -17, -15, -10, -6,
        // South
        17, 15, 10, 6};

    constexpr int KNIGHT_OFFSET_START = 8;
    constexpr int KNIGHT_OFFSET_END = 16;

    constexpr int HALFMOVE_CLOCK_THRESHOLD = 50;

    enum OccupiedSquareMapFlags
    {
        PAWN_OCCUPIED_SQUARES_MAP = 1 << 1,
        KING_OCCUPIED_SQUARES_MAP = 1 << 2,
        OPPONENT_OCCUPIED_SQUARES_MAP = 1 << 3,
        PLAYER_TO_MOVE_OCCUPIED_SQUARES_MAP = 1 << 4
    };

    // Add the square in the legal move array
    void
    addMove(const int t_square, const int old_square) noexcept;

    // Set the square as an "occupied square"
    void addOccupancySquare(const int t_square, const int old_square) noexcept;

    // Only render the possible moves of the selected piece.
    void renderMove(const int t_square, const int old_square) noexcept;

    // Get the distance between the delta square and the target square.
    int getMaxDeltaSquares(const int delta_square, const int square_prime);

    // Make an imaginary move and update the bitboard temporarily.
    const ImaginaryMove makeMove(const LegalMove &move);

    // Unmake the imaginary move and restore the old bitboard data.
    void unmakeMove(const LegalMove &move, const ImaginaryMove &data);

    // Translate squares into the algebraic notation.
    [[nodiscard]] const std::string toAlgebraicNotation(int type, int old_square, int square,
                                                        bool is_capture, bool is_a_castling_move, int dx);

    // TODO: Implement pawn underpromotion as a "legal move".
    void pawnPromotion(const int t_square);

    // Check if the square does not contain any pieces.
    bool notEmpty(const int t_square);

    // Check if we can capture the square.
    bool canCapture(const int t_square, const bool for_occupied_square = false);

    // En Passant implementation.
    void enPassant(const int t_square, const std::function<void(int, int)> moveFunc);

    void generatePawnCaptures(int t_square, const std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    const std::vector<int> &getDistToEdge(const SDL_Point pos);

    void precomputeMaxSquaresToEdge();

    void generateSlidingMoves(int t_square, std::function<void(int, int)> moveFunc, bool for_occupied_square);

    // This is used to check for pieces within the home rank from the king or queen side.
    void isFlankEmpty(int flank, int t_square, std::function<void(int, int)> moveFunc);

    // Check if castling is legal or not.
    void generateCastlingMove(int t_square, std::function<void(int, int)> moveFunc);

    void generateKnightMoves(int t_square, std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    // Exclude moves that can put the king in check.
    void generateKingMoves(int t_square, std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    // Render pseudo-legal move hints.
    // TODO: Refactor the pawn moves.
    // TODO: Replace the boolean arguments with an unsigned int flag.
    // TODO: Update pawn promotions.
    void searchPseudoLegalMoves(const int t_square, std::function<void(int, int)> moveFunc,
                                bool for_occupied_squares = false, bool for_legal_moves = false, bool only_captures = false);

    void searchForOccupiedSquares(int filter = OPPONENT_OCCUPIED_SQUARES_MAP);

    // Scan the whole bitboard to find the king.
    const int getOwnKing();

    const bool isInCheck();

    void filterPseudoLegalMoves(std::vector<LegalMove> &hint_square_array, bool only_captures = false);

    std::vector<LegalMove> &generateLegalMoves(const bool only_captures = false);

    // Check for possible terminations.
    inline const bool noMoreLegalMove();

    const bool isInTerminalCondition();
    const bool isCheckmate();

    const bool isStalemate();
    const bool isInsufficientMaterial();
    const bool isThreefoldRepetition();
    const bool isFiftyMoveRule();
}; // namespace MoveGenerator