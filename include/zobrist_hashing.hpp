#pragma once

#include "globals.hpp"
#include <random>
#include <array>

using ZobristTable = std::vector<std::vector<unsigned long long>>;

class ZobristHashing
{
public:
    ZobristHashing();
    ~ZobristHashing();

    // Generate the seed.
    void init();

    const std::uint64_t hashPosition();

    const ZobristTable& getZobristTable() const; 

private:
    ZobristTable m_zobrist_table;

    std::random_device m_random_device;
    std::mt19937_64 m_random_number_generator;
    std::uniform_int_distribution<unsigned long long> distribution;
};