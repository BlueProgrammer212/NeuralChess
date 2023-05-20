#include "game.hpp"

using namespace Globals;

Game::Game() : m_running(true), m_interface(std::make_unique<Interface>()) {
  std::cout << "Initializing the game\n";
}

Game::~Game() {
  for (SDL_Window* window : window_set) {
    SDL_DestroyWindow(window);
  }

  SDL_DestroyRenderer(renderer);
  SDL_Quit();
}

void Game::init(const int width, const int height) {
  //Initialize window.
  addWindow("NeuralChess", width, height);

  //Initialize the renderer.
  renderer =
      SDL_CreateRenderer(window_set[0], -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

  if (renderer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create the renderer.");
    m_running = false;
  }

  BOX_WIDTH = width / (Bitboard::BOARD_SIZE + 1);
  BOX_HEIGHT = height / (Bitboard::BOARD_SIZE + 1);

  //Load the chess piece texture atlas.
  TextureManager::LoadTexture("../../res/atlas.png");

  //Save the texture dimensions.
  TextureManager::QueryTexture(6, 2);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  //Precalculate chess board squares for the UI.
  for (int FILE = 0; FILE < (Bitboard::BOARD_SIZE + 1); ++FILE) {
    for (int RANK = 0; RANK < (Bitboard::BOARD_SIZE + 1); ++RANK) {
      SDL_Rect rect = {FILE * BOX_WIDTH, RANK * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};
      quad_vector.push_back(rect);
    }
  }

  //By default, it's white to move.
  side |= Bitboard::WHITE;
}

void Game::update() {}

void Game::render() {
  SDL_RenderClear(renderer);

  // Render the chess board.
  for (int lsf = 0; lsf < static_cast<int>(quad_vector.size()); ++lsf) {
    const auto& pos = Bitboard::lsfToCoord(lsf);
    if ((pos.x + pos.y) % 2 == 0) {
      SDL_SetRenderDrawColor(renderer, 249, 224, 169, 255);
    } else {
      SDL_SetRenderDrawColor(renderer, 176, 112, 56, 255);
    }

    SDL_RenderFillRect(renderer, &quad_vector[lsf]);
  }

  //Highlight the selected quad.
  if (selected_lsf != Bitboard::no_sq) {
    const auto& pos = Bitboard::lsfToCoord(selected_lsf);
    SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 125);

    SDL_RenderFillRect(renderer, &dest);
  }

  //Render hints.
  for (int lsf : move_hints) {
    const auto& pos = Bitboard::lsfToCoord(lsf);

    SDL_Rect dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_WIDTH};

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 125);
    SDL_RenderFillRect(renderer, &dest);
  }

  //Highlight the last move.
  if (last_move.x != Bitboard::no_sq && last_move.y != Bitboard::no_sq) {
    const auto& initial_pos = Bitboard::lsfToCoord(last_move.x);
    const auto& pos = Bitboard::lsfToCoord(last_move.y);

    SDL_Rect dest = {initial_pos.x * BOX_WIDTH, initial_pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};

    SDL_Rect final_dest = {pos.x * BOX_WIDTH, pos.y * BOX_HEIGHT, BOX_WIDTH, BOX_HEIGHT};

    SDL_SetRenderDrawColor(renderer, 200, 200, 0, 125);
    SDL_RenderFillRect(renderer, &dest);

    SDL_SetRenderDrawColor(renderer, 220, 220, 0, 125);
    SDL_RenderFillRect(renderer, &final_dest);
  }

  //Render the pieces.
  for (int i = 0; i < static_cast<int>(bitboard.size()); i++) {
    TextureManager::RenderTexture(i, bitboard[i]);
  }

  SDL_RenderPresent(renderer);
}

void Game::events() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.window.type) {
      case SDL_QUIT:
        m_running = false;
        break;
      case SDL_MOUSEBUTTONDOWN:
        int new_lsf = Interface::AABB(event.button.y, event.button.x);

        if (selected_lsf == Bitboard::no_sq && Globals::bitboard[new_lsf] != Bitboard::e) {
          selected_lsf = (Bitboard::SHOULD_FLIP ? new_lsf ^ 0x38 : new_lsf);
        } else if (selected_lsf != Bitboard::no_sq) {
          //Reset the pseudolegal moves.
          Globals::move_hints.clear();

          m_interface->drop(new_lsf, selected_lsf, BOX_WIDTH, BOX_HEIGHT);
          selected_lsf = Bitboard::no_sq;
        }
    }
  }
}