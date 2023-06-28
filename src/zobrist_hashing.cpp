#include "zobrist_hashing.hpp"

ZobristHashing::ZobristHashing() {
  init();
}

ZobristHashing::~ZobristHashing() {}

const ZobristTable& ZobristHashing::getZobristTable() const {
    // Return the zobrist table as a const reference
    return m_zobrist_table;
}

void ZobristHashing::init() {
  m_random_number_generator = std::mt19937_64(m_random_device());
  
  distribution = std::uniform_int_distribution<unsigned long long>(
      0, std::numeric_limits<unsigned long long>::max());

  // Resize the zobrist table to accommodate the 64 squares and 12 different piece types
  m_zobrist_table.resize(64ULL, std::vector<std::uint64_t>(12ULL));

  // Generate random numbers for each square and piece combination
  for (int square = 0; square < Bitboard::NUM_OF_SQUARES; ++square) {
    for (int piece = 0; piece < 12; ++piece) {
      m_zobrist_table[square][piece] = distribution(m_random_number_generator);
    }
  }
}

const std::uint64_t ZobristHashing::hashPosition() {
  std::uint64_t hash = 0;

  // Iterate over the bitboard and XOR the corresponding random number from the zobrist table
  for (int square = 0; square < Bitboard::NUM_OF_SQUARES; ++square) {
    const int piece = Globals::bitboard[square];
    
    if (piece != Bitboard::Pieces::e) {
      hash ^= m_zobrist_table[square][piece];
    }
  }

  return hash;
}