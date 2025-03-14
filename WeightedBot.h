#ifndef WEIGHTED_BOT_H
#define WEIGHTED_BOT_H

#include "ChessGame.h"
#include "bot.h"
#include <SFML/System.hpp>
#include <utility>
#include <vector>

// A WeightedBot selects moves based solely on the positional weight assigned to the destination square.
class WeightedBot : public ChessBot {
public:
    WeightedBot(bool isWhite) : ChessBot(isWhite) {}
    virtual std::pair<sf::Vector2i, sf::Vector2i> chooseMove(const ChessGame& game) override;
};

#endif // WEIGHTED_BOT_H
