#include "ChessGame.h"
#include <cctype>
#include <cmath>
#include <cstdint>
#include <vector>
#include <sstream>

// Cross-platform trailing zero counter.
#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
inline int countTrailingZeros(uint64_t x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return (int)index;
}
#else
inline int countTrailingZeros(uint64_t x) {
    return __builtin_ctzll(x);
}
#endif

// Global bitboard for knight moves.
static uint64_t arrKnightAttacks[64];
static bool arrKnightAttacksInitialized = false;

// Precompute knight moves for each square on the board.
void initKnightAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacks = 0ULL;
        int row = sq / 8;
        int col = sq % 8;
        int knightOffsets[8][2] = { {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                    { 1, -2}, { 1, 2}, { 2, -1}, { 2, 1} };
        for (int i = 0; i < 8; i++) {
            int r = row + knightOffsets[i][0];
            int c = col + knightOffsets[i][1];
            if (ChessGame::inBounds(r, c)) {
                int targetSq = r * 8 + c;
                attacks |= (1ULL << targetSq);
            }
        }
        arrKnightAttacks[sq] = attacks;
    }
    arrKnightAttacksInitialized = true;
}

ChessGame::ChessGame() {
    // Initialize board with starting position.
    board = {
        'r','n','b','q','k','b','n','r',
        'p','p','p','p','p','p','p','p',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        'P','P','P','P','P','P','P','P',
        'R','N','B','Q','K','B','N','R'
    };
    whiteKingMoved = false;
    whiteRookKingsideMoved = false;
    whiteRookQueensideMoved = false;
    blackKingMoved = false;
    blackRookKingsideMoved = false;
    blackRookQueensideMoved = false;
    enPassantTarget = sf::Vector2i(-1, -1);
    halfMoveClock = 0;
    moveNumber = 0;
}

bool ChessGame::inBounds(int row, int col) {
    return (row >= 0 && row < 8 && col >= 0 && col < 8);
}

ChessGame ChessGame::simulateMove(const sf::Vector2i& from, const sf::Vector2i& to) const {
    ChessGame newGame = *this; // Copy current state
    char piece = newGame.board[from.x * 8 + from.y];
    bool white = std::isupper(piece);
    newGame.board[from.x * 8 + from.y] = ' ';

    // Handle castling.
    if ((piece == 'K' || piece == 'k') && std::abs(to.y - from.y) == 2) {
        newGame.board[to.x * 8 + to.y] = piece;
        if (to.y > from.y) { // kingside
            if (white) {
                newGame.board[7 * 8 + 7] = ' ';
                newGame.board[7 * 8 + 5] = 'R';
            }
            else {
                newGame.board[0 * 8 + 7] = ' ';
                newGame.board[0 * 8 + 5] = 'r';
            }
        }
        else { // queenside
            if (white) {
                newGame.board[7 * 8 + 0] = ' ';
                newGame.board[7 * 8 + 3] = 'R';
            }
            else {
                newGame.board[0 * 8 + 0] = ' ';
                newGame.board[0 * 8 + 3] = 'r';
            }
        }
    }
    // Handle en passant.
    else if ((piece == 'P' || piece == 'p') && from.y != to.y && newGame.board[to.x * 8 + to.y] == ' ') {
        newGame.board[to.x * 8 + to.y] = piece;
        newGame.board[from.x * 8 + to.y] = ' ';  // Remove captured pawn.
    }
    else {
        newGame.board[to.x * 8 + to.y] = piece;
    }
    // Pawn promotion (auto-promote to queen for simulation).
    if ((piece == 'P' && to.x == 0) || (piece == 'p' && to.x == 7)) {
        newGame.board[to.x * 8 + to.y] = white ? 'Q' : 'q';
    }
    newGame.enPassantTarget = sf::Vector2i(-1, -1);
    return newGame;
}

bool ChessGame::kingIsInCheck(bool white) const {
    char kingChar = white ? 'K' : 'k';
    sf::Vector2i kingPos(-1, -1);
    // Find the king.
    for (int i = 0; i < 64; i++) {
        if (board[i] == kingChar) {
            kingPos = sf::Vector2i(i / 8, i % 8);
            break;
        }
    }
    if (kingPos.x == -1)
        return true;  // Error case.
    // Check each opponent piece for an attack on the king.
    for (int i = 0; i < 64; i++) {
        if (board[i] == ' ')
            continue;
        bool pieceWhite = std::isupper(board[i]);
        if (pieceWhite == white)
            continue;
        int row = i / 8, col = i % 8;
        std::vector<sf::Vector2i> oppMoves = getLegalMoves(row, col, true);
        for (auto& move : oppMoves) {
            if (move == kingPos)
                return true;
        }
    }
    return false;
}

