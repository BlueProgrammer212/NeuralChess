#include "interface.hpp"

using namespace Globals;

Interface::Interface() : mouse_position_x(0), mouse_position_y(0) {}

Interface::~Interface() {
  quad_vector.clear();
}

void Interface::drop(int lsf, int old_lsf, int width, int height) {
  //Check for friendly pieces.
  int selected_piece = Bitboard::getColor(bitboard[old_lsf]);
  int target_lsf = Bitboard::getColor(bitboard[lsf]);

  if ((selected_piece != target_lsf || bitboard[lsf] == Bitboard::Pieces::e) &&
      selected_piece == side) {
    for (int moves : Globals::move_hints) {
      if (lsf == moves) {
        //Exchange turns.
        side ^= Bitboard::Sides::WHITE;
        side ^= Bitboard::Sides::BLACK;

        bitboard[lsf] = bitboard[old_lsf];
        bitboard[old_lsf] = Bitboard::e;

        //Update the move bitset.
        move_bitset[lsf] = true;
        move_bitset[old_lsf] = false;

        last_move.x = old_lsf;
        last_move.y = lsf;

        for (int castling_sq : Globals::castling_square) {
          if (lsf == castling_sq && Bitboard::isKing(bitboard[lsf])) {
            int color = Bitboard::getColor(Globals::bitboard[lsf]);

            int new_rook =
                (color & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

            bitboard[lsf - 1] = Bitboard::e;
            bitboard[castling_sq] = new_rook;
          }
        }

        Globals::recorded_time = static_cast<double>(SDL_GetTicks());
        Globals::linear_interpolant = Bitboard::lsfToCoord(old_lsf);

        Globals::scaled_linear_interpolant =
            SDL_Point{Globals::linear_interpolant.x * Globals::BOX_WIDTH,
                      Globals::linear_interpolant.y * Globals::BOX_HEIGHT};

        Globals::time = 0.0;

        //Check for pawn promotions.
        MoveGenerator::pawnPromotion(lsf);

        //Update occupancy squares.
        MoveGenerator::searchForOccupiedSquares();
      }
    }

    move_hints.clear();
  }
}

void Interface::drag(int width, int height) {}

int Interface::AABB(int x, int y) {
  for (int file = 0; file < Bitboard::BOARD_SIZE + 1; ++file) {
    for (int rank = 0; rank < Bitboard::BOARD_SIZE + 1; ++rank) {
      int index = Bitboard::toLSF(file, rank, false);
      SDL_Rect rect = quad_vector[index];

      if (rect.x + rect.w > x && x > rect.x && rect.y + rect.h > y && y > rect.y) {
        MoveGenerator::searchPseudoLegalMoves(index, &MoveGenerator::renderMove);
        return index;
      }
    }
  }

  return Bitboard::Squares::no_sq;
}

//This is used for smooth transitions. It returns y = t(Δx) + a;
int Interface::lerp(int a, int b, int t) {
  int dx = b - a;
  return t * dx + a;
}