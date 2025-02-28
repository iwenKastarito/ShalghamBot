#ifndef BITBOARD_CHESS_GAME_H
#define BITBOARD_CHESS_GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cstdint>

// The BitboardChessGame class uses bitboards for more efficient chess operations
// while maintaining the same interface as ChessGame for easy swapping.
class BitboardChessGame {
public:
    // Expose a board representation for compatibility
    std::vector<std::string> board;
    std::string fenString;
    
    // Castling flags
    bool whiteKingMoved;
    bool whiteRookKingsideMoved;
    bool whiteRookQueenSideMoved;
    bool blackKingMoved;
    bool blackRookKingsideMoved;
    bool blackRookQueenSideMoved;
    
    sf::Vector2i enPassantTarget;
    int halfMoveClock;
    int fullMoveNumber;
    bool whiteToMove;

    // Bitboard representation (internal)
    typedef uint64_t Bitboard;
    
    // Constructors
    BitboardChessGame();
    BitboardChessGame(const std::string& fen);

    // FEN string manipulation
    void setPositionFromFEN(const std::string& fen);
    std::string getCurrentFEN() const;
    
    // Standard interface methods (same as ChessGame)
    static bool inBounds(int row, int col);
    BitboardChessGame simulateMove(const sf::Vector2i& from, const sf::Vector2i& to) const;
    bool kingIsInCheck(bool white) const;
    bool hasLegalMoves(bool white) const;
    std::vector<sf::Vector2i> getLegalMoves(int row, int col, bool allowKingCapture = false) const;
    void applyMove(const sf::Vector2i& from, const sf::Vector2i& to);
    
    // Game state evaluation
    bool isCheckmate(bool white) const;
    bool isStalemate(bool white) const;
    bool isDraw50MoveRule() const;
    bool verifyCheckmate(bool white) const;

private:
    // Bitboards for each piece type and color
    Bitboard whitePawns;
    Bitboard whiteKnights;
    Bitboard whiteBishops;
    Bitboard whiteRooks;
    Bitboard whiteQueens;
    Bitboard whiteKing;
    Bitboard blackPawns;
    Bitboard blackKnights;
    Bitboard blackBishops;
    Bitboard blackRooks;
    Bitboard blackQueens;
    Bitboard blackKing;
    
    // Helper bitboards
    Bitboard whitePieces;
    Bitboard blackPieces;
    Bitboard allPieces;
    
    // Conversion methods
    void boardFromFEN(const std::string& fen);
    std::string boardToFEN() const;
    void updateFenString();
    void updateBoardFromBitboards();
    
    // Bitboard operations
    Bitboard getPieceAttacks(int square, bool isWhite, int pieceType) const;
    Bitboard getKnightAttacks(int square) const;
    Bitboard getBishopAttacks(int square, Bitboard occupied) const;
    Bitboard getRookAttacks(int square, Bitboard occupied) const;
    Bitboard getQueenAttacks(int square, Bitboard occupied) const;
    Bitboard getKingAttacks(int square) const;
    Bitboard getPawnAttacks(int square, bool isWhite) const;
    Bitboard getPawnMoves(int square, bool isWhite) const;
    
    // Square conversion
    static int coordToSquare(int row, int col);
    static sf::Vector2i squareToCoord(int square);
    
    // Bitboard lookup tables (for move generation)
    static const Bitboard KNIGHT_ATTACKS[64];
    static const Bitboard KING_ATTACKS[64];
    static const Bitboard PAWN_ATTACKS[2][64]; // [color][square]
};

#endif // BITBOARD_CHESS_GAME_H 