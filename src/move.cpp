#include "move.hpp"

namespace MoveGenerator {
// Add a move to the LSF array.
void addMove(const int t_lsf, const int old_lsf) noexcept {
  Globals::legal_moves.push_back(LegalMove{t_lsf, old_lsf});
}

//Set an LSF as a "controlled square."
void addOccupancySquare(const int t_lsf, const int old_lsf) noexcept {
  Globals::opponent_occupancy.push_back(SDL_Point{t_lsf, old_lsf});
}

//Render the move hint.
void renderMove(const int t_lsf, const int old_lsf) noexcept {
  Globals::move_hints.push_back(LegalMove{t_lsf, old_lsf});
}

//Get the total squares travelled by the piece.
int getMaxDeltaSquares(const int delta_lsf, const int lsf_prime) {
  const SDL_Point current_pos = Bitboard::lsfToCoord(lsf_prime);
  const SDL_Point dt_lsf_pos = Bitboard::lsfToCoord(delta_lsf);

  return std::max(static_cast<int>(std::abs(dt_lsf_pos.x - current_pos.x)),
                  static_cast<int>(std::abs(dt_lsf_pos.y - current_pos.y)));
}

//TODO: Implement pawn underpromotion as a "legal move".
void pawnPromotion(const int t_lsf) {
  int piece_color = Bitboard::getColor(Globals::bitboard[t_lsf]);

  int new_type = (piece_color & Bitboard::Sides::WHITE ? Bitboard::Pieces::Q : Bitboard::Pieces::q);

  if (Bitboard::isPawn(Globals::bitboard[t_lsf]) &&
      ((t_lsf >> 3 == 0 && piece_color & Bitboard::Sides::WHITE) ||
       (t_lsf >> 3 == Bitboard::BOARD_SIZE && piece_color & Bitboard::Sides::BLACK))) {
    // Globals::addWindow("Pawn Promotion", 700, 400);

    Globals::should_show_promotion_dialog = true;

    //  Auto queen feature.
    Globals::bitboard[t_lsf] = new_type;
  }
}

//This function moves a bit in the bitboard but does not
//display it in the screen. This is useful for legal move generation.
auto makeMove(const LegalMove& move) -> const ImaginaryMove {
  const int team = Bitboard::getColor(Globals::bitboard[move.x]);

  //Store the old types of the data to be overwritten.
  const int old_piece = Globals::bitboard[move.y];
  const int captured_piece = Globals::bitboard[move.x];

  int en_passant_capture_lsf = Bitboard::Squares::no_sq;
  int en_passant_capture_piece_type = Bitboard::Pieces::e;

  const bool initial_move_bit = Globals::move_bitset[move.y];
  const bool initial_move_bit_new_lsf = Globals::move_bitset[move.x];

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
    en_passant_capture_lsf = (rank_increment << 3) + Globals::en_passant;
    en_passant_capture_piece_type = Globals::bitboard[en_passant_capture_lsf];

    //Remove the bawn from the bitboard temporarily.
    Globals::bitboard[en_passant_capture_lsf] = Bitboard::Pieces::e;
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
      old_piece,   captured_piece, initial_move_bit,       initial_move_bit_new_lsf,
      is_castling, is_en_passant,  en_passant_capture_lsf, en_passant_capture_piece_type,
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
    Globals::bitboard[data.en_passant_capture_lsf] = data.en_passant_capture_piece_type;
  }

  //Restore the old data of the LSFs.
  Globals::bitboard[move.x] = data.captured_piece;
  Globals::bitboard[move.y] = data.old_piece;

  //Restore the bits of the move bitset.
  Globals::move_bitset[move.x] = data.old_move_bit_of_dest;
  Globals::move_bitset[move.y] = data.old_move_bit;
}

