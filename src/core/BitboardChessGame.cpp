#include "BitboardChessGame.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <bitset>
#include <algorithm>

// Initialize static lookup tables
// These would contain precomputed attack patterns for each piece type
// (implementation omitted for brevity but would be defined here)

BitboardChessGame::BitboardChessGame() 
    : BitboardChessGame("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
}

BitboardChessGame::BitboardChessGame(const std::string& fen) {
    setPositionFromFEN(fen);
}

void BitboardChessGame::setPositionFromFEN(const std::string& fen) {
    // Reset all bitboards
    whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0;
    blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0;
    
    fenString = fen;
    
    // Split the FEN string
    std::vector<std::string> fenParts;
    std::string part;
    std::istringstream ss(fen);
    
    while (std::getline(ss, part, ' ')) {
        fenParts.push_back(part);
    }
    
    if (fenParts.size() < 6) {
        std::cerr << "Invalid FEN string: " << fen << std::endl;
        setPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        return;
    }
    
    // Parse board position into bitboards
    boardFromFEN(fenParts[0]);
    
    // Parse active color
    whiteToMove = (fenParts[1] == "w");
    
    // Parse castling availability
    whiteKingMoved = !(fenParts[2].find('K') != std::string::npos || 
                      fenParts[2].find('Q') != std::string::npos);
    blackKingMoved = !(fenParts[2].find('k') != std::string::npos || 
                      fenParts[2].find('q') != std::string::npos);
    whiteRookKingsideMoved = fenParts[2].find('K') == std::string::npos;
    whiteRookQueenSideMoved = fenParts[2].find('Q') == std::string::npos;
    blackRookKingsideMoved = fenParts[2].find('k') == std::string::npos;
    blackRookQueenSideMoved = fenParts[2].find('q') == std::string::npos;
    
    // Parse en passant target square
    if (fenParts[3] == "-") {
        enPassantTarget = sf::Vector2i(-1, -1);
    } else {
        int col = fenParts[3][0] - 'a';
        int row = '8' - fenParts[3][1];
        enPassantTarget = sf::Vector2i(row, col);
    }
    
    // Parse halfmove clock and fullmove number
    halfMoveClock = std::stoi(fenParts[4]);
    fullMoveNumber = std::stoi(fenParts[5]);
    
    // Update composite bitboards
    whitePieces = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKing;
    blackPieces = blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKing;
    allPieces = whitePieces | blackPieces;
    
    // Sync standard board representation for UI compatibility
    updateBoardFromBitboards();
}

// Convert a row,col coordinate to a 0-63 square index
int BitboardChessGame::coordToSquare(int row, int col) {
    return row * 8 + col;
}

// Convert a 0-63 square index to a row,col coordinate
sf::Vector2i BitboardChessGame::squareToCoord(int square) {
    return sf::Vector2i(square / 8, square % 8);
}

void BitboardChessGame::boardFromFEN(const std::string& fen) {
    // Create temporary standard board for conversion
    board.clear();
    board.resize(8);
    
    std::istringstream ss(fen);
    std::string row;
    int rowIndex = 0;
    
    // Parse FEN board into temporary string board
    while (std::getline(ss, row, '/') && rowIndex < 8) {
        std::string boardRow;
        for (char c : row) {
            if (std::isdigit(c)) {
                int emptySquares = c - '0';
                boardRow.append(emptySquares, ' ');
            } else {
                boardRow.push_back(c);
            }
        }
        
        // Ensure the row has exactly 8 characters
        if (boardRow.length() != 8) {
            std::cerr << "Invalid FEN row: " << row << std::endl;
            boardRow.resize(8, ' ');
        }
        
        board[rowIndex++] = boardRow;
    }
    
    // Ensure the board has exactly 8 rows
    while (rowIndex < 8) {
        board[rowIndex++] = "        ";
    }
    
    // Convert string board to bitboards
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int square = coordToSquare(r, c);
            Bitboard mask = 1ULL << square;
            
            switch (board[r][c]) {
                case 'P': whitePawns |= mask; break;
                case 'N': whiteKnights |= mask; break;
                case 'B': whiteBishops |= mask; break;
                case 'R': whiteRooks |= mask; break;
                case 'Q': whiteQueens |= mask; break;
                case 'K': whiteKing |= mask; break;
                case 'p': blackPawns |= mask; break;
                case 'n': blackKnights |= mask; break;
                case 'b': blackBishops |= mask; break;
                case 'r': blackRooks |= mask; break;
                case 'q': blackQueens |= mask; break;
                case 'k': blackKing |= mask; break;
                default: break;
            }
        }
    }
}

// Synchronize the string board representation with the current bitboard state
void BitboardChessGame::updateBoardFromBitboards() {
    // Create empty board
    board.clear();
    board.resize(8, "        ");
    
    // Helper lambda to set a piece on the string board
    auto setPiece = [this](int square, char piece) {
        sf::Vector2i coord = squareToCoord(square);
        board[coord.x][coord.y] = piece;
    };
    
    // Iterate through all squares in each bitboard and set the corresponding pieces
    for (int square = 0; square < 64; square++) {
        Bitboard mask = 1ULL << square;
        
        if (whitePawns & mask) setPiece(square, 'P');
        else if (whiteKnights & mask) setPiece(square, 'N');
        else if (whiteBishops & mask) setPiece(square, 'B');
        else if (whiteRooks & mask) setPiece(square, 'R');
        else if (whiteQueens & mask) setPiece(square, 'Q');
        else if (whiteKing & mask) setPiece(square, 'K');
        else if (blackPawns & mask) setPiece(square, 'p');
        else if (blackKnights & mask) setPiece(square, 'n');
        else if (blackBishops & mask) setPiece(square, 'b');
        else if (blackRooks & mask) setPiece(square, 'r');
        else if (blackQueens & mask) setPiece(square, 'q');
        else if (blackKing & mask) setPiece(square, 'k');
    }
}

