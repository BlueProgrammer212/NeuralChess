#include "evaluation.hpp"

namespace Evaluation {
int pawn_count = 0;
int bishop_count = 0;
int knight_count = 0;
int rook_count = 0;
int queen_count = 0;

int countMaterial() {
  Globals::black_eval = 0;

  pawn_count = 0;
  bishop_count = 0;
  knight_count = 0;
  rook_count = 0; 
  queen_count = 0;

  for (int type : Globals::bitboard) {
    int color = Bitboard::getColor(type);

    if (color & Bitboard::Sides::WHITE ||
        type == Bitboard::Pieces::e) {
        continue;
    } 

    if (Bitboard::isPawn(type)) {
      ++pawn_count;
    } else if (Bitboard::isBishop(type)) {
      ++bishop_count;
    } else if (Bitboard::isKnight(type)) {
      ++knight_count;
    } else if (Bitboard::isRook(type)) {
      ++rook_count;
    } else if (Bitboard::isQueen(type)) {
      ++queen_count;
    }
  }

  //Summation of material value.
  Globals::black_eval += pawn_count * MaterialValue::PAWN;
  Globals::black_eval += bishop_count * MaterialValue::BISHOP;
  Globals::black_eval += knight_count * MaterialValue::KNIGHT;
  Globals::black_eval += rook_count * MaterialValue::ROOK;
  Globals::black_eval += queen_count * MaterialValue::QUEEN;

  return Globals::black_eval;
}
}  // namespace Evaluation