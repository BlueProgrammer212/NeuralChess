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

    inline void addMove(int t_lsf) noexcept
    {
        // Add a move to the LSF array.
        if (t_lsf > 0 && t_lsf < Bitboard::NUM_OF_SQUARES - 1)
        {
            Globals::pseudolegal_moves.push_back(t_lsf);
        }
    }

    // Set the LSF as an "occupied square"
    inline void addOccupancySquare(int t_lsf) noexcept
    {
        Globals::opponent_occupancy.push_back(t_lsf);
    }

    // Only render the possible moves of the selected piece.
    inline void renderMove(int t_lsf) noexcept
    {
        Globals::move_hints.push_back(t_lsf);
    }

    inline int getMaxDeltaSquares(int delta_lsf, int lsf_prime)
    {
        const SDL_Point current_pos = Bitboard::lsfToCoord(lsf_prime);
        const SDL_Point dt_lsf_pos = Bitboard::lsfToCoord(delta_lsf);

        return std::max(static_cast<int>(std::abs(dt_lsf_pos.x - current_pos.x)),
                        static_cast<int>(std::abs(dt_lsf_pos.y - current_pos.y)));
    }

    inline void pawnPromotion(int t_lsf)
    {
        int piece_color = Bitboard::getColor(Globals::bitboard[t_lsf]);

        int new_type = (piece_color & Bitboard::Sides::WHITE ? Bitboard::Pieces::Q : Bitboard::Pieces::q);

        if (Bitboard::isPawn(Globals::bitboard[t_lsf]) &&
            ((t_lsf >> 3 == 0 && piece_color & Bitboard::Sides::WHITE) ||
             (t_lsf >> 3 == Bitboard::BOARD_SIZE && piece_color & Bitboard::Sides::BLACK)))
        {
            // Globals::addWindow("Pawn Promotion", 700, 400);
            //  Auto queen feature.
            Globals::bitboard[t_lsf] = new_type;
        }
    }

    inline bool notEmpty(int t_lsf)
    {
        return Globals::bitboard[t_lsf] != Bitboard::Pieces::e;
    }

    inline bool canCapture(int t_lsf, bool for_occupied_square = false)
    {
        int which_side = for_occupied_square ? ~Globals::side : Globals::side;
        bool is_allies = which_side & Bitboard::getColor(Globals::bitboard[t_lsf]);
        return !is_allies && notEmpty(t_lsf);
    }

    inline void generatePawnCaptures(int t_lsf, std::function<void(int)> moveFunc, bool for_occupied_squares)
    {
        const int pawn = Globals::bitboard[t_lsf];

        if (!Bitboard::isPawn(pawn))
        {
            return;
        }

        const int color = Bitboard::getColor(pawn);

        const int direction_offset_start = (color & Bitboard::Sides::WHITE ? 6 : 4);
        const int direction_offset_end = (color & Bitboard::Sides::WHITE ? 8 : 6);

        for (int i = direction_offset_start; i < direction_offset_end; ++i)
        {
            const int dt_lsf = t_lsf + OFFSETS[i];

            const int max_delta_squares = getMaxDeltaSquares(dt_lsf, t_lsf);

            if ((canCapture(dt_lsf, for_occupied_squares) || for_occupied_squares) && max_delta_squares == 1)
            {
                moveFunc(dt_lsf);
            }
        }
    }

    inline const std::vector<int> &getDistToEdge(SDL_Point pos)
    {
        const int right = Bitboard::MAX_SQUARES_TO_EDGE - pos.x;
        const int left = pos.x + 1;
        const int top = pos.y + 1;
        const int bottom = Bitboard::MAX_SQUARES_TO_EDGE - pos.y;

        Globals::max_squares = {
            right,  // Right
            left,   // Left
            bottom, // Bottom
            top,    // Top

            std::min(bottom, right), // South East
            std::min(bottom, left),  // South West
            std::min(top, right),    // North East
            std::min(top, left),     // North West
        };

        return Globals::max_squares;
    }

    inline void generateSlidingMoves(int t_lsf, std::function<void(int)> moveFunc, bool for_occupied_square)
    {
        const int sliding_piece = Globals::bitboard[t_lsf];

        const int start_index = (Bitboard::isBishop(sliding_piece) ? 4 : 0);
        const int end_index = (Bitboard::isRook(sliding_piece) ? 4 : 8);

        const SDL_Point pos = Bitboard::lsfToCoord(t_lsf);
        getDistToEdge(pos);

        for (int i = start_index; i < end_index; ++i)
        {
            for (int target_sq = 1; target_sq < Globals::max_squares[i]; target_sq++)
            {
                const int dt_lsf = target_sq * OFFSETS[i] + t_lsf;

                if (notEmpty(dt_lsf))
                {
                    if (canCapture(dt_lsf, for_occupied_square) || for_occupied_square)
                    {
                        moveFunc(dt_lsf);
                    }

                    break;
                }

                moveFunc(dt_lsf);
            }
        }
    }

    inline void generateCastlingMove(int t_lsf, std::function<void(int)> moveFunc)
    {
        if (Globals::is_in_check) {
            return;
        }

        const int king_color = Bitboard::getColor(Globals::bitboard[t_lsf]);
        const int shift = (king_color & Bitboard::Sides::WHITE ? 0 : 2);

        int long_castle = Bitboard::Castle::LONG_CASTLE << shift;
        int short_castle = Bitboard::Castle::SHORT_CASTLE << shift;

        if (Globals::move_bitset[t_lsf])
        {
            // If the king has moved, disable castling for both sides.
            Globals::castling &= ~long_castle;
            Globals::castling &= ~short_castle;

            return;
        }

        // Check for pieces that may block the castle.
        const int king_side_rook = Globals::bitboard[t_lsf + 3];
        const int queen_side_rook = Globals::bitboard[t_lsf - 4];

        // TODO: Refactor this into one function.
        //  Queenside Castling (O-O-O)
        for (int dx = 1; dx < 4; ++dx)
        {
            const int delta_lsf = t_lsf - dx;

            const bool rook_conditions = !Bitboard::isRook(queen_side_rook) || Globals::move_bitset[t_lsf - 4];

            if (notEmpty(delta_lsf) || rook_conditions)
            {
                // Remove long castling rights.
                Globals::castling &= ~long_castle;
                break;
            }

            // If there's no intercepting piece, then allow castling.
            if (dx == 3)
            {
                Globals::castling |= long_castle;
                Globals::castling_square = t_lsf - 2;
                moveFunc(t_lsf - 2);
            }
        }

        // King side castling (O-O)
        for (int dx = 1; dx < 3; ++dx)
        {
            const int delta_lsf = t_lsf + dx;
            const bool rook_conditions = !Bitboard::isRook(king_side_rook) || Globals::move_bitset[t_lsf + 3];

            if (notEmpty(delta_lsf) || rook_conditions)
            {
                // Remove short castling rights.
                Globals::castling &= ~short_castle;
                break;
            }

            if (dx == 2)
            {
                Globals::castling |= short_castle;
                Globals::castling_square = t_lsf + 2;
                moveFunc(t_lsf + 2);
            }
        }
    }

    inline void generateKnightMoves(int t_lsf, std::function<void(int)> moveFunc, bool for_occupied_squares)
    {
        for (int i = KNIGHT_OFFSET_START; i < KNIGHT_OFFSET_END; ++i)
        {
            const int dt_lsf = t_lsf + OFFSETS[i];

            // The change or the delta position must be restricted to only 2 squares.
            const int max_delta_squares = getMaxDeltaSquares(dt_lsf, t_lsf);

            // Check if the square will be "out of bounds" if we add the delta LSF.
            const bool is_out_of_bounds = dt_lsf < 0 || dt_lsf > Bitboard::Squares::h8;

            // Check if the square does not contain a friendly piece.
            const bool contains_friendly_piece = notEmpty(dt_lsf) && !canCapture(dt_lsf, for_occupied_squares);

            if (!contains_friendly_piece && !is_out_of_bounds && max_delta_squares == 2)
            {
                moveFunc(dt_lsf);
            }
        }
    }

    inline void generateKingMoves(int t_lsf, std::function<void(int)> moveFunc, bool for_occupied_squares)
    {
        int king_moves = 0;

        for (int i = 0; i < 8; ++i)
        {
            const int target_square = OFFSETS[i] + t_lsf;

            // Ensure that the king can only move 1 square.
            const int max_delta_squares = getMaxDeltaSquares(target_square, t_lsf);

            // Check if the square will be "out of bounds" if we add the delta LSF.
            const bool is_out_of_bounds = target_square < 0 || target_square > Bitboard::Squares::h8;

            // Check if the square does not contain a friendly piece.
            const bool contains_friendly_piece = notEmpty(target_square) && !canCapture(target_square);

            // Check if the square is in the occupancy square array.
            bool is_an_occupied_square = false;

            for (int occupied_square : Globals::opponent_occupancy)
            {
                is_an_occupied_square = (target_square == occupied_square);

                if (is_an_occupied_square)
                {
                    break;
                }
            }

            if (is_an_occupied_square)
            {
                continue;
            }

            if ((!contains_friendly_piece || for_occupied_squares) && !is_out_of_bounds && max_delta_squares == 1)
            {
                moveFunc(target_square);
                king_moves++;
            }
        }
    }

    // Render pseudo-legal move hints.
    inline void searchPseudoLegalMoves(int t_lsf, std::function<void(int)> moveFunc,
                                       bool for_occupied_squares = false)
    {
        int team = Bitboard::getColor(Globals::bitboard[t_lsf]);

        bool check_side = (for_occupied_squares && team & Globals::side) 
                        || (!for_occupied_squares && !(team & Globals::side));

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
                        break;

                    moveFunc(target_square);
                }
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

                    moveFunc(target_square);
                }
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

        for (int t_lsf = 0; t_lsf < Bitboard::Squares::h8; ++t_lsf)
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
        int piece_type = (Globals::side & Bitboard::Sides::WHITE ? Bitboard::Pieces::K : Bitboard::Pieces::k);

        for (int lsf = 0; lsf < Bitboard::Squares::h8; ++lsf)
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

        for (int occupied_sq : Globals::opponent_occupancy)
        {
            // If any squares matches with the king's LSF, then the king must in check.
            if (occupied_sq == king)
            {
                std::cout << "Check!\n";
                return true;
            }
        }

        return false;
    }

    inline bool isPinned(int piece_lsf)
    {
        int king = getOwnKing();

        for (int &occupied_squares : Globals::opponent_occupancy)
        {
            
        }
    }

    inline void generatePseudoLegalMoves()
    {
        Globals::pseudolegal_moves.clear();
        for (int i = 0; i < Bitboard::Squares::h8; ++i)
        {
            if (Globals::bitboard[i] == Bitboard::Pieces::e)
            {
                continue;
            }

            searchPseudoLegalMoves(i, &addMove);
        }
    }
}; // namespace MoveGenerator