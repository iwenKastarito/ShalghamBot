#include "Evaluation.h"
#include "Square.h"
#include "Piece.h"
#include "Bitboard.h"
#include <algorithm>
#include <iostream>

Evaluation::Evaluation(Board& boardPar, TranspositionTable& ttPar)
    : board(boardPar), tt(ttPar)
{
    pieceValues = {
        {KING, 1000},
        {QUEEN, 850},
        {BISHOP, 310},
        {KNIGHT, 312},
        {ROOK, 496},
        {PAWN, 79},
        {EMPTY, 0}
    };
    pawnShieldBBs[0][0] = 0x0007070000000000ULL;
    pawnShieldBBs[1][0] = 0x0000000000070700ULL;
    pawnShieldBBs[0][1] = 0x00e0e00000000000ULL;
    pawnShieldBBs[1][1] = 0x0000000000e0e000ULL;
    fileBBs[0] = 0x0101010101010101ULL;
    fileBBs[1] = 0x0202020202020202ULL;
    fileBBs[2] = 0x0404040404040404ULL;
    fileBBs[3] = 0x0808080808080808ULL;
    fileBBs[4] = 0x1010101010101010ULL;
    fileBBs[5] = 0x2020202020202020ULL;
    fileBBs[6] = 0x4040404040404040ULL;
    fileBBs[7] = 0x8080808080808080ULL;
    for (int i = 0; i < 64; i++) {
        U64 square = 0;
        for (int j = 0; j < 64; j++) {
            if (std::abs(Square::rankOf(i) - Square::rankOf(j)) <= 3 &&
                std::abs(Square::fileOf(i) - Square::fileOf(j)) <= 2)
            {
                square |= (U64(1) << j);
            }
        }
        nearKingSquares[i] = square;
    }
}

void Evaluation::orderMoves(std::vector<Move>& moves) {
    std::optional<Move> ttMove = tt.getStoredMove(board, false);
    int color = board.getTurnColor();
    U64 pawnAttacks = BB::pawnAnyAttacks(board.getPiecesBB()[PAWN + (1 - color)], 1 - color);
    std::vector<Move> newMoves;
    for (Move& move : moves) {
        move.score = 0;
        if (move.cPiece != EMPTY) {
            move.score += 15 * pieceValues.at(Piece::typeOf(move.cPiece)) - pieceValues.at(Piece::typeOf(move.piece));
        }
        if (move.promotion != EMPTY) {
            move.score += pieceValues.at(Piece::typeOf(move.promotion));
        }
        if ((pawnAttacks & (U64(1) << move.toSquare())) > 0) {
            move.score -= pieceValues.at(Piece::typeOf(move.piece));
        }
        if (ttMove.has_value() && (move.fromSquare() == ttMove->fromSquare() && move.toSquare() == ttMove->toSquare()))
            move.score = 10000;
        size_t i;
        for (i = 0; (i < newMoves.size()) && (move.score < newMoves[i].score); i++);
        newMoves.insert(newMoves.begin() + i, move);
    }
    moves = newMoves;
}

std::array<int, 2> Evaluation::countMaterial(std::array<PieceList, 12>& pieceLists) {
    std::array<int, 2> material = { 0, 0 };
    for (int i = 0; i < 12; i++) {
        if (Piece::typeOf(i) != KING) {
            material[Piece::colorOf(i)] += pieceLists[i].getCount() * pieceValues.at(Piece::typeOf(i));
        }
    }
    return material;
}

double Evaluation::getOpeningWeight() {
    return 1 - std::min(1.0, (board.getMoveCount() - 1) / 10.0);
}

double Evaluation::getEndgameWeight(std::array<int, 2> material) {
    return 1 - std::min(1.0, (material[WHITE] + material[BLACK]) / 3200.0);
}

int Evaluation::countPieceSquareEval(std::array<PieceList, 12>& pieceLists, int color, double endgameWeight) {
    int pieceSquareEval = 0;
    for (int i = 0; i < 12; i++) {
        PieceList pl = pieceLists[i];
        for (int j = 0; j < pl.getCount(); j++) {
            pieceSquareEval += (int)(pieceSquareTables.getScore(i, pl[j], endgameWeight) *
                ((Piece::colorOf(i) == color) ? 1 : -1) * 1.79);
        }
    }
    return pieceSquareEval;
}

