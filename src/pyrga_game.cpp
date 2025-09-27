#include "pyrga_game.h"

void PyrgaGame::UpdateValidMoves()
{
    currentValidMovePositions.clear();
    // 如果是第一次放，那么可以放在任意位置
    if (nextMoveIsFirstMove)
    {
        currentValidMovePositionsIsAllGrid = true;
        return;
    }
    // 如果最后一次放的是圆，那么只能放在同一格（但不要着急，还要检查能不能放）
    else if (lastMovePiece == PyrgaPieceType::Cylinder)
    {
        currentValidMovePositions.push_back(lastMovePosition);
        // 不要急着return，后面还有统一的剩余空间检查。
    }
    // 如果最后一次放的是方块，那么可以放在上下左右四个格子
    else if (lastMovePiece == PyrgaPieceType::Square)
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
    // 如果最后一次放的是三角形，
}