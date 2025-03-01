// minimax_gpu.cu
#include "bot.h"
#include <cuda_runtime.h>
#include "device_launch_parameters.h"

// Define the size of our transposition table.
#define TT_SIZE 1024

// Structure to hold a transposition table entry.
struct TTEntry {
    unsigned long long hash; // A hash for the game state.
    int depth;               // Depth at which the state was evaluated.
    int value;               // The minimax evaluation.
};

// Global transposition table in device memory.
// This table persists across kernel launches so that later calls can reuse previous results.
__device__ TTEntry transpositionTable[TT_SIZE];

//-----------------------------------------------------------------
// Device-side helper functions for min and max.
__device__ int dev_min(int a, int b) {
    return (a < b) ? a : b;
}
__device__ int dev_max(int a, int b) {
    return (a > b) ? a : b;
}

//-----------------------------------------------------------------
// Device function: a simple evaluation function.
__device__ int evaluateGPU(const GameStateGPU& state) {
    int eval = 0;
    for (int i = 0; i < 64; i++) {
        eval += (int)state.board[i];
    }
    return eval;
}

//-----------------------------------------------------------------
// Device function: compute a simple hash for a given game state using FNV-1a.
__device__ unsigned long long hashState(const GameStateGPU& state) {
    unsigned long long hash = 1469598103934665603ULL; // FNV offset basis
    for (int i = 0; i < 64; i++) {
        hash ^= (unsigned long long)state.board[i];
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

//-----------------------------------------------------------------
// Device function: a simplified minimax recursion with alpha–beta pruning
// with caching using the transposition table.
__device__ int minimaxGPU(GameStateGPU state, int depth, int alpha, int beta, bool whiteTurn) {
    // First, try to retrieve a cached result.
    unsigned long long stateHash = hashState(state);
    // A very simple linear search over our fixed-size table.
    for (int i = 0; i < TT_SIZE; i++) {
        if (transpositionTable[i].hash == stateHash && transpositionTable[i].depth >= depth) {
            return transpositionTable[i].value;
        }
    }

    // Terminal condition: if depth is 0, evaluate the state.
    if (depth == 0) {
        int eval = evaluateGPU(state);
        return eval;
    }

    int bestVal = whiteTurn ? -100000 : 100000;

    // Dummy move-generation: For every occupied square, try "moving" right.
    for (int i = 0; i < 64; i++) {
        if (state.board[i] != ' ') {
            int row = i / 8;
            int col = i % 8;
            if (col < 7) {
                GameStateGPU newState = state;
                // Simulate moving the piece one square to the right.
                newState.board[i + 1] = newState.board[i];
                newState.board[i] = ' ';
                int score = minimaxGPU(newState, depth - 1,
                    whiteTurn ? alpha : -100000,
                    whiteTurn ? 100000 : beta,
                    !whiteTurn);
                if (whiteTurn) {
                    bestVal = dev_max(bestVal, score);
                    alpha = dev_max(alpha, score);
                }
                else {
                    bestVal = dev_min(bestVal, score);
                    beta = dev_min(beta, score);
                }
                if (beta <= alpha)
                    break;
            }
        }
    }

    // After calculating bestVal, store it in the transposition table.
    // Use a simple modulo hash to choose an index. In a real implementation, you would handle collisions more robustly.
    int index = stateHash % TT_SIZE;
    transpositionTable[index].hash = stateHash;
    transpositionTable[index].depth = depth;
    transpositionTable[index].value = bestVal;

    return bestVal;
}

//-----------------------------------------------------------------
// Kernel: for each candidate state, compute its minimax value.
extern "C" __global__ void minimaxKernel(GameStateGPU* d_states, int numCandidates,
    int depth, bool whiteTurn, int* d_results) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < numCandidates) {
        d_results[idx] = minimaxGPU(d_states[idx], depth,
            whiteTurn ? -100000 : 100000,
            whiteTurn ? 100000 : -100000,
            whiteTurn);
    }
}
