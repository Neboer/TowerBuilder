#include "pyrga_game.h"

// 返回玩家手中还剩余的棋子类型。
std::vector<PyrgaPieceType> PyrgaPlayer::PiecesPlayerHave() {
    std::vector<PyrgaPieceType> pieces;
    for (const auto& [type, count] : piecesCount) {
        if (count > 0) {
            pieces.push_back(type);
        }
    }
    return pieces;
}
