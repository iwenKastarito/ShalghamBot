#pragma once
#include <optional>
#include <cmath>
#include <map>
#include <array>
#include <vector>
#include "Board.h"
#include "TranspositionTable.h"
#include "PieceSquareTables.h"
#include "Move.h"

class Evaluation {
    Board& board;
    TranspositionTable& tt;
    std::map<int, int> pieceValues;
    PieceSquareTables pieceSquareTables;
    std::array<std::array<U64, 2>, 2> pawnShieldBBs;
    std::array<U64, 64> nearKingSquares;
    std::array<U64, 8> fileBBs;
    std::array<int, 16> dirs = { EAST, WEST, NORTH, SOUTH,
                                  NORTH_EAST, SOUTH_WEST, SOUTH_EAST, NORTH_WEST };
public:
    Evaluation(Board& boardPar, TranspositionTable& ttPar);
    void orderMoves(std::vector<Move>& moves);
    std::array<int, 2> countMaterial(std::array<PieceList, 12>& pieceLists);
    double getOpeningWeight();
    double getEndgameWeight(std::array<int, 2> material);
    int countPieceSquareEval(std::array<PieceList, 12>& pieceLists, int color, double endgameWeight);
    int countMopUpEval(std::array<PieceList, 12>& pieceLists, int materialEval, double endgameWeight);
    int countKnightPawnPenalty(std::array<PieceList, 12>& pieceLists, int color);
    int countBadBishopPenalty(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color);
    int countBishopPairReward(std::array<PieceList, 12>& pieceLists, int color);
    int countRookOpenFileReward(std::array<U64, 12>& piecesBB, int color);
    int countDoubledPawnPenalty(std::array<U64, 12>& piecesBB, int color);
    int countIsolatedPawnPenalty(std::array<U64, 12>& piecesBB, int color);
    int countPassedPawnReward(std::array<U64, 12>& piecesBB, int color);
    int countBackwardPawnPenalty(std::array<U64, 12>& piecesBB, int color);
    int countPawnShieldEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double openingWeight, double endgameWeight);
    int countPawnStormEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double endgameWeight);
    int evaluate();
};
