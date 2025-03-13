#pragma once
#include <optional>
#include "Move.h"
#include "Board.h"

class TranspositionTable {
public:
    TranspositionTable(Board& board) {}
    void clear() {}
    std::optional<int> getStoredEval(int depth, int plyFromRoot, int alpha, int beta) { return std::nullopt; }
    std::optional<Move> getStoredMove(Board& board, bool mainLine) { return std::nullopt; }
    void storeEntry(int eval, int depth, Move move, int nodeType, int plyFromRoot) {}
    void setSizeMB(int sizeMB) {}
};
