#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>

#include "globals.hpp"

struct ImaginaryMove
{
    int old_piece = Bitboard::Squares::no_sq;
    int captured_piece = Bitboard::Squares::no_sq;
    bool old_move_bit = false;
    bool old_move_bit_of_dest = false;
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

    void addMove(int t_lsf, int old_lsf) noexcept;

    // Set the LSF as an "occupied square"
    void addOccupancySquare(int t_lsf, int old_lsf) noexcept;

    // Only render the possible moves of the selected piece.
    void renderMove(int t_lsf, int old_lsf) noexcept;

    // Get the distance between the delta lsf and the target LSF.
    int getMaxDeltaSquares(int delta_lsf, int lsf_prime);

    const ImaginaryMove makeMove(const SDL_Point &move);

    void unmakeMove(const SDL_Point &move, const ImaginaryMove &data);

    // TODO: Implement pawn underpromotion as a "legal move".
    void pawnPromotion(int t_lsf);

    bool notEmpty(int t_lsf);

    inline bool canCapture(int t_lsf, bool for_occupied_square = false)
    {
        int which_side = for_occupied_square ? ~Globals::side : Globals::side;
        bool is_allies = which_side & Bitboard::getColor(Globals::bitboard[t_lsf]);
        return !is_allies && notEmpty(t_lsf);
    }

    inline void enPassant(int t_lsf, std::function<void(int, int)> moveFunc)
    {
        if (!Bitboard::isPawn(Globals::bitboard[Globals::last_move.y]))
        {
            Globals::en_passant = Bitboard::Squares::no_sq;
            return;
        }

        // These are the positions of the last move.
        SDL_Point old_pos = Bitboard::lsfToCoord(Globals::last_move.x);
        SDL_Point new_pos = Bitboard::lsfToCoord(Globals::last_move.y);

        // This is for the selected pawn.
        SDL_Point current_pawn_pos = Bitboard::lsfToCoord(t_lsf);

        // Check if the pawn advances 2 squares for its initial move.
        // TODO: Check if the pawns are in the correct ranks for better
        // verification.

        int dx = new_pos.x - current_pawn_pos.x;
        int dy = old_pos.y - new_pos.y;

        int rank_increment = (Globals::side & Bitboard::Sides::WHITE ? -1 : 1);

        if (dy == 2 * rank_increment && (dx == 1 || dx == -1) &&
            new_pos.y == current_pawn_pos.y)
        {
            // If the en passant square contains a piece, then en passant is not valid.
            Globals::en_passant = ((rank_increment << 3) + Globals::last_move.y);

            if (notEmpty(Globals::en_passant))
            {
                Globals::en_passant = Bitboard::Squares::no_sq;
            }
            else
            {
                moveFunc(Globals::en_passant, t_lsf);
                return;
            }
        }

        Globals::en_passant = Bitboard::Squares::no_sq;
    }

    inline void generatePawnCaptures(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_squares)
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
                moveFunc(dt_lsf, t_lsf);
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

