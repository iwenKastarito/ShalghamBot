#include "Minimax.h"
#include <cctype>
#include <sstream>
#include <limits>
#include <vector>
#include <algorithm>

// -------------------------
// MinimaxAlgorithm Methods
// -------------------------

MinimaxAlgorithm::MinimaxAlgorithm(int depth)
    : searchDepth(depth), nodesEvaluated(0) {
    transpositionTable.clear();
}

int MinimaxAlgorithm::countMajorPieces(const ChessGame& game) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = game.board[i][j];
            char p = std::tolower(piece);
            if (p == 'q' || p == 'r' || p == 'b' || p == 'n')
                count++;
        }
    }
    return count;
}

GamePhase MinimaxAlgorithm::getGamePhase(const ChessGame& game) {
    if (game.moveNumber / 2 <= 3)
        return OPENING;
    if (countMajorPieces(game) <= 2)
        return ENDING;
    return MIDDLE;
}

std::string MinimaxAlgorithm::boardToString(const ChessGame& game) {
    std::ostringstream oss;
    for (auto& row : game.board)
        oss << row;
    oss << game.moveNumber;
    return oss.str();
}

int MinimaxAlgorithm::evaluate(const ChessGame& game, bool botIsWhite) {
    // Piece ratings: Pawn=100, Knight=320, Bishop=330, Rook=500, Queen=900, King=20000.
    std::unordered_map<char, int> rating = {
        {'p', 100}, {'n', 320}, {'b', 330}, {'r', 500}, {'q', 900}, {'k', 20000}
    };

    int materialScore = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = game.board[i][j];
            if (piece == ' ')
                continue;
            int value = rating[std::tolower(piece)];
            if (std::isupper(piece))
                materialScore += value;
            else
                materialScore -= value;
        }
    }

    int bonus = 0;
    // Castling bonus.
    if (game.board[7][6] == 'K' || game.board[7][2] == 'K')
        bonus += 50;
    if (game.board[0][6] == 'k' || game.board[0][2] == 'k')
        bonus -= 50;

    // Promotion bonus.
    for (int j = 0; j < 8; j++) {
        if (game.board[0][j] == 'P')
            bonus += 200;
        if (game.board[7][j] == 'p')
            bonus -= 200;
    }

    // Central control bonus.
    std::vector<std::pair<int, int>> center = { {3,3}, {3,4}, {4,3}, {4,4} };
    for (auto& sq : center) {
        char piece = game.board[sq.first][sq.second];
        if (piece != ' ') {
            if (std::isupper(piece))
                bonus += 10;
            else
                bonus -= 10;
        }
    }

    // King safety penalty.
    const int checkPenalty = 5000;
    if (botIsWhite) {
        if (game.kingIsInCheck(true))
            bonus -= checkPenalty;
        if (!game.kingIsInCheck(false))
            bonus += 200;
    }
    else {
        if (game.kingIsInCheck(false))
            bonus -= checkPenalty;
        if (!game.kingIsInCheck(true))
            bonus -= 200;
    }

    return materialScore + bonus;
}

int MinimaxAlgorithm::minimax(ChessGame game, int depth, int alpha, int beta, bool whiteTurn, bool botIsWhite) {
    nodesEvaluated++;
    std::string key = boardToString(game) + (whiteTurn ? "W" : "B") + std::to_string(depth);
    if (transpositionTable.find(key) != transpositionTable.end())
        return transpositionTable[key];

    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> moves;
    // Generate legal moves.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = game.board[i][j];
            if (piece == ' ')
                continue;
            bool isWhite = std::isupper(piece);
            if (isWhite != whiteTurn)
                continue;
            auto pseudo = game.getLegalMoves(i, j, false);
            for (auto move : pseudo) {
                ChessGame simState = game.simulateMove(sf::Vector2i(i, j), move);
                if (!simState.kingIsInCheck(whiteTurn))
                    moves.push_back({ sf::Vector2i(i, j), move });
            }
        }
    }

    if (depth == 0 || moves.empty()) {
        if (moves.empty()) {
            if (game.kingIsInCheck(whiteTurn))
                return whiteTurn ? -1000 : 1000;
            return 0;
        }
        int eval = evaluate(game, botIsWhite);
        transpositionTable[key] = eval;
        return eval;
    }

    int bestVal = (whiteTurn == botIsWhite) ? -std::numeric_limits<int>::max() : std::numeric_limits<int>::max();
    bool maximizing = (whiteTurn == botIsWhite);
    for (auto m : moves) {
        ChessGame newGame = game.simulateMove(m.first, m.second);
        int eval = minimax(newGame, depth - 1, alpha, beta, !whiteTurn, botIsWhite);
        if (maximizing) {
            bestVal = std::max(bestVal, eval);
            alpha = std::max(alpha, eval);
        }
        else {
            bestVal = std::min(bestVal, eval);
            beta = std::min(beta, eval);
        }
        if (beta <= alpha)
            break;
    }
    transpositionTable[key] = bestVal;
    return bestVal;
}

// -------------------------
// MinimaxBot Methods
// -------------------------

MinimaxBot::MinimaxBot(bool isWhite, int depth)
    : ChessBot(isWhite), minimaxAlgo(depth) {
}

std::pair<sf::Vector2i, sf::Vector2i> MinimaxBot::chooseMove(const ChessGame& game) {
    minimaxAlgo.nodesEvaluated = 0;
    std::vector<CandidateMove> candidates;

    // Gather candidate moves.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = game.board[i][j];
            if (piece == ' ')
                continue;
            bool isWhite = std::isupper(piece);
            if (isWhite != this->botIsWhite)
                continue;
            std::vector<sf::Vector2i> pseudo = game.getLegalMoves(i, j, false);
            for (auto move : pseudo) {
                CandidateMove cand;
                cand.from = sf::Vector2i(i, j);
                cand.to = move;
                candidates.push_back(cand);
            }
        }
    }

    int numCandidates = candidates.size();
    if (numCandidates == 0)
        return { sf::Vector2i(-1, -1), sf::Vector2i(-1, -1) };

    std::vector<int> evaluations(numCandidates);
    for (int i = 0; i < numCandidates; i++) {
        evaluations[i] = minimaxAlgo.minimax(
            game.simulateMove(candidates[i].from, candidates[i].to),
            minimaxAlgo.searchDepth - 1,
            -100000, 100000,
            !this->botIsWhite,
            this->botIsWhite
        );
    }

    int bestEval = this->botIsWhite ? -std::numeric_limits<int>::max() : std::numeric_limits<int>::max();
    int bestIndex = 0;
    for (int i = 0; i < numCandidates; i++) {
        if (this->botIsWhite && evaluations[i] > bestEval) {
            bestEval = evaluations[i];
            bestIndex = i;
        }
        else if (!this->botIsWhite && evaluations[i] < bestEval) {
            bestEval = evaluations[i];
            bestIndex = i;
        }
    }

    return { candidates[bestIndex].from, candidates[bestIndex].to };
}