bool ChessGame::hasLegalMoves(bool white) const {
    for (int i = 0; i < 64; i++) {
        if (board[i] == ' ')
            continue;
        int row = i / 8, col = i % 8;
        if (std::isupper(board[i]) != white)
            continue;
        std::vector<sf::Vector2i> legalMoves = getLegalMoves(row, col, false);
        if (!legalMoves.empty())
            return true;
    }
    return false;
}

std::vector<sf::Vector2i> ChessGame::getLegalMoves(int row, int col, bool allowKingCapture) const {
    std::vector<sf::Vector2i> pseudoMoves;
    char piece = board[row * 8 + col];
    if (piece == ' ')
        return pseudoMoves;
    bool white = std::isupper(piece);
    char lowerPiece = std::tolower(piece);

    // Pawn moves.
    if (lowerPiece == 'p') {
        int direction = white ? -1 : 1;
        int startRow = white ? 6 : 1;
        int newRow = row + direction;
        if (ChessGame::inBounds(newRow, col) && board[newRow * 8 + col] == ' ')
            pseudoMoves.push_back(sf::Vector2i(newRow, col));
        if (row == startRow && ChessGame::inBounds(newRow + direction, col) &&
            board[newRow * 8 + col] == ' ' && board[(newRow + direction) * 8 + col] == ' ')
            pseudoMoves.push_back(sf::Vector2i(newRow + direction, col));
        for (int dc = -1; dc <= 1; dc += 2) {
            int newCol = col + dc;
            if (ChessGame::inBounds(newRow, newCol)) {
                char target = board[newRow * 8 + newCol];
                if (target != ' ') {
                    if (white ? std::islower(target) : std::isupper(target)) {
                        if ((white && target == 'k') || (!white && target == 'K')) {
                            if (allowKingCapture)
                                pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                        }
                        else {
                            pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                        }
                    }
                }
            }
        }
        if (enPassantTarget != sf::Vector2i(-1, -1)) {
            for (int dc = -1; dc <= 1; dc += 2) {
                int newCol = col + dc;
                if (ChessGame::inBounds(newRow, newCol)) {
                    if (enPassantTarget == sf::Vector2i(newRow, newCol))
                        pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                }
            }
        }
    }
    // Knight moves.
    else if (lowerPiece == 'n') {
        if (!arrKnightAttacksInitialized) {
            initKnightAttacks();
        }
        int sqIndex = row * 8 + col;
        uint64_t moves = arrKnightAttacks[sqIndex];
        while (moves) {
            int targetIndex = countTrailingZeros(moves);
            moves &= moves - 1;
            int newRow = targetIndex / 8;
            int newCol = targetIndex % 8;
            char target = board[newRow * 8 + newCol];
            if (target == ' ')
                pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
            else if (white ? std::islower(target) : std::isupper(target)) {
                if ((white && target == 'k') || (!white && target == 'K')) {
                    if (allowKingCapture)
                        pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                }
                else {
                    pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                }
            }
        }
    }
    // King moves.
    else if (lowerPiece == 'k') {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0)
                    continue;
                int newRow = row + dr, newCol = col + dc;
                if (ChessGame::inBounds(newRow, newCol)) {
                    char target = board[newRow * 8 + newCol];
                    if (target == ' ')
                        pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                    else if (white ? std::islower(target) : std::isupper(target)) {
                        if ((white && target == 'k') || (!white && target == 'K')) {
                            if (allowKingCapture)
                                pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                        }
                        else {
                            pseudoMoves.push_back(sf::Vector2i(newRow, newCol));
                        }
                    }
                }
            }
        }
        // Castling moves.
        if (white && row == 7 && col == 4 && !whiteKingMoved) {
            if (!whiteRookKingsideMoved && board[7 * 8 + 7] == 'R') {
                if (board[7 * 8 + 5] == ' ' && board[7 * 8 + 6] == ' ')
                    pseudoMoves.push_back(sf::Vector2i(7, 6));
            }
            if (!whiteRookQueensideMoved && board[7 * 8 + 0] == 'R') {
                if (board[7 * 8 + 1] == ' ' && board[7 * 8 + 2] == ' ' && board[7 * 8 + 3] == ' ')
                    pseudoMoves.push_back(sf::Vector2i(7, 2));
            }
        }
        if (!white && row == 0 && col == 4 && !blackKingMoved) {
            if (!blackRookKingsideMoved && board[0 * 8 + 7] == 'r') {
                if (board[0 * 8 + 5] == ' ' && board[0 * 8 + 6] == ' ')
                    pseudoMoves.push_back(sf::Vector2i(0, 6));
            }
            if (!blackRookQueensideMoved && board[0 * 8 + 0] == 'r') {
                if (board[0 * 8 + 1] == ' ' && board[0 * 8 + 2] == ' ' && board[0 * 8 + 3] == ' ')
                    pseudoMoves.push_back(sf::Vector2i(0, 2));
            }
        }
    }
    // Linear moves for Rook, Bishop, and Queen.
    else if (lowerPiece == 'r' || lowerPiece == 'b' || lowerPiece == 'q') {
        auto addLinearMoves = [&](int dr, int dc) {
            int r = row + dr, c = col + dc;
            while (ChessGame::inBounds(r, c)) {
                char target = board[r * 8 + c];
                if (target == ' ')
                    pseudoMoves.push_back(sf::Vector2i(r, c));
                else {
                    if (white ? std::islower(target) : std::isupper(target)) {
                        if ((white && target == 'k') || (!white && target == 'K')) {
                            if (allowKingCapture)
                                pseudoMoves.push_back(sf::Vector2i(r, c));
                            break;
                        }
                        pseudoMoves.push_back(sf::Vector2i(r, c));
                    }
                    break;
                }
                r += dr; c += dc;
            }
            };
        if (lowerPiece == 'r' || lowerPiece == 'q') {
            addLinearMoves(1, 0);
            addLinearMoves(-1, 0);
            addLinearMoves(0, 1);
            addLinearMoves(0, -1);
        }
        if (lowerPiece == 'b' || lowerPiece == 'q') {
            addLinearMoves(1, 1);
            addLinearMoves(1, -1);
            addLinearMoves(-1, 1);
            addLinearMoves(-1, -1);
        }
    }

    if (!allowKingCapture) {
        std::vector<sf::Vector2i> legalMoves;
        for (auto move : pseudoMoves) {
            ChessGame newState = simulateMove(sf::Vector2i(row, col), move);
            if (!newState.kingIsInCheck(white))
                legalMoves.push_back(move);
        }
        return legalMoves;
    }
    return pseudoMoves;
}

