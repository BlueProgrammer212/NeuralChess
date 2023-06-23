#include "game.hpp"

using namespace Globals;

Game::Game()
    : m_running(true),
      m_interface(std::make_unique<Interface>()),
      m_fen_parser(FenParser::getInstance()),
      m_console(GetStdHandle(STD_OUTPUT_HANDLE)),
      m_show_occupied_squares(false) {

  SetConsoleTextAttribute(m_console, 10);

  std::cout << R"( _   _ ______ _    _ _____            _      _____ _    _ ______  _____ _____ 
| \ | |  ____| |  | |  __ \     /\   | |    / ____| |  | |  ____|/ ____/ ____|
|  \| | |__  | |  | | |__) |   /  \  | |   | |    | |__| | |__  | (___| (___  
| . ` |  __| | |  | |  _  /   / /\ \ | |   | |    |  __  |  __|  \___ \\___ \ 
| |\  | |____| |__| | | \ \  / ____ \| |___| |____| |  | | |____ ____) |___) |
|_| \_|______|\____/|_|  \_\/_/    \_\______\_____|_|  |_|______|_____/_____/ 
                                                                            )"
            << "\n\n\n\n";

  SetConsoleTextAttribute(m_console, 15);

  std::cout << R"(
----------KEY SHORTCUTS-----------
P: Run perft test. 
R: Remove the selected piece.
C: Reset the board to the initial position.
O: Reveal the occupied/controlled squares of the adversary. 
(toggle)
P: Go to the previous move.
----------------------------------)"
            << "\n\n\n";
}

Game::~Game() {
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);

  SDL_Quit();
  Mix_Quit();
  IMG_Quit();
}

void Game::playRandomly() {
  if (Globals::side & Bitboard::Sides::WHITE) {
    return;
  }

  move_delay++;

  if (move_delay >= 30) {
    MoveGenerator::generateLegalMoves();

    if (game_state & GameState::DRAW || game_state & GameState::CHECKMATE) {
      resetBoard();

      return;
    }

    int random = rand() % (static_cast<int>(Globals::legal_moves.size()));

    if (Globals::legal_moves[random].x & Bitboard::Squares::no_sq) {
      playRandomly();
      return;
    }

    //En passant is forced.
    if (!(Globals::en_passant & Bitboard::Squares::no_sq) &&
        Globals::en_passant_legal_move_index >= 0) {
      random = Globals::en_passant_legal_move_index;
    }

    m_interface->drop(Globals::legal_moves[random].x, Globals::legal_moves[random].y,
                      SHOULD_SUPRESS_HINTS | SHOULD_EXCHANGE_TURN);

    move_delay = 0;
  }
}

int Game::minimaxSearch(int depth, int alpha, int beta, bool is_maximizing) {
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

  if (is_maximizing) {
    int maxEval = INT_MIN;

    for (const LegalMove& move : legal_moves_copy) {
      const auto& move_data = MoveGenerator::makeMove(move);

      side ^= 0b11;

      int score = 0;

      if (MoveGenerator::isCheckmate()) {
        score = INT_MAX;
      } else {
        score = minimaxSearch(depth - 1, alpha, beta, false);
      }

      side ^= 0b11;

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

      side ^= 0b11;

      int score = 0;

      if (MoveGenerator::isCheckmate()) {
        score = INT_MIN;
      } else {
        score = minimaxSearch(depth - 1, alpha, beta, true);
      }

      side ^= 0b11;

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

const std::vector<LegalMove>& Game::moveOrdering() {
  std::vector<LegalMove>& moves = MoveGenerator::generateLegalMoves();

  for (LegalMove& move : moves) {
    const int move_piece_type = Globals::bitboard[move.y];
    const int target_piece_type = Globals::bitboard[move.x];

    if (target_piece_type != Bitboard::Pieces::e) {
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

void Game::playBestMove(int depth) {
  if (Globals::side & Bitboard::Sides::WHITE) {
    return;
  }

  move_delay++;

  if (move_delay < 5 || MoveGenerator::isInTerminalCondition()) {
    return;
  }

  is_calculating = true;

  const std::vector<LegalMove> legal_moves_copy = MoveGenerator::generateLegalMoves();

  int best_score = INT_MIN;
  LegalMove best_move = legal_moves_copy.back();

  for (const LegalMove& move : legal_moves_copy) {
    const auto& move_data = MoveGenerator::makeMove(move);

    side ^= 0b11;

    int score = -minimaxSearch(depth - 1, INT_MIN, INT_MAX, true);

    if (score > best_score) {
      best_score = score;
      best_move = move;
    }

    MoveGenerator::unmakeMove(move, move_data);
    side ^= 0b11;
  }

  m_interface->drop(best_move.x, best_move.y, SHOULD_SUPRESS_HINTS | SHOULD_EXCHANGE_TURN);

  move_delay = 0;

  is_calculating = false;
}

void Game::init(const int width, const int height) {
  SDL_Init(SDL_INIT_EVERYTHING);

  //Initialize the SDL Mixer Library.
  if (Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, 2, CHUNK_SIZE) == -1) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL_Mixer: %s\n",
                 SDL_GetError());
    return;
  }

  //Initialize window.
  window = SDL_CreateWindow("NeuralChess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                            height, SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_FOCUS);

  //Initialize the renderer.
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

  if (renderer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create the renderer.");
    m_running = false;
  }

  BOX_WIDTH = 600 / (Bitboard::BOARD_SIZE + 1);
  BOX_HEIGHT = 600 / (Bitboard::BOARD_SIZE + 1);

  //Load the chess piece texture atlas.
  TextureManager::LoadTexture("C:\\Users\\yayma\\Documents\\NeuralChess\\res\\atlas.png");

  SDL_RenderSetLogicalSize(renderer, width, height);

  //Save the texture dimensions.
  TextureManager::QueryTexture(6, 2);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  //Precalculate chess board squares for the UI.
  for (int file = 0; file < (Bitboard::BOARD_SIZE + 1); ++file) {
    for (int rank = 0; rank < (Bitboard::BOARD_SIZE + 1); ++rank) {
      SDL_Rect rect = {file * BOX_WIDTH, rank * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};
      quad_vector.push_back(rect);
    }
  }

  //By default, it's white to move.
  side |= Bitboard::WHITE;

  //Initialize the bitboard using the FEN parser.
  m_fen_parser.init();

  MoveGenerator::searchForOccupiedSquares();

  is_in_check = MoveGenerator::isInCheck();

  //Update the legal move array.
  MoveGenerator::generateLegalMoves();

  if (is_in_check) {
    audio_manager->PlayWAV("../../res/check.wav");
    square_of_king_in_check = MoveGenerator::getOwnKing();
  }

  //Settings::init();

  MoveGenerator::renderMove(-1, -1);
}

void Game::update() {
  time += 1;
  delta_time = time - last_time;
  last_time = time;
}

void Game::render() {
  SDL_RenderClear(renderer);

  // Render the chess board.
  for (int square = 0; square < static_cast<int>(quad_vector.size()); ++square) {
    const auto& pos = Bitboard::squareToCoord(square);
    if (((pos.x + pos.y) & 1) == 0) {
      SDL_SetRenderDrawColor(renderer, 249, 224, 169, 255);
    } else {
      SDL_SetRenderDrawColor(renderer, 176, 112, 56, 255);
    }

    SDL_RenderFillRect(renderer, &quad_vector[square]);
  }

  //Render the legal moves. (For debugging purposes.)
  if (show_legal_moves) {
    for (LegalMove square : Globals::legal_moves) {
      const auto& pos = Bitboard::squareToCoord(square.x);

      const SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_WIDTH};

      SDL_SetRenderDrawColor(renderer, 75 * square.y, 255, 125, 125);
      SDL_RenderFillRect(renderer, &dest);
    }
  }

  // //Render the opponent occupied squares.
  if (m_show_occupied_squares) {
    for (SDL_Point square : Globals::opponent_occupancy) {
      const auto& pos = Bitboard::squareToCoord(square.x);

      const SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_WIDTH};

      SDL_SetRenderDrawColor(renderer, 0, 125, 255, 125);
      SDL_RenderFillRect(renderer, &dest);
    }
  }

  //Highlight the selected quad.
  if (selected_square != Bitboard::no_sq) {
    const auto& pos = Bitboard::squareToCoord(selected_square);
    SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};

    SDL_SetRenderDrawColor(renderer, 255, 255, 125, 125);

    SDL_RenderFillRect(renderer, &dest);
  }

  //Highlight the king in check.
  if (is_in_check) {
    const auto& pos = Bitboard::squareToCoord(square_of_king_in_check);

    const SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_WIDTH};

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
    SDL_RenderFillRect(renderer, &dest);
  }

  //Render hints.
  if (display_legal_move_hints) {
    for (const LegalMove& hint : move_hints) {
      if (hint.x & Bitboard::Squares::no_sq) {
        continue;
      }

      const auto& pos = Bitboard::squareToCoord(hint.x);

      const SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_WIDTH};

      SDL_SetRenderDrawColor(renderer, 255, 60, 70, 125);
      SDL_RenderFillRect(renderer, &dest);
    }
  }

  //Highlight the last move.

  //Prevent segmentation fault.
  if (!Globals::ply_array.empty()) {
    SDL_Point last_move = Globals::ply_array.back().move;

    if (!(last_move.x & Bitboard::no_sq) && !(last_move.y & Bitboard::no_sq)) {
      const auto& initial_pos = Bitboard::squareToCoord(last_move.x);
      const auto& pos = Bitboard::squareToCoord(last_move.y);

      const SDL_Rect dest = {initial_pos.x * BOX_WIDTH, initial_pos.y * BOX_HEIGHT, BOX_WIDTH,
                             BOX_HEIGHT};

      const SDL_Rect final_dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};

      SDL_SetRenderDrawColor(renderer, 200, 200, 0, 125);
      SDL_RenderFillRect(renderer, &dest);

      SDL_SetRenderDrawColor(renderer, 220, 220, 0, 125);
      SDL_RenderFillRect(renderer, &final_dest);
    }
  }

  //Render the pieces.
  for (int i = 0; i < static_cast<int>(bitboard.size()); i++) {
    TextureManager::AnimatePiece(i, bitboard[i]);
  }

  if (!(Globals::selected_square & Bitboard::no_sq) && Globals::is_mouse_down) {
    TextureManager::DrawPiece(bitboard[selected_square]);
  }

  //Render the evaluation bar.
  //TODO: Refactor this later.
  SDL_Rect black_eval_rect = {600, 0, 25, black_eval / 10};
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
  SDL_RenderFillRect(renderer, &black_eval_rect);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  if (should_show_promotion_dialog) {
    SDL_Rect bg_color = {0, 0, 600, 600};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 125);
    SDL_RenderFillRect(renderer, &bg_color);

    //Promotion dialog container.
    const int screenWidth = 600;
    const int screenHeight = 600;

    //Specify the dimensions of the dialog.
    const int dialogWidth = 400;
    const int dialogHeight = 250;

    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    SDL_Rect promotion_dialog = {dialogX, dialogY, dialogWidth, dialogHeight};

    SDL_SetRenderDrawColor(renderer, 0, 175, 190, 255);
    SDL_RenderFillRect(renderer, &promotion_dialog);

    //Render button test.
    for (int i = 0; i < 4; ++i) {
      const int pawn_color = ((promotion_squares & 0b01) * 334);

      SDL_Rect button_source = {(i % 6 + 1) * 333, pawn_color, 333, 334};

      const int margin = 27;

      SDL_Rect button_dest = {dialogX + margin + (i * (BOX_WIDTH + 15)), dialogY + 70,
                              BOX_WIDTH + 10, BOX_HEIGHT + 10};

      SDL_RenderCopy(renderer, texture, &button_source, &button_dest);
    }
  }

  //For white eval value.
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_RenderPresent(renderer);
}

void Game::resetBoard() {
  std::cout << "\nRestoring initial position... Please wait\n";

  move_bitset.reset();
  opponent_occupancy.clear();

  //Clear the moves recorded from the previous game.
  ply_array.clear();
  move_squares.clear();

  //Reset every data.
  is_in_check = false;

  square_of_king_in_check = Bitboard::Squares::no_sq;

  m_fen_parser.init();

  is_in_check = MoveGenerator::isInCheck();

  //Update the legal move array.
  MoveGenerator::generateLegalMoves();

  should_show_promotion_dialog = false;

  if (is_in_check) {
    audio_manager->PlayWAV("../../res/check.wav");
    square_of_king_in_check = MoveGenerator::getOwnKing();
  }

  //Clear all the bits and add the OPENING flag on restart.
  game_state = GameState::OPENING;
}

void Game::events() {
  SDL_Event event;

  int new_square;

  while (SDL_PollEvent(&event)) {
    switch (event.window.type) {
      case SDL_QUIT:
        m_running = false;
        break;

      case SDL_MOUSEMOTION:
        if (Globals::game_state & GameState::DRAW || Globals::game_state & GameState::CHECKMATE) {
          break;
        }

        if (selected_square != Bitboard::no_sq && is_mouse_down) {
          SDL_GetMouseState(&mouse_coord.x, &mouse_coord.y);
        }

        break;

      case SDL_MOUSEBUTTONDOWN:
        //If the game already ended, then we should disable board interaction.
        if (Globals::game_state & GameState::DRAW || Globals::game_state & GameState::CHECKMATE) {
          break;
        }

        //If the dialog is open, board interaction is temporarily disabled.
        if (Globals::should_show_promotion_dialog) {

          break;
        }

        SDL_GetMouseState(&mouse_coord.x, &mouse_coord.y);

        new_square = Interface::AABB(event.button.y, event.button.x);

        if (selected_square == Bitboard::no_sq && Globals::bitboard[new_square] != Bitboard::e) {
          //If there is no selected square, then allow the selection.
          selected_square = (Bitboard::SHOULD_FLIP ? new_square ^ 0x38 : new_square);
          //is_mouse_down = true;

          MoveGenerator::filterPseudoLegalMoves(Globals::move_hints);

          break;
        }

        //If there is a selected square, then drop it to the new square.
        m_interface->drop(new_square, selected_square, SHOULD_EXCHANGE_TURN);

        //If it's not checkmate yet or draw.
        // if (game_state & GameState::OPENING) {
        //   playRandomly();
        // }

        Globals::move_hints.clear();
        Globals::selected_square = Bitboard::no_sq;

        break;

      case SDL_MOUSEBUTTONUP:
        if (Globals::game_state & GameState::DRAW || Globals::game_state & GameState::CHECKMATE ||
            !is_mouse_down || selected_square & Bitboard::no_sq) {
          break;
        }

        new_square = Interface::AABB(event.button.y, event.button.x);
        m_interface->drop(new_square, selected_square, SHOULD_EXCHANGE_TURN);

        Globals::move_hints.clear();
        Globals::selected_square = Bitboard::no_sq;

        is_mouse_down = false;

        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_p) {
          playBestMove(2);
        }

        if (event.key.keysym.sym == SDLK_r && selected_square != Bitboard::Squares::no_sq) {
          Globals::bitboard[selected_square] = Bitboard::e;

          is_in_check = MoveGenerator::isInCheck();

          //Update the legal move array.
          MoveGenerator::generateLegalMoves();

          if (is_in_check) {
            audio_manager->PlayWAV("../../res/check.wav");
            square_of_king_in_check = MoveGenerator::getOwnKing();
          }

          Globals::move_hints.clear();
        }

        if (event.key.keysym.sym == SDLK_o) {
          m_show_occupied_squares ^= true;
        }

        if (event.key.keysym.sym == SDLK_h) {
          display_legal_move_hints ^= true;
        }

        if (event.key.keysym.sym == SDLK_l) {
          show_legal_moves ^= true;
        }

        if (event.key.keysym.sym == SDLK_c) {
          resetBoard();
        }

        if (Globals::game_state & GameState::DRAW) {
          break;
        }

        if (event.key.keysym.sym == SDLK_u) {
          m_interface->undo();
        }

        break;
    }
  }
}