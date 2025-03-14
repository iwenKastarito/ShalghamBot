#include "Minimax.h"
#include "ChessGame.h"
#include <cctype>
#include <sstream>
#include <limits>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <chrono>
#include <iostream>

// Cross-platform trailing zero counter.
#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
inline int countTrailingZeros(uint64_t x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return (int)index;
}
#else
inline int countTrailingZeros(uint64_t x) {
    return __builtin_ctzll(x);
}
#endif

MinimaxAlgorithm::MinimaxAlgorithm(int depth)
    : searchDepth(depth), nodesEvaluated(0) {
    transpositionTable.clear();
}

int MinimaxAlgorithm::countMajorPieces(const ChessGame& game) {
    int count = 0;
    for (int i = 0; i < 64; i++) {
        char piece = game.board[i];
        char p = std::tolower(piece);
        if (p == 'q' || p == 'r' || p == 'b' || p == 'n')
            count++;
    }
    return count;
}

GamePhase MinimaxAlgorithm::getGamePhase(const ChessGame& game) {
    if (game.moveNumber / 2 <= 3)
        return OPENING;
    if (countMajorPieces(game) <= 2)
        return ENDING;
    return MIDDLE;
}

uint64_t MinimaxAlgorithm::boardHash(const ChessGame& game) {
    // FNV-1a hash over the 64-cell board and moveNumber.
    uint64_t hash = 1469598103934665603ULL;
    for (int i = 0; i < 64; i++) {
        hash ^= static_cast<uint64_t>(game.board[i]);
        hash *= 1099511628211ULL;
    }
    hash ^= static_cast<uint64_t>(game.moveNumber);
    hash *= 1099511628211ULL;
    return hash;
}

int MinimaxAlgorithm::evaluate(const ChessGame& game, bool botIsWhite) {
    // Piece ratings.
    std::unordered_map<char, int> rating = {
        {'p', 100}, {'n', 320}, {'b', 330}, {'r', 500}, {'q', 900}, {'k', 20000}
    };

    int materialScore = 0;
    for (int i = 0; i < 64; i++) {
        char piece = game.board[i];
        if (piece == ' ')
            continue;
        int value = rating[std::tolower(piece)];
        materialScore += std::isupper(piece) ? value : -value;
    }

    int bonus = 0;
    // Castling bonus.
    if (game.board[7 * 8 + 6] == 'K' || game.board[7 * 8 + 2] == 'K')
        bonus += 50;
    if (game.board[0 * 8 + 6] == 'k' || game.board[0 * 8 + 2] == 'k')
        bonus -= 50;

    // Promotion bonus.
    for (int j = 0; j < 8; j++) {
        if (game.board[0 * 8 + j] == 'P')
            bonus += 200;
        if (game.board[7 * 8 + j] == 'p')
            bonus -= 200;
    }

    // Central control bonus.
    std::vector<std::pair<int, int>> center = { {3,3}, {3,4}, {4,3}, {4,4} };
    for (auto& sq : center) {
        char piece = game.board[sq.first * 8 + sq.second];
        if (piece != ' ')
            bonus += std::isupper(piece) ? 10 : -10;
    }

    // King safety penalty.
    const int checkPenalty = 5000;
    if (botIsWhite) {
        if (game.kingIsInCheck(true))
            bonus -= checkPenalty;
        if (!game.kingIsInCheck(false))
            bonus += 200;
    }
    else {
        if (game.kingIsInCheck(false))
            bonus -= checkPenalty;
        if (!game.kingIsInCheck(true))
            bonus -= 200;
    }

    return materialScore + bonus;
}

inline int branchlessUpdate(int curr, int cand, bool maximize) {
    if (maximize) {
        int mask = -((int)(cand > curr));
        return curr ^ ((curr ^ cand) & mask);
    }
    else {
        int mask = -((int)(cand < curr));
        return curr ^ ((curr ^ cand) & mask);
    }
}

inline void branchlessBoundUpdate(int cand, bool maximize, int& bestVal, int& bound) {
    bestVal = branchlessUpdate(bestVal, cand, maximize);
    if (maximize)
        bound = branchlessUpdate(bound, cand, true);
    else
        bound = branchlessUpdate(bound, cand, false);
}

