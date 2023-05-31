#include "interface.hpp"

using namespace Globals;

Interface::Interface() : mouse_position_x(0), mouse_position_y(0) {}

Interface::~Interface() {
  quad_vector.clear();
}

void Interface::drop(int lsf, int old_lsf, int width, int height, bool supress_hints) {
  //Check for friendly pieces.
  const int selected_piece = Bitboard::getColor(bitboard[old_lsf]);
  const int target_lsf = Bitboard::getColor(bitboard[lsf]);

  if (((selected_piece != target_lsf || bitboard[lsf] == Bitboard::Pieces::e) &&
       selected_piece == side) ||
      supress_hints) {

    const int num_of_execution = supress_hints ? 1 : static_cast<int>(move_hints.size());

    for (int i = 0; i < num_of_execution; ++i) {
      if (lsf == move_hints[i] || supress_hints) {
        //Exchange turns.
        side ^= Bitboard::Sides::WHITE;
        side ^= Bitboard::Sides::BLACK;

        bitboard[lsf] = bitboard[old_lsf];
        bitboard[old_lsf] = Bitboard::e;

        //Update the move bitset.
        move_bitset[lsf] = true;
        move_bitset[old_lsf] = false;

        last_ply.x = last_move.x;
        last_ply.y = last_move.y;

        last_move.x = old_lsf;
        last_move.y = lsf;

        if (lsf == castling_square && Bitboard::isKing(bitboard[lsf])) {
          int color = Bitboard::getColor(Globals::bitboard[lsf]);

          int new_rook =
              (color & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

          int dx = castling_square - old_lsf;

          //Move the rook depending on which side the king castled.
          //This is relative to the king.
          int new_rook_delta_pos = (dx < 0 ? 1 : -1);
          int delta_old_rook_pos = (dx < 0 ? -4 : 3);

          bitboard[old_lsf + delta_old_rook_pos] = Bitboard::e;
          bitboard[lsf + new_rook_delta_pos] = new_rook;
        }

        Globals::recorded_time = static_cast<double>(SDL_GetTicks());
        Globals::linear_interpolant = Bitboard::lsfToCoord(old_lsf);

        Globals::scaled_linear_interpolant =
            SDL_Point{Globals::linear_interpolant.x * Globals::BOX_WIDTH,
                      Globals::linear_interpolant.y * Globals::BOX_HEIGHT};

        Globals::time = 0.0;

        //Check for pawn promotions.
        MoveGenerator::pawnPromotion(lsf);

        Globals::is_in_check = MoveGenerator::isInCheck(lsf);

        if (supress_hints) {
          break;
        }
      }
    }

    move_hints.clear();
  }
}

void Interface::undo() {
  bool no_previous_move =
      last_move.x == Bitboard::Squares::no_sq || last_move.y == Bitboard::Squares::no_sq;

  if (no_previous_move) {
    std::cout << "No previous move :C\n";
    return;
  }

  drop(last_move.x, last_move.y, BOX_WIDTH, BOX_HEIGHT, true);
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