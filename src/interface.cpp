#include "interface.hpp"

using namespace Globals;

Interface::Interface() 
    : mouse_position_x(0),
      mouse_position_y(0)
{

}

Interface::~Interface() {
    quad_vector.clear();
}

void Interface::drop(int lsf, int old_lsf, int width, int height) {
    //Check for friendly pieces.
    int selected_piece = Bitboard::getColor(bitboard[old_lsf]);
    int target_lsf = Bitboard::getColor(bitboard[lsf]);

    if ((selected_piece != target_lsf || bitboard[lsf] == Bitboard::Pieces::e)
        && selected_piece == side) {
        //Exchange turns.
        side ^= Bitboard::Sides::WHITE;
        side ^= Bitboard::Sides::BLACK;

        bitboard[lsf] = bitboard[old_lsf]; 

        bitboard[old_lsf] = Bitboard::e;
        last_move.x = old_lsf;
        last_move.y = lsf;
    }

    MoveGenerator::generatePseudoLegalMoves();
}

void Interface::drag(int width, int height) {

}

int Interface::AABB(int x, int y) {
    for (int file = 0; file < Bitboard::BOARD_SIZE + 1; ++file) {
    for (int rank = 0; rank < Bitboard::BOARD_SIZE + 1; ++rank) {
        int index = Bitboard::toLSF(file, rank);
        SDL_Rect rect = quad_vector[index];
       
        if (rect.x + rect.w > x && x > rect.x &&
            rect.y + rect.h > y && y > rect.y) {
            std::cout << index - (Bitboard::BOARD_SIZE + 1) << "\n";

            return index;
        }
    }
    }

    return Bitboard::Squares::no_sq;
}