#include "Board.h"
#include "Square.h"
#include "Piece.h"         // Ensure piece constants (EMPTY, PAWN, etc.) are defined.
#include "Bitboard.h"
#include <sstream>
#include <cctype>
#include <algorithm>
#include <numeric>

// Load starting position FEN.
void Board::loadStartPosition() {
    loadFromFen(startPosition);
}

// Load board position from FEN.
void Board::loadFromFen(std::string fen) {
    normalStart = (fen == startPosition);
    while (!previousInfo.empty()) previousInfo.pop();
    previousPositions.clear();
    moveHistory.clear();
    std::array<std::string, 6> splitFen;
    int i = 0;
    for (char c : fen) {
        if (c == ' ')
            i++;
        else
            splitFen[i] += c;
    }
    std::fill(piecesMB.begin(), piecesMB.end(), EMPTY);
    for (int i = 0; i < 12; i++) {
        piecesBB[i] = 0;
        pieceLists[i] = PieceList();
    }
    takenBB = 0;
    colorBB[0] = 0; colorBB[1] = 0;
    zobrist.reset();

    int x = 0, y = 0;
    for (char c : splitFen[0]) {
        if (c == '/') {
            y++;
            x = 0;
        }
        else if (std::isdigit(c)) {
            x += c - '0';
        }
        else {
            int sq = Square::fromCoords(x, y);
            addPiece(Piece::charToInt(c), sq);
            x++;
        }
    }
    turnColor = (splitFen[1] == "w" ? WHITE : BLACK);
    for (int i = 0; i < 4; i++)
        castlingRights[i] = false;
    for (char c : splitFen[2]) {
        switch (c) {
        case 'K': castlingRights[0] = true; zobrist.changeCastling(0); break;
        case 'Q': castlingRights[1] = true; zobrist.changeCastling(1); break;
        case 'k': castlingRights[2] = true; zobrist.changeCastling(2); break;
        case 'q': castlingRights[3] = true; zobrist.changeCastling(3); break;
        }
    }
    if (splitFen[3] == "-")
        enPassant = -1;
    else {
        enPassant = Square::fromString(splitFen[3]);
        zobrist.changeEnPassant(Square::fileOf(enPassant));
    }
    halfMoveClock = std::stoi(splitFen[4]);
    moveCount = std::stoi(splitFen[5]);
}

std::string Board::getFen() {
    std::string fen = "";
    for (int y = 0; y < 8; y++) {
        int spaces = 0;
        for (int x = 0; x < 8; x++) {
            int sq = Square::fromCoords(x, y);
            if (piecesMB[sq] == EMPTY)
                spaces++;
            else {
                if (spaces > 0) { fen += std::to_string(spaces); spaces = 0; }
                fen += Piece::intToChar(piecesMB[sq]);
            }
        }
        if (spaces > 0) fen += std::to_string(spaces);
        if (y != 7) fen += "/";
    }
    fen += " ";
    fen += (turnColor == WHITE ? "w" : "b");
    fen += " ";
    std::string castlingString = "KQkq";
    bool any = false;
    for (int i = 0; i < 4; i++) {
        if (castlingRights[i]) { fen.push_back(castlingString[i]); any = true; }
    }
    if (!any) fen += "-";
    fen += " ";
    fen += (enPassant == -1 ? "-" : Square::toString(enPassant));
    fen += " ";
    fen += std::to_string(halfMoveClock);
    fen += " ";
    fen += std::to_string(moveCount);
    return fen;
}

void Board::movePiece(int piece, int from, int to) {
    int pieceColor = Piece::colorOf(piece);
    U64 fromBB = U64(1) << from;
    U64 toBB = U64(1) << to;
    U64 fromToBB = fromBB ^ toBB;
    piecesBB[piece] ^= fromToBB;
    takenBB ^= fromBB;
    takenBB |= toBB;
    colorBB[pieceColor] ^= fromToBB;
    piecesMB[from] = EMPTY;
    piecesMB[to] = piece;
    pieceLists[piece].move(from, to);
    zobrist.movePiece(piece, from, to);
}

