#ifndef CHESSGAME_H
#define CHESSGAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// The ChessGame class encapsulates the board state, special flags (castling, en passant)
// and game logic such as move simulation, legal move generation, and check detection.
class ChessGame {
public:
    // Data members representing the board and flags.
    std::vector<std::string> board;
    std::string fenString; // Store the current position as FEN
    
    // Piece position tracking for optimization
    std::vector<sf::Vector2i> whitePiecePositions;
    std::vector<sf::Vector2i> blackPiecePositions;
    
    bool whiteKingMoved;
    bool whiteRookKingsideMoved;
    bool whiteRookQueenSideMoved;
    bool blackKingMoved;
    bool blackRookKingsideMoved;
    bool blackRookQueenSideMoved;
    sf::Vector2i enPassantTarget;  // (-1,-1) if none available.
    int halfMoveClock;
    int fullMoveNumber;
    bool whiteToMove;

    // Constructors
    ChessGame(); // Default constructor uses standard starting position
    ChessGame(const std::string& fen); // Constructor with custom FEN position

    // FEN string manipulation
    void setPositionFromFEN(const std::string& fen);
    std::string getCurrentFEN() const;
    
    // Returns true if (row, col) is inside the board.
    static bool inBounds(int row, int col);
    
    // Returns true if the piece at (fromRow, fromCol) can move to (toRow, toCol).
    bool isValidMove(const sf::Vector2i& from, const sf::Vector2i& to, bool checkKingSafety = true) const;
    
    // Moves the piece at (fromRow, fromCol) to (toRow, toCol) if the move is valid.
    // Returns true if the move was successful.
    bool move(const sf::Vector2i& from, const sf::Vector2i& to, char promotionPiece = 'Q');
    
    // Returns a new game state after making a move (without validation).
    ChessGame simulateMove(const sf::Vector2i& from, const sf::Vector2i& to, char promotionPiece = 'Q') const;
    
    // Returns the list of legal moves for the piece at (row, col).
    // includeCastling is false by default because it requires checking king safety.
    std::vector<sf::Vector2i> getLegalMoves(int row, int col, bool includeCastling = false) const;
    
    // Returns all legal moves for a specific side
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> getAllLegalMoves(bool white) const;
    
    // Returns true if the king of the specified color is in check.
    bool kingIsInCheck(bool white) const;
    
    // Returns true if the player has at least one legal move
    bool hasLegalMoves(bool white) const;
    
    // Returns true if the player is in checkmate
    bool isCheckmate(bool white) const;
    
    // Returns true if the player is in stalemate
    bool isStalemate(bool white) const;
    
    // Returns true if it's a draw by 50-move rule
    bool isDraw50MoveRule() const;
    
    // For debugging - verifies checkmate with detailed checking
    bool verifyCheckmate(bool white) const;

private:
    // Helper methods for FEN conversion
    void boardFromFEN(const std::string& fen);
    std::string boardToFEN() const;
    void updateFenString();
    
    // Helper methods for piece position tracking
    void updatePiecePosition(const sf::Vector2i& from, const sf::Vector2i& to, std::vector<sf::Vector2i>& positions);
    void removePiecePosition(const sf::Vector2i& pos, std::vector<sf::Vector2i>& positions);
    
    // Piece-specific move validation
    bool isValidPawnMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
    bool isValidRookMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
    bool isValidKnightMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
    bool isValidBishopMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
    bool isValidQueenMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
    bool isValidKingMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
};

#endif // CHESSGAME_H
