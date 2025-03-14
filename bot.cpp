#include "bot.h"
#include "ChessGame.h"
#include <random>
#include <cstdint>
#include <cctype>
#include <chrono>
#include <iostream>

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

// Helper function to apply a bot move, including handling castling, en passant, and promotion.
static void applyBotMove(ChessGame& game, const std::pair<sf::Vector2i, sf::Vector2i>& move, ChessBot* bot) {
    int fromX = move.first.x;
    int fromY = move.first.y;
    int toX = move.second.x;
    int toY = move.second.y;
    char movingPiece = game.board[fromX * 8 + fromY];

    // Handle castling.
    if (std::tolower(movingPiece) == 'k' && std::abs(toY - fromY) == 2) {
        game.board[toX * 8 + toY] = movingPiece;
        game.board[fromX * 8 + fromY] = ' ';
        if (std::isupper(movingPiece)) {
            if (toY == 6) {
                game.board[7 * 8 + 7] = ' ';
                game.board[7 * 8 + 5] = 'R';
            }
            else if (toY == 2) {
                game.board[7 * 8 + 0] = ' ';
                game.board[7 * 8 + 3] = 'R';
            }
        }
        else {
            if (toY == 6) {
                game.board[0 * 8 + 7] = ' ';
                game.board[0 * 8 + 5] = 'r';
            }
            else if (toY == 2) {
                game.board[0 * 8 + 0] = ' ';
                game.board[0 * 8 + 3] = 'r';
            }
        }
        game.halfMoveClock++;
    }
    else {
        bool isEnPassant = false;
        bool isCapture = false;
        if (std::tolower(movingPiece) == 'p') {
            if (fromY != toY && game.board[toX * 8 + toY] == ' ') {
                isEnPassant = true;
                isCapture = true;
            }
            if (game.board[toX * 8 + toY] != ' ')
                isCapture = true;
            game.board[toX * 8 + toY] = movingPiece;
            game.board[fromX * 8 + fromY] = ' ';
            bool whitePawn = (movingPiece == 'P');
            int direction = whitePawn ? -1 : 1;
            if (std::abs(toX - fromX) == 2)
                game.enPassantTarget = sf::Vector2i(fromX + direction, fromY);
            else
                game.enPassantTarget = sf::Vector2i(-1, -1);
            if (isEnPassant)
                game.board[fromX * 8 + toY] = ' ';
            if ((whitePawn && toX == 0) || (!whitePawn && toX == 7)) {
                char promotionPiece = bot->choosePromotionPiece(game, movingPiece);
                game.board[toX * 8 + toY] = promotionPiece;
            }
        }
        else {
            if (game.board[toX * 8 + toY] != ' ')
                isCapture = true;
            game.board[toX * 8 + toY] = movingPiece;
            game.board[fromX * 8 + fromY] = ' ';
            game.enPassantTarget = sf::Vector2i(-1, -1);
        }
        if (std::tolower(movingPiece) == 'p' || isCapture)
            game.halfMoveClock = 0;
        else
            game.halfMoveClock++;
    }
}

void ChessBot::makeMove(ChessGame& game) {
    auto start = std::chrono::high_resolution_clock::now();
    auto move = chooseMove(game);
    if (move.first.x == -1 || move.second.x == -1)
        return;
    applyBotMove(game, move, this);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Bot move time: " << duration.count() << " microseconds\n";
}

char ChessBot::choosePromotionPiece(const ChessGame& game, char pawn) {
    return (std::isupper(pawn) ? 'Q' : 'q');
}

std::pair<sf::Vector2i, sf::Vector2i> RandomBot::chooseMove(const ChessGame& game) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> legalMoves;

    uint64_t piecesBB = 0ULL;
    for (int i = 0; i < 64; i++) {
        int row = i / 8;
        int col = i % 8;
        char piece = game.board[i];
        if (piece != ' ' && (std::isupper(piece) == botIsWhite))
            piecesBB |= (1ULL << i);
    }

    while (piecesBB) {
        int index = countTrailingZeros(piecesBB);
        piecesBB &= piecesBB - 1;
        int row = index / 8;
        int col = index % 8;
        auto moves = game.getLegalMoves(row, col, false);
        for (auto move : moves)
            legalMoves.push_back({ sf::Vector2i(row, col), move });
    }

    if (legalMoves.empty()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "RandomBot move time: " << duration.count() << " microseconds\n";
        return { sf::Vector2i(-1, -1), sf::Vector2i(-1, -1) };
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, legalMoves.size() - 1);
    int index = dis(gen);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "RandomBot move time: " << duration.count() << " microseconds\n";
    return legalMoves[index];
}
