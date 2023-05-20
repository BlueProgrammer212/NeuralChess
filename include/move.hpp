#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "globals.hpp"

namespace MoveGenerator
{
    // TODO: Define orthogonal and diagonal offsets.
    // TODO: Add Knight offset definitions.
    // TODO: Check for the remaining squares until it hits the edge.

    const std::vector<int> OFFSETS = {
        // Rook (R, L, B, T)
        1, -1, 7, -7,
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

    // Only render the possible moves of the selected piece.
    inline void renderMove(int lsf) noexcept { Globals::move_hints.push_back(lsf); }

    inline void pawnPromotion(int lsf)
    {
        int piece_color = Bitboard::getColor(Globals::bitboard[lsf]);

        int new_type = (piece_color & Bitboard::Sides::WHITE ? Bitboard::Pieces::Q : Bitboard::Pieces::q);

        if (Bitboard::isPawn(Globals::bitboard[lsf]) &&
            ((lsf >> 3 == 0 && piece_color & Bitboard::Sides::WHITE) ||
             (lsf >> 3 == Bitboard::BOARD_SIZE && piece_color & Bitboard::Sides::BLACK)))
        {
            // Globals::addWindow("Pawn Promotion", 700, 400);
            // Auto queen feature.
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

    inline void generatePawnCaptures(int lsf)
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
                renderMove(dlsf);
            }
        }
    }

    inline void generateSlidingMoves(int lsf)
    {
        int sliding_piece = Globals::bitboard[lsf];
        if (!Bitboard::isSlidingPiece(sliding_piece))
            return;
    }

    inline void generateKnightMoves(int lsf)
    {
        for (int i = KNIGHT_OFFSET_START; i < KNIGHT_OFFSET_END; ++i)
        {
            int dlsf = lsf + OFFSETS[i];

            SDL_Point current_pos = Bitboard::lsfToCoord(lsf);
            SDL_Point dlsf_pos = Bitboard::lsfToCoord(dlsf);

            int max_coord_dest = std::max(static_cast<int>(std::abs(dlsf_pos.x - current_pos.x)),
                                          static_cast<int>(std::abs(dlsf_pos.y - current_pos.y)));

            if ((notEmpty(dlsf) && !canCapture(dlsf)) || dlsf < 0 || dlsf > 63)
            {
                continue;
            }

            if (max_coord_dest == 2)
            {
                renderMove(dlsf);
            }
        }
    }

    // Render pseudo-legal move hints.
    inline void renderPseudoLegalMoves(int lsf)
    {
        Globals::move_hints.clear();

        int alliance_value = Bitboard::getColor(Globals::bitboard[lsf]);

        // Skip empty pieces and if the selected piece is invalid.
        if (Globals::bitboard[lsf] == Bitboard::Pieces::e || alliance_value != Globals::side)
            return;

        int pawn_range = (Globals::move_bitset[lsf] ? 2 : 3);
        int target_square;

        // Generate moves for various types of piece.
        switch (Globals::bitboard[lsf])
        {
        case Bitboard::Pieces::P:
            for (int i = 1; i < pawn_range; ++i)
            {
                target_square = Bitboard::addRank(lsf, -i);

                // Halt the iteration if there is an intercepting piece.
                if (notEmpty(target_square))
                    break;

                renderMove(target_square);
            }

            // Capture Offsets
            generatePawnCaptures(lsf);

            break;
        case Bitboard::Pieces::p:
            for (int i = 1; i < pawn_range; ++i)
            {
                target_square = Bitboard::addRank(lsf, i);

                // Halt the iteration if there is an intercepting piece.
                if (notEmpty(target_square))
                    break;

                renderMove(target_square);
            }

            // Capture Offsets
            generatePawnCaptures(lsf);

            break;
        case Bitboard::Pieces::N:
            generateKnightMoves(lsf);
        case Bitboard::Pieces::n:
            generateKnightMoves(lsf);
        }
    }
}; // namespace MoveGenerator