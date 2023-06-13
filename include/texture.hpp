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
    static void DrawPiece(const int type);
    static void DrawButton(const SDL_Point& pos, const int width, const int height);
    
    static void AnimatePiece(int target_square, int type);
    static void QueryTexture(int max_columns, int max_rows);

private:
    static int m_source_width;
    static int m_source_height;
};