#include "interface.hpp"

using namespace Globals;

Interface::Interface() : mouse_position_x(0), mouse_position_y(0) {}

Interface::~Interface() {
  //Clear the precalculated SDL_Rect vector.
  quad_vector.clear();
}

//TODO: Extract some regions into functions to increase clarity.
void Interface::drop(int square, int old_square, const unsigned int flags) {
  //Check for friendly pieces.
  const int selected_piece_color = Bitboard::getColor(bitboard[old_square]);
  const int target_square_color = Bitboard::getColor(bitboard[square]);

  const bool is_friendly = selected_piece_color != target_square_color;
  const bool is_empty = !MoveGenerator::notEmpty(square);

  const bool can_place = is_friendly || is_empty;

  const bool rules = can_place && (selected_piece_color & side);

  //Check if the square is in the hints array.
  auto isSquareValid = [square](const LegalMove& move) {
    return move.x == square;
  };

  auto it = std::find_if(move_hints.begin(), move_hints.end(), isSquareValid);

  if (!(rules || flags & MoveFlags::SHOULD_SUPRESS_HINTS) ||
      !(it != move_hints.end() || flags & MoveFlags::SHOULD_SUPRESS_HINTS)) {
    return;
  }

  bool exchange_turn = flags & MoveFlags::SHOULD_EXCHANGE_TURN;
  side ^= ((exchange_turn & 0b1) << 1) | (exchange_turn & 0b1);

  //Check if the move can alter material.
  bool can_alter_material = MoveGenerator::notEmpty(square);

  if (!(flags & WILL_UNDO_MOVE)) {
    auto last_move = SDL_Point{old_square, square};

    // Record the previous move for the "undo" feature.
    // clang-format off
      const auto move = Ply {
          last_move,
          can_alter_material,
          
          bitboard[square],
          bitboard[old_square],
          move_bitset[old_square],
          move_bitset[square]
      };

      Globals::move_squares.push_back(
          std::make_tuple(square, bitboard[square], Globals::side));
    // clang-format on

    ply_array.push_back(move);

    ++current_move;
  }

  //Increment the halfmove clock if it's white to move.
  if (side & Bitboard::Sides::WHITE) {
    Globals::halfmove_clock++;
  }

  if (can_alter_material || Bitboard::isPawn(bitboard[old_square])) {
    Globals::halfmove_clock = 0;
  }

  bitboard[square] = bitboard[old_square];
  bitboard[old_square] = Bitboard::e;

  //Check for pawn promotions.
  MoveGenerator::pawnPromotion(square);

  //Update the move bitset.
  move_bitset[square] = true;
  move_bitset[old_square] = false;

  // Note: The IS_CASTLING flag is used for recursion to move the
  // rook. While is_a_castling_move is used to "detect" if the legal move
  // is considered castling.

  // clang-format off
  const bool is_a_castling_move = 
        (square == castling_square.x || square == castling_square.y) && 
        Bitboard::isKing(bitboard[square]) && ~(target_square_color & Globals::side);

  const bool is_en_passant = ((square == en_passant) 
                              & ~(en_passant & Bitboard::Squares::no_sq));
  // clang-format on

  if (is_a_castling_move) {
    const int dx = square - old_square;

    //Move the rook depending on which side the king castled.
    //This is relative to the king.
    const int new_rook_delta_pos = ((dx < 0) * 1) | ((dx > 0) * -1);
    const int delta_old_rook_pos = ((dx < 0) * -4) | ((dx > 0) * 3);

    //Move the rook to its respective square by using recursion.
    drop(square + new_rook_delta_pos, old_square + delta_old_rook_pos,
         SHOULD_SUPRESS_HINTS | IS_CASTLING);
  }

  if (is_en_passant) {
    //Clear the data of the "en passant" square.
    const int square_increment = ((Globals::side & 0b01) * 1) | ((~Globals::side & 0b01) * -1);
    bitboard[(square_increment << 3) + square] = Bitboard::Pieces::e;
  }

  // Generate a zobrist hash and push it to the position history for threefold repetition detection.
  const std::uint64_t zobrist_hash = Globals::zobrist_hashing->hashPosition();
  Globals::position_history.push_back(zobrist_hash);

  //Start the piece animation.
  elapsed_time = static_cast<double>(SDL_GetTicks());
  linear_interpolant = Bitboard::squareToCoord(old_square);

  scaled_linear_interpolant =
      SDL_Point{linear_interpolant.x * BOX_WIDTH, linear_interpolant.y * BOX_HEIGHT};

  time = 0.0;

  is_in_check = MoveGenerator::isInCheck();

  //Update the legal move array.
  MoveGenerator::generateLegalMoves();

  //Play the audio.
  if (is_in_check) {
    //Highlight the king in check.
    square_of_king_in_check = MoveGenerator::getOwnKing();
    audio_manager->PlayWAV("../../res/check.wav");
  } else if (is_a_castling_move) {
    audio_manager->PlayWAV("../../res/castle.wav");
  } else if (can_alter_material || is_en_passant) {
    audio_manager->PlayWAV("../../res/capture.wav");
  } else {
    audio_manager->PlayWAV("../../res/move.wav");
  }

  // Check for possible game terminations.
  game_state |= GameState::CHECKMATE * MoveGenerator::isCheckmate() |
                GameState::DRAW * MoveGenerator::isStalemate() |
                GameState::DRAW * MoveGenerator::isInsufficientMaterial() |
                GameState::DRAW * MoveGenerator::isThreefoldRepetition() |
                GameState::DRAW * MoveGenerator::isFiftyMoveRule();

  // Log the algebraic notation of the move.
  if (!(flags & MoveFlags::IS_CASTLING) && !(flags & MoveFlags::WILL_UNDO_MOVE)) {
    // clang-format off
    const int delta_x = square - old_square;

    std::cout << MoveGenerator::toAlgebraicNotation(bitboard[square], old_square, square,
      can_alter_material || is_en_passant, is_a_castling_move, delta_x);
    // clang-format on
  }

  if (game_state & GameState::CHECKMATE) {
    const char* winner = (Globals::side & Bitboard::Sides::BLACK ? "White" : "Black");
    std::cout << "\n\nCheckmate! " << winner << " is victorious.\n";
  } else if (halfmove_clock >= MoveGenerator::HALFMOVE_CLOCK_THRESHOLD) {
    std::cout << "\n\nDraw by 50-move rule.\n";
  } else if (MoveGenerator::isInsufficientMaterial()) {
    std::cout << "\n\nDraw by Insufficient Material.\n";
  } else if (MoveGenerator::isThreefoldRepetition()) {
    std::cout << "\n\nDraw by Threefold Repetition.\n";
  } else if (game_state & GameState::DRAW) {
    std::cout << "\n\nDraw by Stalemate.\n";
  }

  move_hints.clear();
  Globals::selected_square = Bitboard::Squares::no_sq;
}

