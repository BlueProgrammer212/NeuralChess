#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include "globals.hpp"
#include "move.hpp"

enum MoveFlags : int
{
    SHOULD_SUPRESS_HINTS = 1 << 0,
    SHOULD_EXCHANGE_TURN = 1 << 1,
    WILL_UNDO_MOVE = 1 << 2,
    IS_CASTLING = 1 << 3
};

class Interface
{
public:
    Interface();
    ~Interface();

    // This is for the buttons.
    static int AABB(int x, int y);

    void drop(int lsf, int old_lsf, int width, int height, const unsigned int flags);
    void undo();
    void drag(int width, int height);

private:
    int mouse_position_x;
    int mouse_position_y;
};