int MinimaxAlgorithm::minimax(ChessGame game, int depth, int alpha, int beta, bool whiteTurn, bool botIsWhite) {
    nodesEvaluated++;
    uint64_t key = boardHash(game) ^ (whiteTurn ? 0xABCDEFULL : 0x123456ULL) ^ depth;
    if (transpositionTable.find(key) != transpositionTable.end())
        return transpositionTable[key];

    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> moves;
    uint64_t piecesBB = 0ULL;
    for (int i = 0; i < 64; i++) {
        int row = i / 8, col = i % 8;
        char piece = game.board[i];
        if (piece != ' ' && (std::isupper(piece) == whiteTurn))
            piecesBB |= (1ULL << i);
    }
    while (piecesBB) {
        int index = countTrailingZeros(piecesBB);
        piecesBB &= piecesBB - 1;
        int row = index / 8, col = index % 8;
        auto pseudo = game.getLegalMoves(row, col, false);
        for (auto move : pseudo) {
            ChessGame simState = game.simulateMove(sf::Vector2i(row, col), move);
            if (!simState.kingIsInCheck(whiteTurn))
                moves.push_back({ sf::Vector2i(row, col), move });
        }
    }

    if (depth == 0 || moves.empty()) {
        if (moves.empty()) {
            if (game.kingIsInCheck(whiteTurn))
                return whiteTurn ? -1000 : 1000;
            return 0;
        }
        int eval = evaluate(game, botIsWhite);
        transpositionTable[key] = eval;
        return eval;
    }

    bool maximizing = (whiteTurn == botIsWhite);
    int bestVal = maximizing ? -std::numeric_limits<int>::max() : std::numeric_limits<int>::max();

    for (auto m : moves) {
        ChessGame newGame = game.simulateMove(m.first, m.second);
        int eval = minimax(newGame, depth - 1, alpha, beta, !whiteTurn, botIsWhite);
        branchlessBoundUpdate(eval, maximizing, bestVal, (maximizing ? alpha : beta));
        if (beta <= alpha)
            break;
    }
    transpositionTable[key] = bestVal;
    return bestVal;
}

MinimaxBot::MinimaxBot(bool isWhite, int depth)
    : ChessBot(isWhite), minimaxAlgo(depth) {
}

std::pair<sf::Vector2i, sf::Vector2i> MinimaxBot::chooseMove(const ChessGame& game) {
    auto start = std::chrono::high_resolution_clock::now();
    minimaxAlgo.nodesEvaluated = 0;
    std::vector<CandidateMove> candidates;
    uint64_t piecesBB = 0ULL;
    for (int i = 0; i < 64; i++) {
        char piece = game.board[i];
        if (piece != ' ' && (std::isupper(piece) == this->botIsWhite))
            piecesBB |= (1ULL << i);
    }
    while (piecesBB) {
        int index = countTrailingZeros(piecesBB);
        piecesBB &= piecesBB - 1;
        int row = index / 8, col = index % 8;
        std::vector<sf::Vector2i> pseudo = game.getLegalMoves(row, col, false);
        for (auto move : pseudo) {
            CandidateMove cand;
            cand.from = sf::Vector2i(row, col);
            cand.to = move;
            candidates.push_back(cand);
        }
    }

    int numCandidates = candidates.size();
    if (numCandidates == 0) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "MinimaxBot move time: " << duration.count() << " microseconds\n";
        return { sf::Vector2i(-1, -1), sf::Vector2i(-1, -1) };
    }

    std::vector<int> evaluations(numCandidates);
    for (int i = 0; i < numCandidates; i++) {
        evaluations[i] = minimaxAlgo.minimax(
            game.simulateMove(candidates[i].from, candidates[i].to),
            minimaxAlgo.searchDepth - 1,
            -100000, 100000,
            !this->botIsWhite,
            this->botIsWhite
        );
    }

    int bestEval = this->botIsWhite ? -std::numeric_limits<int>::max() : std::numeric_limits<int>::max();
    int bestIndex = 0;
    for (int i = 0; i < numCandidates; i++) {
        if (this->botIsWhite) {
            int mask = -((int)(evaluations[i] > bestEval));
            bestEval = bestEval ^ ((bestEval ^ evaluations[i]) & mask);
            bestIndex = (mask & i) | (~mask & bestIndex);
        }
        else {
            int mask = -((int)(evaluations[i] < bestEval));
            bestEval = bestEval ^ ((bestEval ^ evaluations[i]) & mask);
            bestIndex = (mask & i) | (~mask & bestIndex);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "MinimaxBot move time: " << duration.count() << " microseconds, nodes evaluated: "
        << minimaxAlgo.nodesEvaluated << "\n";

    return { candidates[bestIndex].from, candidates[bestIndex].to };
}
