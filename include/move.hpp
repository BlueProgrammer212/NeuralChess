#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>
#include <array>
#include <string>
#include <utility>

#include "globals.hpp"

struct ImaginaryMove
{
    int old_piece = Bitboard::Squares::no_sq;
    int captured_piece = Bitboard::Squares::no_sq;
    bool old_move_bit = false;
    bool old_move_bit_of_dest = false;

    bool is_castling = false;
    bool is_en_passant = false;

    int en_passant_capture_lsf = Bitboard::Squares::no_sq;
    int en_passant_capture_piece_type = Bitboard::Pieces::e;

    int team = Bitboard::Sides::WHITE;
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

    // These are the types the pawn can promote to.
    // 6 is added to these values for "black".
    constexpr std::array<int, 4> possible_promotions = {2, 3, 4, 5};

    constexpr int KNIGHT_OFFSET_START = 8;
    constexpr int KNIGHT_OFFSET_END = 16;

    // Add the LSF in the legal move array
    void addMove(const int t_lsf, const int old_lsf) noexcept;

    // Set the LSF as an "occupied square"
    void addOccupancySquare(const int t_lsf, const int old_lsf) noexcept;

    // Only render the possible moves of the selected piece.
    void renderMove(const int t_lsf, const int old_lsf) noexcept;

    // Get the distance between the delta lsf and the target LSF.
    int getMaxDeltaSquares(const int delta_lsf, const int lsf_prime);

    // Make an imaginary move and update the bitboard temporarily.
    const ImaginaryMove makeMove(const LegalMove &move);

    // Unmake the imaginary move and restore the old bitboard data.
    void unmakeMove(const LegalMove &move, const ImaginaryMove &data);

    // Translate LSFs into the algebraic notation.
    [[nodiscard]] const std::string toAlgebraicNotation(int type, int old_lsf, int lsf,
                                                        bool is_capture, bool is_a_castling_move, int dx);

    // TODO: Implement pawn underpromotion as a "legal move".
    void pawnPromotion(const int t_lsf);

    // Check if the square does not contain any pieces.
    bool notEmpty(const int t_lsf);

    // Check if we can capture the LSF.
    bool canCapture(const int t_lsf, const bool for_occupied_square = false);

    // En Passant implementation.
    void enPassant(const int t_lsf, const std::function<void(int, int)> moveFunc);

    void generatePawnCaptures(int t_lsf, const std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    const std::vector<int> &getDistToEdge(const SDL_Point pos);

    void generateSlidingMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_square);

    // This is used to check for pieces within the home rank from the king or queen side.
    void isFlankEmpty(int flank, int t_lsf, std::function<void(int, int)> moveFunc);

    // Check if castling is legal or not.
    void generateCastlingMove(int t_lsf, std::function<void(int, int)> moveFunc);

    void generateKnightMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    // Exclude moves that can put the king in check.
    void generateKingMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    // Render pseudo-legal move hints.
    // TODO: Refactor the pawn moves.
    // TODO: Replace the boolean arguments with an unsigned int flag.
    // TODO: Consider not using a switch-case block.
    // TODO: Update pawn promotions.
    inline void searchPseudoLegalMoves(int t_lsf, std::function<void(int, int)> moveFunc,
                                       bool for_occupied_squares = false, bool for_legal_moves = false)
    {
        const int team = Bitboard::getColor(Globals::bitboard[t_lsf]);

        bool check_side = (for_occupied_squares && team & Globals::side) || (!for_occupied_squares && !(team & Globals::side));

        // Skip empty pieces.
        if (Globals::bitboard[t_lsf] == Bitboard::Pieces::e || check_side)
        {
            return;
        }

        const int pawn_range = (Globals::move_bitset[t_lsf] ? 2 : 3);
        int target_square;

        // Generate moves for various types of piece.
        switch (Globals::bitboard[t_lsf])
        {
        case Bitboard::Pieces::P:
            if (!for_occupied_squares)
            {
                for (int i = 1; i < pawn_range; ++i)
                {
                    target_square = Bitboard::addRank(t_lsf, -i);

                    // Halt the iteration if there is an intercepting piece.
                    if (notEmpty(target_square))
                    {
                        break;
                    }

                    if (for_legal_moves && target_square <= 7)
                    {
                        // Possible promotions.

                        int piece_color = (Globals::side & Bitboard::Sides::WHITE ? 0 : 6);

                        for (const int promote_to_type : possible_promotions)
                        {
                            Globals::legal_moves.push_back(LegalMove{
                                target_square,
                                t_lsf,
                                true,
                                promote_to_type + piece_color});
                        }
                    }
                    else
                    {
                        moveFunc(target_square, t_lsf);
                    }
                }

                enPassant(t_lsf, moveFunc);
            }

            // Capture Offsets
            generatePawnCaptures(t_lsf, moveFunc, for_occupied_squares);

            break;
        case Bitboard::Pieces::p:
            if (!for_occupied_squares)
            {
                for (int i = 1; i < pawn_range; ++i)
                {
                    target_square = Bitboard::addRank(t_lsf, i);

                    // Halt the iteration if there is an intercepting piece.
                    if (notEmpty(target_square))
                        break;

                    if (for_legal_moves && target_square >= 56)
                    {
                        // Possible promotions.

                        int piece_color = (Globals::side & Bitboard::Sides::WHITE ? 0 : 6);

                        for (const int promote_to_type : possible_promotions)
                        {
                            Globals::legal_moves.push_back(LegalMove{
                                target_square,
                                t_lsf,
                                true,
                                promote_to_type + piece_color});
                        }
                    }
                    else
                    {
                        moveFunc(target_square, t_lsf);
                    }
                }

                enPassant(t_lsf, moveFunc);
            }

            // Capture Offsets
            generatePawnCaptures(t_lsf, moveFunc, for_occupied_squares);

            break;
        case Bitboard::Pieces::N:
            generateKnightMoves(t_lsf, moveFunc, for_occupied_squares);
            break;
        case Bitboard::Pieces::n:
            generateKnightMoves(t_lsf, moveFunc, for_occupied_squares);
            break;

        case Bitboard::Pieces::K:
            generateKingMoves(t_lsf, moveFunc, for_occupied_squares);
            if (!for_occupied_squares)
            {
                generateCastlingMove(t_lsf, moveFunc);
            }
            break;

        case Bitboard::Pieces::k:
            generateKingMoves(t_lsf, moveFunc, for_occupied_squares);
            if (!for_occupied_squares)
            {
                generateCastlingMove(t_lsf, moveFunc);
            }
            break;
        }

        if (Bitboard::isSlidingPiece(Globals::bitboard[t_lsf]))
        {
            generateSlidingMoves(t_lsf, moveFunc, for_occupied_squares);
        }
    }

