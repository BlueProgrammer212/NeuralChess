#include "game.hpp"

using namespace Globals;

Game::Game() : m_running(true) {}

Game::~Game() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Game::init(const int width, const int height) {
    window = SDL_CreateWindow("Chess Engine", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create the window");
        m_running = false;
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create the renderer.");
        m_running = false;
    }

    BOX_WIDTH = width / BOARD_SIZE;
    BOX_HEIGHT = height / BOARD_SIZE;
}

void Game::update() {}

void Game::render() {
    SDL_RenderClear(renderer);
    // Render the chess board.
    for (int file = 0; file < BOARD_SIZE; ++file) {
        for (int rank = 0; rank < BOARD_SIZE; ++rank) {
            if ((file + rank) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 249, 224, 169, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 176, 112, 56, 255);
            }

            SDL_Rect dest = {file * BOX_WIDTH, rank * BOX_HEIGHT, BOX_WIDTH,
                             BOX_HEIGHT};
            SDL_RenderFillRect(renderer, &dest);
        }
    }
    SDL_RenderPresent(renderer);
}

void Game::events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            m_running = false;
        }
    }
}