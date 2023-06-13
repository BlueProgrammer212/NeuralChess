# NeuralChess

## How it Should Work
NeuralChess is an ongoing chess engine project. The chess engine should be able to evaluate material and positional advantage. It should also be able to evaluate billions of positions, which is possible with alpha-beta pruning and other optimization techniques such as move ordering and Zobrist hashing. We can reduce the number of nodes of the minimax search algorithm to evaluate more positions and potentially increase the depth.

If you're interested, you can visit this website: https://www.chessprogramming.org/Main_Page

<p align="center">
  <img src="https://github.com/BlueProgrammer212/NeuralChess/assets/87359245/28cd13eb-4bbc-4f00-a8cc-423064c47ad3" 
       width="400" height="400" />
 </p>
 
## Features
- [x] Pseudo-legal move generator
- [x] Legal moves (including pins and checks)
- [x] Castling
- [x] En Passant
- [x] FEN parser 
- [ ] Simple GUI
- [ ] Evaluation
- [ ] PGN reader and "Game Review"
- [ ] Minimax algorithm
- [ ] Alpha-beta pruning 
- [ ] Move ordering for optimization
- [ ] Transposition tables for various chess openings
- [ ] Zobrist Hashing
