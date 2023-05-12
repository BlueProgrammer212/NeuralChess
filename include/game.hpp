#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <memory>

#include "globals.hpp"
#include "texture.hpp"
#include "interface.hpp"
#include "move.hpp"

class Game {
  public:
    Game();
    ~Game();

    void init(const int width, const int height);
    void update();
    void render();
    void events();

    inline bool isRunning() const { return m_running; }

  private:
    bool m_running;
    std::unique_ptr<Interface> m_interface{};
};