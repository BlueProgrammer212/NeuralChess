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
  SDL_Rect dest = {pos.x * Globals::BOX_WIDTH, pos.y * Globals::BOX_HEIGHT, Globals::BOX_WIDTH,
                   Globals::BOX_HEIGHT};

  SDL_RenderCopy(Globals::renderer, Globals::texture, &src, &dest);
}