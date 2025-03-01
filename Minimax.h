#ifndef MINIMAX_H
#define MINIMAX_H

#include "ChessGame.h"
#include "bot.h"
#include <SFML/System/Vector2.hpp>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

// Enumeration for game phase.
enum GamePhase {
    OPENING,
    MIDDLE,
    ENDING
};

// GPU-friendly state structure.
struct GameStateGPU {
    char board[64];
    int moveNumber;
};

// Candidate move structure.
struct CandidateMove {
    sf::Vector2i from;
    sf::Vector2i to;
    GameStateGPU state;
};

// Minimax algorithm implementation.
// This class encapsulates all minimax-specific logic (e.g. evaluation and search).
class MinimaxAlgorithm {
public:
    int searchDepth;       // e.g., 13
    int nodesEvaluated;    // Counts nodes evaluated during search.
    std::unordered_map<std::string, int> transpositionTable;

    MinimaxAlgorithm(int depth);
    int minimax(ChessGame game, int depth, int alpha, int beta, bool whiteTurn, bool botIsWhite);
    int evaluate(const ChessGame& game, bool botIsWhite);
    GamePhase getGamePhase(const ChessGame& game);
    int countMajorPieces(const ChessGame& game);
    std::string boardToString(const ChessGame& game);
};

// A minimax-based bot that uses the MinimaxAlgorithm.
class MinimaxBot : public ChessBot {
public:
    MinimaxAlgorithm minimaxAlgo;
    MinimaxBot(bool isWhite, int depth);
    std::pair<sf::Vector2i, sf::Vector2i> chooseMove(const ChessGame& game) override;
};

#endif // MINIMAX_H
