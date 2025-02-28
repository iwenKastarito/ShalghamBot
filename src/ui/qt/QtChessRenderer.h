#ifndef QT_CHESS_RENDERER_H
#define QT_CHESS_RENDERER_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QMap>
#include <QFont>
#include <QGraphicsTextItem>
#include "../../core/ChessGame.h"

// The QtChessRenderer class is responsible for drawing the chess board and pieces using Qt
class QtChessRenderer : public QGraphicsScene {
    Q_OBJECT

public:
    // Constructor requires the square size (in pixels) and a reference to the game state
    QtChessRenderer(int squareSize, ChessGame& game, QObject* parent = nullptr);

    // Loads the piece textures
    void loadTextures();

    // Updates the scene with the current board state
    void updateScene(
        const sf::Vector2i& selectedSquare,
        const std::vector<sf::Vector2i>& legalMoves,
        bool promotionPending,
        const sf::Vector2i& promotionSquare,
        const QString& gameMessage = "");

private:
    int squareSize;
    ChessGame& game;
    QMap<char, QPixmap> pieceTextures;
    QFont messageFont;
    QGraphicsTextItem* messageItem;

    // Clear and rebuild the scene
    void rebuildScene();

    // Helper to convert chess coordinates to Qt scene coordinates
    QPointF chessToScene(int row, int col) const;
};

#endif // QT_CHESS_RENDERER_H 