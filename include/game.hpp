#include "Globals.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <memory>

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
};