// For now, we stub out countMopUpEval.
int Evaluation::countMopUpEval(std::array<PieceList, 12>& /*pieceLists*/, int /*materialEval*/, double /*endgameWeight*/) {
    return 0;
}

int Evaluation::countKnightPawnPenalty(std::array<PieceList, 12>& pieceLists, int color) {
    std::array<int, 2> knightPawnPenalty = { 0, 0 };
    int pawnCount = pieceLists[WHITE + PAWN].getCount() + pieceLists[BLACK + PAWN].getCount();
    for (int col = 0; col < 2; col++) {
        knightPawnPenalty[col] = (int)(pieceLists[col + KNIGHT].getCount() * (16 - pawnCount) * 1);
    }
    return -(knightPawnPenalty[color] - knightPawnPenalty[1 - color]);
}

int Evaluation::countBadBishopPenalty(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color) {
    std::array<int, 2> badBishopPenalty = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        for (int i = 0; i < pieceLists[col + BISHOP].getCount(); i++) {
            U64 sameColorBB = Square::isLight(pieceLists[col + BISHOP][i]) ? 0xAA55AA55AA55AA55ULL : 0x55AA55AA55AA55AAULL;
            badBishopPenalty[col] += (int)((BB::popCount(piecesBB[col + PAWN] & sameColorBB) - 4) * 9);
        }
    }
    return -(badBishopPenalty[color] - badBishopPenalty[1 - color]);
}

int Evaluation::countBishopPairReward(std::array<PieceList, 12>& pieceLists, int color) {
    std::array<int, 2> bishopPairReward = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        bishopPairReward[col] = pieceLists[col + BISHOP].getCount() >= 2 ? 37 : 0;
    }
    return bishopPairReward[color] - bishopPairReward[1 - color];
}

int Evaluation::countRookOpenFileReward(std::array<U64, 12>& piecesBB, int color) {
    std::array<int, 2> openFileReward = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        U64 openFiles = ~BB::fileFill(piecesBB[col + PAWN] | piecesBB[(1 - col) + PAWN]);
        openFileReward[col] = (int)(BB::popCount(piecesBB[col + ROOK] & openFiles) * 37);
    }
    return openFileReward[color] - openFileReward[1 - color];
}

int Evaluation::countDoubledPawnPenalty(std::array<U64, 12>& piecesBB, int color) {
    std::array<int, 2> doubledPawnPenalty = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        U64 doubledPawns = piecesBB[col + PAWN] & BB::dirFill(piecesBB[col + PAWN], NORTH, true);
        doubledPawnPenalty[col] = (int)(BB::popCount(doubledPawns) * -21);
    }
    return -(doubledPawnPenalty[color] - doubledPawnPenalty[1 - color]);
}

int Evaluation::countIsolatedPawnPenalty(std::array<U64, 12>& piecesBB, int color) {
    std::array<int, 2> isolatedPawnPenalty = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        U64 isolatedPawns = piecesBB[col + PAWN] & ~BB::fileFill(BB::shiftTwo(piecesBB[col + PAWN], WEST)) & ~BB::fileFill(BB::shiftTwo(piecesBB[col + PAWN], EAST));
        isolatedPawnPenalty[col] = (int)(BB::popCount(isolatedPawns) * 8);
    }
    return -(isolatedPawnPenalty[color] - isolatedPawnPenalty[1 - color]);
}

int Evaluation::countPassedPawnReward(std::array<U64, 12>& piecesBB, int color) {
    std::array<int, 2> passedPawnReward = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        U64 allFrontSpans = BB::dirFill(piecesBB[(1 - col) + PAWN], (col == WHITE ? SOUTH : NORTH), true);
        allFrontSpans |= BB::shiftTwo(allFrontSpans, WEST) | BB::shiftTwo(allFrontSpans, EAST);
        U64 passedPawns = piecesBB[col + PAWN] & ~allFrontSpans;
        passedPawnReward[col] = (int)(BB::popCount(passedPawns) * 37);
    }
    return passedPawnReward[color] - passedPawnReward[1 - color];
}

