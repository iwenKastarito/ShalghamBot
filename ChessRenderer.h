#ifndef CHESSRENDERER_H
#define CHESSRENDERER_H

#include <SFML/Graphics.hpp>
#include <map>
#include "ChessGame.h"

// The ChessRenderer class is responsible for drawing the board, pieces,
// move hints, the pawn promotion menu, and game messages (like check or checkmate).
class ChessRenderer {
public:
    // Constructor requires the square size (in pixels) and a reference to the game state.
    ChessRenderer(int squareSize, ChessGame& game);

    // Loads the piece textures.
    void loadTextures();

    // Renders the current board state.
    // The extra parameter gameMessage is used to display "Check!" or checkmate messages.
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
