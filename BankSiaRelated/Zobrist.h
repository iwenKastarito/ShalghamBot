#pragma once
#include <cstdint>
typedef uint64_t U64;

class Zobrist {
    U64 hash;
public:
    Zobrist() : hash(0) {}
    void reset() { hash = 0; }
    void changeTurn() { hash ^= 0xF0F0F0F0F0F0F0F0ULL; }
    void changeCastling(int index) { hash ^= (0xF0F0F0F0ULL << index); }
    void changeEnPassant(int file) { hash ^= (0x12345678ULL << file); }
    void movePiece(int piece, int from, int to) { hash ^= (0xABCDEFULL + from + to); }
    void changePiece(int piece, int square) { hash ^= (0xDEADBEEFULL + square); }
    U64 getHashKey() { return hash; }
    void set(U64 newHash) { hash = newHash; }
};
