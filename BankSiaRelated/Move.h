#pragma once
#include <string>
#include <cctype>
#include "Piece.h"

struct Move {
    int from_r, from_c, to_r, to_c;
    int piece;      // moving piece
    int cPiece;     // captured piece; EMPTY if none
    int promotion;  // promotion piece; EMPTY if none
    bool enPassant; // true if en passant
    bool castling;  // true if castling
    int score;      // for move ordering

    std::string getNotation() const;
    inline int fromSquare() const { return from_r * 8 + from_c; }
    inline int toSquare() const { return to_r * 8 + to_c; }

    static Move nullmove() {
        return Move{ -1, -1, -1, -1, EMPTY, EMPTY, EMPTY, false, false, 0 };
    }
    static bool isNull(const Move& m) { return m.from_r == -1; }
};
std::string indexToSquare(int r, int c);
void squareToIndex(const std::string& sq, int& r, int& c);
