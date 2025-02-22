#include "ChessRenderer.h"
#include <iostream>
#include <cmath>

ChessRenderer::ChessRenderer(int squareSize, ChessGame& game)
    : squareSize(squareSize), game(game)
{
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font 'arial.ttf'" << std::endl;
    }
    messageText.setFont(font);
    messageText.setCharacterSize(32);
    messageText.setFillColor(sf::Color::Red);
    messageText.setPosition(10, 10);
    loadTextures();
}

void ChessRenderer::loadTextures() {
    std::map<char, std::string> pieceFiles = {
        {'K', "Pices/white_king.png"},
        {'Q', "Pices/white_queen.png"},
        {'R', "Pices/white_rook.png"},
        {'B', "Pices/white_bishop.png"},
        {'N', "Pices/white_knight.png"},
        {'P', "Pices/white_pawn.png"},
        {'k', "Pices/black_king.png"},
        {'q', "Pices/black_queen.png"},
        {'r', "Pices/black_rook.png"},
        {'b', "Pices/black_bishop.png"},
        {'n', "Pices/black_knight.png"},
        {'p', "Pices/black_pawn.png"}
    };
    for (const auto& entry : pieceFiles) {
        sf::Texture texture;
        if (!texture.loadFromFile(entry.second)) {
            std::cerr << "Error loading texture: " << entry.second << std::endl;
        }
        pieceTextures[entry.first] = texture;
    }
}

void ChessRenderer::render(sf::RenderWindow& window,
    const sf::Vector2i& selectedSquare,
    const std::vector<sf::Vector2i>& legalMoves,
    bool promotionPending,
    const sf::Vector2i& promotionSquare,
    const std::string& gameMessage)
{
    int boardSize = 8;
    // 1. Draw board squares.
    for (int row = 0; row < boardSize; ++row) {
        for (int col = 0; col < boardSize; ++col) {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(col * squareSize, row * squareSize);
            if ((row + col) % 2 == 0)
                square.setFillColor(sf::Color(240, 217, 181));
            else
                square.setFillColor(sf::Color(181, 136, 99));
            window.draw(square);
        }
    }
    // 2. Draw pieces.
    for (int row = 0; row < boardSize; ++row) {
        for (int col = 0; col < boardSize; ++col) {
            char piece = game.board[row][col];
            if (piece != ' ') {
                sf::Sprite sprite;
                sprite.setTexture(pieceTextures[piece]);
                sf::Vector2u texSize = pieceTextures[piece].getSize();
                float scaleX = static_cast<float>(squareSize) / texSize.x;
                float scaleY = static_cast<float>(squareSize) / texSize.y;
                sprite.setScale(scaleX, scaleY);
                sprite.setPosition(col * squareSize, row * squareSize);
                window.draw(sprite);
            }
        }
    }
    // 3. Highlight the selected square.
    if (selectedSquare.x != -1) {
        sf::RectangleShape highlight(sf::Vector2f(squareSize, squareSize));
        highlight.setPosition(selectedSquare.y * squareSize, selectedSquare.x * squareSize);
        highlight.setFillColor(sf::Color(255, 255, 0, 100));
        window.draw(highlight);
    }
    // 4. Draw legal move hints.
    for (const auto& move : legalMoves) {
        sf::CircleShape hint(squareSize / 6.0f);
        sf::Color hintColor;
        if (selectedSquare.x != -1) {
            char selPiece = game.board[selectedSquare.x][selectedSquare.y];
            // For a king making a two-square move, mark castling with blue.
            if (std::tolower(selPiece) == 'k' && std::abs(move.y - selectedSquare.y) == 2) {
                hintColor = sf::Color(0, 0, 255, 150); // Blue for castling.
            }
            // If the move is an en passant capture or a normal capture,
            // (en passant: pawn move to enPassantTarget, which is empty on board).
            else if ((std::tolower(selPiece) == 'p' && game.enPassantTarget == move) ||
                game.board[move.x][move.y] != ' ') {
                hintColor = sf::Color(255, 0, 0, 150); // Red for capture/en passant.
            }
            else {
                hintColor = sf::Color(0, 255, 0, 150); // Green for a normal move.
            }
        }
        else {
            hintColor = sf::Color(0, 255, 0, 150);
        }
        hint.setFillColor(hintColor);
        hint.setOrigin(hint.getRadius(), hint.getRadius());
        hint.setPosition(move.y * squareSize + squareSize / 2.0f,
            move.x * squareSize + squareSize / 2.0f);
        window.draw(hint);
    }

    // 5. Draw pawn promotion menu if pending.
    if (promotionPending) {
        std::vector<char> options;
        char currentPawn = game.board[promotionSquare.x][promotionSquare.y];
        if (currentPawn == 'P')
            options = { 'N', 'R', 'B', 'Q' };
        else if (currentPawn == 'p')
            options = { 'n', 'r', 'b', 'q' };
        float menuWidth = squareSize * 4;
        float menuHeight = squareSize;
        float startX = (window.getSize().x - menuWidth) / 2;
        float startY = (window.getSize().y - menuHeight) / 2;
        for (int i = 0; i < 4; i++) {
            sf::RectangleShape optionBox(sf::Vector2f(squareSize, squareSize));
            optionBox.setPosition(startX + i * squareSize, startY);
            optionBox.setFillColor(sf::Color(200, 200, 200, 200));
            window.draw(optionBox);
            sf::Sprite sprite;
            sprite.setTexture(pieceTextures[options[i]]);
            sf::Vector2u texSize = pieceTextures[options[i]].getSize();
            float scaleX = static_cast<float>(squareSize) / texSize.x;
            float scaleY = static_cast<float>(squareSize) / texSize.y;
            sprite.setScale(scaleX, scaleY);
            sprite.setPosition(startX + i * squareSize, startY);
            window.draw(sprite);
        }
    }
    // 6. Draw the game message (e.g., "Check!" or "Checkmate! ...").
    if (!gameMessage.empty()) {
        messageText.setString(gameMessage);
        window.draw(messageText);
    }
}
