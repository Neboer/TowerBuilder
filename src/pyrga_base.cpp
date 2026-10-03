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
bool PyrgaBoard::placePiece(const Point &pos, const PyrgaPiece &piece)
{
    if (pos.x < 0 || pos.x >= BOARD_SIZE || pos.y < 0 || pos.y >= BOARD_SIZE)
    {
        return false; // Out of bounds
    }
    auto &box = grid[pos.x][pos.y];
    if (piece.Empty || piece.owner < 0 || piece.owner > 1 ||
        (piece.type != PyrgaPieceType::Square && piece.type != PyrgaPieceType::Triangle && piece.type != PyrgaPieceType::Cylinder) ||
        box[piece.type])
    {
        return false;
    }
    for (auto &p : box.pieces)
    {
        if (p.Empty)
        {
            p = piece;
            return true; // Successfully placed
        }
    }
    return false; // No empty slot available
}

bool PyrgaGridBox::IsFull() const
{
    for (const auto &p : pieces)
    {
        if (p.Empty)
        {
            return false; // Found an empty slot
        }
    }
    return true; // No empty slots found
}

bool PyrgaGridBox::operator[](PyrgaPieceType type) const
{
    for (const auto &p : pieces)
    {
        if (!p.Empty && p.type == type)
        {
            return true; // Found a piece of the specified type
        }
    }
    return false; // No piece of the specified type found
}

PyrgaPlayerID PyrgaGridBox::TowerOwner(int completeLevel) const
{
    if (completeLevel < 1 || completeLevel > 3) return PLAYER_UNKNOWN;
    std::array<int, 2> counts = {0, 0};
    int height = 0;
    for (const auto &piece : pieces)
    {
        if (piece.Empty) continue;
        ++height;
        if (piece.owner < 0 || piece.owner > 1) return PLAYER_UNKNOWN;
        ++counts[piece.owner];
    }
    if (height != completeLevel || counts[0] == counts[1]) return PLAYER_UNKNOWN;
    return counts[0] > counts[1] ? 0 : 1;
}

const PyrgaGridBox &PyrgaBoard::operator[](const Point &pos) const
{
    return grid[pos.x][pos.y];
}