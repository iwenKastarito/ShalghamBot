#ifndef CHESSGAME_H
#define CHESSGAME_H

#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>

// The ChessGame class encapsulates the board state, special flags (castling, en passant)
// and game logic such as move simulation, legal move generation, and check detection.
class ChessGame {
public:
    // Board represented as a 64-element array (8x8 board).
    std::array<char, 64> board;

    bool whiteKingMoved;
    bool whiteRookKingsideMoved;
    bool whiteRookQueensideMoved;
    bool blackKingMoved;
    bool blackRookKingsideMoved;
    bool blackRookQueensideMoved;
    sf::Vector2i enPassantTarget;  // (-1,-1) if none available.

    int moveNumber;
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
};

#endif // CHESSGAME_H