void ChessGame::applyMove(const sf::Vector2i& from, const sf::Vector2i& to) {
    char piece = board[from.x * 8 + from.y];
    bool white = std::isupper(piece);
    // Handle castling.
    if ((piece == 'K' || piece == 'k') && std::abs(to.y - from.y) == 2) {
        board[to.x * 8 + to.y] = piece;
        board[from.x * 8 + from.y] = ' ';
        if (white) {
            if (to.y > from.y) {
                board[7 * 8 + 7] = ' ';
                board[7 * 8 + 5] = 'R';
                whiteRookKingsideMoved = true;
            }
            else {
                board[7 * 8 + 0] = ' ';
                board[7 * 8 + 3] = 'R';
                whiteRookQueensideMoved = true;
            }
            whiteKingMoved = true;
        }
        else {
            if (to.y > from.y) {
                board[0 * 8 + 7] = ' ';
                board[0 * 8 + 5] = 'r';
                blackRookKingsideMoved = true;
            }
            else {
                board[0 * 8 + 0] = ' ';
                board[0 * 8 + 3] = 'r';
                blackRookQueensideMoved = true;
            }
            blackKingMoved = true;
        }
    }
    else {
        bool isEnPassant = false;
        if ((std::tolower(piece) == 'p') && (from.y != to.y) && board[to.x * 8 + to.y] == ' ')
            isEnPassant = true;
        board[to.x * 8 + to.y] = piece;
        board[from.x * 8 + from.y] = ' ';
        if (std::tolower(piece) == 'p') {
            bool whitePawn = (piece == 'P');
            int direction = whitePawn ? -1 : 1;
            if (std::abs(to.x - from.x) == 2)
                enPassantTarget = sf::Vector2i(from.x + direction, from.y);
            else
                enPassantTarget = sf::Vector2i(-1, -1);
            if (isEnPassant)
                board[from.x * 8 + to.y] = ' ';
        }
        else {
            enPassantTarget = sf::Vector2i(-1, -1);
        }
    }
}
