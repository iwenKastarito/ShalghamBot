#pragma once
#include <cctype>
#include <string>

// Global constant for an empty square.
const int EMPTY = 0;

// Piece types (for evaluation, piece square tables, etc.)
const int PAWN = 1;
const int KNIGHT = 2;
const int BISHOP = 3;
const int ROOK = 4;
const int QUEEN = 5;
const int KING = 6;

// Colors.
const int WHITE = 0;
const int BLACK = 1;

// Board indices for bitboards and piece lists.
// White pieces will be stored at indices 0..5; Black pieces at indices 6..11.
const int WP = 0;
const int WN = 1;
const int WB = 2;
const int WR = 3;
const int WQ = 4;
const int WK = 5;
const int BP = 6;
const int BN = 7;
const int BB = 8;
const int BR = 9;
const int BQ = 10;
const int BK = 11;

namespace Piece {
    inline int charToInt(char c) {
        // Assume uppercase is white, lowercase is black.
        // Here we only return the piece type.
        switch (std::toupper(c)) {
        case 'P': return PAWN;
        case 'N': return KNIGHT;
        case 'B': return BISHOP;
        case 'R': return ROOK;
        case 'Q': return QUEEN;
        case 'K': return KING;
        default:  return EMPTY;
        }
    }
    inline char intToChar(int piece) {
        if (piece == EMPTY) return '.';
        char c;
        switch (piece) {
        case PAWN:   c = 'P'; break;
        case KNIGHT: c = 'N'; break;
        case BISHOP: c = 'B'; break;
        case ROOK:   c = 'R'; break;
        case QUEEN:  c = 'Q'; break;
        case KING:   c = 'K'; break;
        default:     c = '?'; break;
        }
        return c;
    }
    inline int typeOf(int piece) { return piece; }
    inline int colorOf(int piece) {
        // For this stub, assume that pieces with value between PAWN and KING (inclusive)
        // are white, and black pieces are stored separately (e.g. in Board indices BP..BK).
        // In a full implementation you would store color separately.
        return (piece >= PAWN && piece <= KING) ? WHITE : BLACK;
    }
}
