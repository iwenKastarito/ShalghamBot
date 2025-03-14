#ifndef CHESSRENDERER_H
#define CHESSRENDERER_H

#include <SFML/Graphics.hpp>
#include <map>
#include "ChessGame.h"

// The ChessRenderer class is responsible for drawing the board, pieces,
// move hints, the pawn promotion menu, and game messages.
class ChessRenderer {
public:
    ChessRenderer(int squareSize, ChessGame& game);
    void loadTextures();
    void render(sf::RenderWindow& window,
        const sf::Vector2i& selectedSquare,
        const std::vector<sf::Vector2i>& legalMoves,
        bool promotionPending,
        const sf::Vector2i& promotionSquare,
        const std::string& gameMessage = "");
private:
    int squareSize;
    ChessGame& game;
    std::map<char, sf::Texture> pieceTextures;
    sf::Font font;
    sf::Text messageText;
};

#endif // CHESSRENDERER_H
