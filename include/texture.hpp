#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "globals.hpp"

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();

    static SDL_Texture *LoadTexture(const char *path);
    static void RenderTexture(int target_square, int type);
    static void QueryTexture(int max_columns, int max_rows);

private:
    static int m_source_width;
    static int m_source_height;
};