//This is useful for the LSF.
[[nodiscard]] const std::string toAlgebraicNotation(int type, int old_lsf, int lsf, bool is_capture,
                                                    bool is_a_castling_move, int dx) {
  static int current_ply_index = 0;

  const std::string file_string = "abcdefgh";
  const std::string rank_string = "87654321";
  const std::string ascii_pieces = ".KQBNR kqbnr ";

  const SDL_Point coord = Bitboard::lsfToCoord(lsf);

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

  //TODO: Add algebraic notation for castling.

  //If it's a pawn, only include the file if the move can alter material.
  if (is_capture && Bitboard::isPawn(type)) {
    const SDL_Point old_coord = Bitboard::lsfToCoord(old_lsf);
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
bool notEmpty(const int t_lsf) {
  return Globals::bitboard[t_lsf] != Bitboard::Pieces::e;
}

bool canCapture(const int t_lsf, const bool for_occupied_square) {
  int which_side = for_occupied_square ? ~Globals::side : Globals::side;
  bool is_allies = which_side & Bitboard::getColor(Globals::bitboard[t_lsf]);
  return !is_allies && notEmpty(t_lsf);
}

void enPassant(const int t_lsf, const std::function<void(int, int)> moveFunc) {
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
  const SDL_Point old_pos = Bitboard::lsfToCoord(last_move.x);
  const SDL_Point new_pos = Bitboard::lsfToCoord(last_move.y);

  // This is for the selected pawn.
  const SDL_Point current_pawn_pos = Bitboard::lsfToCoord(t_lsf);

  // Check if the pawn advances 2 squares for its initial move.
  // TODO: Check if the pawns are in the correct ranks for better
  // verification.

  const int dx = new_pos.x - current_pawn_pos.x;
  const int dy = old_pos.y - new_pos.y;

  const int rank_increment = (Globals::side & Bitboard::Sides::WHITE ? -1 : 1);

  if (dy == 2 * rank_increment && (dx == 1 || dx == -1) && new_pos.y == current_pawn_pos.y) {
    // If the en passant square contains a piece, then en passant is not valid.
    Globals::en_passant = (rank_increment << 3) + last_move.y;

    if (notEmpty(Globals::en_passant)) {
      Globals::en_passant = Bitboard::Squares::no_sq;
    } else {
      moveFunc(Globals::en_passant, t_lsf);
      return;
    }
  }

  Globals::en_passant = Bitboard::Squares::no_sq;
}

void generatePawnCaptures(int t_lsf, const std::function<void(int, int)> moveFunc,
                          bool for_occupied_squares) {
  const int color = Bitboard::getColor(Globals::bitboard[t_lsf]);

  //Instead of using a conditional to check whether it is
  //white to move or black to move, we can use the equation
  //y = 2(x - 1) + b. Where b represents the "mininum index" and
  //2x - 2 determines if the color is 1 or 2 (0b01 or 0b10).

  const int direction_offset_start = (2 * (color - 1)) + 4;
  const int direction_offset_end = (2 * (color - 1)) + 6;

  for (int i = direction_offset_start; i < direction_offset_end; ++i) {
    const int dt_lsf = t_lsf + OFFSETS[i];
    const int max_delta_squares = getMaxDeltaSquares(dt_lsf, t_lsf);

    //Check for friendly pieces.
    const bool can_capture = canCapture(dt_lsf, for_occupied_squares) || for_occupied_squares;

    if (can_capture && max_delta_squares == 1) {
      moveFunc(dt_lsf, t_lsf);
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

void generateSlidingMoves(int t_lsf, std::function<void(int, int)> moveFunc,
                          bool for_occupied_square) {
  const int sliding_piece = Globals::bitboard[t_lsf];

  const int start_index = (Bitboard::isBishop(sliding_piece) ? 4 : 0);
  const int end_index = (Bitboard::isRook(sliding_piece) ? 4 : 8);

  const SDL_Point pos = Bitboard::lsfToCoord(t_lsf);
  getDistToEdge(pos);

  for (int i = start_index; i < end_index; ++i) {
    for (int target_sq = 1; target_sq < Globals::max_squares[i]; target_sq++) {
      const int dt_lsf = target_sq * OFFSETS[i] + t_lsf;

      if (notEmpty(dt_lsf)) {
        if (canCapture(dt_lsf, for_occupied_square) || for_occupied_square) {
          moveFunc(dt_lsf, t_lsf);
        }

        const int king_color =
            (Globals::side & Bitboard::Sides::WHITE ? Bitboard::Pieces::K : Bitboard::Pieces::k);

        bool is_opponent_king = (Globals::bitboard[dt_lsf] == king_color &&
                                 Bitboard::isKing(Globals::bitboard[dt_lsf]));

        if (!is_opponent_king || !for_occupied_square) {
          break;
        }
      }

      moveFunc(dt_lsf, t_lsf);
    }
  }
}

};  // namespace MoveGenerator