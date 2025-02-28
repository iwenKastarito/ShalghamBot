#include <QApplication>
#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>
#include <QDir>
#include <QInputDialog>
#include "core/ChessGame.h"
#include "ui/qt/QtChessRenderer.h"

class ChessWindow : public QMainWindow {
    Q_OBJECT
    
public:
    ChessWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        const int squareSize = 80;
        const int boardSize = 8;
        
        setWindowTitle("Qt Chess Game");
        resize(squareSize * boardSize, squareSize * boardSize);
        
        game = new ChessGame();
        renderer = new QtChessRenderer(squareSize, *game);
        
        view = new QGraphicsView(renderer);
        view->setRenderHint(QPainter::Antialiasing);
        view->setRenderHint(QPainter::SmoothPixmapTransform);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setFixedSize(squareSize * boardSize, squareSize * boardSize);
        
        setCentralWidget(view);
        
        // Game state
        gameOver = false;
        gameResult = "";
        selectedSquare = sf::Vector2i(-1, -1);
        promotionPending = false;
        promotionSquare = sf::Vector2i(-1, -1);
        
        // Connect signals/slots
        view->installEventFilter(this);
        
        // Initial render
        updateRenderer();
    }
    
    ~ChessWindow() {
        delete game;
    }
    
protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj == view && event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            handleMouseClick(mouseEvent->pos());
            return true;
        }
        return QMainWindow::eventFilter(obj, event);
    }
    
