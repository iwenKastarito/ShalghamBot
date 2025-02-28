#include "ChessGame.h"
#include <cctype>
#include <cmath>
#include <iostream>

// Initialize the game to the starting position.
ChessGame::ChessGame() {
    board = {
        "rnbqkbnr",
        "pppppppp",
        "        ",
        "        ",
        "        ",
        "        ",
        "PPPPPPPP",
        "RNBQKBNR"
    };
    whiteKingMoved = false;
    whiteRookKingsideMoved = false;
    whiteRookQueensideMoved = false;
    blackKingMoved = false;
    blackRookKingsideMoved = false;
    blackRookQueensideMoved = false;
    enPassantTarget = sf::Vector2i(-1, -1);
    halfMoveClock = 0;
}

bool ChessGame::inBounds(int row, int col) {
    return (row >= 0 && row < 8 && col >= 0 && col < 8);
}

ChessGame ChessGame::simulateMove(const sf::Vector2i& from, const sf::Vector2i& to) const {
    ChessGame newGame = *this; // Make a copy of the current state.
    char piece = newGame.board[from.x][from.y];
    bool white = std::isupper(piece);
    newGame.board[from.x][from.y] = ' ';

    // Handle castling.
    if ((piece == 'K' || piece == 'k') && std::abs(to.y - from.y) == 2) {
        newGame.board[to.x][to.y] = piece;
        if (to.y > from.y) { // kingside
            if (white) {
                newGame.board[7][7] = ' ';
                newGame.board[7][5] = 'R';
            }
            else {
                newGame.board[0][7] = ' ';
                newGame.board[0][5] = 'r';
            }
        }
        else { // queenside
            if (white) {
                newGame.board[7][0] = ' ';
                newGame.board[7][3] = 'R';
            }
            else {
                newGame.board[0][0] = ' ';
                newGame.board[0][3] = 'r';
            }
        }
    }
    // Handle en passant.
    else if ((piece == 'P' || piece == 'p') && from.y != to.y && newGame.board[to.x][to.y] == ' ') {
        newGame.board[to.x][to.y] = piece;
        newGame.board[from.x][to.y] = ' ';  // Remove captured pawn.
    }
    else {
        newGame.board[to.x][to.y] = piece;
    }
    // Pawn promotion (auto-promote to queen for simulation).
    if ((piece == 'P' && to.x == 0) || (piece == 'p' && to.x == 7)) {
        newGame.board[to.x][to.y] = white ? 'Q' : 'q';
    }
    newGame.enPassantTarget = sf::Vector2i(-1, -1);
    return newGame;
}

bool ChessGame::kingIsInCheck(bool white) const {
    char kingChar = white ? 'K' : 'k';
    sf::Vector2i kingPos(-1, -1);
    // Find the king.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == kingChar) {
                kingPos = sf::Vector2i(i, j);
                break;
            }
        }
        if (kingPos.x != -1)
            break;
    }
    if (kingPos.x == -1)
        return true;  // Error case.
    // Check each opponent piece for an attack on the king.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece == ' ')
                continue;
            bool pieceWhite = std::isupper(piece);
            if (pieceWhite == white)
                continue;
            std::vector<sf::Vector2i> oppMoves = getLegalMoves(i, j, true);
            for (auto& move : oppMoves) {
                if (move == kingPos)
                    return true;
            }
        }
    }
    return false;
}

bool ChessGame::hasLegalMoves(bool whiteTurn) const {
    // Scan the board for pieces of the current player's color
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board[row][col];
            if (piece == ' ') continue;
            
            bool pieceIsWhite = std::isupper(piece);
            if (pieceIsWhite == whiteTurn) {
                // Get pseudo-legal moves for this piece
                std::vector<sf::Vector2i> moves = getLegalMoves(row, col, false);
                
                // Check if any of these moves don't leave the king in check
                for (const sf::Vector2i& move : moves) {
                    ChessGame simState = simulateMove(sf::Vector2i(row, col), move);
                    if (!simState.kingIsInCheck(whiteTurn)) {
                        std::cout << "Found legal move for " << piece << " at [" << row << "," << col 
                                  << "] to [" << move.x << "," << move.y << "]" << std::endl;
                        return true; // Found at least one legal move
                    }
                }
            }
        }
    }
    
    std::cout << "No legal moves found for " << (whiteTurn ? "WHITE" : "BLACK") << std::endl;
    return false; // No legal moves found
}

