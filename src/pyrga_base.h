#pragma once
#include <array>
constexpr int BOARD_SIZE = 4;

enum class PyrgaPieceType
{
    Empty,
    Square,
    Triangle,
    Cylinder,
};

using PyrgaPlayerID = int;
constexpr PyrgaPlayerID PLAYER_UNKNOWN = -1;

// 三角形的朝向
enum class Orientation
{
    Up,
    Down,
    Left,
    Right,
    None
};

// 棋子结构体，朝向指的是三角形的朝向。
struct PyrgaPiece
{
    bool Empty = true;
    PyrgaPieceType type = PyrgaPieceType::Empty;
    PyrgaPlayerID owner = PLAYER_UNKNOWN;
    Orientation orientation = Orientation::None;
};

struct Point
{
    int x = 0;
    int y = 0;
};

// 棋盘格子结构体，指的是棋盘上的一个格子，它可以包含最多三个棋子。
struct PyrgaGridBox
{
    Point position;
    std::array<PyrgaPiece, 3> pieces = {};
    bool IsFull() const;
    // 检查格子中是否有指定类型的棋子。比如 grid[3][1][PyrgaPieceType::Square] 可以检查 (3,1) 格子中是否有正方形棋子。
    bool operator[](PyrgaPieceType type) const;
    // Ownership of an exact-height tower; split two-piece towers have no owner.
    PyrgaPlayerID TowerOwner(int completeLevel = 3) const;
};

struct PyrgaBoard
{
    std::array<std::array<PyrgaGridBox, BOARD_SIZE>, BOARD_SIZE> grid;

    PyrgaBoard();

    int BoardWidth = BOARD_SIZE;
    int BoardHeight = BOARD_SIZE;

    bool placePiece(const Point &pos, const PyrgaPiece &piece);
    const PyrgaGridBox &operator[](const Point &pos) const;
};
