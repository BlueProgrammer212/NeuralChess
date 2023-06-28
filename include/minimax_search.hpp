#include <iostream>

#include "globals.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "evaluation.hpp"
#include "interface.hpp"

class Search
{
public:
    Search();
    ~Search();

    const std::vector<LegalMove> &moveOrdering();

    int minimaxSearch(int depth, int alpha, int beta, bool is_maximizing);
    void playBestMove(int depth, const unsigned int human_player = 0b10);

    void playRandomly();
};