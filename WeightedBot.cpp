#include "WeightedBot.h"
#include <vector>
#include <cstdint>
#include <cctype>
#include <random>

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

struct CandidateMove {
    sf::Vector2i from;
    sf::Vector2i to;
    int weight;
};

std::pair<sf::Vector2i, sf::Vector2i> WeightedBot::chooseMove(const ChessGame& game) {
    int positionalMap[8][8] = {
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,1,0,1,0,0,0},
        {0,1,0,0,0,1,0,0},
        {0,0,0,0,0,0,0,0},
        {0,1,0,0,0,1,0,0},
        {0,0,1,0,1,0,0,0},
        {0,0,0,0,0,0,0,0}
    };

    std::vector<CandidateMove> candidates;
    uint64_t piecesBB = 0ULL;
    for (int i = 0; i < 64; i++) {
        int row = i / 8;
        int col = i % 8;
        char piece = game.board[i];
        if (piece != ' ' && (std::isupper(piece) == botIsWhite))
            piecesBB |= (1ULL << i);
    }
    while (piecesBB) {
        int index = countTrailingZeros(piecesBB);
        piecesBB &= piecesBB - 1;
        int row = index / 8;
        int col = index % 8;
        std::vector<sf::Vector2i> moves = game.getLegalMoves(row, col, false);
        for (auto move : moves) {
            CandidateMove cand;
            cand.from = sf::Vector2i(row, col);
            cand.to = move;
            cand.weight = positionalMap[move.x][move.y];
            candidates.push_back(cand);
        }
    }
    if (candidates.empty())
        return { sf::Vector2i(-1, -1), sf::Vector2i(-1, -1) };

    int bestWeight = -1000000;
    for (auto& cand : candidates) {
        if (cand.weight > bestWeight)
            bestWeight = cand.weight;
    }
    std::vector<CandidateMove> bestCandidates;
    for (auto& cand : candidates) {
        if (cand.weight == bestWeight)
            bestCandidates.push_back(cand);
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bestCandidates.size() - 1);
    int selected = dis(gen);
    return { bestCandidates[selected].from, bestCandidates[selected].to };
}
