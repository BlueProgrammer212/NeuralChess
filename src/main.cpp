#include <iostream>
#include "game.hpp"

int main(int argc, char* argv[]) {
  auto game_ptr = std::make_unique<Game>();
  game_ptr->init(600, 600);

  // Game Loop
  while (game_ptr->isRunning()) {
    game_ptr->update();
    game_ptr->render();
    game_ptr->events();
  }

  return 0;
}