bool BitboardChessGame::inBounds(int row, int col) {
    return (row >= 0 && row < 8 && col >= 0 && col < 8);
}

// The following methods would be implemented using bitboard operations

BitboardChessGame BitboardChessGame::simulateMove(const sf::Vector2i& from, const sf::Vector2i& to) const {
    BitboardChessGame newGame = *this;
    
    // Implementation would use bitboard operations to apply the move
    // For compatibility and simplicity, we'll use the string board
    int fromSquare = coordToSquare(from.x, from.y);
    int toSquare = coordToSquare(to.x, to.y);
    char piece = board[from.x][from.y];
    
    // Example of how to clear/set bits in the appropriate bitboards
    Bitboard fromMask = 1ULL << fromSquare;
    Bitboard toMask = 1ULL << toSquare;
    
    // Clear "from" square in all bitboards
    newGame.whitePawns &= ~fromMask;
    newGame.whiteKnights &= ~fromMask;
    // ... clear in all other piece bitboards
    
    // Clear "to" square in all bitboards
    newGame.whitePawns &= ~toMask;
    newGame.whiteKnights &= ~toMask;
    // ... clear in all other piece bitboards
    
    // Set the piece in the correct bitboard at the "to" square
    if (piece == 'P') newGame.whitePawns |= toMask;
    else if (piece == 'N') newGame.whiteKnights |= toMask;
    // ... handle all other piece types
    
    // Handle special moves (castling, en passant, promotion)
    // ...
    
    // Update composite bitboards
    newGame.whitePieces = newGame.whitePawns | newGame.whiteKnights | newGame.whiteBishops | newGame.whiteRooks | newGame.whiteQueens | newGame.whiteKing;
    newGame.blackPieces = newGame.blackPawns | newGame.blackKnights | newGame.blackBishops | newGame.blackRooks | newGame.blackQueens | newGame.blackKing;
    newGame.allPieces = newGame.whitePieces | newGame.blackPieces;
    
    // Sync with string board for compatibility
    newGame.updateBoardFromBitboards();
    
    return newGame;
}

bool BitboardChessGame::kingIsInCheck(bool white) const {
    // Locate the king
    Bitboard kingBB = white ? whiteKing : blackKing;
    if (!kingBB) return true; // Error case, king missing
    
    // Find king's square (bit scan)
    int kingSquare = 0;
    while (!(kingBB & 1) && kingSquare < 63) {
        kingBB >>= 1;
        kingSquare++;
    }
    
    // Check if any enemy piece can attack the king
    // ... bitboard operations for checking attacks
    
    return false; // Placeholder
}

bool BitboardChessGame::hasLegalMoves(bool white) const {
    // Check every piece of the right color for legal moves
    Bitboard pieces = white ? whitePieces : blackPieces;
    
    // For each piece, generate moves and see if any don't leave king in check
    // ... bitboard operations
    
    return true; // Placeholder
}

std::vector<sf::Vector2i> BitboardChessGame::getLegalMoves(int row, int col, bool allowKingCapture) const {
    std::vector<sf::Vector2i> moves;
    int square = coordToSquare(row, col);
    char piece = board[row][col];
    
    // Generate moves for the piece using bitboard operations
    // ... bitboard operations
    
    return moves; // Placeholder
}

void BitboardChessGame::applyMove(const sf::Vector2i& from, const sf::Vector2i& to) {
    // Similar to simulateMove, but modifies this object directly
    
    // Apply move to bitboards
    // ...
    
    // Update game state
    whiteToMove = !whiteToMove;
    if (!whiteToMove) {
        fullMoveNumber++;
    }
    
    // Update FEN string
    updateFenString();
    
    // Sync with string board for compatibility
    updateBoardFromBitboards();
}

// Game state evaluation methods
bool BitboardChessGame::isCheckmate(bool white) const {
    return kingIsInCheck(white) && !hasLegalMoves(white);
}

bool BitboardChessGame::isStalemate(bool white) const {
    return !kingIsInCheck(white) && !hasLegalMoves(white);
}

bool BitboardChessGame::isDraw50MoveRule() const {
    return halfMoveClock >= 100;
}

// Additional methods for FEN and debugging
std::string BitboardChessGame::getCurrentFEN() const {
    return fenString;
}

bool BitboardChessGame::verifyCheckmate(bool white) const {
    // Detailed checkmate verification logic
    // ...
    return isCheckmate(white);
}

void BitboardChessGame::updateFenString() {
    // Update FEN string based on current bitboard state
    // ...
}

std::string BitboardChessGame::boardToFEN() const {
    // Convert bitboards to FEN notation
    // ...
    return ""; // Placeholder
} 