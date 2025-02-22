// main.cpp
#include <SFML/Graphics.hpp>
#include "ChessGame.h"
#include "ChessRenderer.h"
#include <iostream>
#include <vector>
#include <cctype>
#include <cmath>

int main() {
    const int squareSize = 80;
    const int boardSize = 8;
    sf::RenderWindow window(sf::VideoMode(squareSize * boardSize, squareSize * boardSize), "Chess Game");

    ChessGame game;
    ChessRenderer renderer(squareSize, game);

    // Turn and game management.
    bool whiteTurn = true;
    bool gameOver = false;
    std::string gameResult = "";
    sf::Vector2i selectedSquare(-1, -1);
    std::vector<sf::Vector2i> legalMoves;

    // Pawn promotion variables.
    bool promotionPending = false;
    sf::Vector2i promotionSquare(-1, -1);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle promotion menu events exclusively.
            if (promotionPending) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;
                    float menuWidth = squareSize * 4;
                    float menuHeight = squareSize;
                    float startX = (window.getSize().x - menuWidth) / 2;
                    float startY = (window.getSize().y - menuHeight) / 2;
                    if (mouseY >= startY && mouseY < startY + menuHeight &&
                        mouseX >= startX && mouseX < startX + menuWidth) {
                        int optionIndex = (mouseX - startX) / squareSize;
                        char currentPawn = game.board[promotionSquare.x][promotionSquare.y];
                        std::vector<char> options;
                        if (currentPawn == 'P')
                            options = { 'N', 'R', 'B', 'Q' };
                        else if (currentPawn == 'p')
                            options = { 'n', 'r', 'b', 'q' };
                        if (optionIndex >= 0 && optionIndex < options.size())
                            game.board[promotionSquare.x][promotionSquare.y] = options[optionIndex];
                        promotionPending = false;
                    }
                }
                continue;
            }

            // Process normal move events if the game is not over.
            if (!gameOver && event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                int col = event.mouseButton.x / squareSize;
                int row = event.mouseButton.y / squareSize;
                bool clickedLegalMove = false;
                sf::Vector2i clickedMove(-1, -1);
                for (const auto& move : legalMoves) {
                    if (move.x == row && move.y == col) {
                        clickedLegalMove = true;
                        clickedMove = move;
                        break;
                    }
                }
                if (selectedSquare.x != -1 && clickedLegalMove) {
                    char movingPiece = game.board[selectedSquare.x][selectedSquare.y];
                    bool pieceWhite = std::isupper(movingPiece);
                    // Handle castling.
                    if ((std::tolower(movingPiece) == 'k') &&
                        std::abs(clickedMove.y - selectedSquare.y) == 2) {
                        game.board[clickedMove.x][clickedMove.y] = movingPiece;
                        game.board[selectedSquare.x][selectedSquare.y] = ' ';
                        if (pieceWhite) {
                            if (clickedMove.y == 6) {
                                game.board[7][7] = ' ';
                                game.board[7][5] = 'R';
                                game.whiteRookKingsideMoved = true;
                            }
                            else if (clickedMove.y == 2) {
                                game.board[7][0] = ' ';
                                game.board[7][3] = 'R';
                                game.whiteRookQueensideMoved = true;
                            }
                            game.whiteKingMoved = true;
                        }
                        else {
                            if (clickedMove.y == 6) {
                                game.board[0][7] = ' ';
                                game.board[0][5] = 'r';
                                game.blackRookKingsideMoved = true;
                            }
                            else if (clickedMove.y == 2) {
                                game.board[0][0] = ' ';
                                game.board[0][3] = 'r';
                                game.blackRookQueensideMoved = true;
                            }
                            game.blackKingMoved = true;
                        }
                        // Castling is neither a pawn move nor a capture.
                        game.halfMoveClock++;
                    }
                    // Handle normal moves, including en passant and promotion.
                    else {
                        bool isEnPassant = false;
                        bool isCapture = false;
                        // Check for en passant: pawn moves diagonally to an empty square.
                        if ((std::tolower(movingPiece) == 'p') &&
                            (selectedSquare.y != clickedMove.y) &&
                            game.board[clickedMove.x][clickedMove.y] == ' ') {
                            isEnPassant = true;
                            isCapture = true; // En passant counts as a capture.
                        }
                        // If the destination already contains a piece, mark as a capture.
                        if (game.board[clickedMove.x][clickedMove.y] != ' ')
                            isCapture = true;
                        game.board[clickedMove.x][clickedMove.y] = movingPiece;
                        game.board[selectedSquare.x][selectedSquare.y] = ' ';
                        if (std::tolower(movingPiece) == 'p') {
                            bool whitePawn = (movingPiece == 'P');
                            int direction = whitePawn ? -1 : 1;
                            // Set en passant target if moving two squares.
                            if (std::abs(clickedMove.x - selectedSquare.x) == 2)
                                game.enPassantTarget = sf::Vector2i(selectedSquare.x + direction, selectedSquare.y);
                            else
                                game.enPassantTarget = sf::Vector2i(-1, -1);
                            if (isEnPassant)
                                game.board[selectedSquare.x][clickedMove.y] = ' ';
                            // Handle promotion.
                            if ((whitePawn && clickedMove.x == 0) || (!whitePawn && clickedMove.x == 7)) {
                                promotionPending = true;
                                promotionSquare = clickedMove;
                            }
                        }
                        else {
                            game.enPassantTarget = sf::Vector2i(-1, -1);
                        }
                        // Update half-move clock:
                        // Reset if pawn move or capture, otherwise increment.
                        if (std::tolower(movingPiece) == 'p' || isCapture)
                            game.halfMoveClock = 0;
                        else
                            game.halfMoveClock++;
                    }
                    selectedSquare = sf::Vector2i(-1, -1);
                    legalMoves.clear();
                    whiteTurn = !whiteTurn;
                    // Check for stalemate or checkmate.
                    if (!game.hasLegalMoves(whiteTurn)) {
                        if (game.kingIsInCheck(whiteTurn)) {
                            gameOver = true;
                            gameResult = whiteTurn ? "Checkmate! Black wins!" : "Checkmate! White wins!";
                        }
                        else {
                            gameOver = true;
                            gameResult = "Draw by stalemate.";
                        }
                    }
                    // Check for draw by 50-move rule.
                    if (game.halfMoveClock >= 100) {
                        gameOver = true;
                        gameResult = "Draw by 50-move rule.";
                    }
                }
                else {
                    if (ChessGame::inBounds(row, col) && game.board[row][col] != ' ') {
                        char piece = game.board[row][col];
                        bool pieceWhite = std::isupper(piece);
                        if (pieceWhite != whiteTurn)
                            continue;
                        selectedSquare = sf::Vector2i(row, col);
                        std::vector<sf::Vector2i> pseudo = game.getLegalMoves(row, col, false);
                        std::vector<sf::Vector2i> filtered;
                        for (auto move : pseudo) {
                            ChessGame simState = game.simulateMove(sf::Vector2i(row, col), move);
                            if (!simState.kingIsInCheck(whiteTurn))
                                filtered.push_back(move);
                        }
                        legalMoves = filtered;
                    }
                    else {
                        selectedSquare = sf::Vector2i(-1, -1);
                        legalMoves.clear();
                    }
                }
            }
        }
        window.clear();
        // Determine the game message:
        // If game over, display the checkmate/draw message;
        // otherwise, if the current player's king is in check, display "Check!".
        std::string message = "";
        if (gameOver)
            message = gameResult;
        else if (game.kingIsInCheck(whiteTurn))
            message = "Check!";

        renderer.render(window, selectedSquare, legalMoves, promotionPending, promotionSquare, message);
        window.display();
    }
    return 0;
}
