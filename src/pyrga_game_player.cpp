#include "pyrga_game.h"

// 返回玩家手中还剩余的棋子类型。
std::vector<PyrgaPieceType> PyrgaPlayer::PiecesPlayerHave() const {
    std::vector<PyrgaPieceType> pieces;
    for (const auto& [type, count] : piecesCount) {
        if (count > 0 && (type == PyrgaPieceType::Square || type == PyrgaPieceType::Triangle || type == PyrgaPieceType::Cylinder)) {
            pieces.push_back(type);
        }
    }
    return pieces;
}
