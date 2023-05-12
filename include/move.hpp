#pragma once

#include <iostream>
#include <algorithm>
#include "globals.hpp"

namespace MoveGenerator {
inline void generateSlidingMoves() {
    
}

inline void addMove(int lsf) {
    Globals::pseudolegal_moves.push_back(lsf); 
}

inline void generatePseudoLegalMoves() {
    for (int lsf = 0; lsf < 64; ++lsf) {
        if (Globals::bitboard[lsf] == Bitboard::Pieces::e) continue;

        switch (Globals::bitboard[lsf]) {
            case Bitboard::Pieces::P:
               if (lsf - 8 < 0) {
                //Auto-queen
                Globals::bitboard[lsf] = Bitboard::Pieces::Q;
               }

               break;
            case Bitboard::Pieces::p:
                if (lsf + 8 >= 48) {
                //Auto-queen
                Globals::bitboard[lsf] = Bitboard::Pieces::q;
               }
            
                break;
        }
    }
}
}