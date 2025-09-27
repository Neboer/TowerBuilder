#include "pyrga_base.h"

PyrgaBoard::PyrgaBoard()
{
    for (int i = 0; i < BOARD_SIZE; ++i)
    {
        for (int j = 0; j < BOARD_SIZE; ++j)
        {
            grid[i][j].position.x = i;
            grid[i][j].position.y = j;
        }
    }
}

bool PyrgaBoard::placePiece(int x, int y, PyrgaPieceType piece, PyrgaPlayerID player)
{
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    {
        return false; // Out of bounds
    }
    auto &box = grid[x][y];
    for (auto &p : box.pieces)
    {
        if (p.Empty)
        {
            p.Empty = false;
            p.type = piece;
            p.owner = player;
            return true; // Successfully placed
        }
    }
    return false; // No empty slot available
}
