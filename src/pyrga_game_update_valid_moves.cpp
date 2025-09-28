#include "pyrga_game.h"

// 传入一个可能的有效位置列表，返回根据下一个玩家手中还拥有的棋子类型以及实际棋盘上对应位置的棋子放置情况，过滤后的下一个玩家可能可以放的有效位置列表。注意这个方法会修改positions。
void PyrgaGame::FilterValidPositionsByNextPlayerPieces(const std::vector<Point> &positions)
{
    auto nextPlayer = GetNextPlayer();
    auto nextPlayerPieces = players[nextPlayer].PiecesPlayerHave();
    for (const auto &pos : positions)
    {
        // 检查这个位置是否已经满了
        if (board[pos].IsFull())
        {
            continue; // 这个位置满了，不能放
        }
        // 检查这个位置上已经有哪些类型的棋子
        std::map<PyrgaPieceType, bool> existingTypes;
        for (const auto &type : {PyrgaPieceType::Square, PyrgaPieceType::Triangle, PyrgaPieceType::Cylinder})
        {
            existingTypes[type] = board.grid[pos.x][pos.y][type];
        }
        // 检查下一个玩家手中是否有可以放在这个位置的棋子类型
        bool canPlace = false;
        for (const auto &type : nextPlayerPieces)
        {
            if (!existingTypes[type])
            {
                canPlace = true;
                break;
            }
        }
        if (canPlace)
        {
            currentValidMovePositions.push_back(pos);
        }
    }
    // 最后，检查过滤后的列表是否为空。如果为空，说明没有任何格子可以放置了，那么currentValidMovePositionsIsAllGrid就设为true，表示可以放在任意非满格子。
    if (currentValidMovePositions.empty())
    {
        currentValidMovePositionsIsAllGrid = true;
        currentValidMovePositions.clear();
    }
    else
    {
        currentValidMovePositionsIsAllGrid = false;
    }
}

bool PyrgaGame::UpdateValidMoves()
{
    currentValidMovePositions.clear();
    // 如果是第一次放，那么可以放在任意位置
    if (nextMoveIsFirstMove)
    {
        currentValidMovePositionsIsAllGrid = true;
        return;
    }
    // 如果最后一次放的是圆，那么只能放在同一格（但不要着急，还要检查能不能放）
    else if (lastMovePiece.type == PyrgaPieceType::Cylinder)
    {
        currentValidMovePositions.push_back(lastMovePosition);
        // 不要急着return，后面还有统一的剩余空间检查。
    }
    // 如果最后一次放的是方块，那么可以放在上下左右四个格子
    else if (lastMovePiece.type == PyrgaPieceType::Square)
    {
        // 定义四个方向
        const std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (const auto &dir : directions)
        {
            int nx = lastMovePosition.x + dir.first;
            int ny = lastMovePosition.y + dir.second;
            // 检查坐标是否合法
            if (nx >= 0 && nx < board.BoardWidth && ny >= 0 && ny < board.BoardHeight)
            {
                currentValidMovePositions.push_back({nx, ny});
            }
        }
    }
    // 如果最后一次放的是三角形，那么可以放在三角形朝向的方向直到棋盘尽头
    else if (lastMovePiece.type == PyrgaPieceType::Triangle)
    {
        int dx = 0, dy = 0;
        switch (lastMovePiece.orientation)
        {
        case Orientation::Up:
            dx = -1;
            dy = 0;
            break;
        case Orientation::Down:
            dx = 1;
            dy = 0;
            break;
        case Orientation::Left:
            dx = 0;
            dy = -1;
            break;
        case Orientation::Right:
            dx = 0;
            dy = 1;
            break;
        default:
            break;
        }
        int nx = lastMovePosition.x + dx;
        int ny = lastMovePosition.y + dy;
        while (nx >= 0 && nx < board.BoardWidth && ny >= 0 && ny < board.BoardHeight)
        {
            currentValidMovePositions.push_back({nx, ny});
            nx += dx;
            ny += dy;
        }
    }
    // 现在，currentValidMovePositions中已经包含了玩家所以可以放的位置，接下来需要考虑玩家是否真的有棋子可以放进currentValidMovePositions中的空位。

    // 接下来，检查currentValidMovePositions中的每个格子是否还有剩余空间。如果遇到有的格子已满，那么就把它从currentValidMovePositions中移除。
    std::vector<Point> filteredValidPositions;
    for (const auto &pos : currentValidMovePositions)
    {
        if (!board.grid[pos.x][pos.y].IsFull())
        {
            filteredValidPositions.push_back(pos);
        }
    }
    // 然后，检查过滤后的列表是否为空。如果为空，说明没有任何格子可以放置了，那么currentValidMovePositionsIsAllGrid就设为true，表示可以放在任意非满格子。
    if (filteredValidPositions.empty())
    {
        currentValidMovePositionsIsAllGrid = true;
        currentValidMovePositions.clear();
    }
    else
    {
        currentValidMovePositionsIsAllGrid = false;
        currentValidMovePositions = filteredValidPositions;
    }
}