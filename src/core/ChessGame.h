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
    bool whiteKingMoved;
    bool whiteRookKingsideMoved;
    bool whiteRookQueensideMoved;
    bool blackKingMoved;
    bool blackRookKingsideMoved;
    bool blackRookQueensideMoved;
    sf::Vector2i enPassantTarget;  // (-1,-1) if none available.


    int halfMoveClock;

    // Constructor: initializes a standard chess starting position.
    ChessGame();

    // Returns true if (row, col) is inside the board.
    static bool inBounds(int row, int col);

    // Returns a new game state with the move (from → to) applied (used for simulation).
    ChessGame simulateMove(const sf::Vector2i& from, const sf::Vector2i& to) const;

    // Checks whether the king of the given color (true = white) is in check.
    bool kingIsInCheck(bool white) const;

    // Returns true if any legal moves exist for the given side.
    bool hasLegalMoves(bool white) const;

    // Returns pseudo-legal moves for the piece at (row, col).
    // The allowKingCapture flag is used during check detection.
    std::vector<sf::Vector2i> getLegalMoves(int row, int col, bool allowKingCapture = false) const;

    // Applies a move (from → to) to the current game state.
    void applyMove(const sf::Vector2i& from, const sf::Vector2i& to);
    
    // Returns true if the player is in checkmate
    bool isCheckmate(bool white) const;
    
    // Returns true if the player is in stalemate
    bool isStalemate(bool white) const;
    
    // Returns true if it's a draw by 50-move rule
    bool isDraw50MoveRule() const;
    
    // For debugging - verifies checkmate with detailed checking
    bool verifyCheckmate(bool white) const;
};

#endif // CHESSGAME_H
