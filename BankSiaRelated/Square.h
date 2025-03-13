#pragma once
#include <string>
#include <cstdlib>

namespace Square {
    inline int fromCoords(int x, int y) {
        return y * 8 + x;
    }
    inline std::string toString(int square) {
        int file = square % 8;
        int rank = square / 8;
        char f = 'a' + file;
        char r = '1' + rank;
        return std::string() + f + r;
    }
    inline int fileOf(int square) {
        return square % 8;
    }
    inline int rankOf(int square) {
        return square / 8;
    }
    inline int fromString(const std::string& s) {
        if (s.size() < 2)
            return -1;
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        return rank * 8 + file;
    }
    inline bool isLight(int square) {
        int file = square % 8;
        int rank = square / 8;
        return ((file + rank) % 2 == 0);
    }
}
