#ifndef CHESSBOT_H
#define CHESSBOT_H

#include "../core/ChessGame.h"
#include <vector>
#include <algorithm>
#include <limits>

class ChessBot {
public:
    // Constructor takes a reference to the game
    ChessBot(ChessGame& game);
    
    // Calculate the best move for the current position
    sf::Vector2i getBestMove(bool white);
    
    // Set the search depth
    void setSearchDepth(int depth);

private:
    ChessGame& game;
    int searchDepth = 3; // Default search depth
    
    // Minimax with alpha-beta pruning
    int minimax(ChessGame& state, int depth, int alpha, int beta, bool maximizingPlayer);
    
    // Static evaluation function
    int evaluatePosition(const ChessGame& state);
    
    // Material value of each piece
    int getPieceValue(char piece);
    
    // Generate and sort moves for better alpha-beta efficiency
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> generateSortedMoves(ChessGame& state, bool white);
};

#endif // CHESSBOT_H 