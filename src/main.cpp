#include <iostream>
#include "mingw.thread.h"

#include <chrono>

#include "game.hpp"

auto game_ptr = std::make_unique<Game>();

// Flag to indicate whether the AI thread is currently computing
bool is_ai_computing = false;

void aiThreadFunction(Game* game_ptr) {
  while (game_ptr->isRunning()) {
    // Set the flag to indicate that the AI thread is computing
    is_ai_computing = true;

    game_ptr->playBestMove(3, Bitboard::Sides::WHITE);

    // Reset the flag once the AI computation is done
    is_ai_computing = false;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main(int argc, char* argv[]) {
  bool show_evaluation_bar = false;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--show_eval") {
      show_evaluation_bar = true;
      break;
    }
  }

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

  game_ptr->destroy();

  return 0;
}