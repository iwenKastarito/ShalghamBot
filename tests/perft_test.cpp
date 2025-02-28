#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include "../src/core/ChessGame.h"

// Type definition for node count
typedef unsigned long long u64;

// Perft function to count nodes at a given depth - optimized version
u64 Perft(ChessGame& game, int depth) {
    if (depth == 0) 
        return 1ULL;
        
    u64 nodes = 0;
    
    // Get all legal moves for the current side to move
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> allMoves = 
        game.getAllLegalMoves(game.whiteToMove);
    
    // Count each move
    for (const auto& move : allMoves) {
        ChessGame newPosition = game.simulateMove(move.first, move.second);
        nodes += Perft(newPosition, depth - 1);
    }
    
    return nodes;
}

// Bulk counting version for potentially better performance
u64 PerftBulk(ChessGame& game, int depth) {
    if (depth == 1) {
        return game.getAllLegalMoves(game.whiteToMove).size();
    }
    
    u64 nodes = 0;
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> allMoves = 
        game.getAllLegalMoves(game.whiteToMove);
        
    for (const auto& move : allMoves) {
        ChessGame newPosition = game.simulateMove(move.first, move.second);
        nodes += Perft(newPosition, depth - 1);
    }
    
    return nodes;
}

// Divide - shows perft counts for individual moves from position
void Divide(ChessGame& game, int depth) {
    u64 total = 0;
    
    // Get all legal moves for the current side
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> allMoves = 
        game.getAllLegalMoves(game.whiteToMove);
    
    for (const auto& move : allMoves) {
        std::string moveStr = std::string(1, 'a' + move.first.y) + 
                             std::string(1, '8' - move.first.x) + 
                             std::string(1, 'a' + move.second.y) + 
                             std::string(1, '8' - move.second.x);
        
        ChessGame newPosition = game.simulateMove(move.first, move.second);
        u64 nodes = Perft(newPosition, depth - 1);
        
        std::cout << moveStr << ": " << nodes << std::endl;
        total += nodes;
    }
    
    std::cout << "\nTotal nodes: " << total << std::endl;
}

// Add expected counts for each initial move at depth 4
void DivideWithVerification(ChessGame& game, int depth) {
    // Expected counts for the starting position at depth 4
    std::unordered_map<std::string, u64> expected = {
        {"a2a3", 8457}, {"a2a4", 9329}, {"b2b3", 9345}, {"b2b4", 9332},
        {"c2c3", 9272}, {"c2c4", 9744}, {"d2d3", 11959}, {"d2d4", 12435},
        {"e2e3", 13134}, {"e2e4", 13160}, {"f2f3", 8457}, {"f2f4", 8929},
        {"g2g3", 9345}, {"g2g4", 9328}, {"h2h3", 8457}, {"h2h4", 9329},
        {"b1a3", 8885}, {"b1c3", 9755}, {"g1f3", 9748}, {"g1h3", 8881}
    };

    u64 total = 0;
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> allMoves = 
        game.getAllLegalMoves(game.whiteToMove);
    
    for (const auto& move : allMoves) {
        std::string moveStr = std::string(1, 'a' + move.first.y) + 
                             std::string(1, '8' - move.first.x) + 
                             std::string(1, 'a' + move.second.y) + 
                             std::string(1, '8' - move.second.x);
        
        ChessGame newPosition = game.simulateMove(move.first, move.second);
        u64 nodes = Perft(newPosition, depth - 1);
        
        std::cout << moveStr << ": " << nodes;
        
        // Compare with expected value if available
        if (expected.count(moveStr) > 0) {
            u64 expectedNodes = expected[moveStr];
            if (nodes != expectedNodes) {
                std::cout << " [DISCREPANCY - Expected: " << expectedNodes << "]";
            } else {
                std::cout << " [Correct]";
            }
        }
        
        std::cout << std::endl;
        total += nodes;
    }
    
    std::cout << "\nTotal nodes: " << total << std::endl;
}

