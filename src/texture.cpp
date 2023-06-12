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

//This is used for the drag and drop feature.
void TextureManager::DrawPiece(const int type) {
  if (Globals::selected_lsf & Bitboard::no_sq || !Globals::is_mouse_down) {
    return;
  }

  int final_type = (type - 1) % 6;
  int color = (type > 6 ? 1 : 0);

  SDL_Rect dest = {Globals::mouse_coord.x - Globals::BOX_WIDTH / 2,
                   Globals::mouse_coord.y - Globals::BOX_HEIGHT / 2,
                   Globals::BOX_WIDTH, Globals::BOX_HEIGHT};

  SDL_Rect src = {final_type * m_source_width, color * m_source_height, m_source_width,
                  m_source_height};

  SDL_RenderCopy(Globals::renderer, Globals::texture, &src, &dest);
}

void TextureManager::AnimatePiece(int target_square, int type) {

  //Convert the target square index into a point.
  const auto pos = Bitboard::lsfToCoord(target_square);

  SDL_Point last_move{Bitboard::Squares::no_sq, Bitboard::Squares::no_sq};

  //Animate the previous move.
  if (!Globals::ply_array.empty()) {
    last_move = Globals::ply_array.back().move;
  }

  if (Globals::is_mouse_down && Globals::selected_lsf == target_square) {
    //This is important for the drag and drop functionality.
    return;
  }

  int final_type = (type - 1) % 6;
  int color = (type > 6 ? 1 : 0);

  SDL_Rect src = {final_type * m_source_width, color * m_source_height, m_source_width,
                  m_source_height};

  SDL_Rect dest = {0, 0, 0, 0};

  if ((last_move.y & Bitboard::Squares::no_sq || last_move.x & Bitboard::Squares::no_sq) ||
      target_square != last_move.y) {

    dest = {pos.x * Globals::BOX_WIDTH, pos.y * Globals::BOX_HEIGHT, Globals::BOX_WIDTH,
            Globals::BOX_HEIGHT};

  } else if (target_square == last_move.y && !Globals::ply_array.empty()) {
    //Using bilinear interpolation for smooth transistion.
    auto scaled_destination_vector =
        SDL_Point{pos.x * Globals::BOX_WIDTH, pos.y * Globals::BOX_HEIGHT};

    double alpha = 1000.0;  //This is measured in miliseconds.

    //Calculate the delta ticks and divide the alpha to the product.
    Globals::time += (static_cast<double>(SDL_GetTicks()) - Globals::elapsed_time) / alpha;

    int dx = scaled_destination_vector.x - Globals::scaled_linear_interpolant.x;
    int dy = scaled_destination_vector.y - Globals::scaled_linear_interpolant.y;

    //y = mx + b
    Globals::scaled_linear_interpolant.x =
        dx * Globals::time + Globals::scaled_linear_interpolant.x;

    Globals::scaled_linear_interpolant.y =
        dy * Globals::time + Globals::scaled_linear_interpolant.y;

    dest = {Globals::scaled_linear_interpolant.x, Globals::scaled_linear_interpolant.y,
            Globals::BOX_WIDTH, Globals::BOX_HEIGHT};
  }

  SDL_RenderCopy(Globals::renderer, Globals::texture, &src, &dest);
}