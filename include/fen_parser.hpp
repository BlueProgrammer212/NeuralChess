#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <windows.h>

#include "globals.hpp"
#include "singleton.hpp"
#include "bitboard.hpp"

class FenParser : public Singleton
{
public:
    [[nodiscard]] inline static FenParser &getInstance()
    {
        if (s_Instance == nullptr)
        {
            s_Instance = new FenParser();
        }

        return *s_Instance;
    }

    int init();
    void load_fen_from_file(const char *path);

protected:
    FenParser();
    ~FenParser();

private:
    static FenParser *s_Instance;
    std::string m_FEN;
};