void Board::addPiece(int piece, int square) {
    int pieceColor = Piece::colorOf(piece);
    U64 BB = U64(1) << square;
    piecesBB[piece] |= BB;
    takenBB |= BB;
    colorBB[pieceColor] |= BB;
    piecesMB[square] = piece;
    pieceLists[piece].add(square);
    zobrist.changePiece(piece, square);
}

void Board::removePiece(int piece, int square) {
    int pieceColor = Piece::colorOf(piece);
    U64 BB = U64(1) << square;
    piecesBB[piece] &= ~BB;
    takenBB &= ~BB;
    colorBB[pieceColor] &= ~BB;
    piecesMB[square] = EMPTY;
    pieceLists[piece].remove(square);
    zobrist.changePiece(piece, square);
}

void Board::makeMove(Move move) {
    if (Move::isNull(move)) {
        PositionalInfo info = { castlingRights, enPassant, halfMoveClock };
        previousInfo.push(info);
        previousPositions.push_back(zobrist.getHashKey());
        if (enPassant != -1)
            zobrist.changeEnPassant(Square::fileOf(enPassant));
        enPassant = -1;
        halfMoveClock++;
        turnColor = !turnColor;
        zobrist.changeTurn();
        if (turnColor == WHITE) moveCount++;
        moveHistory.push_back(move);
        return;
    }
    PositionalInfo info = { castlingRights, enPassant, halfMoveClock };
    previousInfo.push(info);
    previousPositions.push_back(zobrist.getHashKey());
    int pieceType = Piece::typeOf(move.piece);
    int pieceColor = Piece::colorOf(move.piece);
    if ((move.cPiece != EMPTY) && (!move.enPassant))
        removePiece(move.cPiece, move.toSquare());
    movePiece(move.piece, move.fromSquare(), move.toSquare());
    if (move.enPassant) {
        int capturedSquare = enPassant + ((pieceColor == WHITE) ? SOUTH : NORTH);
        removePiece(move.cPiece, capturedSquare);
    }
    if (move.castling) {
        bool queenside = (Square::fileOf(move.toSquare()) == 2);
        int rank = Square::rankOf(move.fromSquare()) * 8;
        int rookFrom = rank + (queenside ? 0 : 7);
        int rookTo = rank + (queenside ? 3 : 5);
        movePiece(piecesMB[rookFrom], rookFrom, rookTo);
    }
    if (move.promotion != EMPTY) {
        removePiece(move.piece, move.toSquare());
        addPiece(move.promotion, move.toSquare());
    }
    if (std::abs(move.toSquare() - move.fromSquare()) == 16 && pieceType == PAWN) {
        if (enPassant != -1)
            zobrist.changeEnPassant(Square::fileOf(enPassant));
        enPassant = (move.toSquare() + move.fromSquare()) / 2;
        zobrist.changeEnPassant(Square::fileOf(enPassant));
    }
    else {
        if (enPassant != -1)
            zobrist.changeEnPassant(Square::fileOf(enPassant));
        enPassant = -1;
    }
    if (Piece::typeOf(move.piece) != PAWN && move.cPiece == EMPTY)
        halfMoveClock++;
    else
        halfMoveClock = 0;
    turnColor = !turnColor;
    zobrist.changeTurn();
    if (turnColor == WHITE) moveCount++;
    moveHistory.push_back(move);
}

