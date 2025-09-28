#include "pyrga_game.h"

PyrgaPlayerID PyrgaGame::GetWinner(bool gameIsOver = false) {
    // 检查每个格子的所有者。
    // 如果有玩家拥有三座塔，则该玩家获胜。
    // 如果强行要求游戏结束，则谁场上拥有更多的塔谁获胜。
    std::array<int, 2> towerCount = {0, 0};
    for (const auto &row : board.grid) {
        for (const auto &box : row) {
            auto owner = box.TowerOwner();
            if (owner != PLAYER_UNKNOWN) {
                towerCount[owner]++;
            }
        }
    }
    if (towerCount[0] >= 3 && towerCount[1] < 3) return 0;
    if (towerCount[1] >= 3 && towerCount[0] < 3) return 1;
    if (towerCount[0] >= 3 && towerCount[1] >= 3)
    {
        // 如果双方塔树相同，比较
    }
    


    if (gameIsOver) {
        return towerCount[0] > towerCount[1] ? 0 : 1;
    }
    return PLAYER_UNKNOWN;
}