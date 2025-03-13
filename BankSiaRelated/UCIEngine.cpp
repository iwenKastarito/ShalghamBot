#include "UCIEngine.h"
#include "Move.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <cstdlib>

UCIEngine::UCIEngine()
    : tt(board), evaluation(board, tt),
    defaultTimeLimit(5.0), moveOverhead(0.05), maxDepth(-1)
{
    board.loadStartPosition();
}

void UCIEngine::loop() {
    std::string line;
    while (getline(std::cin, line)) {
        processCommand(line);
    }
}

void UCIEngine::processCommand(const std::string& line) {
    if (line == "uci") {
        std::cout << "id name MyChessBot" << std::endl;
        std::cout << "id author YourName" << std::endl;
        std::cout << "option name Depth type spin default 4 min 1 max 64" << std::endl;
        std::cout << "option name DefaultTimeLimit type spin default 5 min 1 max 60" << std::endl;
        std::cout << "option name MoveOverhead type spin default 50 min 0 max 500" << std::endl;
        std::cout << "uciok" << std::endl;
    }
    else if (line == "isready") {
        std::cout << "readyok" << std::endl;
    }
    else if (line.substr(0, 9) == "setoption") {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // setoption
        std::string optionName, optionValue;
        while (iss >> token) {
            if (token == "name") {
                std::string word;
                optionName = "";
                while (iss >> word) {
                    if (word == "value")
                        break;
                    if (!optionName.empty())
                        optionName += " ";
                    optionName += word;
                }
                std::getline(iss, optionValue);
                break;
            }
        }
        size_t start = optionValue.find_first_not_of(" ");
        if (start != std::string::npos)
            optionValue = optionValue.substr(start);
        if (optionName == "Depth") {
            try { maxDepth = std::stoi(optionValue); }
            catch (...) {}
        }
        else if (optionName == "DefaultTimeLimit") {
            try { defaultTimeLimit = std::stod(optionValue); }
            catch (...) {}
        }
        else if (optionName == "MoveOverhead") {
            try { moveOverhead = std::stod(optionValue) / 1000.0; }
            catch (...) {}
        }
    }
    else if (line.substr(0, 8) == "ucinewgame") {
        board.loadStartPosition();
    }
    else if (line.substr(0, 8) == "position") {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // "position"
        iss >> token; // "startpos" or "fen"
        if (token == "startpos") {
            board.loadStartPosition();
            std::string token2;
            if (iss >> token2 && token2 == "moves") {
                while (iss >> token2) {
                    int from_r, from_c, to_r, to_c;
                    squareToIndex(token2.substr(0, 2), from_r, from_c);
                    squareToIndex(token2.substr(2, 2), to_r, to_c);
                    int promotion = EMPTY;
                    if (token2.size() > 4)
                        promotion = Piece::charToInt(token2[4]);
                    Move m{ from_r, from_c, to_r, to_c, board.getPiecesMB()[from_r * 8 + from_c], EMPTY, promotion, false, false, 0 };
                    board.makeMove(m);
                }
            }
        }
        else if (token == "fen") {
            std::string fenPart, fullFEN;
            int count = 0;
            while (count < 6 && iss >> fenPart) {
                fullFEN += fenPart + " ";
                count++;
            }
            board.loadFromFen(fullFEN);
            std::istringstream fenStream(fullFEN);
            std::string dummy, turn;
            fenStream >> dummy >> turn;
            std::string token2;
            if (iss >> token2 && token2 == "moves") {
                while (iss >> token2) {
                    int from_r, from_c, to_r, to_c;
                    squareToIndex(token2.substr(0, 2), from_r, from_c);
                    squareToIndex(token2.substr(2, 2), to_r, to_c);
                    int promotion = EMPTY;
                    if (token2.size() > 4)
                        promotion = Piece::charToInt(token2[4]);
                    Move m{ from_r, from_c, to_r, to_c, board.getPiecesMB()[from_r * 8 + from_c], EMPTY, promotion, false, false, 0 };
                    board.makeMove(m);
                }
            }
        }
    }
    else if (line.substr(0, 2) == "go") {
        int depthLimit = 4;
        if (maxDepth > 0)
            depthLimit = maxDepth;
        Move best = getBestMove(-1, -1, depthLimit, -1);
        board.makeMove(best);
        std::cout << "bestmove " << board.moveToUCI(best) << std::endl;
    }
    else if (line == "quit") {
        exit(0);
    }
}

Move UCIEngine::getBestMove(int timeLeft, int increment, int depthLimit, int exactTime) {
    if (depthLimit != -1)
        timeLimit = 1000000;
    else if (timeLeft != -1)
        timeLimit = ((timeLeft / 40.0) + increment) / 1000.0 - moveOverhead;
    else if (exactTime != -1)
        timeLimit = exactTime / 1000.0;
    else
        timeLimit = defaultTimeLimit;
    bestMove = Move::nullmove();
    bestEval = LOWEST_SCORE;
    nodes = 0;
    searchAborted = false;
    searchStart = std::chrono::system_clock::now();
    int depth;
    for (depth = 1; !searchAborted; depth++) {
        int score = search(LOWEST_SCORE, HIGHEST_SCORE, depth, 0, false);
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - searchStart);
        std::cout << std::fixed << "info score " << score << " depth " << depth
            << " nodes " << nodes << " time " << diff.count()
            << " pv " << getPrincipalVariation(depth) << "\n";
        if (depth == depthLimit)
            break;
    }
    if (bestMove.from_r == -1) {
        board.generateMoves(false);
        std::vector<Move> moves = board.getMoveList();
        if (!moves.empty())
            bestMove = moves[0];
    }
    return bestMove;
}

int UCIEngine::search(int alpha, int beta, int depth, int plyFromRoot, bool nullMove) {
    auto diff = std::chrono::duration<double>(std::chrono::system_clock::now() - searchStart);
    if (diff.count() >= timeLimit) {
        searchAborted = true;
        return alpha;
    }
    nodes++;
    if (depth == 0)
        return quiescenceSearch(alpha, beta);
    board.generateMoves(false);
    std::vector<Move> moves = board.getMoveList();
    if (moves.empty()) {
        return board.getTurnColor() == WHITE ? LOWEST_SCORE + plyFromRoot : LOWEST_SCORE + plyFromRoot;
    }
    for (auto& move : moves) {
        board.makeMove(move);
        int score = -search(-beta, -alpha, depth - 1, plyFromRoot + 1, false);
        board.unmakeMove(move);
        if (searchAborted)
            return alpha;
        if (score >= beta)
            return beta;
        if (score > alpha) {
            alpha = score;
            if (plyFromRoot == 0) {
                bestMove = move;
                bestEval = score;
            }
        }
    }
    return alpha;
}

int UCIEngine::quiescenceSearch(int alpha, int beta) {
    auto diff = std::chrono::duration<double>(std::chrono::system_clock::now() - searchStart);
    if (diff.count() >= timeLimit) {
        searchAborted = true;
        return alpha;
    }
    nodes++;
    int standPat = evaluate();
    if (standPat >= beta)
        return beta;
    if (standPat > alpha)
        alpha = standPat;
    board.generateMoves(true);
    std::vector<Move> moves = board.getMoveList();
    for (auto& move : moves) {
        board.makeMove(move);
        int score = -quiescenceSearch(-beta, -alpha);
        board.unmakeMove(move);
        if (searchAborted)
            return alpha;
        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }
    return alpha;
}

int UCIEngine::evaluate() {
    return evaluation.evaluate();
}

std::string UCIEngine::getPrincipalVariation(int depth) {
    return board.moveToUCI(bestMove);
}
