#include "evaluation.hpp"

namespace Evaluation {
int evaluatePosition() {
  //TODO: Monitor king safety.
  //TODO: Check for possible checkmates.
  //TODO: Implement endgame tablebases.
  //TODO: Use transposition tables for better opening.
  //TODO: Corner the opposing king to checkmate.

  

  return 0;
}

int evaluateMaterial() {
  int score = 0;

  for (int square = 0; square < 64; ++square) {
    const int type = Globals::bitboard[square];
    const int color = Bitboard::getColor(type);

    if (Bitboard::isPawn(type)) {
      score += (color & 0b10 ? PAWN : -PAWN);
    } else if (Bitboard::isKnight(type)) {
      score += (color & 0b10 ? KNIGHT : -KNIGHT);
    } else if (Bitboard::isBishop(type)) {
      score += (color & 0b10 ? BISHOP : -BISHOP);
    } else if (Bitboard::isRook(type)) {
      score += (color & 0b10 ? ROOK : -ROOK);
    } else if (Bitboard::isQueen(type)) {
      score += (color & 0b10 ? QUEEN : -QUEEN);
    }
  }

  return score;
}
}  // namespace Evaluation