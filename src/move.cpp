#include "move.hpp"
#include "evaluation.hpp"

namespace MoveGenerator {
// Add a move to the square array.
void addMove(const int t_square, const int old_square) noexcept {
  Globals::legal_moves.push_back(LegalMove{t_square, old_square});
}

//Set an square as a "controlled square."
void addOccupancySquare(const int t_square, const int old_square) noexcept {
  Globals::opponent_occupancy.push_back(SDL_Point{t_square, old_square});
}

//Render the move hint.
void renderMove(const int t_square, const int old_square) noexcept {
  Globals::move_hints.push_back(LegalMove{t_square, old_square});
}

//Get the total squares travelled by the piece.
int getMaxDeltaSquares(const int delta_square, const int square_prime) {
  const int delta_x = std::abs((delta_square & 7) - (square_prime & 7));
  const int delta_y = std::abs((delta_square >> 3) - (square_prime >> 3));

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
    //Globals::should_show_promotion_dialog = true;

    //Auto queen implementation.
    Globals::bitboard[t_square] =
        (is_white * Bitboard::Pieces::Q) | (is_black * Bitboard::Pieces::q);

    const int pawn_color = (is_white * 0b10) | (!is_white * 0b01);

    //Modify the first and second LSB.
    Globals::promotion_squares |= pawn_color;
    Globals::promotion_squares &= ~((is_white * 0b01) | (!is_white * 0b10));
  }
}

