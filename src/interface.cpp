#include "interface.hpp"

using namespace Globals;

Interface::Interface() : mouse_position_x(0), mouse_position_y(0) {}

Interface::~Interface() {
  quad_vector.clear();
}

void Interface::drop(int lsf, int old_lsf, int width, int height, bool supress_hints,
                     bool exchange_turn) {
  //Check for friendly pieces.
  const int selected_piece = Bitboard::getColor(bitboard[old_lsf]);
  const int target_lsf = Bitboard::getColor(bitboard[lsf]);

  if (((selected_piece != target_lsf || bitboard[lsf] == Bitboard::Pieces::e) &&
       selected_piece == side) ||
      supress_hints) {

    const int num_of_execution = supress_hints ? 1 : static_cast<int>(move_hints.size());

    for (int i = 0; i < num_of_execution; ++i) {
      if (lsf == move_hints[i].x || supress_hints) {
        //Exchange turns.
        if (exchange_turn) {
          side ^= Bitboard::Sides::WHITE;
          side ^= Bitboard::Sides::BLACK;
        }

        bool can_alter_material = MoveGenerator::notEmpty(lsf);

        if (can_alter_material || Bitboard::isPawn(bitboard[old_lsf])) {
          //If the move can alter material or is a pawn move,
          //then reset the halfmove clock
          halfmove_clock = 0;
        }

        bitboard[lsf] = bitboard[old_lsf];
        bitboard[old_lsf] = Bitboard::e;

        //Update the move LSF set.
        move_bitset[lsf] = true;
        move_bitset[old_lsf] = false;

        last_ply.x = last_move.x;
        last_ply.y = last_move.y;

        last_piece_to_move = bitboard[lsf];

        last_move.x = old_lsf;
        last_move.y = lsf;

        bool is_a_castling_move = false;
        bool is_en_passant = (lsf == en_passant && !(en_passant & Bitboard::Squares::no_sq));

        int color = Bitboard::getColor(Globals::bitboard[lsf]);

        if ((lsf == castling_square.x || lsf == castling_square.y) &&
            Bitboard::isKing(bitboard[lsf]) && !(color & Globals::side)) {

          int new_rook =
              (color & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

          int dx = lsf - old_lsf;

          //Move the rook depending on which side the king castled.
          //This is relative to the king.
          int new_rook_delta_pos = (dx < 0 ? 1 : -1);
          int delta_old_rook_pos = (dx < 0 ? -4 : 3);

          drop(lsf + new_rook_delta_pos, old_lsf + delta_old_rook_pos, BOX_WIDTH, BOX_HEIGHT, true,
               false);

          is_a_castling_move = true;
        }

        if (is_en_passant) {
          int rank_increment = (Globals::side & Bitboard::Sides::WHITE ? -1 : 1);
          bitboard[(rank_increment << 3) + lsf] = Bitboard::Pieces::e;
        }

        elapsed_time = static_cast<double>(SDL_GetTicks());
        linear_interpolant = Bitboard::lsfToCoord(old_lsf);

        scaled_linear_interpolant =
            SDL_Point{linear_interpolant.x * BOX_WIDTH, linear_interpolant.y * BOX_HEIGHT};

        time = 0.0;

        //Check for pawn promotions.
        MoveGenerator::pawnPromotion(lsf);

        is_in_check = MoveGenerator::isInCheck(lsf);

        //Update the legal move array.
        MoveGenerator::generateLegalMoves();

        if (is_in_check) {
          audio_manager->PlayWAV("../../res/check.wav");
          lsf_of_king_in_check = MoveGenerator::getOwnKing();
        } else if (is_a_castling_move) {
          audio_manager->PlayWAV("../../res/castle.wav");
        } else if (can_alter_material || is_en_passant) {
          audio_manager->PlayWAV("../../res/capture.wav");
        } else {
          audio_manager->PlayWAV("../../res/move.wav");
        }

        if (halfmove_clock >= 50) {
          std::cout << "Draw by 50-move rule.\n";
          game_state = 0;
          game_state |= GameState::DRAW;
        }

        //Checkmate detection.
        int legal_move_count = 0;

        for (SDL_Point hint_lsf : Globals::legal_moves) {
          if (hint_lsf.x & Bitboard::Squares::no_sq) {
            continue;
          }

          legal_move_count++;
        }

        if (legal_move_count <= 0) {
          if (is_in_check) {
            const char* winner = (Globals::side & Bitboard::Sides::BLACK ? "White" : "Black");
            std::cout << "Checkmate! " << winner << " is victorious.\n";
            game_state = 0;
            game_state |= GameState::CHECKMATE;
          } else {
            std::cout << "Draw by Stalemate.\n";
            game_state = 0;
            game_state |= GameState::DRAW;
          }
        }

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