void Interface::undo() {
  if (ply_array.empty() || current_move <= 0) {
    ply_array.clear();
    return;
  }

  Ply move_data = ply_array[--current_move];

  //TODO: Use the C++17 feature to destructure properties into individual variables.
  SDL_Point last_move = move_data.move;
  bool is_capture = move_data.is_capture;
  int captured_piece = move_data.old_piece_in_dest;

  bool old_move_bit = move_data.old_move_bit;
  bool old_move_bit_in_dest = move_data.old_move_bit_in_dest;

  //Undo the move
  drop(last_move.x, last_move.y, SHOULD_SUPRESS_HINTS | SHOULD_EXCHANGE_TURN | WILL_UNDO_MOVE);

  //Put the "captured" piece back.
  if (is_capture) {
    Globals::bitboard[last_move.y] = captured_piece;
  }

  //Recall the move bit of the piece.
  Globals::move_bitset[last_move.x] = old_move_bit;
  Globals::move_bitset[last_move.y] = old_move_bit_in_dest;
}

int Interface::AABB(int x, int y) {
  for (int file = 0; file < Bitboard::BOARD_SIZE + 1; ++file) {
    for (int rank = 0; rank < Bitboard::BOARD_SIZE + 1; ++rank) {

      int index = Bitboard::toSquareIndex(file, rank, false);
      SDL_Rect rect = quad_vector[index];

      if (rect.x + rect.w > x && x > rect.x && rect.y + rect.h > y && y > rect.y) {
        MoveGenerator::searchPseudoLegalMoves(index, &MoveGenerator::renderMove);

        return index;
      }
    }
  }

  return Bitboard::Squares::no_sq;
}