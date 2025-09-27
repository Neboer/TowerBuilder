#pragma once
#include <iostream>
#include <vector>
#include "pyrga_base.h"

class PyrgaGame
{
public:
    PyrgaBoard board;
    PyrgaPlayerID currentPlayer = 0; // Player 0 starts

    // Last Move Info
    bool nextMoveIsFirstMove = true;
    Point lastMovePosition = {-1, -1};
    PyrgaPieceType lastMovePiece = PyrgaPieceType::Empty;

    // valid move positions
    std::vector<Point> currentValidMovePositions;
    // 是否所有格子都可以放置（除了满的格子）
    bool currentValidMovePositionsIsAllGrid = true;

    // Highlight valid moves position grid.
    void UpdateValidMoves();
    // Check if the move is in the valid moves grids and the piece can be placed.
    bool CheckPlaceValid(int x, int y, PyrgaPieceType piece);

    // Game Make Turns.
    bool MakeMove(int x, int y, PyrgaPieceType piece);
};