void Board::unmakeMove(Move move) {
    if (Move::isNull(move)) {
        PositionalInfo lastInfo = previousInfo.top();
        previousInfo.pop();
        castlingRights = lastInfo.castlingRights;
        enPassant = lastInfo.enPassant;
        halfMoveClock = lastInfo.halfMoveClock;
        turnColor = !turnColor;
        if (turnColor == BLACK) moveCount--;
        zobrist.set(previousPositions.back());
        previousPositions.pop_back();
        moveHistory.pop_back();
        return;
    }
    PositionalInfo lastInfo = previousInfo.top();
    previousInfo.pop();
    castlingRights = lastInfo.castlingRights;
    enPassant = lastInfo.enPassant;
    halfMoveClock = lastInfo.halfMoveClock;
    if (move.promotion != EMPTY) {
        removePiece(move.promotion, move.toSquare());
        addPiece(move.piece, move.toSquare());
    }
    movePiece(move.piece, move.toSquare(), move.fromSquare());
    if ((move.cPiece != EMPTY) && (!move.enPassant))
        addPiece(move.cPiece, move.toSquare());
    if (move.enPassant) {
        int capturedSquare = enPassant + ((Piece::colorOf(move.piece) == WHITE) ? SOUTH : NORTH);
        addPiece(move.cPiece, capturedSquare);
    }
    if (move.castling) {
        bool queenside = (Square::fileOf(move.toSquare()) == 2);
        int rank = Square::rankOf(move.fromSquare()) * 8;
        int rookFrom = rank + (queenside ? 0 : 7);
        int rookTo = rank + (queenside ? 3 : 5);
        movePiece(piecesMB[rookTo], rookTo, rookFrom);
    }
    turnColor = !turnColor;
    if (turnColor == BLACK) moveCount--;
    zobrist.set(previousPositions.back());
    previousPositions.pop_back();
    moveHistory.pop_back();
}

void Board::generateMoves(bool onlyCaptures) {
    moveList.clear();
    std::vector<Move> pseudoLegalMoves;  // Use std::vector instead of undefined MoveList

    // Call your (stubbed) pseudo-legal move generator.
    // (Make sure you have a function defined in Bitboard or elsewhere that fills this vector.)
    BB::generatePseudoLegalMoves(pseudoLegalMoves, *this, onlyCaptures);



    std::cout << "Generated moves: ";
    for (const Move& move : pseudoLegalMoves) {
        std::cout << move.getNotation() << " ";
    }
    std::cout << std::endl;

    // Filter out illegal moves (moves that leave the king in check)
    for (const Move& move : pseudoLegalMoves) {
        makeMove(move);
        if (!isKingInCheck(turnColor))
            moveList.push_back(move);
        else
            std::cout << "Filtered out illegal move: " << move.getNotation() << std::endl;
        unmakeMove(move);
    }
}

// Add this helper function to check if a king is in check.
bool Board::isKingInCheck(bool white) {
    int kingSquare = getKingSquare(white);
    if (kingSquare == -1) return false; // Should never happen.
    return isSquareAttacked(kingSquare, !white);
}

bool Board::checkDraw() {
    if (halfMoveClock >= 100)
        return true;
    if (std::count(previousPositions.begin(), previousPositions.end(), zobrist.getHashKey()) >= 2)
        return true;
    return false;
}

int Board::getState() {
    if (moveList.size() == 0) {
        if (isCheck)
            return (turnColor == WHITE ? BLACK_WIN : WHITE_WIN);
        else
            return DRAW;
    }
    if (checkDraw())
        return DRAW;
    return PLAY;
}

bool Board::checkRepetition() {
    return std::count(previousPositions.begin(), previousPositions.end(), zobrist.getHashKey()) >= 1;
}

int Board::getTurnColor() const { return turnColor; }

bool Board::getCheck() { return isCheck; }
int Board::getHalfMoveClock() { return halfMoveClock; }
int Board::getMoveCount() { return moveCount; }
std::array<U64, 12> Board::getPiecesBB() { return piecesBB; }
std::array<int, 64> Board::getPiecesMB() { return piecesMB; }
std::array<PieceList, 12> Board::getPieceLists() { return pieceLists; }
U64 Board::getZobristKey() { return zobrist.getHashKey(); }
std::vector<Move> Board::getMoveList() { return moveList; }
std::vector<Move> Board::getMoveHistory() { return moveHistory; }
bool Board::getNormalStart() { return normalStart; }

int Board::getKingSquare(bool white) {
    U64 kingBB = white ? piecesBB[WK] : piecesBB[BK];
    if (kingBB == 0) return -1;
    return BB::bitScanForward(kingBB);
}

bool Board::isSquareAttacked(int square, bool byWhite) {
    // Minimal stub: a full implementation would check pawn, knight, king, and sliding attacks.
    return false;
}

bool Board::isKingSafeForSide(bool white) {
    int kingSquare = getKingSquare(white);
    if (kingSquare == -1) return false;
    return !isSquareAttacked(kingSquare, !white);
}
