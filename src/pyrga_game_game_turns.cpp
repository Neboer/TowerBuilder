#include "pyrga_game.h"

bool PyrgaGame::CanPlacePiece(const Point &pos, const PyrgaPiece &piece) const
{
    if (gameOver || currentPlayer < 0 || currentPlayer > 1 || piece.Empty ||
        piece.owner != currentPlayer || pos.x < 0 || pos.x >= BOARD_SIZE ||
        pos.y < 0 || pos.y >= BOARD_SIZE)
        return false;
    if (piece.type != PyrgaPieceType::Square && piece.type != PyrgaPieceType::Triangle &&
        piece.type != PyrgaPieceType::Cylinder)
        return false;
    const auto stock = players[currentPlayer].piecesCount.find(piece.type);
    if (stock == players[currentPlayer].piecesCount.end() || stock->second <= 0)
        return false;
    if (piece.type == PyrgaPieceType::Square && currentPlayer == startingPlayer_ &&
        openingWasSquare_ && movesPlayed_[currentPlayer] == 1)
        return false;
    if (piece.type == PyrgaPieceType::Triangle)
    {
        switch (piece.orientation)
        {
        case Orientation::Up: if (pos.y == 0) return false; break;
        case Orientation::Down: if (pos.y == BOARD_SIZE - 1) return false; break;
        case Orientation::Left: if (pos.x == 0) return false; break;
        case Orientation::Right: if (pos.x == BOARD_SIZE - 1) return false; break;
        default: return false;
        }
    }
    else if (piece.orientation != Orientation::None)
        return false;
    const auto &box = board[pos];
    return !box.IsFull() && !box[piece.type];
}

bool PyrgaGame::HasLegalPieceAt(const Point &pos) const
{
    for (const auto type : {PyrgaPieceType::Square, PyrgaPieceType::Triangle, PyrgaPieceType::Cylinder})
    {
        if (type == PyrgaPieceType::Triangle)
        {
            for (const auto direction : {Orientation::Up, Orientation::Down, Orientation::Left, Orientation::Right})
                if (CanPlacePiece(pos, PyrgaPiece{false, type, currentPlayer, direction})) return true;
        }
        else if (CanPlacePiece(pos, PyrgaPiece{false, type, currentPlayer, Orientation::None}))
            return true;
    }
    return false;
}

bool PyrgaGame::CheckPlaceValid(const Point &pos, const PyrgaPiece &piece) const
{
    if (!CanPlacePiece(pos, piece)) return false;
    for (const auto &candidate : currentValidMovePositions)
        if (candidate.x == pos.x && candidate.y == pos.y) return true;
    return false;
}

bool PyrgaGame::MakeMove(const Point &pos, const PyrgaPiece &piece)
{
    if (!CheckPlaceValid(pos, piece) || !board.placePiece(pos, piece)) return false;
    --players[currentPlayer].piecesCount.find(piece.type)->second;
    if (nextMoveIsFirstMove) openingWasSquare_ = piece.type == PyrgaPieceType::Square;
    ++movesPlayed_[currentPlayer];
    lastMovePosition = pos;
    lastMovePiece = piece;
    nextMoveIsFirstMove = false;
    currentPlayer = GetNextPlayer();

    // Three complete towers wins immediately, before testing whether play can continue.
    winner = GetWinner();
    if (winner != PLAYER_UNKNOWN)
    {
        gameOver = true;
        currentValidMovePositions.clear();
        currentValidMovePositionsIsAllGrid = false;
    }
    else if (!UpdateValidMoves())
    {
        gameOver = true;
        winner = GetWinner(true);
    }
    return true;
}

PyrgaPlayerID PyrgaGame::GetNextPlayer() const
{
    return 1 - currentPlayer;
}

PyrgaGame::PyrgaGame(PyrgaPlayerID startingPlayer)
    : currentPlayer(startingPlayer == 1 ? 1 : 0), startingPlayer_(currentPlayer)
{
    currentValidMovePositions.reserve(BOARD_SIZE * BOARD_SIZE);
    UpdateValidMoves();
}
