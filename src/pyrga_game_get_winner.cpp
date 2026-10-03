#include "pyrga_game.h"

std::array<int, 3> PyrgaGame::TowerCounts(PyrgaPlayerID player) const
{
    std::array<int, 3> counts = {0, 0, 0};
    if (player < 0 || player > 1) return counts;
    for (const auto &column : board.grid)
    {
        for (const auto &box : column)
        {
            int height = 0;
            for (const auto &piece : box.pieces)
                if (!piece.Empty) ++height;
            if (height > 0 && box.TowerOwner(height) == player) ++counts[height - 1];
        }
    }
    return counts;
}

PyrgaPlayerID PyrgaGame::GetWinner(bool gameIsOver) const
{
    const auto first = TowerCounts(0);
    const auto second = TowerCounts(1);
    if (first[2] >= 3 && second[2] < 3) return 0;
    if (second[2] >= 3 && first[2] < 3) return 1;
    if (!gameIsOver && !gameOver) return PLAYER_UNKNOWN;
    for (int height = 2; height >= 0; --height)
    {
        if (first[height] > second[height]) return 0;
        if (second[height] > first[height]) return 1;
    }
    return PLAYER_UNKNOWN;
}
