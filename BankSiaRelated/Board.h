#pragma once
#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <chrono>
#include <algorithm>
#include <array>
#include <map>
#include "Bitboard.h"
#include "PieceList.h"
#include "Zobrist.h"
#include "Move.h"
#include "Square.h"
#include "Piece.h"

// Game state enum.
enum State { PLAY, WHITE_WIN, BLACK_WIN, DRAW };

// Structure for positional info.
struct PositionalInfo {
    std::array<bool, 4> castlingRights;
    int enPassant;
    int halfMoveClock;
};

class Board {
    const std::string startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::array<U64, 12> piecesBB = {}; // 0:WP,1:WN,2:WB,3:WR,4:WQ,5:WK,6:BP,7:BN,8:BB,9:BR,10:BQ,11:BK
    U64 takenBB = 0;
    std::array<U64, 2> colorBB = { 0,0 };
    std::array<int, 64> piecesMB = {};
    std::array<PieceList, 12> pieceLists = {};

    bool turnColor = WHITE;
    std::array<bool, 4> castlingRights = { false, false, false, false };
    int enPassant = -1;
    int halfMoveClock = 0;
    int moveCount = 1;
    Zobrist zobrist;

    bool isCheck = false;
    std::stack<PositionalInfo> previousInfo;
    std::vector<U64> previousPositions;
    std::vector<Move> moveHistory;
    bool normalStart = true;
    std::vector<Move> moveList;

    std::array<int, 16> dirs = { EAST, WEST, NORTH, SOUTH,
                                 NORTH_EAST, SOUTH_WEST, SOUTH_EAST, NORTH_WEST,
                                 NORTH_NORTH_EAST, SOUTH_SOUTH_EAST, NORTH_NORTH_WEST, SOUTH_SOUTH_WEST,
                                 NORTH_EAST_EAST, NORTH_WEST_WEST, SOUTH_EAST_EAST, SOUTH_WEST_WEST };
    std::map<int, int> dirToIndex = { {EAST, 0}, {WEST, 1}, {NORTH, 2}, {SOUTH, 3},
                                       {NORTH_EAST, 4}, {SOUTH_WEST, 5}, {SOUTH_EAST, 6}, {NORTH_WEST, 7},
                                       {NORTH_NORTH_EAST, 8}, {SOUTH_SOUTH_EAST, 9}, {NORTH_NORTH_WEST, 10}, {SOUTH_SOUTH_WEST, 11},
                                       {NORTH_EAST_EAST, 12}, {NORTH_WEST_WEST, 13}, {SOUTH_EAST_EAST, 14}, {SOUTH_WEST_WEST, 15} };
public:
    void loadStartPosition();
    void loadFromFen(std::string fen);
    std::string getFen();

    void movePiece(int piece, int from, int to);
    void addPiece(int piece, int square);
    void removePiece(int piece, int square);

    void makeMove(Move move);
    void unmakeMove(Move move);
    bool isKingInCheck(bool white);

    // Use your internal move generation (e.g., DirGolem) to populate moveList.
    void generateMoves(bool onlyCaptures = false);

    bool checkDraw();
    int getState();
    bool checkRepetition();

    int getTurnColor() const;

    bool getCheck();
    int getHalfMoveClock();
    int getMoveCount();
    std::array<U64, 12> getPiecesBB();
    std::array<int, 64> getPiecesMB();
    std::array<PieceList, 12> getPieceLists();
    U64 getZobristKey();
    std::vector<Move> getMoveList();
    std::vector<Move> getMoveHistory();
    bool getNormalStart();

    bool isKingSafeForSide(bool white);
    int getKingSquare(bool white);
    bool isSquareAttacked(int square, bool byWhite);

    // Provide moveToUCI inline.
    inline std::string moveToUCI(const Move& m) { return m.getNotation(); }
};
