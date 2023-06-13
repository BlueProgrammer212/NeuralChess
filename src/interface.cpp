#include "interface.hpp"

using namespace Globals;

Interface::Interface() : mouse_position_x(0), mouse_position_y(0) {}

Interface::~Interface() {
  //Clear the precalculated SDL_Rect vector.
  quad_vector.clear();
}

//TODO: Consider using an unsigned int instead of these bools.
//TODO: Extract some regions into functions to increase clarity.
void Interface::drop(int lsf, int old_lsf, int width, int height, const unsigned int flags) {
  //Check for friendly pieces.
  const int selected_piece = Bitboard::getColor(bitboard[old_lsf]);
  const int target_lsf = Bitboard::getColor(bitboard[lsf]);

  if (((selected_piece != target_lsf || bitboard[lsf] == Bitboard::Pieces::e) &&
       selected_piece == side) ||
      flags & MoveFlags::SHOULD_SUPRESS_HINTS) {

    const int num_of_execution =
        flags & MoveFlags::SHOULD_SUPRESS_HINTS ? 1 : static_cast<int>(move_hints.size());

    for (int i = 0; i < num_of_execution; ++i) {
      if (lsf == move_hints[i].x || flags & MoveFlags::SHOULD_SUPRESS_HINTS) {
        //Exchange turns.
        bool exchange_turn = flags & MoveFlags::SHOULD_EXCHANGE_TURN;
        side ^= ((exchange_turn & 0b1) << 1) | (exchange_turn & 0b1);

        //Check if the move can alter material.
        bool can_alter_material = MoveGenerator::notEmpty(lsf);

        if (!(flags & WILL_UNDO_MOVE)) {
          auto last_move = SDL_Point{old_lsf, lsf};

          //Record the previous move for the "undo" feature.
          ply_array.push_back(Ply{last_move, can_alter_material, bitboard[lsf], bitboard[old_lsf],
                                  move_bitset[old_lsf], move_bitset[lsf]});

          ++current_move;
        }

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

        //Note: This differs from the "is_castling" argument.
        //"is_castling" is the argument passed by the recursive call
        //to "place" the rook to their respective LSF.
        bool is_a_castling_move = false;

        bool is_en_passant = (lsf == en_passant && !(en_passant & Bitboard::Squares::no_sq));

        int color = Bitboard::getColor(Globals::bitboard[lsf]);

        if ((lsf == castling_square.x || lsf == castling_square.y) &&
            Bitboard::isKing(bitboard[lsf]) && !(color & Globals::side)) {
          int dx = lsf - old_lsf;

          //Move the rook depending on which side the king castled.
          //This is relative to the king.
          int new_rook_delta_pos = (dx < 0 ? 1 : -1);
          int delta_old_rook_pos = (dx < 0 ? -4 : 3);

          drop(lsf + new_rook_delta_pos, old_lsf + delta_old_rook_pos, BOX_WIDTH, BOX_HEIGHT,
               SHOULD_SUPRESS_HINTS | IS_CASTLING);

          is_a_castling_move = true;
        }

        if (is_en_passant) {
          int rank_increment = (Globals::side & Bitboard::Sides::WHITE ? -1 : 1);

          //Clear the data of the "en passant" square.
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

        //Play the audio.
        if (is_in_check) {
          //Highlight the king in check.
          lsf_of_king_in_check = MoveGenerator::getOwnKing();
          audio_manager->PlayWAV("../../res/check.wav");
        } else if (is_a_castling_move) {
          audio_manager->PlayWAV("../../res/castle.wav");
        } else if (can_alter_material || is_en_passant) {
          audio_manager->PlayWAV("../../res/capture.wav");
        } else {
          audio_manager->PlayWAV("../../res/move.wav");
        }

        //50-move rule implementation.
        if (halfmove_clock >= 50) {
          std::cout << "Draw by 50-move rule.\n";
          game_state = 0;
          game_state |= GameState::DRAW;
        }

        //Checkmate detection.
        int legal_move_count = 0;

        for (LegalMove hint_lsf : Globals::legal_moves) {
          if (hint_lsf.x & Bitboard::Squares::no_sq) {
            continue;
          }

          legal_move_count++;
        }

        if (legal_move_count <= 0) {
          if (is_in_check) {
            game_state = 0;
            game_state |= GameState::CHECKMATE;
          } else {
            game_state = 0;
            game_state |= GameState::DRAW;
          }
        }

        //Log the algebraic notation of the move.
        //TODO: Use flags instead of boolean parameters to decrease ambiguity.
        if (!(flags & MoveFlags::IS_CASTLING) && !(flags & MoveFlags::WILL_UNDO_MOVE)) {
          std::cout << MoveGenerator::toAlgebraicNotation(bitboard[lsf], old_lsf, lsf,
                                                          can_alter_material || is_en_passant,
                                                          is_a_castling_move, lsf - old_lsf);
        }

        if (game_state & GameState::CHECKMATE) {
          const char* winner = (Globals::side & Bitboard::Sides::BLACK ? "White" : "Black");
          std::cout << "\n\nCheckmate! " << winner << " is victorious.\n";
        } else if (game_state & GameState::DRAW) {
          std::cout << "\n\nDraw by Stalemate\n";
        }

        if (flags & MoveFlags::SHOULD_SUPRESS_HINTS) {
          break;
        }
      }
    }

    move_hints.clear();
    Globals::selected_lsf = Bitboard::no_sq;
  }
}

void Interface::undo() {
  if (ply_array.empty() || current_move <= 0) {
    ply_array.clear();
    return;
  }

  Ply move_data = ply_array[--current_move];

  //TODO: Use the C++17 feature
  SDL_Point last_move = move_data.move;
  bool is_capture = move_data.is_capture;
  int captured_piece = move_data.old_piece_in_dest;

  bool old_move_bit = move_data.old_move_bit;
  bool old_move_bit_in_dest = move_data.old_move_bit_in_dest;

  drop(last_move.x, last_move.y, BOX_WIDTH, BOX_HEIGHT,
       SHOULD_SUPRESS_HINTS | SHOULD_EXCHANGE_TURN | WILL_UNDO_MOVE);

  //Put the "captured" piece back.
  if (is_capture) {
    Globals::bitboard[last_move.y] = captured_piece;
  }

  //Recall the move bit of the piece.
  Globals::move_bitset[last_move.x] = old_move_bit;
  Globals::move_bitset[last_move.y] = old_move_bit_in_dest;
}

//TODO: Implement drag and drop functionality.
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