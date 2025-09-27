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

struct PyrgaGridBox
{
    Point position;
    std::array<PyrgaPiece, BOARD_SIZE> pieces = {};

};

struct PyrgaBoard
{
    std::array<std::array<PyrgaGridBox, BOARD_SIZE>, BOARD_SIZE> grid;

    PyrgaBoard();

    int BoardWidth = BOARD_SIZE;
    int BoardHeight = BOARD_SIZE;

    bool placePiece(int x, int y, PyrgaPieceType piece, PyrgaPlayerID player);
};
