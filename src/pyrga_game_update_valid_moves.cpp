#include "pyrga_game.h"

bool PyrgaGame::UpdateValidMoves()
{
    currentValidMovePositions.clear();
    currentValidMovePositionsIsAllGrid = false;
    if (gameOver) return false;

    auto addIfLegal = [this](const Point &pos) {
        if (HasLegalPieceAt(pos)) currentValidMovePositions.push_back(pos);
    };
    if (nextMoveIsFirstMove)
    {
        currentValidMovePositionsIsAllGrid = true;
        for (int x = 0; x < BOARD_SIZE; ++x)
            for (int y = 0; y < BOARD_SIZE; ++y)
                addIfLegal(Point{x, y});
        return !currentValidMovePositions.empty();
    }

    switch (lastMovePiece.type)
    {
    case PyrgaPieceType::Cylinder:
        addIfLegal(lastMovePosition);
        break;
    case PyrgaPieceType::Square:
        addIfLegal(Point{lastMovePosition.x - 1, lastMovePosition.y});
        addIfLegal(Point{lastMovePosition.x + 1, lastMovePosition.y});
        addIfLegal(Point{lastMovePosition.x, lastMovePosition.y - 1});
        addIfLegal(Point{lastMovePosition.x, lastMovePosition.y + 1});
        break;
    case PyrgaPieceType::Triangle:
    {
        Point step;
        switch (lastMovePiece.orientation)
        {
        case Orientation::Up: step = {0, -1}; break;
        case Orientation::Down: step = {0, 1}; break;
        case Orientation::Left: step = {-1, 0}; break;
        case Orientation::Right: step = {1, 0}; break;
        default: break;
        }
        if (step.x != 0 || step.y != 0)
        {
            Point pos{lastMovePosition.x + step.x, lastMovePosition.y + step.y};
            while (pos.x >= 0 && pos.x < BOARD_SIZE && pos.y >= 0 && pos.y < BOARD_SIZE)
            {
                addIfLegal(pos);
                pos.x += step.x;
                pos.y += step.y;
            }
        }
        break;
    }
    default: break;
    }

    // Release only if no piece/orientation can be played in the restricted range.
    // A released move must start a new tower, never add to an occupied square.
    if (currentValidMovePositions.empty())
    {
        currentValidMovePositionsIsAllGrid = true;
        for (int x = 0; x < BOARD_SIZE; ++x)
        {
            for (int y = 0; y < BOARD_SIZE; ++y)
            {
                const auto &box = board.grid[x][y];
                bool empty = true;
                for (const auto &piece : box.pieces)
                    if (!piece.Empty) { empty = false; break; }
                if (empty) addIfLegal(Point{x, y});
            }
        }
    }
    return !currentValidMovePositions.empty();
}