    inline void searchForOccupiedSquares()
    {
        // Reset the occupancy squares data.
        Globals::opponent_occupancy.clear();

        for (int t_lsf = 0; t_lsf < Bitboard::Squares::h8 + 1; ++t_lsf)
        {
            const int piece_color = Bitboard::getColor(Globals::bitboard[t_lsf]);

            if (Globals::bitboard[t_lsf] == Bitboard::Pieces::e || piece_color & Globals::side)
            {
                continue;
            }

            searchPseudoLegalMoves(t_lsf, &addOccupancySquare, true);
        }
    }

    // Scan the whole bitboard to find the king.
    inline int getOwnKing()
    {
        // Yields {1, 7} depending on the player to move.
        const int piece_type = ((Globals::side & 0b01) * 7) | ((~Globals::side & 0b01) * 1);

        for (int lsf = 0; lsf < Bitboard::Squares::h8 + 1; ++lsf)
        {
            if (Globals::bitboard[lsf] == piece_type)
            {
                return lsf;
            }
        }

        return Bitboard::Squares::no_sq;
    }

    inline bool isInCheck(int t_lsf)
    {
        int king = getOwnKing();
        MoveGenerator::searchForOccupiedSquares();

        for (SDL_Point occupied_sq : Globals::opponent_occupancy)
        {
            // If any squares matches with the king's LSF, then the king must in check.
            if (occupied_sq.x == king)
            {
                return true;
            }
        }

        return false;
    }

    inline void filterPseudoLegalMoves(const int t_lsf, std::vector<LegalMove> &hint_lsf_array)
    {
        int king = getOwnKing();

        if (!notEmpty(t_lsf))
        {
            return;
        }

        const std::vector<LegalMove> legal_moves_copy = hint_lsf_array;

        // Loop through all of the legal moves.
        for (int i = 0; i < static_cast<int>(hint_lsf_array.size()); ++i)
        {
            const LegalMove &move = legal_moves_copy[i];

            // If the move is deleted, ignore it.
            if (move.x & Bitboard::Squares::no_sq)
            {
                continue;
            }

            // Make the move and record the captured piece and old move bits to unmake the move.
            const auto &move_data = makeMove(move);

            // Delete the move if it puts the king in check.
            if (isInCheck(king))
            {
                hint_lsf_array[i].x = Bitboard::Squares::no_sq;
            };

            // Restore the initial bitboard data.
            unmakeMove(move, move_data);

            searchForOccupiedSquares();
        }
    }

    inline const std::vector<LegalMove> &generateLegalMoves()
    {
        Globals::legal_moves.clear();
        for (int i = 0; i < Bitboard::Squares::h8 + 1; ++i)
        {
            if (Globals::bitboard[i] == Bitboard::Pieces::e ||
                !(Globals::side & Bitboard::getColor(Globals::bitboard[i])))
            {
                continue;
            }

            searchPseudoLegalMoves(i, &addMove, false, true);
            filterPseudoLegalMoves(i, Globals::legal_moves);
        }

        return Globals::legal_moves;
    }
}; // namespace MoveGenerator