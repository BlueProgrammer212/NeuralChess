#include "evaluation.hpp"

namespace Evaluation {
int getPieceValue(int type) {
  if (Bitboard::isPawn(type)) {
    return PAWN;
  } else if (Bitboard::isKnight(type)) {
    return KNIGHT;
  } else if (Bitboard::isBishop(type)) {
    return BISHOP;
  } else if (Bitboard::isRook(type)) {
    return ROOK;
  } else if (Bitboard::isQueen(type)) {
    return QUEEN;
  } else if (Bitboard::isKing(type)) {
    return KING;
  }

  return 0;
}

int evaluateFactors() {
  if (MoveGenerator::isCheckmate()) {
    return INT_MAX;
  }

  int material_white = 0;
  int material_black = 0;

  int pawn_structure_white = 0;
  int pawn_structure_black = 0;

  // const int king = MoveGenerator::getOwnKing();

  // const int friendlyPawn = ((Globals::side & 0b01) * Bitboard::Pieces::p |
  //                           ((~Globals::side & 0b01) * Bitboard::Pieces::P));

  const int rank_increment = ((Globals::side & 0b01) * 1) | ((~Globals::side & 0b01) * -1);

  //Material evaluation.
  for (int square = 0; square < 64; ++square) {
    const int type = Globals::bitboard[square];
    const int color = Bitboard::getColor(type);

    if (!MoveGenerator::notEmpty(square)) {
      continue;
    }

    material_white += (color & 0b10) * getPieceValue(type);
    material_black += (~color & 0b10) * getPieceValue(type);

    //Prevent doubled pawn structure.
    if (Bitboard::isPawn(Globals::bitboard[square]) &&
        Bitboard::isPawn(Globals::bitboard[(rank_increment << 3) + square])) {
      pawn_structure_white -= (color & 0b10) * getPieceValue(type);
      pawn_structure_black -= (~color & 0b10) * getPieceValue(type);
    }
  }

  //Evaluate the score of how much control squares there are.
  MoveGenerator::searchForOccupiedSquares(
      MoveGenerator::OccupiedSquareMapFlags::PLAYER_TO_MOVE_OCCUPIED_SQUARES_MAP);

  //Evaluate control squares.
  const int spatialAdvantage = static_cast<int>(Globals::opponent_occupancy.size()) * 10;

  MoveGenerator::searchForOccupiedSquares(
      MoveGenerator::OccupiedSquareMapFlags::OPPONENT_OCCUPIED_SQUARES_MAP);

  const int spatialDisadvantage = static_cast<int>(Globals::opponent_occupancy.size()) * 10;

  Globals::side ^= 0b11;

  //Make sure every pieces are active and prevent trapped pieces if possible.
  MoveGenerator::generateLegalMoves();

  const int mobilityDisadvantage = static_cast<int>(Globals::legal_moves.size()) * 10;

  Globals::side ^= 0b11;

  MoveGenerator::generateLegalMoves();

  const int mobilityAdvantage = static_cast<int>(Globals::legal_moves.size()) * 10;

  const int mobility_evaluation = mobilityAdvantage - mobilityDisadvantage;
  const int material_evaluation = material_white - material_black;
  const int spatial_evaluation = spatialAdvantage - spatialDisadvantage;

  const int pawn_structure_eval = pawn_structure_white - pawn_structure_black;

  return material_evaluation + mobility_evaluation + spatial_evaluation + pawn_structure_eval;
}
}  // namespace Evaluation