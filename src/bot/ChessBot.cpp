#include "ChessBot.h"
#include <iostream>

ChessBot::ChessBot(ChessGame& game) : game(game) {
}

void ChessBot::setSearchDepth(int depth) {
    if (depth > 0) {
        searchDepth = depth;
    }
}

sf::Vector2i ChessBot::getBestMove(bool white) {
    // Get all possible moves for current player
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> possibleMoves = generateSortedMoves(game, white);
    
    if (possibleMoves.empty()) {
        return sf::Vector2i(-1, -1); // No legal moves
    }
    
    int bestScore = white ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    std::pair<sf::Vector2i, sf::Vector2i> bestMove = possibleMoves[0]; // Default to first move
    
    // For each possible move
    for (const auto& move : possibleMoves) {
        // Create a copy of the game state and make the move
        ChessGame tempGame = game.simulateMove(move.first, move.second);
        
        // Run minimax on this new position
        int score = minimax(tempGame, searchDepth - 1, 
                           std::numeric_limits<int>::min(), 
                           std::numeric_limits<int>::max(), 
                           !white);
        
        // Update best move if needed
        if ((white && score > bestScore) || (!white && score < bestScore)) {
            bestScore = score;
            bestMove = move;
        }
    }
    
    // Return the origin of the best move (the square with the piece to move)
    return bestMove.first;
}

int ChessBot::minimax(ChessGame& state, int depth, int alpha, int beta, bool maximizingPlayer) {
    // Base case: we've reached the maximum depth or the game is over
    if (depth == 0 || !state.hasLegalMoves(maximizingPlayer)) {
        return evaluatePosition(state);
    }
    
    // Generate all possible moves for current player
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> possibleMoves = 
        generateSortedMoves(state, maximizingPlayer);
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : possibleMoves) {
            ChessGame tempState = state.simulateMove(move.first, move.second);
            int eval = minimax(tempState, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break; // Beta cutoff
            }
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : possibleMoves) {
            ChessGame tempState = state.simulateMove(move.first, move.second);
            int eval = minimax(tempState, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
        }
        return minEval;
    }
}

int ChessBot::evaluatePosition(const ChessGame& state) {
    // Basic evaluation: material count
    int score = 0;
    
    // Count material
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];
            if (piece != ' ') {
                int pieceValue = getPieceValue(piece);
                score += pieceValue;
            }
        }
    }
    
    // Additional factors could be added here:
    // - Piece position (using piece-square tables)
    // - King safety
    // - Pawn structure
    // - Mobility
    // - Control of center
    
    return score;
}

int ChessBot::getPieceValue(char piece) {
    // Positive values for white pieces, negative for black
    switch (std::toupper(piece)) {
        case 'P': return std::isupper(piece) ? 100 : -100;   // Pawn
        case 'N': return std::isupper(piece) ? 320 : -320;   // Knight
        case 'B': return std::isupper(piece) ? 330 : -330;   // Bishop
        case 'R': return std::isupper(piece) ? 500 : -500;   // Rook
        case 'Q': return std::isupper(piece) ? 900 : -900;   // Queen
        case 'K': return std::isupper(piece) ? 20000 : -20000; // King (very high value)
        default: return 0;
    }
}

std::vector<std::pair<sf::Vector2i, sf::Vector2i>> ChessBot::generateSortedMoves(ChessGame& state, bool white) {
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> allMoves;
    
    // For each square on the board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];
            
            // If the piece belongs to the current player
            if (piece != ' ' && (std::isupper(piece) == white)) {
                sf::Vector2i from(row, col);
                
                // Get all legal moves for this piece
                std::vector<sf::Vector2i> legalMoves = state.getLegalMoves(row, col, false);
                
                // Filter moves that would leave the king in check
                for (const auto& to : legalMoves) {
                    ChessGame tempState = state.simulateMove(from, to);
                    if (!tempState.kingIsInCheck(white)) {
                        allMoves.push_back(std::make_pair(from, to));
                    }
                }
            }
        }
    }
    
    // Sort moves for better alpha-beta pruning efficiency
    // This is a simple sorting by captured piece value
    std::sort(allMoves.begin(), allMoves.end(), 
        [this, &state](const auto& move1, const auto& move2) {
            char capturedPiece1 = state.board[move1.second.x][move1.second.y];
            char capturedPiece2 = state.board[move2.second.x][move2.second.y];
            
            // Higher value captures first
            return std::abs(getPieceValue(capturedPiece1)) > 
                   std::abs(getPieceValue(capturedPiece2));
        });
    
    return allMoves;
} 