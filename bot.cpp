#include "bot.h"
#include <random>

std::pair<sf::Vector2i, sf::Vector2i> RandomBot::chooseMove(const ChessGame& game) {
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> legalMoves;

    // Iterate over the board and gather all legal moves.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = game.board[i][j];
            if (piece == ' ')
                continue;
            bool isWhite = std::isupper(piece);
            if (isWhite != botIsWhite)
                continue;
            auto moves = game.getLegalMoves(i, j, false);
            for (auto move : moves) {
                legalMoves.push_back({ sf::Vector2i(i, j), move });
            }
        }
    }

    if (legalMoves.empty())
        return { sf::Vector2i(-1, -1), sf::Vector2i(-1, -1) };

    // Choose a random move.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, legalMoves.size() - 1);
    int index = dis(gen);
    return legalMoves[index];
}
