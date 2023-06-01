#include "texture.hpp"

TextureManager::TextureManager() {
  Globals::texture = nullptr;
}

TextureManager::~TextureManager() {
  SDL_DestroyTexture(Globals::texture);
}

int TextureManager::m_source_width = 0;
int TextureManager::m_source_height = 0;

SDL_Texture* TextureManager::LoadTexture(const char* path) {
  //Create the texture from the surface.
  Globals::texture = IMG_LoadTexture(Globals::renderer, path);

  //Error Handling
  if (Globals::texture == nullptr) {
    std::cout << "Failed to load the texture atlas\n";
  }

  return Globals::texture;
}

//Query the texture dimensions.
void TextureManager::QueryTexture(int max_columns, int max_rows) {
  SDL_QueryTexture(Globals::texture, nullptr, nullptr, &m_source_width, &m_source_height);
  m_source_width /= max_columns;
  m_source_height /= max_rows;
}

void TextureManager::RenderTexture(int target_square, int type) {
  //Convert the target square index into a point.
  const auto pos = Bitboard::lsfToCoord(target_square);

  int final_type = (type - 1) % 6;
  int color = (type > 6 ? 1 : 0);

  SDL_Rect src = {final_type * m_source_width, color * m_source_height, m_source_width,
                  m_source_height};

  SDL_Rect dest = {0, 0, 0, 0};

  if ((Globals::last_move.x == Bitboard::Squares::no_sq ||
       Globals::last_move.y == Bitboard::Squares::no_sq) ||
      target_square != Globals::last_move.y) {
    dest = {pos.x * Globals::BOX_WIDTH, pos.y * Globals::BOX_HEIGHT, Globals::BOX_WIDTH,
            Globals::BOX_HEIGHT};
  } else if (target_square == Globals::last_move.y) {
    //Using bilinear interpolation for smooth transistion.
    auto scaled_destination_vector =
        SDL_Point{pos.x * Globals::BOX_WIDTH, pos.y * Globals::BOX_HEIGHT};

    double alpha = 1000.0;
    Globals::time += ((double)SDL_GetTicks() - Globals::elapsed_time) / alpha;

    int dx = scaled_destination_vector.x - Globals::scaled_linear_interpolant.x;
    int dy = scaled_destination_vector.y - Globals::scaled_linear_interpolant.y;

    Globals::scaled_linear_interpolant.x = dx * Globals::time + Globals::scaled_linear_interpolant.x;
    Globals::scaled_linear_interpolant.y = dy * Globals::time + Globals::scaled_linear_interpolant.y;

    dest = {Globals::scaled_linear_interpolant.x, Globals::scaled_linear_interpolant.y,
            Globals::BOX_WIDTH, Globals::BOX_HEIGHT};
  }

  SDL_RenderCopy(Globals::renderer, Globals::texture, &src, &dest);
}