// Utility.h
#ifndef UTILITY_H
#define UTILITY_H

inline bool inBounds(int row, int col) {
    return (row >= 0 && row < 8 && col >= 0 && col < 8);
}

#endif // UTILITY_H
