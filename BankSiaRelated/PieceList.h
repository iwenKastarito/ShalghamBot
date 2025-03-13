#pragma once
#include <vector>
#include <algorithm>

class PieceList {
    std::vector<int> squares;
public:
    PieceList() : squares() {}
    void add(int square) { squares.push_back(square); }
    void remove(int square) {
        auto it = std::find(squares.begin(), squares.end(), square);
        if (it != squares.end())
            squares.erase(it);
    }
    void move(int from, int to) {
        auto it = std::find(squares.begin(), squares.end(), from);
        if (it != squares.end())
            *it = to;
    }
    int getCount() const { return squares.size(); }
    int operator[](int index) const { return squares[index]; }
};