// Test positions defined in FEN format with expected perft results
struct TestPosition {
    std::string name;
    std::string fen;
    std::unordered_map<int, u64> expected_results;
};

void DebugPerft(ChessGame& game, int maxDepth) {
    // Test some known problematic positions
    std::vector<std::string> testPositions = {
        // En passant positions
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1", // White can en passant
        "rnbqkbnr/pppppppp/8/8/3pP3/8/PPP2PPP/RNBQKBNR b KQkq e3 0 1",  // Black can en passant
        
        // Castling positions
        "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",  // Both sides can castle both ways
        
        // Promotion positions
        "4k3/1P6/8/8/8/8/8/4K3 w - - 0 1", // White pawn about to promote
        "4k3/8/8/8/8/8/1p6/4K3 b - - 0 1", // Black pawn about to promote
        
        // Check evasion positions
        "4k3/8/8/8/8/8/8/4K2R b K - 0 1",  // Black in check, must move king
        "r3k3/8/8/8/8/8/8/4K3 b q - 0 1",  // Black in check, must move king or rook to block
    };
    
    for (const auto& pos : testPositions) {
        ChessGame testGame(pos);
        std::cout << "\nTesting position: " << pos << "\n";
        
        for (int depth = 1; depth <= maxDepth; depth++) {
            u64 nodes = PerftBulk(testGame, depth);
            std::cout << "Depth " << depth << ": " << nodes << " nodes\n";
            
            if (depth == 1) {
                std::cout << "Moves: ";
                Divide(testGame, depth);
            }
        }
    }
}

// Add these debugging functions to diagnose specific move types
void DebugMoveGeneration(const ChessGame& game) {
    std::cout << "\n=== DETAILED MOVE GENERATION ANALYSIS ===\n";
    
    // Test pawn moves
    std::cout << "\nPAWN MOVE GENERATION:\n";
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char piece = game.board[r][c];
            if (piece == 'P' || piece == 'p') {
                bool isWhite = (piece == 'P');
                if (isWhite == game.whiteToMove) {
                    auto moves = game.getLegalMoves(r, c, true);
                    std::cout << "Pawn at " << (char)('a' + c) << (8 - r) << " has " 
                              << moves.size() << " moves: ";
                    for (const auto& move : moves) {
                        std::cout << (char)('a' + move.y) << (8 - move.x) << " ";
                    }
                    std::cout << "\n";
                }
            }
        }
    }
    
    // Test castling
    std::cout << "\nCASTLING STATUS:\n";
    std::cout << "White king moved: " << game.whiteKingMoved << "\n";
    std::cout << "White kingside rook moved: " << game.whiteRookKingsideMoved << "\n";
    std::cout << "White QueenSide rook moved: " << game.whiteRookQueenSideMoved << "\n";
    std::cout << "Black king moved: " << game.blackKingMoved << "\n";
    std::cout << "Black kingside rook moved: " << game.blackRookKingsideMoved << "\n";
    std::cout << "Black QueenSide rook moved: " << game.blackRookQueenSideMoved << "\n";
    
    // Test en passant
    std::cout << "\nEN PASSANT STATUS:\n";
    if (game.enPassantTarget.x != -1) {
        std::cout << "En passant target: " << (char)('a' + game.enPassantTarget.y) 
                  << (8 - game.enPassantTarget.x) << "\n";
    } else {
        std::cout << "No en passant target\n";
    }
    
    // Test check detection
    std::cout << "\nCHECK STATUS:\n";
    std::cout << "White king in check: " << game.kingIsInCheck(true) << "\n";
    std::cout << "Black king in check: " << game.kingIsInCheck(false) << "\n";
    
    std::cout << "\n=== END ANALYSIS ===\n\n";
}

