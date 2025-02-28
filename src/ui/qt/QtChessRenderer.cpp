#include "QtChessRenderer.h"
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <QPainter>

QtChessRenderer::QtChessRenderer(int squareSize, ChessGame& game, QObject* parent)
    : QGraphicsScene(parent), squareSize(squareSize), game(game)
{
    setSceneRect(0, 0, squareSize * 8, squareSize * 8);
    messageFont = QFont("Arial", 16);
    messageFont.setBold(true);
    
    messageItem = addText("", messageFont);
    messageItem->setDefaultTextColor(Qt::red);
    messageItem->setPos(10, 10);
    messageItem->setZValue(10); // Keep above other items
    
    loadTextures();
    rebuildScene();
}

void QtChessRenderer::loadTextures()
{
    QMap<char, QString> pieceFiles = {
        {'K', "resources/Pices/white_king.png"},
        {'Q', "resources/Pices/white_queen.png"},
        {'R', "resources/Pices/white_rook.png"},
        {'B', "resources/Pices/white_bishop.png"},
        {'N', "resources/Pices/white_knight.png"},
        {'P', "resources/Pices/white_pawn.png"},
        {'k', "resources/Pices/black_king.png"},
        {'q', "resources/Pices/black_queen.png"},
        {'r', "resources/Pices/black_rook.png"},
        {'b', "resources/Pices/black_bishop.png"},
        {'n', "resources/Pices/black_knight.png"},
        {'p', "resources/Pices/black_pawn.png"}
    };
    
    // Loop through each piece type
    for (auto it = pieceFiles.begin(); it != pieceFiles.end(); ++it) {
        QPixmap pixmap;
        
        // Attempt to load the texture
        if (!pixmap.load(it.value())) {
            qWarning() << "Error loading texture:" << it.value();
            
            // Create a fallback colored square instead of crashing
            pixmap = QPixmap(squareSize, squareSize);
            pixmap.fill(isupper(it.key()) ? Qt::white : Qt::black);
            
            // Draw the piece letter on it
            QPainter painter(&pixmap);
            painter.setPen(isupper(it.key()) ? Qt::black : Qt::white);
            painter.setFont(QFont("Arial", squareSize/2, QFont::Bold));
            painter.drawText(pixmap.rect(), Qt::AlignCenter, QString(it.key()));
        }
        
        pieceTextures[it.key()] = pixmap.scaled(squareSize, squareSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

void QtChessRenderer::updateScene(
    const sf::Vector2i& selectedSquare,
    const std::vector<sf::Vector2i>& legalMoves,
    bool promotionPending,
    const sf::Vector2i& promotionSquare,
    const QString& gameMessage)
{
    // Update message
    messageItem->setPlainText(gameMessage);
    
    // Rebuild the scene
    rebuildScene();
    
    // Draw selected square
    if (selectedSquare.x != -1) {
        QGraphicsRectItem* highlight = new QGraphicsRectItem(
            selectedSquare.y * squareSize, 
            selectedSquare.x * squareSize, 
            squareSize, 
            squareSize
        );
        highlight->setBrush(QBrush(QColor(255, 255, 0, 150)));
        highlight->setPen(Qt::NoPen);
        highlight->setZValue(1); // Above board, below pieces
        addItem(highlight);
    }
    
    // Draw legal move indicators
    for (const auto& move : legalMoves) {
        QColor hintColor;
        
        if (selectedSquare.x != -1) {
            char selPiece = game.board[selectedSquare.x][selectedSquare.y];
            
            // For a king making a two-square move, mark castling with blue
            if (std::tolower(selPiece) == 'k' && std::abs(move.y - selectedSquare.y) == 2) {
                hintColor = QColor(0, 0, 255, 150);
            }
            // Capture or en passant
            else if ((std::tolower(selPiece) == 'p' && game.enPassantTarget == move) ||
                    game.board[move.x][move.y] != ' ') {
                hintColor = QColor(255, 0, 0, 150);
            }
            else {
                hintColor = QColor(0, 255, 0, 150);
            }
        }
        else {
            hintColor = QColor(0, 255, 0, 150);
        }
        
        QGraphicsEllipseItem* hint = new QGraphicsEllipseItem(
            move.y * squareSize + squareSize/3,
            move.x * squareSize + squareSize/3,
            squareSize/3, 
            squareSize/3
        );
        hint->setBrush(QBrush(hintColor));
        hint->setPen(Qt::NoPen);
        hint->setZValue(2); // Above board and selection, below pieces
        addItem(hint);
    }
    
    // Handle promotion pending
    if (promotionPending) {
        // Draw a semi-transparent overlay
        QGraphicsRectItem* overlay = new QGraphicsRectItem(0, 0, squareSize * 8, squareSize * 8);
        overlay->setBrush(QBrush(QColor(0, 0, 0, 180)));
        overlay->setPen(Qt::NoPen);
        overlay->setZValue(5);
        addItem(overlay);
        
        // Draw promotion options
        float menuWidth = squareSize * 4;
        float menuHeight = squareSize;
        float startX = (8 * squareSize - menuWidth) / 2;
        float startY = (8 * squareSize - menuHeight) / 2;
        
        QGraphicsRectItem* menu = new QGraphicsRectItem(startX, startY, menuWidth, menuHeight);
        menu->setBrush(QBrush(QColor(220, 220, 220)));
        menu->setPen(QPen(Qt::black));
        menu->setZValue(6);
        addItem(menu);
        
        char currentPawn = game.board[promotionSquare.x][promotionSquare.y];
        std::vector<char> options;
        if (currentPawn == 'P')
            options = { 'N', 'R', 'B', 'Q' };
        else if (currentPawn == 'p')
            options = { 'n', 'r', 'b', 'q' };
            
        for (size_t i = 0; i < options.size(); i++) {
            QGraphicsPixmapItem* piece = new QGraphicsPixmapItem(pieceTextures[options[i]]);
            piece->setPos(startX + i * squareSize, startY);
            piece->setZValue(7);
            addItem(piece);
        }
    }
}

void QtChessRenderer::rebuildScene()
{
    // Clear existing items but keep the message
    QGraphicsTextItem* savedMessage = messageItem;
    clear();
    messageItem = savedMessage;
    addItem(messageItem);
    
    // Draw board squares
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            QGraphicsRectItem* square = new QGraphicsRectItem(col * squareSize, row * squareSize, squareSize, squareSize);
            
            if ((row + col) % 2 == 0)
                square->setBrush(QBrush(QColor(240, 217, 181))); // Light squares
            else
                square->setBrush(QBrush(QColor(181, 136, 99))); // Dark squares
                
            square->setPen(Qt::NoPen);
            addItem(square);
        }
    }
    
    // Draw pieces
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = game.board[row][col];
            if (piece != ' ') {
                QGraphicsPixmapItem* pieceItem = new QGraphicsPixmapItem(pieceTextures[piece]);
                pieceItem->setPos(col * squareSize, row * squareSize);
                pieceItem->setZValue(3); // Above board, selection, and hints
                addItem(pieceItem);
            }
        }
    }
}

QPointF QtChessRenderer::chessToScene(int row, int col) const
{
    return QPointF(col * squareSize, row * squareSize);
} 