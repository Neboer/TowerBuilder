#include "pyrga_game.h"
#include <string>
#include <stdexcept>

bool PyrgaGame::CheckPlaceValid(Point &pos, PyrgaPiece &piece)
{
    // 检查是否有同类型的棋子在目标格子中。
    if (board.grid[pos.x][pos.y][piece.type])
    {
        return false; // Same type piece already exists in the grid box
    }
    // 既然当前格没有同类棋子，如果当前所有格子都可以放置，那么一定是可以放置的。
    if (currentValidMovePositionsIsAllGrid)
    {
        return true;
    }
    else
    {
        // 否则检查是否在有效位置列表中
        for (const auto &validPos : currentValidMovePositions)
        {
            if (validPos.x == pos.x && validPos.y == pos.y)
            {
                return true;
            }
        }
        return false; // Not a valid position
    }
}

bool PyrgaGame::MakeMove(Point &pos, PyrgaPiece &piece)
{
    // 检查是否是当前玩家的棋子
    if (piece.owner != currentPlayer)
    {
        throw std::runtime_error("It's not player " + std::to_string(piece.owner) + "'s turn!");
    }
    // 检查玩家是否还有这种类型的棋子
    if (players[currentPlayer].piecesCount[piece.type] <= 0)
    {
        throw std::runtime_error("Player " + std::to_string(currentPlayer) + " has no more pieces of type " + std::to_string(static_cast<int>(piece.type)) + "!");
    }
    // 检查位置是否合法
    if (pos.x < 0 || pos.x >= board.BoardWidth || pos.y < 0 || pos.y >= board.BoardHeight)
    {
        throw std::runtime_error("Position (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ") is out of bounds!");
    }
    // 检查放置是否合法
    if (!CheckPlaceValid(pos, piece))
    {
        throw std::runtime_error("Cannot place piece at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")!");
    }
    // 尝试放置棋子
    if (!board.placePiece(pos, piece))
    {
        throw std::runtime_error("Failed to place piece at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")!");
    }
    // 成功放置后，更新玩家的棋子数量
    players[currentPlayer].piecesCount[piece.type]--;
    // 更新最后一次移动的信息
    lastMovePosition = pos;
    lastMovePiece = piece;
    nextMoveIsFirstMove = false;
    // 切换到下一个玩家
    currentPlayer = (currentPlayer + 1) % 2;
    // 更新有效移动位置
    UpdateValidMoves();
    return true;
}

PyrgaPlayerID PyrgaGame::GetNextPlayer() const {
    return (currentPlayer + 1) % 2;
}

PyrgaGame::PyrgaGame()
{
    UpdateValidMoves();
}