int Evaluation::countBackwardPawnPenalty(std::array<U64, 12>& piecesBB, int color) {
    std::array<int, 2> backwardPawnPenalty = { 0, 0 };
    for (int col = 0; col < 2; col++) {
        U64 stops = BB::shiftTwo(piecesBB[col + PAWN], (col == WHITE ? NORTH : SOUTH));
        U64 frontSpans = BB::dirFill(piecesBB[col + PAWN], (col == WHITE ? NORTH : SOUTH), true);
        U64 attackSpans = BB::shiftTwo(frontSpans, WEST) | BB::shiftTwo(frontSpans, EAST);
        U64 enemyAttacks = BB::pawnAnyAttacks(piecesBB[(1 - col) + PAWN], 1 - col);
        U64 backwardPawnStops = stops & ~attackSpans & enemyAttacks;
        backwardPawnPenalty[col] = (int)(BB::popCount(backwardPawnStops) * 11);
    }
    return -(backwardPawnPenalty[color] - backwardPawnPenalty[1 - color]);
}

int Evaluation::countPawnShieldEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double openingWeight, double endgameWeight) {
    int allyKingFile = Square::fileOf(pieceLists[color + KING][0]);
    int enemyKingFile = Square::fileOf(pieceLists[(1 - color) + KING][0]);
    int allyKingWing = allyKingFile / 4;
    int enemyKingWing = enemyKingFile / 4;
    bool allyKingInMiddle = allyKingFile > 2 && allyKingFile < 5;
    bool enemyKingInMiddle = enemyKingFile > 2 && enemyKingFile < 5;
    int allyPawnShield = BB::popCount(pawnShieldBBs[color][allyKingWing] & piecesBB[color + PAWN]);
    int enemyPawnShield = BB::popCount(pawnShieldBBs[1 - color][enemyKingWing] & piecesBB[(1 - color) + PAWN]);
    int pawnShieldEval = (int)((allyPawnShield - enemyPawnShield) * (allyKingInMiddle ? 0.5 : 1) * (enemyKingInMiddle ? 0.5 : 1) * 34 * (1 - openingWeight) * std::max(0.0, 1 - endgameWeight * 1.5));
    return pawnShieldEval;
}

int Evaluation::countPawnStormEval(std::array<PieceList, 12>& pieceLists, std::array<U64, 12>& piecesBB, int color, double endgameWeight) {
    int allyPawnStorm = BB::popCount(nearKingSquares[pieceLists[color + KING][0]] & piecesBB[(1 - color) + PAWN]);
    int enemyPawnStorm = BB::popCount(nearKingSquares[pieceLists[(1 - color) + KING][0]] & piecesBB[color + PAWN]);
    int pawnStormEval = (int)((enemyPawnStorm - allyPawnStorm) * 1.4 * std::max(0.0, 1 - endgameWeight * 1.5));
    return pawnStormEval;
}

int Evaluation::evaluate() {
    int color = board.getTurnColor();
    std::array<PieceList, 12> pLists = board.getPieceLists();
    std::array<U64, 12> pBB = board.getPiecesBB();
    std::array<int, 2> material = countMaterial(pLists);
    double openingWeight = getOpeningWeight();
    double endgameWeight = getEndgameWeight(material);
    int materialEval = material[color] - material[1 - color];
    int pieceSquareEval = countPieceSquareEval(pLists, color, endgameWeight);
    int mopUpEval = countMopUpEval(pLists, materialEval, endgameWeight);
    int knightPawnPenalty = countKnightPawnPenalty(pLists, color);
    int badBishopPenalty = countBadBishopPenalty(pLists, pBB, color);
    int bishopPairReward = countBishopPairReward(pLists, color);
    int rookOpenFileReward = countRookOpenFileReward(pBB, color);
    int doubledPawnPenalty = countDoubledPawnPenalty(pBB, color);
    int isolatedPawnPenalty = countIsolatedPawnPenalty(pBB, color);
    int passedPawnReward = countPassedPawnReward(pBB, color);
    int backwardPawnPenalty = countBackwardPawnPenalty(pBB, color);
    int pawnShieldEval = countPawnShieldEval(pLists, pBB, color, openingWeight, endgameWeight);
    int pawnStormEval = countPawnStormEval(pLists, pBB, color, endgameWeight);
    return materialEval + pieceSquareEval + mopUpEval + knightPawnPenalty + badBishopPenalty +
        bishopPairReward + rookOpenFileReward + doubledPawnPenalty + isolatedPawnPenalty +
        passedPawnReward + backwardPawnPenalty + pawnShieldEval + pawnStormEval;
}