//This function moves a bit in the bitboard but does not
//display it in the screen. This is useful for legal move generation.
auto makeMove(const LegalMove& move) -> const ImaginaryMove {
  const int team = Bitboard::getColor(Globals::bitboard[move.x]);

  //Store the old types of the data to be overwritten.
  int old_piece = Globals::bitboard[move.y];
  int captured_piece = Globals::bitboard[move.x];

  int old_halfmove_clock = Globals::halfmove_clock;

  if (Globals::side & Bitboard::Sides::WHITE) {
    ++Globals::halfmove_clock;
  }

  if (notEmpty(move.x) || Bitboard::isPawn(old_piece)) {
    Globals::halfmove_clock = 0;
  }

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

  const int rank = move.x >> 3;

  const int pawn_color = team & 0b10 ? Bitboard::Pieces::Q : Bitboard::Pieces::q;
  const int back_rank = team & 0b01 ? Bitboard::BOARD_SIZE : 0;

  if (Bitboard::isPawn(old_piece) && rank == back_rank) {
    //Auto queen implementation.
    Globals::bitboard[move.x] = pawn_color;
  }

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

  const std::uint64_t hash = Globals::zobrist_hashing->hashPosition();
  Globals::position_history.push_back(hash);

  //Use these values to unmake the moves in the bitboard.
  const auto imaginary_move_data = ImaginaryMove{
      old_piece,   captured_piece,    initial_move_bit,          initial_move_bit_new_square,
      is_castling, is_en_passant,     en_passant_capture_square, en_passant_capture_piece_type,
      team,        old_halfmove_clock};

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

  //Restore the old data of the squares.
  Globals::bitboard[move.x] = data.captured_piece;
  Globals::bitboard[move.y] = data.old_piece;

  //Restore the bits of the move bitset.
  Globals::move_bitset[move.x] = data.old_move_bit_of_dest;
  Globals::move_bitset[move.y] = data.old_move_bit;

  //Reset the halfmove clock
  Globals::halfmove_clock = data.old_half_move_clock;

  //Restore the old zobrist hash array.
  Globals::position_history.pop_back();
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

//Check if the square contains a piece or not.
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

  const int direction_offset_start = (color - 1) * 2 + 4;
  const int direction_offset_end = (color - 1) * 2 + 6;

  for (int i = direction_offset_start; i < direction_offset_end; ++i) {
    const int dt_square = t_square + OFFSETS[i];

    // Get the manhattan distance of the pawn to the target square.
    const int max_delta_squares = getMaxDeltaSquares(dt_square, t_square);

    // Check for friendly pieces.
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

void precomputeMaxSquaresToEdge() {
  Globals::precomputed_max_squares_to_edge.resize(64ULL, std::vector<int>(8ULL));

  for (int file = 0; file < Bitboard::BOARD_SIZE + 1; ++file) {
    for (int rank = 0; rank < Bitboard::BOARD_SIZE + 1; ++rank) {
      const SDL_Point pos = {file, rank};

      const int right = Bitboard::MAX_SQUARES_TO_EDGE - pos.x;
      const int left = pos.x + 1;
      const int top = pos.y + 1;
      const int bottom = Bitboard::MAX_SQUARES_TO_EDGE - pos.y;

      Globals::precomputed_max_squares_to_edge.at((rank << 3) + file) = {
          right,   // Right
          left,    // Left
          bottom,  // Bottom
          top,     // Top

          std::min(bottom, right),  // South East
          std::min(bottom, left),   // South West
          std::min(top, right),     // North East
          std::min(top, left),      // North West
      };
    }
  }
}

void generatePawnMoves(const int t_square, const int team, std::function<void(int, int)> moveFunc) {
  const int pawn_range = (Globals::move_bitset[t_square] ? 2 : 3);
  const int pawn_rank_increment = team & 0b10 ? -1 : 1;

  for (int i = 1; i < pawn_range; ++i) {
    int target_square = Bitboard::addRank(t_square, i * pawn_rank_increment);

    // Halt the iteration if there is an intercepting piece.
    if (notEmpty(target_square)) {
      break;
    }

    moveFunc(target_square, t_square);
  }
}

void generateSlidingMoves(int t_square, std::function<void(int, int)> moveFunc,
                          bool for_occupied_square) {
  const int sliding_piece = Globals::bitboard[t_square];

  const bool is_bishop = Bitboard::isBishop(sliding_piece);
  const bool is_rook = Bitboard::isRook(sliding_piece);

  const int start_index = is_bishop ? 4 : 0;
  const int end_index = is_rook ? 4 : 8;

  for (int i = start_index; i < end_index; ++i) {
    const int direction_max_squares = Globals::precomputed_max_squares_to_edge[t_square][i];

    for (int target_sq = 1; target_sq < direction_max_squares; target_sq++) {

      const int dt_square = target_sq * OFFSETS[i] + t_square;
      const int target_sq_type = Globals::bitboard[dt_square];

      if (notEmpty(dt_square)) {
        if (canCapture(dt_square, for_occupied_square) || for_occupied_square) {
          moveFunc(dt_square, t_square);
        }

        const int king_color =
            (Globals::side & Bitboard::Sides::WHITE ? Bitboard::Pieces::K : Bitboard::Pieces::k);

        //Prevent the opponent king from going to the check ray.
        const bool is_opponent_king =
            (target_sq_type == king_color && Bitboard::isKing(Globals::bitboard[dt_square]));

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

  const int king_color = Bitboard::getColor(Globals::bitboard[t_square]);
  const int shift = (king_color & Bitboard::Sides::WHITE) * 2;

  const int target_rook = Globals::bitboard[t_square + target_rook_square];

  for (int dx = 1; dx < max_dx; ++dx) {
    const int delta_square = (flank & 0b01) * (t_square + dx) + (~flank & 0b01) * (t_square - dx);

    const bool rook_conditions =
        !Bitboard::isRook(target_rook) || Globals::move_bitset[t_square + target_rook_square];

    bool is_an_occupied_square = false;

    // If any of the squares are "occupied", temporarily prevent castling.
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

      Globals::castling |= Bitboard::Castle::SHORT_CASTLE << shift;
    } else if (dx == max_dx - 1 && dx == 3) {
      Globals::castling_square.y = t_square - 2;
      moveFunc(t_square - 2, t_square);

      Globals::castling |= Bitboard::Castle::LONG_CASTLE << shift;
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
  const int shift = (king_color & Bitboard::Sides::WHITE) * 2;

  int long_castle = Bitboard::Castle::LONG_CASTLE << shift;
  int short_castle = Bitboard::Castle::SHORT_CASTLE << shift;

  if (Globals::move_bitset[t_square]) {
    // If the king has moved, disable castling for both sides.
    Globals::castling &= ~long_castle;
    Globals::castling &= ~short_castle;

    return;
  }

  // Check flanks for both castling sides.
  Globals::castling = 0;

  for (int flank = 0b01; flank < 0b100; flank <<= 1) {
    isFlankEmpty(flank, t_square, moveFunc);
  }
}

void generateKnightMoves(int t_square, std::function<void(int, int)> moveFunc,
                         bool for_occupied_squares) {
  for (int i = KNIGHT_OFFSET_START; i < KNIGHT_OFFSET_END; ++i) {
    const int dt_square = t_square + OFFSETS[i];

    // The manhattan distance of the target square to the current position
    // must be restricted to only 2 squares.
    const int max_delta_squares = getMaxDeltaSquares(dt_square, t_square);

    // Check if the square will be "out of bounds" if we add the delta square.
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
  for (int i = 0; i < 8; ++i) {
    const int target_square = OFFSETS[i] + t_square;

    // Ensure that the king can only move 1 square.
    const int max_delta_squares = getMaxDeltaSquares(target_square, t_square);

    // Check if the square will be "out of bounds" if we add the delta square.
    const bool is_out_of_bounds = target_square < 0 || target_square > Bitboard::Squares::h8;

    // Check if the square does not contain a friendly piece.
    const bool contains_friendly_piece = notEmpty(target_square) && !canCapture(target_square);

    if ((!contains_friendly_piece || for_occupied_squares) && !is_out_of_bounds &&
        max_delta_squares == 1) {
      moveFunc(target_square, t_square);
    }
  }
}

void searchPseudoLegalMoves(const int t_square, std::function<void(int, int)> moveFunc,
                            bool for_occupied_squares, bool for_legal_moves, bool only_captures) {
  const int type = Globals::bitboard[t_square];
  const int team = Bitboard::getColor(type);

  const bool check_side = for_occupied_squares ? (team & Globals::side) : !(team & Globals::side);

  // Skip empty pieces.
  if (type == Bitboard::Pieces::e || check_side) {
    return;
  }

  // Generate moves for various types of piece.
  switch (type) {
    case Bitboard::Pieces::P:
    case Bitboard::Pieces::p:
      // Capture Offsets
      generatePawnCaptures(t_square, moveFunc, for_occupied_squares);

      if (for_occupied_squares && !only_captures) {
        break;
      }

      generatePawnMoves(t_square, team, moveFunc);
      enPassant(t_square, moveFunc);

      break;
      
    case Bitboard::Pieces::N:
    case Bitboard::Pieces::n:
      generateKnightMoves(t_square, moveFunc, for_occupied_squares);
      break;

    case Bitboard::Pieces::K:
    case Bitboard::Pieces::k:
      generateKingMoves(t_square, moveFunc, for_occupied_squares);

      if (!for_occupied_squares) {
        generateCastlingMove(t_square, moveFunc);
      }
      break;

    default:
      generateSlidingMoves(t_square, moveFunc, for_occupied_squares);
  }
}

void searchForOccupiedSquares(int filter) {
  // Reset the occupancy squares data.
  Globals::opponent_occupancy.clear();

  for (int t_square = 0; t_square < Bitboard::NUM_OF_SQUARES; ++t_square) {
    const int piece_color = Bitboard::getColor(Globals::bitboard[t_square]);

    if ((filter & PAWN_OCCUPIED_SQUARES_MAP && !Bitboard::isPawn(Globals::bitboard[t_square])) ||
        (filter & KING_OCCUPIED_SQUARES_MAP && !Bitboard::isKing(Globals::bitboard[t_square]))) {
      continue;
    }

    const bool which_side =
        ((piece_color & Globals::side) && (filter & OPPONENT_OCCUPIED_SQUARES_MAP)) ||
        (!(piece_color & Globals::side) && (filter & PLAYER_TO_MOVE_OCCUPIED_SQUARES_MAP));

    if (Globals::bitboard[t_square] == Bitboard::Pieces::e || which_side) {
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
    //Locate the square from the square where the king is located.
    const int square = std::distance(std::begin(Globals::bitboard), it);
    return square;
  }

  return Bitboard::Squares::no_sq;
}

const bool isInCheck() {
  const int king = getOwnKing();

  //Reload the occupied squares array.
  MoveGenerator::searchForOccupiedSquares();

  auto willCaptureKing = [king](const SDL_Point& occupied_sq) {
    return occupied_sq.x == king;
  };

  return std::any_of(Globals::opponent_occupancy.begin(), Globals::opponent_occupancy.end(),
                     willCaptureKing);
}

void filterPseudoLegalMoves(std::vector<LegalMove>& hint_square_array, bool only_captures) {
  const std::vector<LegalMove> legal_moves_copy = hint_square_array;

  hint_square_array.clear();  // Clear the vector to rebuild it with valid moves.

  for (const LegalMove& move : legal_moves_copy) {
    if (move.x & Bitboard::Squares::no_sq ||
        (only_captures && Globals::bitboard[move.x] == Bitboard::Pieces::e)) {
      continue;
    }

    const auto& move_data = makeMove(move);

    if (!isInCheck()) {
      hint_square_array.push_back(move);
    }

    unmakeMove(move, move_data);
    searchForOccupiedSquares();
  }
}

std::vector<LegalMove>& generateLegalMoves(const bool only_captures) {
  Globals::legal_moves.clear();

  for (int i = 0; i < Bitboard::NUM_OF_SQUARES; ++i) {
    if (Globals::bitboard[i] == Bitboard::Pieces::e ||
        !(Globals::side & Bitboard::getColor(Globals::bitboard[i]))) {
      continue;
    }

    searchPseudoLegalMoves(i, &addMove, false, true, only_captures);
  }

  filterPseudoLegalMoves(Globals::legal_moves, only_captures);

  return Globals::legal_moves;
}

const bool isInsufficientMaterial() {
  // If there are pawns, then it is not insufficient due to pawn promotion.
  const int pawn_count =
      std::count_if(Globals::bitboard.begin(), Globals::bitboard.end(),
                    [](const int piece) -> bool { return Bitboard::isPawn(piece); });

  if (pawn_count > 0) {
    return false;
  }

  const int KING_VALUE = Evaluation::getPieceValue(Bitboard::Pieces::K);
  const int BISHOP_VALUE = Evaluation::getPieceValue(Bitboard::Pieces::B);

  auto whiteMaterial = [KING_VALUE](int sum, const auto& piece) {
    bool is_white = Bitboard::getColor(piece) & Bitboard::Sides::WHITE;
    return (is_white * Evaluation::getPieceValue(piece)) + sum;
  };

  auto blackMaterial = [KING_VALUE](int sum, const auto& piece) {
    bool is_black = Bitboard::getColor(piece) & Bitboard::Sides::BLACK;
    return (is_black * Evaluation::getPieceValue(piece)) + sum;
  };

  int white_piece_value =
      std::accumulate(Globals::bitboard.begin(), Globals::bitboard.end(), 0, whiteMaterial);

  int black_piece_value =
      std::accumulate(Globals::bitboard.begin(), Globals::bitboard.end(), 0, blackMaterial);

  return (white_piece_value + black_piece_value <= (2 * KING_VALUE) + BISHOP_VALUE);
}

inline const bool noMoreLegalMove() {
  return Globals::legal_moves.size() <= 0U;
}

const bool isInTerminalCondition() {
  // The game must be terminated if it is stalemate or checkmate.
  // We must also check for material sufficiency and threefold repetition.

  return noMoreLegalMove() || isInsufficientMaterial() || isThreefoldRepetition();
}

const bool isCheckmate() {
  // If there is no more legal moves and the king is in check. Then
  // it must be checkmate.

  Globals::is_in_check = isInCheck();
  return Globals::is_in_check && noMoreLegalMove();
}

const bool isStalemate() {
  // If there are no more legal moves and the king is not in check.
  // Then it must be stalemate.

  return !isInCheck() && noMoreLegalMove();
}

const bool isFiftyMoveRule() {
  return Globals::halfmove_clock >= HALFMOVE_CLOCK_THRESHOLD;
}

const bool isThreefoldRepetition() {
  std::unordered_map<std::uint64_t, int> position_occurrences;

  // Count the occurrences of each position
  std::for_each(
      Globals::position_history.begin(), Globals::position_history.end(),
      [&position_occurrences](const std::uint64_t& hash) { ++position_occurrences[hash]; });

  // Check if any position has occurred three times
  auto it = std::find_if(position_occurrences.begin(), position_occurrences.end(),
                         [=](const auto& entry) { return entry.second >= 3; });

  return it != position_occurrences.end();
}

};  // namespace MoveGenerator