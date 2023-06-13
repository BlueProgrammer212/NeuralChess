#include "fen_parser.hpp"

FenParser* FenParser::s_Instance = nullptr;

constexpr const char* INITIAL_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* KIWIPETE = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

FenParser::FenParser() : m_FEN(INITIAL_POSITION) {}

FenParser::~FenParser() {}

int FenParser::init() {
  auto coord = SDL_Point{0, 0};

  std::string ascii_pieces = ".KQBNRPkqbnrp";

  for (int i = 0; i < 64; ++i) {
    Globals::bitboard[i] = Bitboard::Pieces::e;
  };

  std::stringstream ss(m_FEN);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> fields(begin, end);

  HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);

  SetConsoleTextAttribute(h_console, 1);
  std::cout << "[INFO] ";
  SetConsoleTextAttribute(h_console, 15);
  std::cout << "Parsing the FEN string.\n";

  bool is_white_to_move = fields[1] == "w";

  //Log every information in the console.
  std::cout << "--------FEN INFORMATION--------\n"
            << "FEN string: " << fields[0] << "\n"
            << "White to move: " << (is_white_to_move ? "Yes" : "No") << "\n"
            << "Castling Rights: " << fields[2] << "\n"
            << "En Passant square: " << fields[3] << "\n"
            << "Halfmove Clock: " << fields[4] << "\n"
            << "Fullmove Clock: " << fields[5] << "\n"
            << "-------------------------------\n";

  //Loop over the FEN string.
  for (const char symbol : fields[0]) {
    if (symbol == ' ') {
      break;  //End of the first field.
    }

    //Find the index of the symbol in the piece string.
    std::size_t piece_type = ascii_pieces.find(symbol);

    if (symbol == '/') {
      coord.x = 0;  //Reset the file when the rank decrements.
      coord.y++;
      continue;
    }

    if (isdigit(symbol)) {
      coord.x += symbol - '0';  //Convert char to integer.
      continue;
    }

    if (piece_type != std::string::npos) {
      int lsf = Bitboard::toLSF(coord.x, coord.y);
      Globals::bitboard[lsf] = piece_type;
      coord.x++;
      continue;
    }
  }

  if (is_white_to_move) {
    Globals::side |= Bitboard::Sides::WHITE;
    Globals::side &= ~Bitboard::Sides::BLACK;
  } else {
    Globals::side |= Bitboard::Sides::BLACK;
    Globals::side &= ~Bitboard::Sides::WHITE;
  }

  return 0;
}
