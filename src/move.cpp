#include "move.hpp"

namespace MoveGenerator {
// Add a move to the LSF array.
void addMove(int t_lsf, int old_lsf) noexcept {
  Globals::legal_moves.push_back(SDL_Point{t_lsf, old_lsf});
}

//Set an LSF as a "controlled square."
void addOccupancySquare(int t_lsf, int old_lsf) noexcept {
  Globals::opponent_occupancy.push_back(SDL_Point{t_lsf, old_lsf});
}

//Render the move hint.
void renderMove(int t_lsf, int old_lsf) noexcept {
  Globals::move_hints.push_back(SDL_Point{t_lsf, old_lsf});
}

//Get the total squares travelled by the piece.
int getMaxDeltaSquares(int delta_lsf, int lsf_prime) {
  const SDL_Point current_pos = Bitboard::lsfToCoord(lsf_prime);
  const SDL_Point dt_lsf_pos = Bitboard::lsfToCoord(delta_lsf);

  return std::max(static_cast<int>(std::abs(dt_lsf_pos.x - current_pos.x)),
                  static_cast<int>(std::abs(dt_lsf_pos.y - current_pos.y)));
}

//TODO: Implement pawn underpromotion as a "legal move".
void pawnPromotion(int t_lsf) {
  int piece_color = Bitboard::getColor(Globals::bitboard[t_lsf]);

  int new_type = (piece_color & Bitboard::Sides::WHITE ? Bitboard::Pieces::Q : Bitboard::Pieces::q);

  if (Bitboard::isPawn(Globals::bitboard[t_lsf]) &&
      ((t_lsf >> 3 == 0 && piece_color & Bitboard::Sides::WHITE) ||
       (t_lsf >> 3 == Bitboard::BOARD_SIZE && piece_color & Bitboard::Sides::BLACK))) {
    // Globals::addWindow("Pawn Promotion", 700, 400);

    //Globals::should_show_promotion_dialog = true;

    //  Auto queen feature.
    Globals::bitboard[t_lsf] = new_type;
  }
}

//This function moves a bit in the bitboard but does not
//display it in the screen. This is useful for legal move generation.
auto makeMove(const SDL_Point& move) -> const ImaginaryMove {
  //Store the old types of the data to be overwritten.
  int old_piece = Globals::bitboard[move.x];
  int captured_piece = Globals::bitboard[move.y];

  bool old_move_bit = Globals::move_bitset[move.x];
  bool old_move_bit_of_dest = Globals::move_bitset[move.y];

  //Temporarily modify the bitboard.
  Globals::bitboard[move.x] = old_piece;
  Globals::bitboard[move.y] = captured_piece;

  //Update the move bitset temporarily.
  Globals::move_bitset[move.x] = true;
  Globals::move_bitset[move.y] = false;

  //Use these values to unmake the moves in the bitboard.
  return ImaginaryMove{old_piece, captured_piece, old_move_bit, old_move_bit_of_dest};
}

void unmakeMove(const SDL_Point& move, const ImaginaryMove& data) {
  //Restore the old data of the LSFs.
  Globals::bitboard[move.x] = data.old_piece;
  Globals::bitboard[move.y] = data.captured_piece;

  //Restore the bits of the move bitset.
  Globals::move_bitset[move.x] = data.old_move_bit;
  Globals::move_bitset[move.y] = data.old_move_bit_of_dest;
}

bool notEmpty(int t_lsf) {
  return Globals::bitboard[t_lsf] != Bitboard::Pieces::e;
}

};  // namespace MoveGenerator