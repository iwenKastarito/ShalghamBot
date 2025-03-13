#pragma once
#include <string>
#include <chrono>
#include "Board.h"
#include "Move.h"
#include "Evaluation.h"
#include "TranspositionTable.h"

class UCIEngine {
public:
    UCIEngine();
    void loop();
private:
    Board board;
    TranspositionTable tt;
    Evaluation evaluation;
    Move bestMove;
    int bestEval;
    unsigned long long nodes;
    double timeLimit; // seconds
    bool searchAborted;
    std::chrono::time_point<std::chrono::system_clock> searchStart;
    double defaultTimeLimit; // seconds
    double moveOverhead;     // seconds
    int maxDepth;            // from UCI option (-1 if not forced)

    static const int LOWEST_SCORE = -100000;
    static const int HIGHEST_SCORE = 100000;
    static const int DRAW_SCORE = 0;

    void processCommand(const std::string& line);
    Move getBestMove(int timeLeft, int increment, int depthLimit, int exactTime);
    int search(int alpha, int beta, int depth, int plyFromRoot, bool nullMove);
    int quiescenceSearch(int alpha, int beta);
    int evaluate();
    std::string getPrincipalVariation(int depth);
};
