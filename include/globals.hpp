#include <SDL2/SDL.h>
#include "bitboard.hpp"

constexpr unsigned int FPS = 60;
constexpr unsigned int FRAME_DELAY = FPS / 1000;

namespace Globals {
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* texture;

extern int BOX_WIDTH;
extern int BOX_HEIGHT;

extern std::array<int, NUM_OF_SQUARES> bitboard;
extern int en_passant;

extern int castling;
} // namespace Globals