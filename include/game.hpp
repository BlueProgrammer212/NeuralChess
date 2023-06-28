#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <windows.h>
#include <iostream>
#include <memory>

#include "globals.hpp"
#include "texture.hpp"
#include "interface.hpp"
#include "move.hpp"
#include "audio_manager.hpp"
#include "gui/settings.hpp"
#include "evaluation.hpp"
#include "fen_parser.hpp"
#include "minimax_search.hpp"

class Game : public Search
{
public:
  Game();
  ~Game();

  void init(const int width, const int height);
  void update();
  void render();
  void events(bool is_ai_computing);

  inline bool isRunning() const { return m_running; }

  void playRandomly();

  void resetBoard();

  void destroy() { delete this; }

  bool is_calculating = false;

private:
  int time;
  int last_time;
  int delta_time;

  bool m_running;

  FenParser &m_fen_parser;
  HANDLE m_console;

  bool m_show_occupied_squares;
};