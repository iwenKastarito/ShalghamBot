#ifndef CHESSMOVE_H
#define CHESSMOVE_H

#include <SFML/Graphics.hpp>

// A structure to represent a complete chess move, including promotion information
struct ChessMove {
    sf::Vector2i from;
    sf::Vector2i to;
    char promotionPiece;  // 'Q', 'R', 'B', 'N' or 0 if not a promotion
    
    ChessMove(sf::Vector2i f, sf::Vector2i t, char p = 0)
        : from(f), to(t), promotionPiece(p) {}
    
    // For comparison and use in containers
    bool operator==(const ChessMove& other) const {
        return from.x == other.from.x && from.y == other.from.y &&
               to.x == other.to.x && to.y == other.to.y &&
               promotionPiece == other.promotionPiece;
    }
};

#endif // CHESSMOVE_H 