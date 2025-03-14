#include <SFML/Graphics.hpp>
#include "ChessGame.h"
#include "ChessRenderer.h"
#include "bot.h"
#include "Minimax.h"
#include "WeightedBot.h"
#include <iostream>
#include <vector>
#include <cctype>
#include <cmath>
#include <thread>
#include <chrono>

// Helper function to print the board to the console.
void printBoard(const ChessGame& game) {
    std::cout << "Current board:" << std::endl;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            std::cout << game.board[row * 8 + col];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Returns true if only kings remain.
bool insufficientMaterial(const ChessGame& game) {
    for (int i = 0; i < 64; i++) {
        char piece = game.board[i];
        if (piece != ' ' && std::tolower(piece) != 'k')
            return false;
    }
    return true;
}

int main() {
    int gameMode = 0;
    std::cout << "Select game mode: 1) Human vs Bot, 2) Bot vs Bot: ";
    std::cin >> gameMode;

    ChessBot* humanVsBot = nullptr;
    ChessBot* whiteBot = nullptr;
    ChessBot* blackBot = nullptr;

    bool botIsWhite = false;
    if (gameMode == 1) {
        char botColorChoice;
        std::cout << "Choose bot color (w for white, b for black): ";
        std::cin >> botColorChoice;
        botIsWhite = (botColorChoice == 'w' || botColorChoice == 'W');

        char botTypeChoice;
        std::cout << "Choose bot type (m for minimax, r for random, w for weighted): ";
        std::cin >> botTypeChoice;
        if (botTypeChoice == 'm' || botTypeChoice == 'M') {
            humanVsBot = new MinimaxBot(botIsWhite, 3);
        }
        else if (botTypeChoice == 'w' || botTypeChoice == 'W') {
            humanVsBot = new WeightedBot(botIsWhite);
        }
        else {
            humanVsBot = new RandomBot(botIsWhite);
        }
    }
    else if (gameMode == 2) {
        char botTypeChoice;
        std::cout << "Choose bot type for both bots (m for minimax, r for random, w for weighted): ";
        std::cin >> botTypeChoice;
        if (botTypeChoice == 'm' || botTypeChoice == 'M') {
            whiteBot = new MinimaxBot(true, 3);
            blackBot = new MinimaxBot(false, 3);
        }
        else if (botTypeChoice == 'w' || botTypeChoice == 'W') {
            whiteBot = new WeightedBot(true);
            blackBot = new WeightedBot(false);
        }
        else {
            whiteBot = new RandomBot(true);
            blackBot = new RandomBot(false);
        }
    }
    else {
        std::cout << "Invalid game mode selection." << std::endl;
        return 1;
    }

    const int squareSize = 80;
    const int boardSize = 8;
    sf::RenderWindow window(sf::VideoMode(squareSize * boardSize, squareSize * boardSize), "Chess Game");
    ChessGame game;
    ChessRenderer renderer(squareSize, game);

    bool whiteTurn = true;
    bool gameOver = false;
    std::string gameResult = "";
    sf::Vector2i selectedSquare(-1, -1);
    std::vector<sf::Vector2i> legalMoves;

    bool promotionPending = false;
    sf::Vector2i promotionSquare(-1, -1);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

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
                        char currentPawn = game.board[promotionSquare.x * 8 + promotionSquare.y];
                        std::vector<char> options;
                        if (currentPawn == 'P')
                            options = { 'N', 'R', 'B', 'Q' };
                        else if (currentPawn == 'p')
                            options = { 'n', 'r', 'b', 'q' };
                        if (optionIndex >= 0 && optionIndex < options.size())
                            game.board[promotionSquare.x * 8 + promotionSquare.y] = options[optionIndex];
                        promotionPending = false;
                        printBoard(game);
                    }
                }
                continue;
            }

            if (!gameOver && (gameMode == 1) && (whiteTurn != botIsWhite) &&
                event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
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
                    char movingPiece = game.board[selectedSquare.x * 8 + selectedSquare.y];
                    if ((std::tolower(movingPiece) == 'k') &&
                        std::abs(clickedMove.y - selectedSquare.y) == 2) {
                        game.board[clickedMove.x * 8 + clickedMove.y] = movingPiece;
                        game.board[selectedSquare.x * 8 + selectedSquare.y] = ' ';
                        if (std::isupper(movingPiece)) {
                            if (clickedMove.y == 6) {
                                game.board[7 * 8 + 7] = ' ';
                                game.board[7 * 8 + 5] = 'R';
                            }
                            else if (clickedMove.y == 2) {
                                game.board[7 * 8 + 0] = ' ';
                                game.board[7 * 8 + 3] = 'R';
                            }
                        }
                        else {
                            if (clickedMove.y == 6) {
                                game.board[0 * 8 + 7] = ' ';
                                game.board[0 * 8 + 5] = 'r';
                            }
                            else if (clickedMove.y == 2) {
                                game.board[0 * 8 + 0] = ' ';
                                game.board[0 * 8 + 3] = 'r';
                            }
                        }
                        game.halfMoveClock++;
                    }
                    else {
                        bool isEnPassant = false;
                        bool isCapture = false;
                        if ((std::tolower(movingPiece) == 'p') &&
                            (selectedSquare.y != clickedMove.y) &&
                            game.board[clickedMove.x * 8 + clickedMove.y] == ' ') {
                            isEnPassant = true;
                            isCapture = true;
                        }
                        if (game.board[clickedMove.x * 8 + clickedMove.y] != ' ')
                            isCapture = true;
                        game.board[clickedMove.x * 8 + clickedMove.y] = movingPiece;
                        game.board[selectedSquare.x * 8 + selectedSquare.y] = ' ';
                        if (std::tolower(movingPiece) == 'p') {
                            bool whitePawn = (movingPiece == 'P');
                            int direction = whitePawn ? -1 : 1;
                            if (std::abs(clickedMove.x - selectedSquare.x) == 2)
                                game.enPassantTarget = sf::Vector2i(selectedSquare.x + direction, selectedSquare.y);
                            else
                                game.enPassantTarget = sf::Vector2i(-1, -1);
                            if (isEnPassant)
                                game.board[selectedSquare.x * 8 + clickedMove.y] = ' ';
                            if ((whitePawn && clickedMove.x == 0) || (!whitePawn && clickedMove.x == 7)) {
                                promotionPending = true;
                                promotionSquare = clickedMove;
                            }
                        }
                        else {
                            game.enPassantTarget = sf::Vector2i(-1, -1);
                        }
                        if (std::tolower(movingPiece) == 'p' || isCapture)
                            game.halfMoveClock = 0;
                        else
                            game.halfMoveClock++;
                    }
                    selectedSquare = sf::Vector2i(-1, -1);
                    legalMoves.clear();
                    whiteTurn = !whiteTurn;
                    game.moveNumber++;
                    printBoard(game);

                    if (insufficientMaterial(game)) {
                        gameResult = "Draw by insufficient material.";
                        gameOver = true;
                    }
                    else if (!game.hasLegalMoves(whiteTurn)) {
                        if (game.kingIsInCheck(whiteTurn))
                            gameResult = whiteTurn ? "Checkmate! Black wins!" : "Checkmate! White wins!";
                        else
                            gameResult = "Draw by stalemate.";
                        gameOver = true;
                    }
                    else if (game.halfMoveClock >= 100) {
                        gameResult = "Draw by 50-move rule.";
                        gameOver = true;
                    }
                }
                else {
                    if (ChessGame::inBounds(row, col) && game.board[row * 8 + col] != ' ') {
                        char piece = game.board[row * 8 + col];
                        if (std::isupper(piece) == whiteTurn) {
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
                    else {
                        selectedSquare = sf::Vector2i(-1, -1);
                        legalMoves.clear();
                    }
                }
            }
        }

        if (!gameOver) {
            bool botTurn = false;
            if (gameMode == 1 && (whiteTurn == botIsWhite))
                botTurn = true;
            else if (gameMode == 2)
                botTurn = true;

            if (botTurn) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (gameMode == 1)
                    humanVsBot->makeMove(game);
                else {
                    if (whiteTurn)
                        whiteBot->makeMove(game);
                    else
                        blackBot->makeMove(game);
                }
                whiteTurn = !whiteTurn;
                game.moveNumber++;
                printBoard(game);

                if (insufficientMaterial(game)) {
                    gameResult = "Draw by insufficient material.";
                    gameOver = true;
                }
                else if (!game.hasLegalMoves(whiteTurn)) {
                    if (game.kingIsInCheck(whiteTurn))
                        gameResult = whiteTurn ? "Checkmate! Black wins!" : "Checkmate! White wins!";
                    else
                        gameResult = "Draw by stalemate.";
                    gameOver = true;
                }
                else if (game.halfMoveClock >= 100) {
                    gameResult = "Draw by 50-move rule.";
                    gameOver = true;
                }
            }
        }

        window.clear();
        std::string message = "";
        if (gameOver)
            message = gameResult;
        else if (game.kingIsInCheck(whiteTurn))
            message = "Check!";
        renderer.render(window, selectedSquare, legalMoves, promotionPending, promotionSquare, message);
        window.display();
    }

    if (humanVsBot)
        delete humanVsBot;
    if (whiteBot)
        delete whiteBot;
    if (blackBot)
        delete blackBot;

    return 0;
}
