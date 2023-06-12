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

    constexpr std::array<int, 4> possible_promotions = {2, 3, 4, 5};

    constexpr int KNIGHT_OFFSET_START = 8;
    constexpr int KNIGHT_OFFSET_END = 16;

    void addMove(const int t_lsf, const int old_lsf) noexcept;

    // Set the LSF as an "occupied square"
    void addOccupancySquare(const int t_lsf, const int old_lsf) noexcept;

    // Only render the possible moves of the selected piece.
    void renderMove(const int t_lsf, const int old_lsf) noexcept;

    // Get the distance between the delta lsf and the target LSF.
    int getMaxDeltaSquares(const int delta_lsf, const int lsf_prime);

    const ImaginaryMove makeMove(const LegalMove &move);

    void unmakeMove(const LegalMove &move, const ImaginaryMove &data);

    [[nodiscard]] const std::string toAlgebraicNotation(int type, int old_lsf, int lsf,
                                                        bool is_capture, bool is_a_castling_move, int dx);

    // TODO: Implement pawn underpromotion as a "legal move".
    void pawnPromotion(const int t_lsf);

    bool notEmpty(const int t_lsf);

    bool canCapture(const int t_lsf, const bool for_occupied_square = false);

    void enPassant(const int t_lsf, const std::function<void(int, int)> moveFunc);

    void generatePawnCaptures(int t_lsf, const std::function<void(int, int)> moveFunc, bool for_occupied_squares);

    const std::vector<int> &getDistToEdge(const SDL_Point pos);

    void generateSlidingMoves(int t_lsf, std::function<void(int, int)> moveFunc, bool for_occupied_square);

    inline void generateCastlingMove(int t_lsf, std::function<void(int, int)> moveFunc)
    {
        //TODO: Refactor the "flank checker" using extraction.
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
                                       bool for_occupied_squares = false, bool for_legal_moves = false)
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

    inline void filterPseudoLegalMoves(int t_lsf, std::vector<LegalMove> &hint_lsf_array)
    {
        int king = getOwnKing();

        if (!notEmpty(t_lsf))
        {
            return;
        }

        const std::vector<LegalMove> legal_moves_copy = hint_lsf_array;

        for (int i = 0; i < static_cast<int>(legal_moves_copy.size()); i++)
        {
            // Make the move.
            const int lsf_to_verify = legal_moves_copy[i].x;

            if (lsf_to_verify & Bitboard::Squares::no_sq)
            {
                continue;
            }

            // Make the move and record the captured piece and old move bits to unmake the move.
            const auto &move_data = makeMove(legal_moves_copy[i]);

            if (isInCheck(king))
            {
                hint_lsf_array[i].x = Bitboard::Squares::no_sq;
            };

            unmakeMove(legal_moves_copy[i], move_data);

            searchForOccupiedSquares();
        }

        searchForOccupiedSquares();
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