// main.cpp
#include <SFML/Graphics.hpp>
#include "ChessGame.h"
#include "ChessRenderer.h"
#include "bot.h"
#include "Minimax.h"
#include <iostream>
#include <vector>
#include <cctype>
#include <cmath>
#include <thread>
#include <chrono>

// Helper function to print the board position to the console.
void printBoard(const ChessGame& game) {
    std::cout << "Current board:" << std::endl;
    for (const auto& row : game.board) {
        std::cout << row << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // Prompt user to choose bot color.
    char botColorChoice;
    std::cout << "Choose bot color (w for white, b for black): ";
    std::cin >> botColorChoice;
    bool botIsWhite = (botColorChoice == 'w' || botColorChoice == 'W');

    const int squareSize = 80;
    const int boardSize = 8;
    sf::RenderWindow window(sf::VideoMode(squareSize * boardSize, squareSize * boardSize), "Chess Game");

    ChessGame game;
    ChessRenderer renderer(squareSize, game);

    // Turn management:
    // Human moves when (whiteTurn != botIsWhite) and bot moves when (whiteTurn == botIsWhite).
    bool whiteTurn = true;
    bool gameOver = false;
    std::string gameResult = "";
    sf::Vector2i selectedSquare(-1, -1);
    std::vector<sf::Vector2i> legalMoves;

    // Pawn promotion variables.
    bool promotionPending = false;
    sf::Vector2i promotionSquare(-1, -1);

    // Create a minimax bot instance (search depth 13).
    MinimaxBot bot(botIsWhite, 1);

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
                        // Print board after promotion.
                        printBoard(game);
                    }
                }
                continue;
            }

            // Process human moves only if it's the human's turn.
            if (!gameOver && (whiteTurn != botIsWhite) && event.type == sf::Event::MouseButtonPressed &&
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
                    // Handle castling.
                    if ((std::tolower(movingPiece) == 'k') &&
                        std::abs(clickedMove.y - selectedSquare.y) == 2) {
                        game.board[clickedMove.x][clickedMove.y] = movingPiece;
                        game.board[selectedSquare.x][selectedSquare.y] = ' ';
                        if (std::isupper(movingPiece)) {
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
                        game.halfMoveClock++;
                    }
                    else {
                        bool isEnPassant = false;
                        bool isCapture = false;
                        if ((std::tolower(movingPiece) == 'p') &&
                            (selectedSquare.y != clickedMove.y) &&
                            game.board[clickedMove.x][clickedMove.y] == ' ') {
                            isEnPassant = true;
                            isCapture = true;
                        }
                        if (game.board[clickedMove.x][clickedMove.y] != ' ')
                            isCapture = true;
                        game.board[clickedMove.x][clickedMove.y] = movingPiece;
                        game.board[selectedSquare.x][selectedSquare.y] = ' ';
                        if (std::tolower(movingPiece) == 'p') {
                            bool whitePawn = (movingPiece == 'P');
                            int direction = whitePawn ? -1 : 1;
                            if (std::abs(clickedMove.x - selectedSquare.x) == 2)
                                game.enPassantTarget = sf::Vector2i(selectedSquare.x + direction, selectedSquare.y);
                            else
                                game.enPassantTarget = sf::Vector2i(-1, -1);
                            if (isEnPassant)
                                game.board[selectedSquare.x][clickedMove.y] = ' ';
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

                    // After human move, print board.
                    printBoard(game);
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
                    if (game.halfMoveClock >= 100) {
                        gameOver = true;
                        gameResult = "Draw by 50-move rule.";
                    }
                }
                else {
                    if (ChessGame::inBounds(row, col) && game.board[row][col] != ' ') {
                        char piece = game.board[row][col];
                        bool pieceWhite = std::isupper(piece);
                        if (pieceWhite == (whiteTurn)) { // must be human's piece
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

        // If it's the bot's turn (whiteTurn == botIsWhite) and no promotion is pending.
        if (!gameOver && !promotionPending && (whiteTurn == botIsWhite)) {
            // Simulate a short thinking delay.
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // Reset node counter before search.
            bot.minimaxAlgo.nodesEvaluated = 0;
            auto movePair = bot.chooseMove(game);
            if (movePair.first.x != -1 && movePair.second.x != -1) {
                char movingPiece = game.board[movePair.first.x][movePair.first.y];
                if ((std::tolower(movingPiece) == 'k') &&
                    std::abs(movePair.second.y - movePair.first.y) == 2) {
                    game.board[movePair.second.x][movePair.second.y] = movingPiece;
                    game.board[movePair.first.x][movePair.first.y] = ' ';
                    if (std::isupper(movingPiece)) {
                        if (movePair.second.y == 6) {
                            game.board[7][7] = ' ';
                            game.board[7][5] = 'R';
                            game.whiteRookKingsideMoved = true;
                        }
                        else if (movePair.second.y == 2) {
                            game.board[7][0] = ' ';
                            game.board[7][3] = 'R';
                            game.whiteRookQueensideMoved = true;
                        }
                        game.whiteKingMoved = true;
                    }
                    else {
                        if (movePair.second.y == 6) {
                            game.board[0][7] = ' ';
                            game.board[0][5] = 'r';
                            game.blackRookKingsideMoved = true;
                        }
                        else if (movePair.second.y == 2) {
                            game.board[0][0] = ' ';
                            game.board[0][3] = 'r';
                            game.blackRookQueensideMoved = true;
                        }
                        game.blackKingMoved = true;
                    }
                    game.halfMoveClock++;
                }
                else {
                    bool isEnPassant = false;
                    bool isCapture = false;
                    if ((std::tolower(movingPiece) == 'p') &&
                        (movePair.first.y != movePair.second.y) &&
                        game.board[movePair.second.x][movePair.second.y] == ' ') {
                        isEnPassant = true;
                        isCapture = true;
                    }
                    if (game.board[movePair.second.x][movePair.second.y] != ' ')
                        isCapture = true;
                    game.board[movePair.second.x][movePair.second.y] = movingPiece;
                    game.board[movePair.first.x][movePair.first.y] = ' ';
                    if (std::tolower(movingPiece) == 'p') {
                        bool whitePawn = (movingPiece == 'P');
                        int direction = whitePawn ? -1 : 1;
                        if (std::abs(movePair.second.x - movePair.first.x) == 2)
                            game.enPassantTarget = sf::Vector2i(movePair.first.x + direction, movePair.first.y);
                        else
                            game.enPassantTarget = sf::Vector2i(-1, -1);
                        if (isEnPassant)
                            game.board[movePair.first.x][movePair.second.y] = ' ';
                        if ((whitePawn && movePair.second.x == 0) || (!whitePawn && movePair.second.x == 7)) {
                            promotionPending = true;
                            promotionSquare = movePair.second;
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
                whiteTurn = !whiteTurn;
                game.moveNumber++;

                // After bot move, print board and the number of minimax nodes evaluated.
                printBoard(game);

                std::cout << "Minimax nodes evaluated: " << bot.minimaxAlgo.nodesEvaluated << std::endl;

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
                if (game.halfMoveClock >= 100) {
                    gameOver = true;
                    gameResult = "Draw by 50-move rule.";
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
    return 0;
}
