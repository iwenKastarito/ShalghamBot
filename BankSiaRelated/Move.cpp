#include "Move.h"
#include <cctype>

std::string indexToSquare(int r, int c) {
    char file = 'a' + c;
    char rank = '1' + r;
    return std::string() + file + rank;
}

void squareToIndex(const std::string& sq, int& r, int& c) {
    if (sq.size() < 2) { r = -1; c = -1; return; }
    c = sq[0] - 'a';
    r = sq[1] - '1';
}

std::string Move::getNotation() const {
    std::string from = indexToSquare(from_r, from_c);
    std::string to = indexToSquare(to_r, to_c);
    std::string moveStr = from + to;
    if (promotion != EMPTY)
        moveStr.push_back(std::tolower('q')); // Assume queen promotion.
    return moveStr;
}