    inline void generateSlidingMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_square)
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
                        moveFunc(dt_lsf, t_lsf);
                    }

                    const int king_color = (Globals::side & Bitboard::Sides::WHITE ? Bitboard::Pieces::K : Bitboard::Pieces::k);

                    bool is_opponent_king = (Globals::bitboard[dt_lsf] == king_color &&
                                             Bitboard::isKing(Globals::bitboard[dt_lsf]));

                    if (!is_opponent_king || !for_occupied_square)
                    {
                        break;
                    }
                }

                moveFunc(dt_lsf, t_lsf);
            }
        }
    }

    inline void generateCastlingMove(int t_lsf, std::function<void(int, int)> moveFunc)
    {
        // Reset the data to prevent bugs
        Globals::castling_square.x = Bitboard::Squares::no_sq;
        Globals::castling_square.y = Bitboard::Squares::no_sq;

        if (Globals::is_in_check)
        {
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

            bool is_an_occupied_square = false;

            for (SDL_Point occupied_square : Globals::opponent_occupancy)
            {
                is_an_occupied_square = (delta_lsf == occupied_square.x);

                if (is_an_occupied_square)
                {
                    break;
                }
            }

            if (notEmpty(delta_lsf) || rook_conditions || is_an_occupied_square)
            {
                // Remove long castling rights.
                Globals::castling &= ~long_castle;
                break;
            }

            // If there's no intercepting piece, then allow castling.
            if (dx == 3)
            {
                Globals::castling |= long_castle;
                Globals::castling_square.y = t_lsf - 2;
                moveFunc(t_lsf - 2, t_lsf);
            }
        }

        // King side castling (O-O)
        for (int dx = 1; dx < 3; ++dx)
        {
            const int delta_lsf = t_lsf + dx;
            const bool rook_conditions = !Bitboard::isRook(king_side_rook) || Globals::move_bitset[t_lsf + 3];

            bool is_an_occupied_square = false;

            for (SDL_Point occupied_square : Globals::opponent_occupancy)
            {
                is_an_occupied_square = (delta_lsf == occupied_square.x);

                if (is_an_occupied_square)
                {
                    break;
                }
            }

            if (notEmpty(delta_lsf) || rook_conditions || is_an_occupied_square)
            {
                // Remove short castling rights.
                Globals::castling &= ~short_castle;
                break;
            }

            if (dx == 2)
            {
                Globals::castling |= short_castle;
                Globals::castling_square.x = t_lsf + 2;
                moveFunc(t_lsf + 2, t_lsf);
            }
        }
    }

    inline void generateKnightMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_squares)
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

            if ((for_occupied_squares || !contains_friendly_piece) && !is_out_of_bounds && max_delta_squares == 2)
            {
                moveFunc(dt_lsf, t_lsf);
            }
        }
    }

    inline void generateKingMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_squares)
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

            for (SDL_Point occupied_square : Globals::opponent_occupancy)
            {
                is_an_occupied_square = (target_square == occupied_square.x);

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
                moveFunc(target_square, t_lsf);
                king_moves++;
            }
        }
    }

    // Render pseudo-legal move hints.
    inline void searchPseudoLegalMoves(int t_lsf, std::function<void(int, int)> moveFunc,
                                       bool for_occupied_squares = false)
    {
        int team = Bitboard::getColor(Globals::bitboard[t_lsf]);

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
                        break;

                    moveFunc(target_square, t_lsf);
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

                    moveFunc(target_square, t_lsf);
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
        int piece_type = (Globals::side & Bitboard::Sides::WHITE ? Bitboard::Pieces::K : Bitboard::Pieces::k);

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

    inline void filterPseudoLegalMoves(int t_lsf, std::vector<SDL_Point> &hint_lsf_array)
    {
        int king = getOwnKing();

        if (!notEmpty(t_lsf))
        {
            return;
        }

        for (int i = 0; i < static_cast<int>(hint_lsf_array.size()); i++)
        {
            // Make the move.
            // TODO: Refactor the entire thing.
            int lsf_to_verify = hint_lsf_array[i].x;

            if (hint_lsf_array[i].x & Bitboard::Squares::no_sq)
            {
                continue;
            }

            int color = Bitboard::getColor(Globals::bitboard[lsf_to_verify]);

            int old_piece = Globals::bitboard[hint_lsf_array[i].y];
            int old_piece_in_dest = Globals::bitboard[lsf_to_verify];

            int en_passant_capture_lsf = -1;
            int en_passant_capture_piece_type = Bitboard::Pieces::e;

            bool is_en_passant = lsf_to_verify == Globals::en_passant &&
                                 !(Globals::en_passant & Bitboard::Squares::no_sq);

            bool is_castling = ((lsf_to_verify == Globals::castling_square.x || lsf_to_verify == Globals::castling_square.y) &&
                                Bitboard::isKing(Globals::bitboard[lsf_to_verify]) && color & Globals::side);

            Globals::bitboard[lsf_to_verify] = old_piece;
            Globals::bitboard[hint_lsf_array[i].y] = Bitboard::Pieces::e;

            if (is_en_passant)
            {
                int rank_increment = (Globals::side & Bitboard::Sides::WHITE ? 1 : -1);
                en_passant_capture_lsf = (rank_increment << 3) + Globals::en_passant;
                en_passant_capture_piece_type = Globals::bitboard[en_passant_capture_lsf];

                Globals::bitboard[en_passant_capture_lsf] = Bitboard::Pieces::e;
            }

            if (is_castling)
            {
                int new_rook =
                    (color & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

                int dx = lsf_to_verify - hint_lsf_array[i].y;

                // Move the rook depending on which side the king castled.
                // This is relative to the king.
                int new_rook_delta_pos = (dx < 0 ? 1 : -1);
                int delta_old_rook_pos = (dx < 0 ? -4 : 3);

                Globals::bitboard[hint_lsf_array[i].y + delta_old_rook_pos] = Bitboard::Pieces::e;
                Globals::bitboard[lsf_to_verify + new_rook_delta_pos] = new_rook;
            }

            if (isInCheck(king))
            {
                hint_lsf_array[i].x = Bitboard::Squares::no_sq;
            };

            if (is_castling)
            {
                int new_rook =
                    (color & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

                int dx = lsf_to_verify - hint_lsf_array[i].y;

                // Move the rook depending on which side the king castled.
                // This is relative to the king.
                int new_rook_delta_pos = (dx < 0 ? 1 : -1);
                int delta_old_rook_pos = (dx < 0 ? -4 : 3);

                Globals::bitboard[hint_lsf_array[i].y + delta_old_rook_pos] = new_rook;
                Globals::bitboard[lsf_to_verify + new_rook_delta_pos] = Bitboard::Pieces::e;
            }

            if (is_en_passant)
            {
                Globals::bitboard[en_passant_capture_lsf] = en_passant_capture_piece_type;
            }

            // Unmake the move.
            Globals::bitboard[lsf_to_verify] = old_piece_in_dest;
            Globals::bitboard[hint_lsf_array[i].y] = old_piece;

            searchForOccupiedSquares();
        }

        searchForOccupiedSquares();
    }

    inline const std::vector<SDL_Point> &generateLegalMoves()
    {
        Globals::legal_moves.clear();
        for (int i = 0; i < Bitboard::Squares::h8 + 1; ++i)
        {
            if (Globals::bitboard[i] == Bitboard::Pieces::e ||
                !(Globals::side & Bitboard::getColor(Globals::bitboard[i])))
            {
                continue;
            }

            searchPseudoLegalMoves(i, &addMove);
            filterPseudoLegalMoves(i, Globals::legal_moves);
        }

        return Globals::legal_moves;
    }
}; // namespace MoveGenerator