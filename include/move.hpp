#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>

#include "globals.hpp"

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

    inline void addMove(int lsf) noexcept
    { // Add a move to the LSF array.
        if (lsf < 0 || lsf > 63)
            return;
        Globals::pseudolegal_moves.push_back(lsf);
    }

    constexpr int IS_OCCUPIED = 0b10;

    // Set the LSF as an "occupied square"
    inline void setOccupiedSquare(int lsf) noexcept
    {
        Globals::opponent_occupancy[lsf] = IS_OCCUPIED;
    }

    // Only render the possible moves of the selected piece.
    inline void renderMove(int lsf) noexcept
    {
        Globals::move_hints.push_back(lsf);
    }

    inline void pawnPromotion(int lsf)
    {
        int piece_color = Bitboard::getColor(Globals::bitboard[lsf]);

        int new_type = (piece_color & Bitboard::Sides::WHITE ? Bitboard::Pieces::Q : Bitboard::Pieces::q);

        if (Bitboard::isPawn(Globals::bitboard[lsf]) &&
            ((lsf >> 3 == 0 && piece_color & Bitboard::Sides::WHITE) ||
             (lsf >> 3 == Bitboard::BOARD_SIZE && piece_color & Bitboard::Sides::BLACK)))
        {
            // Globals::addWindow("Pawn Promotion", 700, 400);
            //  Auto queen feature.
            Globals::bitboard[lsf] = new_type;
        }
    }

    inline bool notEmpty(int lsf)
    {
        return Globals::bitboard[lsf] != Bitboard::Pieces::e;
    }

    inline bool canCapture(int lsf)
    {
        bool is_allies = Globals::side & Bitboard::getColor(Globals::bitboard[lsf]);
        return !is_allies && notEmpty(lsf);
    }

    inline void generatePawnCaptures(int lsf, std::function<void(int)> moveFunc)
    {
        int pawn = Globals::bitboard[lsf];
        if (!Bitboard::isPawn(pawn))
            return;

        int color = Bitboard::getColor(pawn);

        int direction_offset_start = (color & Bitboard::Sides::WHITE ? 6 : 4);
        int direction_offset_end = (color & Bitboard::Sides::WHITE ? 8 : 6);

        for (int i = direction_offset_start; i < direction_offset_end; ++i)
        {
            const int dlsf = lsf + OFFSETS[i];

            if (canCapture(dlsf))
            {
                moveFunc(dlsf);
            }
        }
    }

    inline void generateSlidingMoves(int lsf, std::function<void(int)> moveFunc,
                                     bool for_occupied_squares)
    {
        int sliding_piece = Globals::bitboard[lsf];

        if (!Bitboard::isSlidingPiece(sliding_piece))
            return;

        int start_index = (Bitboard::isBishop(sliding_piece) ? 4 : 0);
        int end_index = (Bitboard::isRook(sliding_piece) ? 4 : 8);

        SDL_Point pos = Bitboard::lsfToCoord(lsf);

        constexpr int MAX_SQUARES_TO_EDGE = Bitboard::BOARD_SIZE + 1;

        int right = MAX_SQUARES_TO_EDGE - pos.x;
        int left = pos.x + 1;
        int top = pos.y + 1;
        int bottom = MAX_SQUARES_TO_EDGE - pos.y;

        std::vector<int> max_squares = {
            right,  // Right
            left,   // Left
            bottom, // Bottom
            top,    // Top

            std::min(bottom, right), // South East
            std::min(bottom, left),  // South West
            std::min(top, right),    // North East
            std::min(top, left),     // North West
        };

        for (int i = start_index; i < end_index; ++i)
        {
            for (int target_sq = 1; target_sq < max_squares[i]; target_sq++)
            {
                int dlsf = target_sq * OFFSETS[i] + lsf;

                if (notEmpty(dlsf) && !for_occupied_squares)
                {
                    if (canCapture(dlsf))
                        moveFunc(dlsf);
                    break;
                }

                moveFunc(dlsf);
            }
        }
    }

    inline void allowCastling(int side) { Globals::castling |= side; }

    inline void revokeCastlingRights(int side) { Globals::castling &= ~side; }

    inline void generateCastlingMove(int lsf, std::function<void(int)> moveFunc)
    {
        int king_color = Bitboard::getColor(Globals::bitboard[lsf]);

        if (!Globals::move_bitset[lsf])
        {
            const int short_castle_rook = Globals::bitboard[lsf + 3];
            const int long_castle_rook = Globals::bitboard[lsf - 4];
            std::cout << Bitboard::isRook(short_castle_rook) << "\n";

            if (Globals::castling & Bitboard::Castle::WHITE_SHORT_CASTLE &&
                Bitboard::isRook(short_castle_rook) &&
                king_color & Bitboard::getColor(short_castle_rook))
            {
                Globals::castling_square[0] = lsf + 2;
                moveFunc(lsf + 2);
            }

            if (Globals::castling & Bitboard::Castle::WHITE_LONG_CASTLE &&
                Bitboard::isRook(long_castle_rook) &&
                king_color & Bitboard::getColor(long_castle_rook))
            {
                Globals::castling_square[1] = lsf - 2;
                moveFunc(lsf - 2);
            }
        }
    }

    inline void generateKnightMoves(int lsf, std::function<void(int)> moveFunc)
    {
        for (int i = KNIGHT_OFFSET_START; i < KNIGHT_OFFSET_END; ++i)
        {
            int dlsf = lsf + OFFSETS[i];

            SDL_Point current_pos = Bitboard::lsfToCoord(lsf);
            SDL_Point dlsf_pos = Bitboard::lsfToCoord(dlsf);

            // The change or the delta position must be restricted to only 2 squares.
            int max_delta_squares = std::max(static_cast<int>(std::abs(dlsf_pos.x - current_pos.x)),
                                             static_cast<int>(std::abs(dlsf_pos.y - current_pos.y)));

            if ((notEmpty(dlsf) && !canCapture(dlsf)) || dlsf < 0 || dlsf > Bitboard::Squares::h8)
                continue;

            if (max_delta_squares == 2)
            {
                moveFunc(dlsf);
            }
        }
    }

    inline void generateKingMoves(int lsf, std::function<void(int)> moveFunc)
    {
        int king_moves = 0;

        for (int i = 0; i < 8; ++i)
        {
            int target_square = OFFSETS[i] + lsf;

            if (Globals::opponent_occupancy[target_square] == 0b10)
            {
                continue;
            }

            if (notEmpty(target_square))
            {
                if (canCapture(target_square))
                {
                    moveFunc(target_square);
                }

                continue;
            }

            moveFunc(target_square);
            king_moves++;
        }

        // If the LSF is in the occupancy square array and there is no more legal
        // moves to evade the check, then it must be checkmate.
        if (Globals::opponent_occupancy[lsf] == IS_OCCUPIED)
        {
            std::cout << "Check!\n";
        }
    }

    // Render pseudo-legal move hints.
    inline void searchPseudoLegalMoves(int lsf, std::function<void(int)> moveFunc,
                                       bool for_occupied_squares = false)
    {
        int team = Bitboard::getColor(Globals::bitboard[lsf]);

        // Skip empty pieces.
        if (Globals::bitboard[lsf] == Bitboard::Pieces::e || (!for_occupied_squares && !(team & Globals::side)))
            return;

        int pawn_range = (Globals::move_bitset[lsf] ? 2 : 3);
        int target_square;

        // Generate moves for various types of piece.
        switch (Globals::bitboard[lsf])
        {
        case Bitboard::Pieces::P:
            if (!for_occupied_squares)
            {
                for (int i = 1; i < pawn_range; ++i)
                {
                    target_square = Bitboard::addRank(lsf, -i);

                    // Halt the iteration if there is an intercepting piece.
                    if (notEmpty(target_square))
                        break;

                    moveFunc(target_square);
                }
            }

            // Capture Offsets
            generatePawnCaptures(lsf, moveFunc);

            break;
        case Bitboard::Pieces::p:
            if (!for_occupied_squares)
            {
                for (int i = 1; i < pawn_range; ++i)
                {
                    target_square = Bitboard::addRank(lsf, i);

                    // Halt the iteration if there is an intercepting piece.
                    if (notEmpty(target_square))
                        break;

                    moveFunc(target_square);
                }
            }

            // Capture Offsets
            generatePawnCaptures(lsf, moveFunc);

            break;
        case Bitboard::Pieces::N:
            generateKnightMoves(lsf, moveFunc);
            break;
        case Bitboard::Pieces::n:
            generateKnightMoves(lsf, moveFunc);
            break;

        case Bitboard::Pieces::K:
            generateKingMoves(lsf, moveFunc);
            if (!for_occupied_squares)
            {
                generateCastlingMove(lsf, moveFunc);
            }
            break;

        case Bitboard::Pieces::k:
            generateKingMoves(lsf, moveFunc);
            if (!for_occupied_squares)
            {
                generateCastlingMove(lsf, moveFunc);
            }
            break;
        }

        if (Bitboard::isSlidingPiece(Globals::bitboard[lsf]))
        {
            generateSlidingMoves(lsf, moveFunc, for_occupied_squares);
        }
    }

    inline void searchForOccupiedSquares()
    {
        for (int lsf = 0; lsf < Bitboard::Squares::h8; ++lsf)
        {
            int piece_color = Bitboard::getColor(Globals::bitboard[lsf]);

            // Reset the occupancy squares data.
            Globals::opponent_occupancy[lsf] = 0;

            if (Globals::bitboard[lsf] == Bitboard::Pieces::e || piece_color & Globals::side)
            {
                continue;
            }

            searchPseudoLegalMoves(lsf, &setOccupiedSquare, true);
        }
    }

    inline void generatePseudoLegalMoves()
    {
        for (int i = 0; i < Bitboard::Squares::h8; ++i)
        {
            if (Globals::bitboard[i] == Bitboard::Pieces::e)
                continue;
            searchPseudoLegalMoves(i, &addMove);
        }
    }
}; // namespace MoveGenerator