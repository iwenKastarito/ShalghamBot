#include "Bitboard.h"
#include "Board.h"
#include "Square.h"
#include "Piece.h"
#include <bit>
#include <array>

// Define the excludeFiles array.
std::array<U64, 8> BB::excludeFiles = {
    0xFFFFFFFFFFFFFFFFULL,
    0x7f7f7f7f7f7f7f7fULL,
    0x3f3f3f3f3f3f3f3fULL,
    0x0ULL,
    0x0ULL,
    0x0ULL,
    0xfcfcfcfcfcfcfcfcULL,
    0xfefefefefefefefeULL
};

int BB::bitScanForward(U64 x) {
    if (x == 0) return -1;
    return std::countr_zero(x);
}

int BB::popCount(U64 x) {
    return std::popcount(x);
}

U64 BB::genShift(U64 x, int shift) {
    return (shift > 0) ? (x << shift) : (x >> -shift);
}

U64 BB::shiftTwo(U64 x, int shift) {
    int horizontal = (shift % 8 + 8) % 8;
    x &= excludeFiles[horizontal];
    return genShift(x, shift);
}

U64 BB::kingAttacks(U64 kingSet) {
    kingSet |= shiftTwo(kingSet, EAST) | shiftTwo(kingSet, WEST);
    kingSet |= shiftTwo(kingSet, SOUTH) | shiftTwo(kingSet, NORTH);
    return kingSet;
}

U64 BB::knightAttacks(U64 knightSet) {
    U64 west, east, attacks;
    east = shiftTwo(knightSet, EAST);
    west = shiftTwo(knightSet, WEST);
    attacks = genShift(east | west, NORTH + NORTH);
    attacks |= genShift(east | west, SOUTH + SOUTH);
    east = shiftTwo(east, EAST);
    west = shiftTwo(west, WEST);
    attacks |= genShift(east | west, NORTH);
    attacks |= genShift(east | west, SOUTH);
    return attacks;
}

U64 BB::rayAttacks(U64 set, U64 empty, int shift) {
    for (int cycle = 0; cycle < 7; cycle++) {
        set |= empty & shiftTwo(set, shift);
    }
    return shiftTwo(set, shift);
}

U64 BB::pawnDirAttacks(U64 pawnSet, int color, int shift) {
    return shiftTwo(pawnSet, shift + (color == WHITE ? NORTH : SOUTH));
}

U64 BB::pawnAnyAttacks(U64 pawnSet, int color) {
    return pawnDirAttacks(pawnSet, color, EAST) | pawnDirAttacks(pawnSet, color, WEST);
}

U64 BB::dirFill(U64 set, int shift, bool excludeOriginal) {
    set |= shiftTwo(set, shift);
    set |= shiftTwo(set, 2 * shift);
    set |= shiftTwo(set, 4 * shift);
    return excludeOriginal ? shiftTwo(set, shift) : set;
}

U64 BB::fileFill(U64 set) {
    return dirFill(set, NORTH, false) | dirFill(set, SOUTH, false);
}

// Stub for generating pseudo-legal moves.
// In a full implementation, this function would generate all pseudo-legal moves
// (including moves that leave the king in check) based on the board state.
void BB::generatePseudoLegalMoves(std::vector<Move>& moves, const Board& board, bool onlyCaptures) {
    moves.clear();
    // For demonstration purposes, generate a single dummy move.
    // If it's white's turn, move a pawn from e2 to e3.
    if (board.getTurnColor() == WHITE) {
        Move dummy;
        dummy.from_r = 1; dummy.from_c = 4; // e2
        dummy.to_r = 2; dummy.to_c = 4;     // e3
        dummy.piece = PAWN;
        dummy.cPiece = EMPTY;
        dummy.promotion = EMPTY;
        dummy.enPassant = false;
        dummy.castling = false;
        dummy.score = 0;
        // If only captures are required, leave moves empty.
        if (!onlyCaptures)
            moves.push_back(dummy);
    }
    else {
        // For black, move a pawn from e7 to e6.
        Move dummy;
        dummy.from_r = 6; dummy.from_c = 4; // e7
        dummy.to_r = 5; dummy.to_c = 4;     // e6
        dummy.piece = PAWN;
        dummy.cPiece = EMPTY;
        dummy.promotion = EMPTY;
        dummy.enPassant = false;
        dummy.castling = false;
        dummy.score = 0;
        if (!onlyCaptures)
            moves.push_back(dummy);
    }
}
