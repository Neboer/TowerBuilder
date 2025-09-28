#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <array>
#include <optional>
#include "pyrga_base.h"

class PyrgaPlayer {
public:
    PyrgaPlayerID id;
    std::string name = "";
    std::map <PyrgaPieceType, int> piecesCount = {
        {PyrgaPieceType::Square, 5},
        {PyrgaPieceType::Triangle, 5},
        {PyrgaPieceType::Cylinder, 5}
    }; // Available pieces count

    std::vector<PyrgaPieceType> PiecesPlayerHave();

    PyrgaPlayer(PyrgaPlayerID pid, const std::string &pname) : id(pid), name(pname) {}
};

class PyrgaGame
{
public:
    PyrgaBoard board;
    PyrgaPlayerID currentPlayer = 0; // Player 0 starts
    PyrgaPlayerID GetNextPlayer() const;
    std::array<PyrgaPlayer, 2> players = {PyrgaPlayer(0, "Player 1"), PyrgaPlayer(1, "Player 2")};

    // Last Move Info
    bool nextMoveIsFirstMove = true;
    Point lastMovePosition = {-1, -1};
    PyrgaPiece lastMovePiece = {};

    // valid move positions
    std::vector<Point> currentValidMovePositions;
    // 是否所有格子都可以放置（除了满的格子）
    bool currentValidMovePositionsIsAllGrid = true;

    // 当前轮到的玩家下一个棋子，这个是游戏推进的主方法。
    bool MakeMove(Point &pos, PyrgaPiece &piece);

    // 在当前玩家落子成功之后执行，用来判断下个玩家的有效落子位置，它设置的currentValidMovePositions是MakeMove中执行的CheckPlaceValid所依赖的重要方法。
    // 会返回一个bool值，表示是否至少有一个格子是可以放置的。如果没有格子可以放置了（也就是返回false），那么游戏应该结束了。
    bool UpdateValidMoves();
    // Check if the move is in the valid moves grids and the piece can be placed.
    bool CheckPlaceValid(Point &pos, PyrgaPiece &piece);
    // 返回根据下一个玩家手中还拥有的棋子类型，过滤后的有效位置列表。
    void FilterValidPositionsByNextPlayerPieces(const std::vector<Point> &positions);

    PyrgaGame();

    PyrgaPlayerID PyrgaGame::GetWinner(bool gameIsOver = false) ; // Returns PLAYER_UNKNOWN if no winner yet
};