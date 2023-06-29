#include <iostream>
#include "mingw.thread.h"

#include <chrono>

#include "game.hpp"

// Flag to indicate whether the AI thread is currently computing
bool is_ai_computing = false;

void testMoveGenerationHelper(int depth) {
  const std::vector<LegalMove> moves = MoveGenerator::generateLegalMoves(false);

  if (depth <= 0) {
    return;
  }

  for (const auto& move : moves) {
    const ImaginaryMove& move_data = MoveGenerator::makeMove(move);
    Globals::side ^= 0b11;

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    testMoveGenerationHelper(depth - 1);

    MoveGenerator::unmakeMove(move, move_data);
    Globals::side ^= 0b11;
  }
}

void testMoveGeneration(Game* game_ptr) {
  while (game_ptr->isRunning()) {
    is_ai_computing = true;

    testMoveGenerationHelper(4);

    is_ai_computing = false;

    // Pause for 1 second before starting the next iteration
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    break;
  }
}

void aiThreadFunction(Game* game_ptr) {
  while (game_ptr->isRunning()) {
    // Set the flag to indicate that the AI thread is computing
    is_ai_computing = true;

    game_ptr->playBestMove(2, Bitboard::Sides::WHITE);

    // Reset the flag once the AI computation is done
    is_ai_computing = false;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main(int argc, char* argv[]) {
  bool show_evaluation_bar = false;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--show-eval") {
      show_evaluation_bar = true;
      break;
    }
  }

  auto game_ptr = std::make_unique<Game>();

  game_ptr->init(600 + (show_evaluation_bar * 25), 600);

  constexpr int FPS = 60;
  constexpr int FRAME_DELAY = FPS / 1000;

  unsigned int frame_start = 0;
  int frame_time = 0;

  std::thread ai_thread(aiThreadFunction, game_ptr.get());

  // Game Loop
  while (game_ptr->isRunning()) {
    frame_start = SDL_GetTicks();

    game_ptr->update();

    // Only render if the AI thread is not currently computing
    if (!is_ai_computing) {
      game_ptr->render();
    }

    game_ptr->events(is_ai_computing);

    frame_time = SDL_GetTicks() - frame_start;

    if (FRAME_DELAY > frame_time) {
      SDL_Delay(FRAME_DELAY - frame_time);
    }
  }

  std::cout << "\n\nWaiting for the AI thread to finish. Please wait.\n";

  ai_thread.join();

  return 0;
}