// main.cpp
#include <SFML/Graphics.hpp>
#include "core/ChessGame.h"
#include "ui/ChessRenderer.h"
#include <iostream>
#include <vector>
#include <cctype>
#include <cmath>
#include <optional>

sf::Vector2i screenToBoard(int mouseX, int mouseY, int squareSize) {
    int col = mouseX / squareSize;
    int row = mouseY / squareSize;

    return sf::Vector2i(row, col);
}

char getPieceAt(const ChessGame& game, const sf::Vector2i& pos) {
    if (ChessGame::inBounds(pos.x, pos.y)) {
        return game.board[pos.x][pos.y];
    }
    return ' ';
}

void printGameState(const ChessGame& game, bool gameOver) {
    std::cout << "=============================" << std::endl;
    std::cout << "Current turn: " << (game.whiteToMove ? "WHITE" : "BLACK") << std::endl;
    std::cout << "Game board state:" << std::endl;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << game.board[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Half move clock: " << game.halfMoveClock << std::endl;
    std::cout << "Game over: " << (gameOver ? "YES" : "NO") << std::endl;
    if (game.kingIsInCheck(game.whiteToMove)) {
        std::cout << "KING IS IN CHECK!" << std::endl;
    }
    std::cout << "=============================" << std::endl;
}

int main() {
    const int squareSize = 80;
    const int boardSize = 8;
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(squareSize * boardSize), 
                            static_cast<unsigned int>(squareSize * boardSize)}), "Chess Game");
    
    // Add this line to limit the frame rate to 60 FPS
    window.setFramerateLimit(60);

    ChessGame game;
    ChessRenderer renderer(squareSize, game);

    // Turn and game management.
    bool gameOver = false;
    std::string gameResult = "";
    sf::Vector2i selectedSquare(-1, -1);
    std::vector<sf::Vector2i> legalMoves;

    // Pawn promotion variables.
    bool promotionPending = false;
    sf::Vector2i promotionSquare(-1, -1);

    // Add this to the beginning of the main function or game loop
    bool debugMode = true;
    int frameCycle = 0;

    while (window.isOpen()) {
        // Process events - using SFML 2.x style
        sf::Event event;
        while (window.pollEvent(event)) {
            // Handle window close
            if (event.type == sf::Event::Closed) {
                window.close();
                continue;
            }
            
            // Skip if not a left mouse button press
            if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Button::Left) {
                continue;
            }
            
            int mouseX = event.mouseButton.x;
            int mouseY = event.mouseButton.y;
            
            // Use the helper function to convert screen coordinates to board coordinates
            sf::Vector2i boardPos = screenToBoard(mouseX, mouseY, squareSize);
            int row = boardPos.x;
            int col = boardPos.y;
            
            // Add a visual marker for debugging at the mouse position
            sf::CircleShape clickMarker(5);
            clickMarker.setFillColor(sf::Color::Red);
            clickMarker.setPosition(mouseX - 5, mouseY - 5);
            window.draw(clickMarker);
            window.display(); // Show the marker temporarily
            sf::sleep(sf::milliseconds(100)); // Pause briefly to see the marker
            
            // Handle promotion menu selection
            if (promotionPending) {
                float menuWidth = squareSize * 4;
                float menuHeight = squareSize;
                float startX = (window.getSize().x - menuWidth) / 2;
                float startY = (window.getSize().y - menuHeight) / 2;
                
                // Skip if click is outside promotion menu
                if (mouseY < startY || mouseY >= startY + menuHeight || 
                    mouseX < startX || mouseX >= startX + menuWidth) {
                    continue;
                }
                
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
                continue;
            }
            
            // Skip if game is over
            if (gameOver) {
                continue;
            }
            
            // Check if clicked on a legal move
            bool clickedLegalMove = false;
            sf::Vector2i clickedMove(-1, -1);
            for (const auto& move : legalMoves) {
                if (move.x == row && move.y == col) {
                    clickedLegalMove = true;
                    clickedMove = move;
                    break;
                }
            }
            
            // Handle move execution if a piece is selected and a legal move is clicked
            if (selectedSquare.x != -1 && clickedLegalMove) {
                char movingPiece = game.board[selectedSquare.x][selectedSquare.y];
                bool pieceWhite = std::isupper(movingPiece);
                
                // Handle castling
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
                            game.whiteRookQueenSideMoved = true;
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
                            game.blackRookQueenSideMoved = true;
                        }
                        game.blackKingMoved = true;
                    }
                    
                    // Castling is neither a pawn move nor a capture
                    game.halfMoveClock++;
                }
                // Handle normal moves, including en passant and promotion
                else {
                    bool isEnPassant = false;
                    bool isCapture = false;
                    
                    // Check for en passant: pawn moves diagonally to an empty square
                    if ((std::tolower(movingPiece) == 'p') &&
                        (selectedSquare.y != clickedMove.y) &&
                        game.board[clickedMove.x][clickedMove.y] == ' ') {
                        isEnPassant = true;
                        isCapture = true; // En passant counts as a capture
                    }
                    
                    // Check if capturing a piece
                    if (game.board[clickedMove.x][clickedMove.y] != ' ')
                        isCapture = true;
                        
                    game.board[clickedMove.x][clickedMove.y] = movingPiece;
                    game.board[selectedSquare.x][selectedSquare.y] = ' ';
                    
                    if (std::tolower(movingPiece) == 'p') {
                        bool whitePawn = (movingPiece == 'P');
                        int direction = whitePawn ? -1 : 1;
                        
                        // Set en passant target if moving two squares
                        if (std::abs(clickedMove.x - selectedSquare.x) == 2)
                            game.enPassantTarget = sf::Vector2i(selectedSquare.x + direction, selectedSquare.y);
                        else
                            game.enPassantTarget = sf::Vector2i(-1, -1);
                            
                        if (isEnPassant)
                            game.board[selectedSquare.x][clickedMove.y] = ' ';
                            
                        // Handle promotion
                        if ((whitePawn && clickedMove.x == 0) || (!whitePawn && clickedMove.x == 7)) {
                            promotionPending = true;
                            promotionSquare = clickedMove;
                        }
                    }
                    else {
                        game.enPassantTarget = sf::Vector2i(-1, -1);
                    }
                    
                    // Update half-move clock:
                    // Reset if pawn move or capture, otherwise increment
                    if (std::tolower(movingPiece) == 'p' || isCapture)
                        game.halfMoveClock = 0;
                    else
                        game.halfMoveClock++;
                }
                
                selectedSquare = sf::Vector2i(-1, -1);
                legalMoves.clear();
                game.whiteToMove = !game.whiteToMove;
                
                // Check for game-ending conditions
                if (game.isCheckmate(game.whiteToMove)) {
                    gameOver = true;
                    gameResult = game.whiteToMove ? "Checkmate! Black wins!" : "Checkmate! White wins!";
                    std::cout << "CHECKMATE DETECTED: " << gameResult << std::endl;
                    
                    // Verify checkmate for debugging
                    game.verifyCheckmate(game.whiteToMove);
                }
                else if (game.isStalemate(game.whiteToMove)) {
                    gameOver = true;
                    gameResult = "Draw by stalemate.";
                    std::cout << "STALEMATE DETECTED!" << std::endl;
                }
                else if (game.isDraw50MoveRule()) {
                    gameOver = true;
                    gameResult = "Draw by 50-move rule.";
                    std::cout << "50-MOVE RULE DETECTED!" << std::endl;
                }
                
                printGameState(game, gameOver);
                
                continue;
            }
            
            // Handle piece selection (when not clicking on a legal move)
            if (ChessGame::inBounds(row, col) && game.board[row][col] != ' ') {
                char piece = game.board[row][col];
                bool pieceWhite = std::isupper(piece);
                
                if (pieceWhite != game.whiteToMove) {
                    selectedSquare = sf::Vector2i(-1, -1);
                    legalMoves.clear();
                    continue;
                }
                
                selectedSquare = sf::Vector2i(row, col);
                std::vector<sf::Vector2i> pseudo = game.getLegalMoves(row, col, false);
                std::vector<sf::Vector2i> filtered;                
                for (auto move : pseudo) {
                    ChessGame simState = game.simulateMove(sf::Vector2i(row, col), move);
                    if (!simState.kingIsInCheck(game.whiteToMove))
                        filtered.push_back(move);
                }
                
                legalMoves = filtered;
                continue;
            }
            
            // Default: deselect the piece if clicking on an empty square
            selectedSquare = sf::Vector2i(-1, -1);
            legalMoves.clear();
        }

        // Render code remains the same
        window.clear();
        // Determine the game message:
        // If game over, display the checkmate/draw message;
        // otherwise, if the current player's king is in check, display "Check!".
        std::string message = "";
        if (gameOver)
            message = gameResult;
        else if (game.kingIsInCheck(game.whiteToMove))
            message = "Check!";

        renderer.render(window, selectedSquare, legalMoves, promotionPending, promotionSquare, message);
        window.display();
    }
    return 0;
}