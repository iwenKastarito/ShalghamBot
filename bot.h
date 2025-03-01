#ifndef BOT_H
#define BOT_H

#include "ChessGame.h"  // Make sure this header defines ChessGame, including member moveNumber.
#include <SFML/System/Vector2.hpp>
#include <utility>
#include <vector>

// Base class for chess bot algorithms.
class ChessBot {
public:
    bool botIsWhite;
    ChessBot(bool isWhite) : botIsWhite(isWhite) {}
    virtual ~ChessBot() = default;
    virtual std::pair<sf::Vector2i, sf::Vector2i> chooseMove(const ChessGame& game) = 0;
};

// A simple bot that selects a random legal move.
class RandomBot : public ChessBot {
public:
    RandomBot(bool isWhite) : ChessBot(isWhite) {}
    std::pair<sf::Vector2i, sf::Vector2i> chooseMove(const ChessGame& game) override;
};

#endif // BOT_H