// Add this function to test specific positions with issues
void TestSpecificPositions() {
    // Special cases known to cause problems
    std::vector<std::pair<std::string, int>> positions = {
        // En passant position
        {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 1", 3},
        
        // Castling position
        {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 3},
        
        // Promotion position
        {"8/P7/8/8/8/8/8/k6K w - - 0 1", 3},
        
        // Check evasion position
        {"5k2/8/8/8/8/8/8/4K2R b K - 0 1", 2}
    };
    
    for (const auto& [fen, depth] : positions) {
        std::cout << "\nTesting position: " << fen << "\n";
        ChessGame game(fen);
        
        // Show the current position analysis
        DebugMoveGeneration(game);
        
        // Analyze each possible move
        Divide(game, depth);
    }
}

// Add a move traversal debug option
void DebugMove(const std::string& fen, const std::string& moveSequence) {
    ChessGame game(fen);
    std::istringstream ss(moveSequence);
    std::string move;
    
    std::cout << "Starting position: " << fen << "\n";
    DebugMoveGeneration(game);
    
    ChessGame savedPosition = game;
    
    while (ss >> move) {
        if (move.length() < 4) continue;
        
        // Convert algebraic notation to coordinates
        int fromCol = move[0] - 'a';
        int fromRow = '8' - move[1];
        int toCol = move[2] - 'a';
        int toRow = '8' - move[3];
        
        sf::Vector2i from(fromRow, fromCol);
        sf::Vector2i to(toRow, toCol);
        
        // Verify the move is legal
        if (!game.isValidMove(from, to, true)) {
            std::cout << "Move " << move << " is not legal!\n";
            break;
        }
        
        // Make the move
        game = game.simulateMove(from, to);
        
        std::cout << "After move " << move << ":\n";
        std::cout << "FEN: " << game.getCurrentFEN() << "\n";
        DebugMoveGeneration(game);
    }
    
    // Compare detailed move lists with another starting position
    std::cout << "\nDetailed move comparison for each piece:\n";
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char piece = savedPosition.board[r][c];
            if (piece != ' ') {
                std::cout << "Moves for " << piece << " at " << (char)('a' + c) << (8 - r) << ":\n";
                auto moves = savedPosition.getLegalMoves(r, c, true);
                for (const auto& move : moves) {
                    std::cout << "  " << (char)('a' + c) << (8 - r) 
                              << (char)('a' + move.y) << (8 - move.x);
                    
                    // Test if this move differs after simulation
                    ChessGame testPos = savedPosition.simulateMove(sf::Vector2i(r, c), move);
                    auto posAfterMove = testPos.getAllLegalMoves(!savedPosition.whiteToMove);
                    std::cout << " (" << posAfterMove.size() << " responses)\n";
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Define test positions
    std::vector<TestPosition> positions = {
        {
            "Initial Position",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {{1, 20}, {2, 400}, {3, 8902}, {4, 197281}, {5, 4865609}, {6, 119060324}}
        },
        {
            "Kiwipete",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
            {{1, 48}, {2, 2039}, {3, 97862}, {4, 4085603}, {5, 193690690}}
        },
        {
            "Position 3",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
            {{1, 14}, {2, 191}, {3, 2812}, {4, 43238}, {5, 674624}}
        },
        {
            "Position 4",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -",
            {{1, 6}, {2, 264}, {3, 9467}, {4, 422333}, {5, 15833292}}
        },
        {
            "Position 5",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -",
            {{1, 44}, {2, 1486}, {3, 62379}, {4, 2103487}, {5, 89941194}}
        },
        {
            "Position 6",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -",
            {{1, 46}, {2, 2079}, {3, 89890}, {4, 3894594}, {5, 164075551}}
        }
    };

    // Parse command-line arguments
    int position_index = 0;
    int max_depth = 5;
    bool divide_mode = false;
    bool debug_mode = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--position") {
            if (i + 1 < argc) {
                position_index = std::stoi(argv[++i]) - 1;
            }
        } else if (arg == "-d" || arg == "--depth") {
            if (i + 1 < argc) {
                max_depth = std::stoi(argv[++i]);
            }
        } else if (arg == "--divide") {
            divide_mode = true;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -p, --position N    Test position N (1-" << positions.size() << ")\n"
                      << "  -d, --depth N       Search to depth N\n"
                      << "  --divide            Show counts for each move\n"
                      << "  --debug             Run specific debug tests\n"
                      << "  --analyze           Show detailed analysis of position\n"
                      << "  -h, --help          Show this help message\n";
            return 0;
        } else if (arg == "--debug") {
            std::cout << "Running debug perft tests...\n";
            ChessGame game;
            DebugPerft(game, 3);  // Test up to depth 3
            return 0;
        } else if (arg == "--debug-moves") {
            debug_mode = true;
        } else if (arg == "--analyze") {
            debug_mode = true;
        } else if (arg == "--test-promotions") {
            std::cout << "Testing promotion counting...\n";
            
            // Setup a position with pawns about to promote
            ChessGame game("8/P7/8/8/8/8/p7/8 w - - 0 1");
            
            std::cout << "White pawn about to promote:\n";
            auto whiteMoves = game.getLegalMoves(1, 0, true);
            std::cout << "Number of moves: " << whiteMoves.size() << " (should be 4 for each promotion type)\n";
            
            // For black pawn
            game = ChessGame("8/P7/8/8/8/8/p7/8 b - - 0 1");
            std::cout << "\nBlack pawn about to promote:\n";
            auto blackMoves = game.getLegalMoves(6, 0, true);
            std::cout << "Number of moves: " << blackMoves.size() << " (should be 4 for each promotion type)\n";
        }
    }
    
    // Validate position index
    if (position_index < 0 || position_index >= positions.size()) {
        std::cerr << "Invalid position index. Should be between 1 and " << positions.size() << ".\n";
        return 1;
    }
    
    // Initialize chess game with the selected position
    const TestPosition& pos = positions[position_index];
    ChessGame game(pos.fen);
    
    std::cout << "Testing " << pos.name << "\n";
    std::cout << "FEN: " << pos.fen << "\n\n";
    
    // If in divide mode, just show the breakdown for the specified depth
    if (divide_mode) {
        std::cout << "Divide at depth " << max_depth << ":\n";
        Divide(game, max_depth);
        return 0;
    }
    
    // If in debug mode, analyze the initial position before running perft tests
    if (debug_mode) {
        std::cout << "\nAnalyzing initial position before perft tests:\n";
        DebugMoveGeneration(game);
    }
    
    // Otherwise run perft test for each depth up to max_depth
    std::cout << std::left 
              << std::setw(8) << "Depth" 
              << std::setw(15) << "Nodes" 
              << std::setw(15) << "Time" 
              << std::setw(10) << "MN/s" 
              << std::setw(15) << "Expected" 
              << "Result\n";
    std::cout << std::string(70, '=') << "\n";
    
    for (int depth = 1; depth <= max_depth; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        u64 nodes = PerftBulk(game, depth);
        auto end = std::chrono::high_resolution_clock::now();
        
        double time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double mnps = (nodes / 1000000.0) / (time_ms / 1000.0);
        
        std::string result = "Unknown";
        if (pos.expected_results.count(depth)) {
            u64 expected = pos.expected_results.at(depth);
            result = (nodes == expected) ? "PASS" : "FAIL";
        }
        
        std::cout << std::left
                  << std::setw(8) << depth
                  << std::setw(15) << nodes
                  << std::setw(15) << std::fixed << std::setprecision(2) << time_ms << " ms"
                  << std::setw(10) << std::fixed << std::setprecision(2) << mnps
                  << std::setw(15);
        
        if (pos.expected_results.count(depth)) {
            std::cout << pos.expected_results.at(depth);
        } else {
            std::cout << "N/A";
        }
        
        std::cout << result << "\n";
        
        if (result == "FAIL" && depth <= 3) {
            std::cout << "\nDetailed debug for depth " << depth << ":\n";
            Divide(game, depth);
        }
    }
    
    return 0;
} 