std::vector<sf::Vector2i> ChessGame::getLegalMoves(int row, int col, bool allowKingCapture) const {
    std::vector<sf::Vector2i> moves;
    const auto& board = this->board;
    char piece = board[row][col];
    if (piece == ' ')
        return moves;
    bool white = std::isupper(piece);
    char lowerPiece = std::tolower(piece);

    // Pawn moves.
    if (lowerPiece == 'p') {
        int direction = white ? -1 : 1;
        int startRow = white ? 6 : 1;
        int newRow = row + direction;
        if (inBounds(newRow, col) && board[newRow][col] == ' ')
            moves.push_back(sf::Vector2i(newRow, col));
        if (row == startRow && inBounds(newRow + direction, col) &&
            board[newRow][col] == ' ' && board[newRow + direction][col] == ' ')
            moves.push_back(sf::Vector2i(newRow + direction, col));
        for (int dc = -1; dc <= 1; dc += 2) {
            int newCol = col + dc;
            if (inBounds(newRow, newCol)) {
                char target = board[newRow][newCol];
                if (target != ' ') {
                    if (white ? std::islower(target) : std::isupper(target)) {
                        if ((white && target == 'k') || (!white && target == 'K')) {
                            if (allowKingCapture)
                                moves.push_back(sf::Vector2i(newRow, newCol));
                        }
                        else {
                            moves.push_back(sf::Vector2i(newRow, newCol));
                        }
                    }
                }
            }
        }
        if (enPassantTarget != sf::Vector2i(-1, -1)) {
            for (int dc = -1; dc <= 1; dc += 2) {
                int newCol = col + dc;
                if (inBounds(newRow, newCol)) {
                    if (enPassantTarget == sf::Vector2i(newRow, newCol))
                        moves.push_back(sf::Vector2i(newRow, newCol));
                }
            }
        }
        return moves;
    }
    // Knight moves.
    if (lowerPiece == 'n') {
        int knightMoves[8][2] = {
            {-2, -1}, {-2, 1},
            {-1, -2}, {-1, 2},
            {1, -2},  {1, 2},
            {2, -1},  {2, 1}
        };
        for (auto& m : knightMoves) {
            int newRow = row + m[0], newCol = col + m[1];
            if (inBounds(newRow, newCol)) {
                char target = board[newRow][newCol];
                if (target == ' ')
                    moves.push_back(sf::Vector2i(newRow, newCol));
                else if (white ? std::islower(target) : std::isupper(target)) {
                    if ((white && target == 'k') || (!white && target == 'K')) {
                        if (allowKingCapture)
                            moves.push_back(sf::Vector2i(newRow, newCol));
                    }
                    else {
                        moves.push_back(sf::Vector2i(newRow, newCol));
                    }
                }
            }
        }
        return moves;
    }
    // King moves.
    if (lowerPiece == 'k') {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0)
                    continue;
                int newRow = row + dr, newCol = col + dc;
                if (inBounds(newRow, newCol)) {
                    char target = board[newRow][newCol];
                    if (target == ' ')
                        moves.push_back(sf::Vector2i(newRow, newCol));
                    else if (white ? std::islower(target) : std::isupper(target)) {
                        if ((white && target == 'k') || (!white && target == 'K')) {
                            if (allowKingCapture)
                                moves.push_back(sf::Vector2i(newRow, newCol));
                        }
                        else {
                            moves.push_back(sf::Vector2i(newRow, newCol));
                        }
                    }
                }
            }
        }
        // Castling moves.
        if (white && row == 7 && col == 4 && !whiteKingMoved) {
            if (!whiteRookKingsideMoved && board[7][7] == 'R') {
                if (board[7][5] == ' ' && board[7][6] == ' ')
                    moves.push_back(sf::Vector2i(7, 6));
            }
            if (!whiteRookQueensideMoved && board[7][0] == 'R') {
                if (board[7][1] == ' ' && board[7][2] == ' ' && board[7][3] == ' ')
                    moves.push_back(sf::Vector2i(7, 2));
            }
        }
        if (!white && row == 0 && col == 4 && !blackKingMoved) {
            if (!blackRookKingsideMoved && board[0][7] == 'r') {
                if (board[0][5] == ' ' && board[0][6] == ' ')
                    moves.push_back(sf::Vector2i(0, 6));
            }
            if (!blackRookQueensideMoved && board[0][0] == 'r') {
                if (board[0][1] == ' ' && board[0][2] == ' ' && board[0][3] == ' ')
                    moves.push_back(sf::Vector2i(0, 2));
            }
        }
        return moves;
    }
    // Linear moves for Rook, Bishop, and Queen.
    auto addLinearMoves = [&](int dr, int dc) {
        int r = row + dr, c = col + dc;
        while (inBounds(r, c)) {
            char target = board[r][c];
            if (target == ' ')
                moves.push_back(sf::Vector2i(r, c));
            else {
                if (white ? std::islower(target) : std::isupper(target)) {
                    if ((white && target == 'k') || (!white && target == 'K')) {
                        if (allowKingCapture)
                            moves.push_back(sf::Vector2i(r, c));
                        break;
                    }
                    moves.push_back(sf::Vector2i(r, c));
                }
                break;
            }
            r += dr; c += dc;
        }
        };
    if (lowerPiece == 'r') {
        addLinearMoves(1, 0);
        addLinearMoves(-1, 0);
        addLinearMoves(0, 1);
        addLinearMoves(0, -1);
        return moves;
    }
    if (lowerPiece == 'b') {
        addLinearMoves(1, 1);
        addLinearMoves(1, -1);
        addLinearMoves(-1, 1);
        addLinearMoves(-1, -1);
        return moves;
    }
    if (lowerPiece == 'q') {
        addLinearMoves(1, 0);
        addLinearMoves(-1, 0);
        addLinearMoves(0, 1);
        addLinearMoves(0, -1);
        addLinearMoves(1, 1);
        addLinearMoves(1, -1);
        addLinearMoves(-1, 1);
        addLinearMoves(-1, -1);
        return moves;
    }
    return moves;
}

