#include "pyrga_base.h"
#include <map>

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
bool PyrgaBoard::placePiece(Point &pos, PyrgaPiece &piece)
{
    if (pos.x < 0 || pos.x >= BOARD_SIZE || pos.y < 0 || pos.y >= BOARD_SIZE)
    {
        return false; // Out of bounds
    }
    auto &box = grid[pos.x][pos.y];
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

// completeLevel 可以取3,2,1，它分别代表在计算塔的所有者时，需要考虑的塔是只有满三层的，还是也要考虑满两层的塔，还是也要考虑满一层的塔。
// 如果也要考虑满两层的塔，那么满三层的塔还会被正常计算，同时满两层的塔会被计算，但满一层的塔不会被计算
// 一层也是同样的道理。三层是正常游戏判定条件，只有在双方同时达到三层时，才会考虑两层的塔，依此类推。
PyrgaPlayerID PyrgaGridBox::TowerOwner(int completeLevel) const
{
    std::map<PyrgaPlayerID, int> ownerCount;
    int filledLevels = 0;
    for (const auto &p : pieces)
    {
        if (!p.Empty)
        {
            filledLevels++;
            ownerCount[p.owner]++;
        }
    }
    if (filledLevels < completeLevel)
    {
        return PLAYER_UNKNOWN; // Not enough pieces to form a tower of the specified level
    }
    // 找到拥有最多棋子的玩家
    PyrgaPlayerID maxOwner = PLAYER_UNKNOWN;
    int maxCount = 0;
    for (const auto &[owner, count] : ownerCount)
    {
        if (count > maxCount)
        {
            maxCount = count;
            maxOwner = owner;
        }
        else if (count == maxCount)
        {
            maxOwner = PLAYER_UNKNOWN; // Tie
        }
    }
    return maxOwner;
}

PyrgaGridBox PyrgaBoard::operator[](Point &pos) const
{
    return grid[pos.x][pos.y];
}