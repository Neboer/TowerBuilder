#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
#include "pyrga_base.h"

class PyrgaPlayer {
public:
    PyrgaPlayerID id;
    std::string name;
    std::map<PyrgaPieceType, int> piecesCount = {
        {PyrgaPieceType::Square, 5},
        {PyrgaPieceType::Triangle, 5},
        {PyrgaPieceType::Cylinder, 5}
    };

    std::vector<PyrgaPieceType> PiecesPlayerHave() const;
    PyrgaPlayer(PyrgaPlayerID pid, const std::string &pname) : id(pid), name(pname) {}
};

class PyrgaGame {
public:
    PyrgaBoard board;
    PyrgaPlayerID currentPlayer = 0;
    std::array<PyrgaPlayer, 2> players = {PyrgaPlayer(0, "Player 1"), PyrgaPlayer(1, "Player 2")};
    bool gameOver = false;
    PyrgaPlayerID winner = PLAYER_UNKNOWN;
    bool nextMoveIsFirstMove = true;
    Point lastMovePosition = {-1, -1};
    PyrgaPiece lastMovePiece = {};
    std::vector<Point> currentValidMovePositions;
    // True for the opening move or when the restriction releases to empty squares.
    bool currentValidMovePositionsIsAllGrid = true;

    explicit PyrgaGame(PyrgaPlayerID startingPlayer = 0);
    PyrgaPlayerID GetNextPlayer() const;
    bool MakeMove(const Point &pos, const PyrgaPiece &piece);
    bool CheckPlaceValid(const Point &pos, const PyrgaPiece &piece) const;
    bool UpdateValidMoves();
    std::array<int, 3> TowerCounts(PyrgaPlayerID player) const;
    PyrgaPlayerID GetWinner(bool gameIsOver = false) const;

private:
    PyrgaPlayerID startingPlayer_;
    std::array<int, 2> movesPlayed_ = {0, 0};
    bool openingWasSquare_ = false;
    bool CanPlacePiece(const Point &pos, const PyrgaPiece &piece) const;
    bool HasLegalPieceAt(const Point &pos) const;
};
