#include "move.hpp"

namespace MoveGenerator {
// Add a move to the LSF array.
void addMove(const int t_square, const int old_square) noexcept {
  Globals::legal_moves.push_back(LegalMove{t_square, old_square});
}

//Set an LSF as a "controlled square."
void addOccupancySquare(const int t_square, const int old_square) noexcept {
  Globals::opponent_occupancy.push_back(SDL_Point{t_square, old_square});
}

//Render the move hint.
void renderMove(const int t_square, const int old_square) noexcept {
  Globals::move_hints.push_back(LegalMove{t_square, old_square});
}

//Get the total squares travelled by the piece.
int getMaxDeltaSquares(const int delta_square, const int square_prime) {
  const auto current_pos = Bitboard::squareToCoord(square_prime);
  const auto dt_square_pos = Bitboard::squareToCoord(delta_square);

  const int delta_x = std::abs(dt_square_pos.x - current_pos.x);
  const int delta_y = std::abs(dt_square_pos.y - current_pos.y);

  return std::max(delta_x, delta_y);
}

//TODO: Implement pawn underpromotion as a "legal move".
void pawnPromotion(const int t_square) {
  const int piece_color = Bitboard::getColor(Globals::bitboard[t_square]);

  if (!Bitboard::isPawn(Globals::bitboard[t_square])) {
    return;
  }

  const int rank = t_square >> 3;
  const bool is_white = piece_color & Bitboard::Sides::WHITE;
  const bool is_black = piece_color & Bitboard::Sides::BLACK;

  if ((rank == 0 && is_white) || (rank == Bitboard::BOARD_SIZE && is_black)) {
    Globals::should_show_promotion_dialog = true;

    const int pawn_color = (is_white * 0b10) | (!is_white * 0b01);

    //Modify the first and second LSB.
    Globals::promotion_squares |= pawn_color;
    Globals::promotion_squares &= ~((is_white * 0b01) | (!is_white * 0b10));

    Globals::side ^= 0b11;
  }
}

//This function moves a bit in the bitboard but does not
//display it in the screen. This is useful for legal move generation.
auto makeMove(const LegalMove& move) -> const ImaginaryMove {
  const int team = Bitboard::getColor(Globals::bitboard[move.x]);

  //Store the old types of the data to be overwritten.
  int old_piece = Globals::bitboard[move.y];
  int captured_piece = Globals::bitboard[move.x];

  int en_passant_capture_square = Bitboard::Squares::no_sq;
  int en_passant_capture_piece_type = Bitboard::Pieces::e;

  const bool initial_move_bit = Globals::move_bitset[move.y];
  const bool initial_move_bit_new_square = Globals::move_bitset[move.x];

  const bool is_en_passant =
      move.x == Globals::en_passant && !(Globals::en_passant & Bitboard::Squares::no_sq);

  const bool is_castling =
      ((move.x == Globals::castling_square.x || move.x == Globals::castling_square.y) &&
       Bitboard::isKing(Globals::bitboard[move.x]) && team & Globals::side);

  //Temporarily modify the bitboard.
  Globals::bitboard[move.x] = old_piece;
  Globals::bitboard[move.y] = Bitboard::Pieces::e;

  if (is_en_passant) {
    const int rank_increment = (Globals::side & Bitboard::Sides::WHITE ? 1 : -1);

    //Find the square where the opponent's pawn is located.
    en_passant_capture_square = (rank_increment << 3) + Globals::en_passant;
    en_passant_capture_piece_type = Globals::bitboard[en_passant_capture_square];

    //Remove the bawn from the bitboard temporarily.
    Globals::bitboard[en_passant_capture_square] = Bitboard::Pieces::e;
  }

  if (is_castling) {
    const int new_rook =
        (team & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

    const int dx = move.x - move.y;

    const int new_rook_pos = (dx < 0 ? 1 : -1);
    const int delta_old_rook_pos = (dx < 0 ? -4 : 3);

    Globals::bitboard[move.y + delta_old_rook_pos] = Bitboard::Pieces::e;
    Globals::bitboard[move.x + new_rook_pos] = new_rook;
  }

  //Update the move bitset temporarily.
  Globals::move_bitset[move.x] = true;
  Globals::move_bitset[move.y] = false;

  //Use these values to unmake the moves in the bitboard.
  const auto imaginary_move_data = ImaginaryMove{
      old_piece,   captured_piece, initial_move_bit,          initial_move_bit_new_square,
      is_castling, is_en_passant,  en_passant_capture_square, en_passant_capture_piece_type,
      team};

  return imaginary_move_data;
}

void unmakeMove(const LegalMove& move, const ImaginaryMove& data) {
  //Undo castling.
  if (data.is_castling) {
    const int new_rook =
        (data.team & Bitboard::Sides::WHITE ? Bitboard::Pieces::R : Bitboard::Pieces::r);

    const int dx = move.x - move.y;

    const int new_rook_pos = (dx < 0 ? 1 : -1);
    const int delta_old_rook_pos = (dx < 0 ? -4 : 3);

    Globals::bitboard[move.y + delta_old_rook_pos] = new_rook;
    Globals::bitboard[move.x + new_rook_pos] = Bitboard::Pieces::e;
  }

  //Undo en passant.
  if (data.is_en_passant) {
    Globals::bitboard[data.en_passant_capture_square] = data.en_passant_capture_piece_type;
  }

  //Restore the old data of the LSFs.
  Globals::bitboard[move.x] = data.captured_piece;
  Globals::bitboard[move.y] = data.old_piece;

  //Restore the bits of the move bitset.
  Globals::move_bitset[move.x] = data.old_move_bit_of_dest;
  Globals::move_bitset[move.y] = data.old_move_bit;
}

//This is useful to translate the square index into algebraic notation.
//TODO: Consider using a struct to increase code readability.
[[nodiscard]] const std::string toAlgebraicNotation(int type, int old_square, int square,
                                                    bool is_capture, bool is_a_castling_move,
                                                    int dx) {
  static int current_ply_index = 0;

  const std::string file_string = "abcdefgh";
  const std::string rank_string = "87654321";
  const std::string ascii_pieces = ".KQBNR kqbnr ";

  const SDL_Point coord = Bitboard::squareToCoord(square);

  std::string algebraic_notation;

  //Note: The sides are flipped.
  if (Globals::side & Bitboard::Sides::BLACK) {
    std::string ply_index_str = std::to_string(++current_ply_index);

    if (current_ply_index > 1) {
      algebraic_notation += " ";
    }

    algebraic_notation += ply_index_str;
    algebraic_notation += ". ";
  }

  if (is_a_castling_move) {
    algebraic_notation += (dx < 0 ? "O-O-O" : "O-O");

    if (Globals::is_in_check) {
      algebraic_notation += "+";
    }

    algebraic_notation += " ";

    return algebraic_notation;
  }

  if (!Bitboard::isPawn(type)) {
    algebraic_notation += std::toupper(ascii_pieces[type]);
  }

  //If it's a pawn, only include the file if the move can alter material.
  if (is_capture && Bitboard::isPawn(type)) {
    const SDL_Point old_coord = Bitboard::squareToCoord(old_square);
    algebraic_notation += file_string[old_coord.x];
  }

  if (is_capture) {
    algebraic_notation += "x";
  }

  //The current square of the piece.
  algebraic_notation += file_string[coord.x];
  algebraic_notation += rank_string[coord.y];

  //Type of termination
  if (Globals::game_state & GameState::CHECKMATE) {

    algebraic_notation += "#";
    std::string winner = (Globals::side & Bitboard::Sides::BLACK ? "1-0" : "0-1");
    algebraic_notation += " " + winner;

    current_ply_index = 0;
  } else if (Globals::is_in_check) {

    algebraic_notation += "+";

  } else if (Globals::game_state & GameState::DRAW) {
    algebraic_notation += " 1/2-1/2";

    current_ply_index = 0;
  }

  algebraic_notation += " ";

  return algebraic_notation;
}

//Check if the LSF contains a piece or not.
bool notEmpty(const int t_square) {
  return Globals::bitboard[t_square] != Bitboard::Pieces::e;
}

bool canCapture(const int t_square, const bool for_occupied_square) {
  int which_side = (for_occupied_square & 0b1) ^ Globals::side;

  bool is_allies = which_side & Bitboard::getColor(Globals::bitboard[t_square]);
  return !is_allies && notEmpty(t_square);
}

void enPassant(const int t_square, const std::function<void(int, int)> moveFunc) {
  if (Globals::ply_array.empty()) {
    Globals::en_passant = Bitboard::Squares::no_sq;
    return;
  }

  SDL_Point last_move = Globals::ply_array.back().move;

  if (!Bitboard::isPawn(Globals::bitboard[last_move.y])) {
    Globals::en_passant = Bitboard::Squares::no_sq;
    return;
  }

  // These are the positions of the last move.
  const SDL_Point old_pos = Bitboard::squareToCoord(last_move.x);
  const SDL_Point new_pos = Bitboard::squareToCoord(last_move.y);

  // This is for the selected pawn.
  const SDL_Point current_pawn_pos = Bitboard::squareToCoord(t_square);

  // Check if the pawn advances 2 squares for its initial move.

  const int dx = new_pos.x - current_pawn_pos.x;
  const int dy = old_pos.y - new_pos.y;

  // clang-format off
  const int rank_increment = ((Globals::side & 0b01) * 1) | 
                             ((~Globals::side & 0b01) * -1);
  // clang-format on

  //En Passant is prevented if the king is on check.
  Globals::is_in_check = isInCheck();

  if (Globals::is_in_check) {
    Globals::en_passant = Bitboard::Squares::no_sq;
    return;
  }

  if (dy == rank_increment << 1 && (dx == 1 || dx == -1) && new_pos.y == current_pawn_pos.y) {
    // If the en passant square contains a piece, then en passant is not valid.
    Globals::en_passant = (rank_increment << 3) + last_move.y;

    if (notEmpty(Globals::en_passant)) {
      Globals::en_passant = Bitboard::Squares::no_sq;
    } else {
      moveFunc(Globals::en_passant, t_square);

      Globals::en_passant_legal_move_index = static_cast<int>(Globals::legal_moves.size()) - 1;
      return;
    }
  }

  Globals::en_passant = Bitboard::Squares::no_sq;
}

void generatePawnCaptures(int t_square, const std::function<void(int, int)> moveFunc,
                          bool for_occupied_squares) {
  const int color = Bitboard::getColor(Globals::bitboard[t_square]);

  const int direction_offset_start = ((color - 1) << 1) + 4;
  const int direction_offset_end = ((color - 1) << 1) + 6;

  for (int i = direction_offset_start; i < direction_offset_end; ++i) {
    const int dt_square = t_square + OFFSETS[i];
    const int max_delta_squares = getMaxDeltaSquares(dt_square, t_square);

    //Check for friendly pieces.
    const bool can_capture = canCapture(dt_square, for_occupied_squares) || for_occupied_squares;

    if (can_capture && max_delta_squares == 1) {
      moveFunc(dt_square, t_square);
    }
  }
}

const std::vector<int>& getDistToEdge(const SDL_Point pos) {
  const int right = Bitboard::MAX_SQUARES_TO_EDGE - pos.x;
  const int left = pos.x + 1;
  const int top = pos.y + 1;
  const int bottom = Bitboard::MAX_SQUARES_TO_EDGE - pos.y;

  Globals::max_squares = {
      right,   // Right
      left,    // Left
      bottom,  // Bottom
      top,     // Top

      std::min(bottom, right),  // South East
      std::min(bottom, left),   // South West
      std::min(top, right),     // North East
      std::min(top, left),      // North West
  };

  return Globals::max_squares;
}

void generateSlidingMoves(int t_square, std::function<void(int, int)> moveFunc,
                          bool for_occupied_square) {
  const int sliding_piece = Globals::bitboard[t_square];

  const bool is_bishop = Bitboard::isBishop(sliding_piece);
  const bool is_rook = Bitboard::isRook(sliding_piece);

  const int start_index = is_bishop * 4;
  const int end_index = (is_rook * 4) | (!is_rook * 8);

  const SDL_Point pos = Bitboard::squareToCoord(t_square);
  getDistToEdge(pos);

  for (int i = start_index; i < end_index; ++i) {
    for (int target_sq = 1; target_sq < Globals::max_squares[i]; target_sq++) {
      const int dt_square = target_sq * OFFSETS[i] + t_square;

      if (notEmpty(dt_square)) {
        if (canCapture(dt_square, for_occupied_square) || for_occupied_square) {
          moveFunc(dt_square, t_square);
        }

        const int king_color =
            (Globals::side & Bitboard::Sides::WHITE ? Bitboard::Pieces::K : Bitboard::Pieces::k);

        bool is_opponent_king = (Globals::bitboard[dt_square] == king_color &&
                                 Bitboard::isKing(Globals::bitboard[dt_square]));

        if (!is_opponent_king || !for_occupied_square) {
          break;
        }
      }

      moveFunc(dt_square, t_square);
    }
  }
}

void isFlankEmpty(int flank, int t_square, std::function<void(int, int)> moveFunc) {
  const int target_rook_square = ((flank & 0b01) * 3) | ((~flank & 0b01) * -4);
  const int max_dx = std::abs(target_rook_square);

  const int target_rook = Globals::bitboard[t_square + target_rook_square];

  for (int dx = 1; dx < max_dx; ++dx) {
    const int delta_square = (flank & 0b01) * (t_square + dx) + (~flank & 0b01) * (t_square - dx);

    const bool rook_conditions =
        !Bitboard::isRook(target_rook) || Globals::move_bitset[t_square + target_rook_square];

    bool is_an_occupied_square = false;

    // If any of the LSFs are "occupied", temporarily prevent castling.
    for (SDL_Point occupied_square : Globals::opponent_occupancy) {
      is_an_occupied_square = (delta_square == occupied_square.x);

      if (is_an_occupied_square) {
        break;
      }
    }

    if (notEmpty(delta_square) || rook_conditions || is_an_occupied_square) {
      break;
    }

    // If there's no intercepting piece, then allow castling.
    if (dx == max_dx - 1 && dx == 2) {
      Globals::castling_square.x = t_square + 2;
      moveFunc(t_square + 2, t_square);
    } else if (dx == max_dx - 1 && dx == 3) {
      Globals::castling_square.y = t_square - 2;
      moveFunc(t_square - 2, t_square);
    }
  }
}

void generateCastlingMove(int t_square, std::function<void(int, int)> moveFunc) {
  //  Reset the data to prevent bugs
  Globals::castling_square.x = Bitboard::Squares::no_sq;
  Globals::castling_square.y = Bitboard::Squares::no_sq;

  // Castling is temporarily prevented if the king is in check.
  if (Globals::is_in_check) {
    return;
  }

  const int king_color = Bitboard::getColor(Globals::bitboard[t_square]);
  const int shift = (king_color & Bitboard::Sides::WHITE ? 0 : 2);

  int long_castle = Bitboard::Castle::LONG_CASTLE << shift;
  int short_castle = Bitboard::Castle::SHORT_CASTLE << shift;

  if (Globals::move_bitset[t_square]) {
    // If the king has moved, disable castling for both sides.
    Globals::castling &= ~long_castle;
    Globals::castling &= ~short_castle;

    return;
  }

  // Check flanks for both castling sides.
  for (int flank = 0b01; flank < 0b100; flank <<= 1) {
    isFlankEmpty(flank, t_square, moveFunc);
  }
}

void generateKnightMoves(int t_square, std::function<void(int, int)> moveFunc,
                         bool for_occupied_squares) {
  for (int i = KNIGHT_OFFSET_START; i < KNIGHT_OFFSET_END; ++i) {
    const int dt_square = t_square + OFFSETS[i];

    // The change or the delta position must be restricted to only 2 squares.
    const int max_delta_squares = getMaxDeltaSquares(dt_square, t_square);

    // Check if the square will be "out of bounds" if we add the delta LSF.
    const bool is_out_of_bounds = dt_square < 0 || dt_square > Bitboard::Squares::h8;

    // Check if the square does not contain a friendly piece.
    const bool contains_friendly_piece =
        notEmpty(dt_square) && !canCapture(dt_square, for_occupied_squares);

    if ((for_occupied_squares || !contains_friendly_piece) && !is_out_of_bounds &&
        max_delta_squares == 2) {
      moveFunc(dt_square, t_square);
    }
  }
}

void generateKingMoves(int t_square, std::function<void(int, int)> moveFunc,
                       bool for_occupied_squares) {
  int king_moves = 0;

  for (int i = 0; i < 8; ++i) {
    const int target_square = OFFSETS[i] + t_square;

    // Ensure that the king can only move 1 square.
    const int max_delta_squares = getMaxDeltaSquares(target_square, t_square);

    // Check if the square will be "out of bounds" if we add the delta LSF.
    const bool is_out_of_bounds = target_square < 0 || target_square > Bitboard::Squares::h8;

    // Check if the square does not contain a friendly piece.
    const bool contains_friendly_piece = notEmpty(target_square) && !canCapture(target_square);

    // Check if the square is in the occupancy square array.
    bool is_an_occupied_square = false;

    for (SDL_Point occupied_square : Globals::opponent_occupancy) {
      is_an_occupied_square = (target_square == occupied_square.x);

      if (is_an_occupied_square) {
        break;
      }
    }

    if (is_an_occupied_square) {
      continue;
    }

    if ((!contains_friendly_piece || for_occupied_squares) && !is_out_of_bounds &&
        max_delta_squares == 1) {
      moveFunc(target_square, t_square);
      king_moves++;
    }
  }
}

void searchPseudoLegalMoves(const int t_square, std::function<void(int, int)> moveFunc,
                            bool for_occupied_squares, bool for_legal_moves) {
  const int team = Bitboard::getColor(Globals::bitboard[t_square]);

  bool check_side = (for_occupied_squares && team & Globals::side) ||
                    (!for_occupied_squares && !(team & Globals::side));

  // Skip empty pieces.
  if (Globals::bitboard[t_square] == Bitboard::Pieces::e || check_side) {
    return;
  }

  const int pawn_range = (Globals::move_bitset[t_square] ? 2 : 3);
  int target_square;

  // Generate moves for various types of piece.
  switch (Globals::bitboard[t_square]) {
    case Bitboard::Pieces::P:
      if (!for_occupied_squares) {
        for (int i = 1; i < pawn_range; ++i) {
          target_square = Bitboard::addRank(t_square, -i);

          // Halt the iteration if there is an intercepting piece.
          if (notEmpty(target_square)) {
            break;
          }

          moveFunc(target_square, t_square);
        }

        enPassant(t_square, moveFunc);
      }

      // Capture Offsets
      generatePawnCaptures(t_square, moveFunc, for_occupied_squares);

      break;
    case Bitboard::Pieces::p:
      if (!for_occupied_squares) {
        for (int i = 1; i < pawn_range; ++i) {
          target_square = Bitboard::addRank(t_square, i);

          // Halt the iteration if there is an intercepting piece.
          if (notEmpty(target_square)) {
            break;
          }

          moveFunc(target_square, t_square);
        }

        enPassant(t_square, moveFunc);
      }

      // Capture Offsets
      generatePawnCaptures(t_square, moveFunc, for_occupied_squares);

      break;
    case Bitboard::Pieces::N:
      generateKnightMoves(t_square, moveFunc, for_occupied_squares);
      break;
    case Bitboard::Pieces::n:
      generateKnightMoves(t_square, moveFunc, for_occupied_squares);
      break;

    case Bitboard::Pieces::K:
      generateKingMoves(t_square, moveFunc, for_occupied_squares);
      if (!for_occupied_squares) {
        generateCastlingMove(t_square, moveFunc);
      }
      break;

    case Bitboard::Pieces::k:
      generateKingMoves(t_square, moveFunc, for_occupied_squares);
      if (!for_occupied_squares) {
        generateCastlingMove(t_square, moveFunc);
      }
      break;
  }

  if (Bitboard::isSlidingPiece(Globals::bitboard[t_square])) {
    generateSlidingMoves(t_square, moveFunc, for_occupied_squares);
  }
}

void searchForOccupiedSquares() {
  // Reset the occupancy squares data.
  Globals::opponent_occupancy.clear();

  for (int t_square = 0; t_square < Bitboard::Squares::h8 + 1; ++t_square) {
    const int piece_color = Bitboard::getColor(Globals::bitboard[t_square]);

    if (Globals::bitboard[t_square] == Bitboard::Pieces::e || piece_color & Globals::side) {
      continue;
    }

    searchPseudoLegalMoves(t_square, &addOccupancySquare, true);
  }
}

const int getOwnKing() {
  // Yields {1, 7} depending on the player to move.
  const int piece_type = ((Globals::side & 0b01) * 7) | ((~Globals::side & 0b01) * 1);

  const auto it = std::find(std::begin(Globals::bitboard), std::end(Globals::bitboard), piece_type);

  if (it != std::end(Globals::bitboard)) {
    //Locate the square from the LSF where the king is located.
    const int square = std::distance(std::begin(Globals::bitboard), it);
    return square;
  }

  return Bitboard::Squares::no_sq;
}

const bool isInCheck() {
  int king = getOwnKing();
  MoveGenerator::searchForOccupiedSquares();

  for (SDL_Point occupied_sq : Globals::opponent_occupancy) {
    // If any squares matches with the king's LSF, then the king must in check.
    if (occupied_sq.x == king) {
      return true;
    }
  }

  return false;
}

void filterPseudoLegalMoves(const int t_square, std::vector<LegalMove>& hint_square_array) {
  if (!notEmpty(t_square)) {
    return;
  }

  const std::vector<LegalMove> legal_moves_copy = hint_square_array;

  // Loop through all of the legal moves.
  for (int i = 0; i < static_cast<int>(hint_square_array.size()); ++i) {
    const LegalMove& move = legal_moves_copy[i];

    // If the move is deleted, ignore it.
    if (move.x & Bitboard::Squares::no_sq) {
      continue;
    }

    // Make the move and record the captured piece and old move bits to unmake the move.
    const auto& move_data = makeMove(move);

    // Delete the move if it puts the king in check.
    if (isInCheck()) {
      hint_square_array[i].x = Bitboard::Squares::no_sq;
    }

    // Restore the initial bitboard data.
    unmakeMove(move, move_data);

    searchForOccupiedSquares();
  }
}

const std::vector<LegalMove>& generateLegalMoves() {
  Globals::legal_moves.clear();
  for (int i = 0; i < Bitboard::Squares::h8 + 1; ++i) {
    if (Globals::bitboard[i] == Bitboard::Pieces::e ||
        !(Globals::side & Bitboard::getColor(Globals::bitboard[i]))) {
      continue;
    }

    searchPseudoLegalMoves(i, &addMove, false, true);
    filterPseudoLegalMoves(i, Globals::legal_moves);
  }

  return Globals::legal_moves;
}

};  // namespace MoveGenerator