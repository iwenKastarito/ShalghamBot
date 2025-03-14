#ifndef BOT_H
#define BOT_H

#include "ChessGame.h"
#include <SFML/System/Vector2.hpp>
#include <utility>
#include <vector>

// Base class for chess bot algorithms.
class ChessBot {
public:
    bool botIsWhite;
    ChessBot(bool isWhite) : botIsWhite(isWhite) {}
    virtual ~ChessBot() = default;

    // Choose a move based on the current game state.
    // Returns a pair of positions (from, to).
    virtual std::pair<sf::Vector2i, sf::Vector2i> chooseMove(const ChessGame& game) = 0;

    // Bot applies its chosen move (including promotion logic) to the game.
    virtual void makeMove(ChessGame& game);

    // When a pawn promotion is needed, the bot chooses the piece.
    // Default behavior: always promote to queen.
    virtual char choosePromotionPiece(const ChessGame& game, char pawn);
};

// A simple bot that selects a random legal move.
class RandomBot : public ChessBot {
public:
    RandomBot(bool isWhite) : ChessBot(isWhite) {}
    std::pair<sf::Vector2i, sf::Vector2i> chooseMove(const ChessGame& game) override;
};

#endif // BOT_H