private:
    ChessGame* game;
    QtChessRenderer* renderer;
    QGraphicsView* view;
    
    // Game state
    bool gameOver;
    QString gameResult;
    sf::Vector2i selectedSquare;
    std::vector<sf::Vector2i> legalMoves;
    bool promotionPending;
    sf::Vector2i promotionSquare;
    
    // Check if a move is a pawn promotion
    bool isPawnPromotion(const sf::Vector2i& from, const sf::Vector2i& to) {
        // Check if the piece is a pawn
        char piece = game->board[from.x][from.y];
        
        // Not a pawn, not a promotion
        if (std::toupper(piece) != 'P') {
            return false;
        }
        
        // Check if destination is on the promotion rank (0 for white pawns, 7 for black pawns)
        if ((piece == 'P' && to.x == 0) || (piece == 'p' && to.x == 7)) {
            return true;
        }
        
        return false;
    }
    
    void handleMouseClick(const QPoint& pos) {
        int squareSize = 80;
        int col = pos.x() / squareSize;
        int row = pos.y() / squareSize;

        // Handle promotion menu selection
        if (promotionPending) {
            float menuWidth = squareSize * 4;
            float menuHeight = squareSize;
            float startX = (view->width() - menuWidth) / 2;
            float startY = (view->height() - menuHeight) / 2;
            
            // Skip if click is outside promotion menu
            if (pos.y() < startY || pos.y() >= startY + menuHeight || 
                pos.x() < startX || pos.x() >= startX + menuWidth) {
                return;
            }
            
            int optionIndex = (pos.x() - startX) / squareSize;
            char currentPawn = game->board[promotionSquare.x][promotionSquare.y];
            std::vector<char> options;
            if (currentPawn == 'P')
                options = { 'N', 'R', 'B', 'Q' };
            else if (currentPawn == 'p')
                options = { 'n', 'r', 'b', 'q' };
                
            if (optionIndex >= 0 && optionIndex < options.size())
                game->board[promotionSquare.x][promotionSquare.y] = options[optionIndex];
            
            promotionPending = false;
            updateRenderer();
            return;
        }
        
        // Skip if game is over
        if (gameOver) {
            return;
        }
        
        // Check if the clicked position is a legal move for the selected piece
        bool clickedLegalMove = false;
        sf::Vector2i clickedMove(-1, -1);
        
        for (const auto& move : legalMoves) {
            if (move.x == row && move.y == col) {
                clickedLegalMove = true;
                clickedMove = move;
                qDebug() << "Legal move found at" << row << col;
                break;
            }
        }
        
        // Handle move execution
        if (selectedSquare.x != -1 && clickedLegalMove) {
            char movingPiece = game->board[selectedSquare.x][selectedSquare.y];
            
            // Apply the move if it's legal
            if (isPawnPromotion(selectedSquare, clickedMove)) {
                // Show visual promotion UI instead of dialog
                showPromotionUI(clickedMove);
                return; // Wait for promotion selection
            } else {
                // Regular move
                if (!game->move(selectedSquare, clickedMove)) {
                    // Move failed
                    return;
                }
            }
            
            // Process game state after successful move
            selectedSquare = sf::Vector2i(-1, -1);
            legalMoves.clear();
            
            // Check for game-ending conditions
            if (game->isCheckmate(!game->whiteToMove)) {
                gameOver = true;
                gameResult = QString("%1 wins by checkmate!").arg(game->whiteToMove ? "Black" : "White");
            } else if (game->isStalemate(!game->whiteToMove)) {
                gameOver = true;
                gameResult = "Stalemate - Draw!";
            } else if (game->isDraw50MoveRule()) {
                gameOver = true;
                gameResult = "Draw by 50-move rule";
            }
            
            updateRenderer();
            return;
        }
        
        // Handle piece selection
        if (ChessGame::inBounds(row, col) && game->board[row][col] != ' ') {
            char piece = game->board[row][col];
            bool pieceWhite = std::isupper(piece);
            
            if (pieceWhite != game->whiteToMove) {
                selectedSquare = sf::Vector2i(-1, -1);
                legalMoves.clear();
                updateRenderer();
                return;
            }
            
            selectedSquare = sf::Vector2i(row, col);
            
            // TEMPORARY FIX: Just show the pawn can move forward without complex validation
            if (toupper(piece) == 'P') {
                int direction = pieceWhite ? -1 : 1;
                if (row + direction >= 0 && row + direction < 8) {
                    if (game->board[row + direction][col] == ' ') {
                        legalMoves.push_back(sf::Vector2i(row + direction, col));
                        
                        // Double move from starting rank
                        int startRank = pieceWhite ? 6 : 1;
                        if (row == startRank && game->board[row + 2*direction][col] == ' ') {
                            legalMoves.push_back(sf::Vector2i(row + 2*direction, col));
                        }
                    }
                }
            } else if (toupper(piece) == 'N') {
                // Knight moves in an L pattern
                int knightMoves[8][2] = {
                    {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                    {1, -2}, {1, 2}, {2, -1}, {2, 1}
                };
                
                for (int i = 0; i < 8; i++) {
                    int newRow = row + knightMoves[i][0];
                    int newCol = col + knightMoves[i][1];
                    
                    if (ChessGame::inBounds(newRow, newCol)) {
                        char targetPiece = game->board[newRow][newCol];
                        bool targetIsWhite = std::isupper(targetPiece);
                        
                        // Empty square or opponent's piece
                        if (targetPiece == ' ' || targetIsWhite != pieceWhite) {
                            legalMoves.push_back(sf::Vector2i(newRow, newCol));
                        }
                    }
                }
            } else {
                // For other pieces, skip move validation temporarily
                legalMoves.clear();
            }
            
            qDebug() << "Using simplified move generation. Found" << legalMoves.size() << "basic moves";
            updateRenderer();
            return;
        }
        
        // Default: deselect the piece if clicking on an empty square
        selectedSquare = sf::Vector2i(-1, -1);
        legalMoves.clear();
        updateRenderer();
    }
    
    void updateRenderer() {
        // Create message text if needed
        QString message = "";
        if (gameOver)
            message = gameResult;
        else if (game->kingIsInCheck(game->whiteToMove))
            message = "Check!";
            
        renderer->updateScene(
            selectedSquare,
            legalMoves,
            promotionPending,
            promotionSquare,
            message
        );
    }
    
    // Create a visual promotion selection UI
    void showPromotionUI(sf::Vector2i square) {
        promotionPending = true;
        promotionSquare = square;
        
        // This will trigger the renderer to show promotion options
        updateRenderer();
        
        // The actual move will be completed after the player clicks on a promotion piece
        // in the handleMouseClick method
    }
};

#include "qt_main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Setup global Qt error handler
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QByteArray localMsg = msg.toLocal8Bit();
        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";
        
        switch (type) {
            case QtDebugMsg:
                fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
                break;
            case QtInfoMsg:
                fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
                break;
            case QtWarningMsg:
                fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
                break;
            case QtCriticalMsg:
                fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
                break;
            case QtFatalMsg:
                fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
                abort();
        }
    });
    
    // Check if resources directory exists
    QDir resourcesDir("resources");
    if (!resourcesDir.exists() || !resourcesDir.exists("Pices")) {
        QMessageBox::critical(nullptr, "Resource Error", 
            "Missing resources directory or chess pieces. Make sure the 'resources/Pices' folder exists.");
        return 1;
    }
    
    try {
        ChessWindow window;
        window.show();
        return app.exec();
    } catch (const std::exception &e) {
        qCritical() << "Exception caught: " << e.what();
        return 1;
    } catch (...) {
        qCritical() << "Unknown exception caught";
        return 1;
    }
} 