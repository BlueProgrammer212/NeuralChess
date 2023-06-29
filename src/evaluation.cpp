#include "evaluation.hpp"

namespace Evaluation {

const int getPieceValue(const int type) {
  // clang-format off
  using namespace Bitboard;

  return isPawn(type)   * PAWN   |
         isKnight(type) * KNIGHT |
         isBishop(type) * BISHOP |
         isRook(type)   * ROOK   |
         isQueen(type)  * QUEEN  |
         isKing(type)   * KING;
  // clang-format on
}

const std::array<int, Bitboard::NUM_OF_SQUARES>& getPieceSquareTable(int type) {
  //Get the bonus of the square.
  return PIECE_SQUARE_TABLES[(type - 1) % 6];
}

int getSquareValue(const int side, int square, const int type) {
  const int color = Bitboard::getColor(type);

  if (color & Bitboard::Sides::BLACK) {
    square = Bitboard::NUM_OF_SQUARES - square;
  }

  int square_bonus = (color & side) * getPieceSquareTable(type)[square];

  return square_bonus;
}

const int evaluateFactors() {
  // clang-format off
  
  auto [
  
    //Material Evaluation
    material_white, material_black, 

    //Pawn Structure Evaluation
    doubled_pawn_structure_white, doubled_pawn_structure_black,
    blocked_pawns_white, blocked_pawns_black, 
  
    //Square Control Evaluation
    piece_square_table_white, piece_square_table_black
  
  ] = Factors();

  // clang-format on

  const int rank_increment = Globals::side & Bitboard::Sides::WHITE ? -1 : 1;

  //Material evaluation.
  for (int square = 0; square < Bitboard::NUM_OF_SQUARES; ++square) {
    const int type = Globals::bitboard[square];
    const int color = Bitboard::getColor(type);

    if (!MoveGenerator::notEmpty(square)) {
      continue;
    }

    if (!Bitboard::isKing(type)) {
      material_white += (color & 0b10) * getPieceValue(type);
      material_black += (~color & 0b10) * getPieceValue(type);
    }

    //Check if a pawn resides in a same file. (Doubled pawn structure)
    if (Bitboard::isPawn(Globals::bitboard[square]) &&
        Bitboard::isPawn(Globals::bitboard[(rank_increment << 3) + square])) {
      doubled_pawn_structure_white -= (color & 0b10) * 50;
      doubled_pawn_structure_black -= (~color & 0b10) * 50;
    }

    //Blocked pawns
    if (Bitboard::isPawn(Globals::bitboard[square]) &&
        ~(color & Bitboard::getColor(Globals::bitboard[(rank_increment << 3) + square]))) {
      blocked_pawns_white -= (color & 0b10) * 50;
      blocked_pawns_black -= (~color & 0b10) * 50;
    }

    //Evaluate piece square tables.
    piece_square_table_black += getSquareValue(0b01, square, type);
    piece_square_table_white += getSquareValue(0b10, square, type);
  }

  //Evaluate the score of how much control squares there are.
  MoveGenerator::searchForOccupiedSquares(
      MoveGenerator::OccupiedSquareMapFlags::PLAYER_TO_MOVE_OCCUPIED_SQUARES_MAP);

  //Evaluate control squares.
  const int spatialAdvantage = static_cast<int>(Globals::opponent_occupancy.size());

  MoveGenerator::searchForOccupiedSquares(
      MoveGenerator::OccupiedSquareMapFlags::OPPONENT_OCCUPIED_SQUARES_MAP);

  const int spatialDisadvantage = static_cast<int>(Globals::opponent_occupancy.size());

  Globals::side ^= 0b11;

  //Make sure every pieces are active and prevent trapped pieces if possible.
  MoveGenerator::generateLegalMoves();

  const int mobilityDisadvantage = static_cast<int>(Globals::legal_moves.size());

  Globals::side ^= 0b11;

  MoveGenerator::generateLegalMoves();

  const int mobilityAdvantage = static_cast<int>(Globals::legal_moves.size());

  //Evaluate mobility, material, and spatial advantage.
  const int mobility_evaluation = mobilityAdvantage - mobilityDisadvantage;
  const int material_evaluation = material_white - material_black;
  const int spatial_evaluation = spatialAdvantage - spatialDisadvantage;

  const int pawn_structure_eval = (doubled_pawn_structure_white - doubled_pawn_structure_black) +
                                  (blocked_pawns_white - blocked_pawns_black);

  const int central_control_eval = (piece_square_table_white - piece_square_table_white);

  return material_evaluation + (10 * mobility_evaluation) + (10 * spatial_evaluation) +
         pawn_structure_eval + central_control_eval;
}

// clang-format off

std::array<std::array<int, Bitboard::NUM_OF_SQUARES>, 7> PIECE_SQUARE_TABLES = { {
   //King Middlegame
   {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
  },

  //Queen
  {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,   0,  5,  5,  5,  5,  0, -5,
     0,   0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
  },
  
  //Bishop
  { 
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
  },

  //Knight
  {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
  },

  //Rook
  {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
  },

  //Pawn
  {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
  },

  //King Endgame
  {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
 }
} };

// clang-format on
}  // namespace Evaluation