#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include <thread>
#include <future>

#include "globals.hpp"
#include "texture.hpp"
#include "interface.hpp"
#include "move.hpp"
#include "audio_manager.hpp"
#include "gui/settings.hpp"
#include "evaluation.hpp"
#include "fen_parser.hpp"

class Game
{
public:
  Game();
  ~Game();

  void init(const int width, const int height);
  void update();
  void render();
  void events();

  inline bool isRunning() const { return m_running; }
  int minimaxSearch(int depth, bool maximizing_player, int alpha, int beta);

  void playRandomly();
  void playBestMove(int depth);

  void resetBoard();

  void destroy() { delete this; }

private:
  int time;
  int last_time;
  int delta_time;

  bool m_running;

  std::unique_ptr<Interface> m_interface{};
  FenParser &m_fen_parser;
  HANDLE m_console;

  bool m_show_occupied_squares;
};