void ChessGame::applyMove(const sf::Vector2i& from, const sf::Vector2i& to) {
    char piece = board[from.x][from.y];
    bool white = std::isupper(piece);
    // Handle castling.
    if ((piece == 'K' || piece == 'k') && std::abs(to.y - from.y) == 2) {
        board[to.x][to.y] = piece;
        board[from.x][from.y] = ' ';
        if (white) {
            if (to.y > from.y) {
                board[7][7] = ' ';
                board[7][5] = 'R';
                whiteRookKingsideMoved = true;
            }
            else {
                board[7][0] = ' ';
                board[7][3] = 'R';
                whiteRookQueensideMoved = true;
            }
            whiteKingMoved = true;
        }
        else {
            if (to.y > from.y) {
                board[0][7] = ' ';
                board[0][5] = 'r';
                blackRookKingsideMoved = true;
            }
            else {
                board[0][0] = ' ';
                board[0][3] = 'r';
                blackRookQueensideMoved = true;
            }
            blackKingMoved = true;
        }
    }
    else {
        bool isEnPassant = false;
        if ((std::tolower(piece) == 'p') && (from.y != to.y) && board[to.x][to.y] == ' ')
            isEnPassant = true;
        board[to.x][to.y] = piece;
        board[from.x][from.y] = ' ';
        if (std::tolower(piece) == 'p') {
            bool whitePawn = (piece == 'P');
            int direction = whitePawn ? -1 : 1;
            if (std::abs(to.x - from.x) == 2)
                enPassantTarget = sf::Vector2i(from.x + direction, from.y);
            else
                enPassantTarget = sf::Vector2i(-1, -1);
            if (isEnPassant)
                board[from.x][to.y] = ' ';
        }
        else {
            enPassantTarget = sf::Vector2i(-1, -1);
        }
    }
}