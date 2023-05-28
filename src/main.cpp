#include <iostream>
#include "game.hpp"

int main(int argc, char* argv[]) {
  auto game_ptr = std::make_unique<Game>();
  game_ptr->init(600, 600);

  constexpr int FPS = 144;
  constexpr int FRAME_DELAY = FPS / 1000;

  unsigned int frame_start = 0;
  int frame_time = 0;

  // Game Loop
  while (game_ptr->isRunning()) {
    frame_start = SDL_GetTicks();

    game_ptr->update();
    game_ptr->render();
    game_ptr->events();

    frame_time = SDL_GetTicks() - frame_start;

    if (FRAME_DELAY > frame_time) {
      SDL_Delay(FRAME_DELAY - frame_time);
    }
  }

  return 0;
}