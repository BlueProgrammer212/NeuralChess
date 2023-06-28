#include "minimax_search.hpp"

Search::Search() {}

Search::~Search() {}

int Search::minimaxSearch(int depth, int alpha, int beta, bool is_maximizing) {
  const int perspective = is_maximizing ? 1 : -1;

  if (depth == 0) {
    // Evaluation for leaf nodes
    return Evaluation::evaluateFactors() * perspective;
  }

  const std::vector<LegalMove> legal_moves_copy = moveOrdering();

  if (MoveGenerator::isCheckmate()) {
    if (Globals::side == Bitboard::Sides::WHITE) {
      return INT_MIN + depth;  // Black has won
    } else {
      return INT_MAX - depth;  // White has won
    }
  }

  if (MoveGenerator::isStalemate() || MoveGenerator::isInsufficientMaterial() ||
      MoveGenerator::isThreefoldRepetition() || MoveGenerator::isFiftyMoveRule()) {
    return 0;
  }

  if (is_maximizing) {
    int maxEval = INT_MIN;

    for (const LegalMove& move : legal_moves_copy) {
      const auto& move_data = MoveGenerator::makeMove(move);

      Globals::side ^= 0b11;

      int score = 0;

      score = minimaxSearch(depth - 1, alpha, beta, false);

      Globals::side ^= 0b11;

      MoveGenerator::unmakeMove(move, move_data);

      maxEval = std::max(score, maxEval);
      alpha = std::max(alpha, maxEval);

      if (alpha >= beta) {
        break;  // Alpha-beta pruning
      }
    }

    return maxEval;
  } else {
    int minEval = INT_MAX;

    for (const LegalMove& move : legal_moves_copy) {
      const auto& move_data = MoveGenerator::makeMove(move);

      Globals::side ^= 0b11;

      int score = 0;

      score = minimaxSearch(depth - 1, alpha, beta, true);

      Globals::side ^= 0b11;

      MoveGenerator::unmakeMove(move, move_data);

      minEval = std::min(score, minEval);
      beta = std::min(beta, minEval);

      if (alpha >= beta) {
        break;  // Alpha-beta pruning
      }
    }

    return minEval;
  }
}

const std::vector<LegalMove>& Search::moveOrdering() {
  std::vector<LegalMove>& moves = MoveGenerator::generateLegalMoves();

  for (LegalMove& move : moves) {
    move.score = 0;

    const int move_piece_type = Globals::bitboard[move.y];
    const int target_piece_type = Globals::bitboard[move.x];

    if (target_piece_type != Bitboard::Pieces::e && !Bitboard::isKing(target_piece_type)) {
      move.score = 10 * Evaluation::getPieceValue(target_piece_type) -
                   Evaluation::getPieceValue(move_piece_type);
    }

    //Look for pawn promotion.
    if (Bitboard::isPawn(move_piece_type) &&
        move.x >> 3 == (7 * (Bitboard::getColor(move_piece_type) & 0b01))) {
      move.score += 900;  //Due to auto-queen.
    }

    //It's usually a bad idea to put a valuable piece in a pawn attack.
    MoveGenerator::searchForOccupiedSquares(MoveGenerator::PAWN_OCCUPIED_SQUARES_MAP);

    auto attackedByPawn = [move_piece_type, &move](const SDL_Point& occupied_square) {
      return occupied_square.x == move.x;
    };

    const bool will_pawn_capture = std::any_of(Globals::opponent_occupancy.begin(),
                                               Globals::opponent_occupancy.end(), attackedByPawn);

    if (will_pawn_capture) {
      //Penalty for moving squares to attacked squares.
      move.score -= Evaluation::PAWN_CAPTURE_PENALTY;
    }

    //Evaluate piece square tables.
    move.score += 10 * Evaluation::getSquareValue(Globals::side, move.x, move_piece_type);

    MoveGenerator::searchForOccupiedSquares();
  }

  std::sort(moves.begin(), moves.end(), [](const LegalMove& a, const LegalMove& b) {
    return a.score > b.score;  // Sort in descending order
  });

  return moves;
}

void Search::playRandomly() {
  if (Globals::side & Bitboard::Sides::WHITE) {
    return;
  }

  Globals::move_delay++;

  if (Globals::move_delay >= 30) {
    MoveGenerator::generateLegalMoves();

    int random = rand() % (static_cast<int>(Globals::legal_moves.size()));

    //En passant is forced.
    if (!(Globals::en_passant & Bitboard::Squares::no_sq) &&
        Globals::en_passant_legal_move_index >= 0) {
      random = Globals::en_passant_legal_move_index;
    }

    Globals::interface_handler->drop(Globals::legal_moves[random].x, Globals::legal_moves[random].y,
                            SHOULD_SUPRESS_HINTS | SHOULD_EXCHANGE_TURN);

    Globals::move_delay = 0;
  }
}

void Search::playBestMove(int depth, const unsigned int human_player) {
  if (Globals::side & human_player) {
    playRandomly();
    return;
  }

  Globals::move_delay++;

  if (Globals::move_delay < 3 || MoveGenerator::isInTerminalCondition()) {
    return;
  }

  const std::vector<LegalMove> legal_moves_copy = MoveGenerator::generateLegalMoves();

  int best_score = INT_MIN;
  LegalMove best_move = legal_moves_copy.back();

  for (const LegalMove& move : legal_moves_copy) {
    const auto& move_data = MoveGenerator::makeMove(move);

    Globals::side ^= 0b11;

    int score = -minimaxSearch(depth - 1, INT_MIN, INT_MAX, true);

    if (score > best_score) {
      best_score = score;
      best_move = move;
    }

    MoveGenerator::unmakeMove(move, move_data);
    Globals::side ^= 0b11;
  }

  Globals::interface_handler->drop(best_move.x, best_move.y,
                                   SHOULD_SUPRESS_HINTS | SHOULD_EXCHANGE_TURN);

  Globals::move_delay = 0;
}