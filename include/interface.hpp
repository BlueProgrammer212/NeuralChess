#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include "globals.hpp"
#include "move.hpp"

class Interface
{
public:
    Interface();
    ~Interface();

    // This is for the buttons.
    static int AABB(int x, int y);

    void drop(int lsf, int old_lsf, int width, int height);
    void drag(int width, int height);

    int lerp(int a, int b, int t);

private:
    int mouse_position_x;
    int mouse_position_y;
};