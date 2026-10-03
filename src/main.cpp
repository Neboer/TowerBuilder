#include "pyrga_game.h"
#include "raylib.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace {
// All geometry uses this logical canvas; drawing and hit testing share its transform.
constexpr float CanvasWidth = 1120;
constexpr float CanvasHeight = 800;
constexpr float Space = 8;
constexpr Color Background{245, 241, 232, 255};
constexpr Color Surface{253, 250, 243, 255};
constexpr Color Ink{44, 48, 45, 255};
constexpr Color Muted{103, 108, 99, 255};
constexpr Color Border{208, 207, 193, 255};
constexpr Color Legal{221, 234, 207, 255};
constexpr Color LegalBorder{90, 122, 65, 255};
constexpr Color Warning{151, 63, 42, 255};
constexpr std::array<Color, 2> PlayerColors{{{165, 67, 45, 255}, {44, 99, 134, 255}}};
constexpr std::array<PyrgaPieceType, 3> Shapes{
    PyrgaPieceType::Square, PyrgaPieceType::Triangle, PyrgaPieceType::Cylinder};
constexpr std::array<Orientation, 4> Directions{
    Orientation::Up, Orientation::Right, Orientation::Down, Orientation::Left};
constexpr std::array<const char *, 3> ShapeNames{"Square", "Triangle", "Cylinder"};
constexpr std::array<const char *, 4> DirectionNames{"Up", "Right", "Down", "Left"};
constexpr Rectangle BoardArea{48, 184, 568, 568};
constexpr float CellSize = 136;

Rectangle Cell(int x, int y) {
    return {BoardArea.x + x * (CellSize + Space), BoardArea.y + y * (CellSize + Space), CellSize, CellSize};
}
Rectangle ShapeButton(int i) { return {648 + i * 144.0f, 480, 136, 64}; }
Rectangle DirectionButton(int i) { return {648 + i * 108.0f, 584, 100, 40}; }
constexpr Rectangle NewButton{840, 48, 120, 40};
constexpr Rectangle HelpButton{976, 48, 96, 40};
constexpr Rectangle ConfirmButton{576, 464, 176, 48};
constexpr Rectangle CancelButton{368, 464, 176, 48};
constexpr Rectangle CloseHelpButton{464, 664, 192, 40};

void Text(const char *text, float x, float y, int size = 20, Color color = Ink) {
    DrawText(text, static_cast<int>(x), static_cast<int>(y), size, color);
}
void Panel(Rectangle rect, Color fill, Color outline = Border) {
    DrawRectangleRounded(rect, 0.08f, 8, fill);
    DrawRectangleRoundedLinesEx(rect, 0.08f, 8, 1, outline);
}
void Focus(Rectangle rect) {
    Rectangle ring{rect.x - 4, rect.y - 4, rect.width + 8, rect.height + 8};
    DrawRectangleRoundedLinesEx(ring, 0.08f, 8, 2, Ink);
}
void Button(Rectangle rect, const char *label, Vector2 mouse, bool focused = false) {
    Panel(rect, CheckCollisionPointRec(mouse, rect) ? Legal : Surface);
    Text(label, rect.x + (rect.width - MeasureText(label, 18)) / 2, rect.y + (rect.height - 18) / 2, 18);
    if (focused) Focus(rect);
}
void Piece(PyrgaPieceType type, Orientation direction, Vector2 center, float radius, Color color) {
    if (type == PyrgaPieceType::Square) {
        DrawRectangleRec({center.x - radius, center.y - radius, radius * 2, radius * 2}, color);
    } else if (type == PyrgaPieceType::Cylinder) {
        DrawCircleV(center, radius, color);
        DrawEllipseLines(static_cast<int>(center.x), static_cast<int>(center.y - radius / 3), radius * 0.7f, radius / 3, Surface);
    } else if (type == PyrgaPieceType::Triangle) {
        Vector2 tip{0, -radius}, left{-radius, radius}, right{radius, radius};
        auto rotate = [direction](Vector2 p) -> Vector2 {
            if (direction == Orientation::Right) return {-p.y, p.x};
            if (direction == Orientation::Down) return {-p.x, -p.y};
            if (direction == Orientation::Left) return {p.y, -p.x};
            return p;
        };
        tip = rotate(tip); left = rotate(left); right = rotate(right);
        // raylib uses counter-clockwise winding in screen coordinates.
        DrawTriangle({center.x + tip.x, center.y + tip.y},
                     {center.x + left.x, center.y + left.y},
                     {center.x + right.x, center.y + right.y}, color);
    }
}
PyrgaPiece Selected(const PyrgaGame &game, int shape, int direction) {
    return {false, Shapes[shape], game.currentPlayer,
            shape == 1 ? Directions[direction] : Orientation::None};
}
int LegalCount(const PyrgaGame &game, int shape, int direction) {
    int count = 0;
    const auto piece = Selected(game, shape, direction);
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            if (game.CheckPlaceValid(Point{x, y}, piece)) ++count;
    return count;
}
void SelectAvailable(const PyrgaGame &game, int &shape, int &direction) {
    if (game.gameOver || LegalCount(game, shape, direction) > 0) return;
    for (int s = 0; s < 3; ++s)
        for (int d = 0; d < (s == 1 ? 4 : 1); ++d)
            if (LegalCount(game, s, d) > 0) { shape = s; direction = d; return; }
}
void PlayerPanel(const PyrgaGame &game, int player, float y) {
    const bool active = !game.gameOver && game.currentPlayer == player;
    Rectangle rect{648, y, 424, 112};
    Panel(rect, Surface, active ? PlayerColors[player] : Border);
    DrawRectangleRec({rect.x, rect.y + 8, 4, rect.height - 16}, PlayerColors[player]);
    Text(TextFormat("PLAYER %d%s", player + 1, active ? "  /  YOUR TURN" : ""), 664, y + 12, 20, PlayerColors[player]);
    for (int i = 0; i < 3; ++i) {
        const float x = 680 + i * 136;
        Piece(Shapes[i], Orientation::Up, {x, y + 52}, 10, PlayerColors[player]);
        Text(TextFormat("%d left", game.players[player].piecesCount.at(Shapes[i])), x + 20, y + 44, 18);
    }
    const auto towers = game.TowerCounts(player);
    Text(TextFormat("Towers: %d full   %d double   %d single", towers[2], towers[1], towers[0]), 664, y + 80, 18, Muted);
}
int SmokeTest() {
    // Exercise complete games with the actual rule engine, without requiring a display.
    for (int match = 0; match < 24; ++match) {
        PyrgaGame game(match % 2);
        std::uint32_t random = static_cast<std::uint32_t>(match + 1);
        int moves = 0;
        while (!game.gameOver && moves < 30) {
            int candidates = 0;
            Point chosen{};
            PyrgaPiece chosenPiece{};
            for (int y = 0; y < BOARD_SIZE; ++y) {
                for (int x = 0; x < BOARD_SIZE; ++x) {
                    for (int s = 0; s < 3; ++s) {
                        for (int d = 0; d < (s == 1 ? 4 : 1); ++d) {
                            auto piece = Selected(game, s, d);
                            Point position{x, y};
                            if (!game.CheckPlaceValid(position, piece)) continue;
                            ++candidates;
                            random = random * 1664525u + 1013904223u;
                            if (random % static_cast<unsigned>(candidates) == 0) {
                                chosen = position;
                                chosenPiece = piece;
                            }
                        }
                    }
                }
            }
            if (candidates == 0 || !game.MakeMove(chosen, chosenPiece)) {
                std::cerr << "Smoke failure: live game has no accepted legal action\n";
                return 1;
            }
            ++moves;
        }
        if (!game.gameOver) {
            std::cerr << "Smoke failure: game did not terminate within its 30-piece inventory\n";
            return 1;
        }
        const auto a = game.TowerCounts(0);
        const auto b = game.TowerCounts(1);
        std::cout << "Game " << match + 1 << ": starter=" << match % 2 + 1
                  << " moves=" << moves << " winner=" << (game.winner == PLAYER_UNKNOWN ? 0 : game.winner + 1)
                  << " towers(full/double/single)=" << a[2] << '/' << a[1] << '/' << a[0]
                  << " vs " << b[2] << '/' << b[1] << '/' << b[0] << '\n';
    }
    std::cout << "PASS: 24 complete legal hot-seat games, alternating starters\n";
    return 0;
}
} // namespace

int main(int argc, char **argv) {
    if (argc == 2 && std::strcmp(argv[1], "--smoke-test") == 0) return SmokeTest();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(static_cast<int>(CanvasWidth), static_cast<int>(CanvasHeight), "Pyrga - two-player hot seat");
    SetWindowMinSize(840, 600);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    PyrgaPlayerID startingPlayer = 0;
    PyrgaGame game(startingPlayer);
    int shape = 0, direction = 0;
    int focus = -1;
    enum class Modal { None, NewGame, Help };
    Modal modal = Modal::None;
    bool confirmFocused = false;
    const char *notice = "Choose a piece, then click a highlighted cell.";
    auto restart = [&]() {
        startingPlayer = 1 - startingPlayer;
        game = PyrgaGame(startingPlayer);
        shape = 0; direction = 0; focus = -1;
        modal = Modal::None;
        notice = "New game. The starting player alternates each game.";
        SelectAvailable(game, shape, direction);
    };
    while (!WindowShouldClose()) {
        const float scale = std::min(GetScreenWidth() / CanvasWidth, GetScreenHeight() / CanvasHeight);
        const Vector2 offset{(GetScreenWidth() - CanvasWidth * scale) / 2,
                             (GetScreenHeight() - CanvasHeight * scale) / 2};
        const Vector2 rawMouse = GetMousePosition();
        const Vector2 mouse{(rawMouse.x - offset.x) / scale, (rawMouse.y - offset.y) / scale};
        const bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        const bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
        auto requestNew = [&]() {
            if (game.gameOver) restart();
            else { modal = Modal::NewGame; confirmFocused = false; }
        };
        if (modal == Modal::NewGame) {
            if (IsKeyPressed(KEY_TAB)) confirmFocused = !confirmFocused;
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_N) ||
                (click && CheckCollisionPointRec(mouse, CancelButton)) || (activate && !confirmFocused)) {
                modal = Modal::None;
            } else if (IsKeyPressed(KEY_Y) || (click && CheckCollisionPointRec(mouse, ConfirmButton)) ||
                       (activate && confirmFocused)) {
                restart();
            }
        } else if (modal == Modal::Help) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_H) || activate ||
                (click && CheckCollisionPointRec(mouse, CloseHelpButton))) modal = Modal::None;
        } else {
            if (IsKeyPressed(KEY_TAB)) {
                focus = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
                            ? (focus < 0 ? 24 : (focus + 24) % 25) : (focus + 1) % 25;
            }
            if (IsKeyPressed(KEY_H) || (click && CheckCollisionPointRec(mouse, HelpButton)) || (activate && focus == 24)) {
                modal = Modal::Help;
            } else if (IsKeyPressed(KEY_R) || (click && CheckCollisionPointRec(mouse, NewButton)) || (activate && focus == 23)) {
                requestNew();
            } else if (!game.gameOver) {
                for (int i = 0; i < 3; ++i) {
                    if (IsKeyPressed(KEY_ONE + i) || (click && CheckCollisionPointRec(mouse, ShapeButton(i))) ||
                        (activate && focus == i)) {
                        shape = i;
                        notice = "Choose a highlighted cell to place this piece.";
                    }
                }
                for (int i = 0; i < 4; ++i) {
                    constexpr std::array<int, 4> keys{KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT};
                    if (shape == 1 && (IsKeyPressed(keys[i]) ||
                        (click && CheckCollisionPointRec(mouse, DirectionButton(i))) || (activate && focus == i + 3))) {
                        direction = i;
                        notice = "Triangle direction selected. Choose a highlighted cell.";
                    }
                }
                for (int y = 0; y < BOARD_SIZE; ++y) {
                    for (int x = 0; x < BOARD_SIZE; ++x) {
                        if (!((click && CheckCollisionPointRec(mouse, Cell(x, y))) || (activate && focus == 7 + y * 4 + x))) continue;
                        const auto piece = Selected(game, shape, direction);
                        const Point position{x, y};
                        if (game.CheckPlaceValid(position, piece) && game.MakeMove(position, piece)) {
                            notice = "Piece placed. Pass the turn to the other player.";
                            SelectAvailable(game, shape, direction);
                        } else {
                            notice = "Not a legal placement. Use a highlighted cell or change piece.";
                        }
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(Background);
        Camera2D camera{};
        camera.offset = offset;
        camera.zoom = scale;
        BeginMode2D(camera);
        Text("PYRGA", 48, 40, 40);
        Text("Two players. Thirty pieces. A contest of towers.", 48, 96, 20, Muted);
        Button(NewButton, game.gameOver ? "Play again" : "New game", mouse, modal == Modal::None && focus == 23);
        Button(HelpButton, "Rules / H", mouse, modal == Modal::None && focus == 24);
        if (game.gameOver) {
            Text(game.winner == PLAYER_UNKNOWN ? "DRAW - equally matched towers" :
                 TextFormat("PLAYER %d WINS", game.winner + 1), 48, 144, 24,
                 game.winner == PLAYER_UNKNOWN ? Ink : PlayerColors[game.winner]);
        } else {
            Text(TextFormat("PLAYER %d TO MOVE", game.currentPlayer + 1), 48, 144, 24, PlayerColors[game.currentPlayer]);
            Text(game.currentValidMovePositionsIsAllGrid ?
                 (game.lastMovePosition.x < 0 ? "Opening move: any cell" : "Restriction released: empty cells only") :
                 "Follow the last piece's restriction", 648, 144, 18, Muted);
        }
        const auto selected = Selected(game, shape, direction);
        const int legalCount = game.gameOver ? 0 : LegalCount(game, shape, direction);
        for (int y = 0; y < BOARD_SIZE; ++y) {
            for (int x = 0; x < BOARD_SIZE; ++x) {
                Rectangle cell = Cell(x, y);
                const bool valid = !game.gameOver && game.CheckPlaceValid(Point{x, y}, selected);
                Panel(cell, valid ? Legal : Surface, valid ? LegalBorder : Border);
                const bool last = game.lastMovePosition.x == x && game.lastMovePosition.y == y;
                Text(TextFormat("%c%d", 'A' + x, y + 1), cell.x + 8, cell.y + 8, 16, Muted);
                if (last) Text("LAST", cell.x + 80, cell.y + 8, 14, Ink);
                for (int level = 0; level < 3; ++level) {
                    const auto &piece = game.board.grid[x][y].pieces[level];
                    const float centerY = cell.y + cell.height - 24 - level * 32;
                    Text(TextFormat("%d", level + 1), cell.x + 16, centerY - 7, 14, Muted);
                    if (!piece.Empty) {
                        Piece(piece.type, piece.orientation, {cell.x + 64, centerY}, 12, PlayerColors[piece.owner]);
                        Text(TextFormat("P%d", piece.owner + 1), cell.x + 96, centerY - 7, 14, PlayerColors[piece.owner]);
                    } else {
                        DrawLineEx({cell.x + 52, centerY}, {cell.x + 76, centerY}, 1, Border);
                    }
                }
                if (modal == Modal::None && focus == 7 + y * 4 + x) Focus(cell);
                if (valid && CheckCollisionPointRec(mouse, cell)) {
                    DrawRectangleRoundedLinesEx(cell, 0.08f, 8, 3, LegalBorder);
                }
            }
        }
        PlayerPanel(game, 0, 184);
        PlayerPanel(game, 1, 312);
        Text(game.gameOver ? "GAME COMPLETE" : "CHOOSE YOUR PIECE  /  1  2  3", 648, 448, 18, Muted);
        for (int i = 0; i < 3; ++i) {
            Rectangle rect = ShapeButton(i);
            const bool stocked = game.players[game.currentPlayer].piecesCount.at(Shapes[i]) > 0;
            Panel(rect, !game.gameOver && shape == i ? Legal : Surface,
                  !game.gameOver && shape == i ? LegalBorder : Border);
            Piece(Shapes[i], i == 1 ? Directions[direction] : Orientation::None,
                  {rect.x + 24, rect.y + 24}, 10, stocked && !game.gameOver ? PlayerColors[game.currentPlayer] : Muted);
            Text(ShapeNames[i], rect.x + 44, rect.y + 16, 16, stocked ? Ink : Muted);
            Text(stocked ? "Select" : "No pieces left", rect.x + 12, rect.y + 42, 14, Muted);
            if (modal == Modal::None && focus == i) Focus(rect);
        }
        Text("TRIANGLE DIRECTION  /  ARROW KEYS", 648, 560, 18, Muted);
        for (int i = 0; i < 4; ++i) {
            Rectangle rect = DirectionButton(i);
            Panel(rect, shape == 1 && !game.gameOver && direction == i ? Legal : Surface);
            Text(DirectionNames[i], rect.x + 12, rect.y + 12, 16, shape == 1 && !game.gameOver ? Ink : Muted);
            if (modal == Modal::None && focus == i + 3) Focus(rect);
        }
        if (game.gameOver) {
            Text("The board is locked. Play again to swap", 648, 656, 18);
            Text("the starting player and begin a fresh game.", 648, 684, 18);
        } else if (legalCount == 0) {
            Text("No legal cells for this selection.", 648, 656, 20, Warning);
            Text("Change shape or triangle direction.", 648, 684, 18, Warning);
        } else {
            Text(TextFormat("%d highlighted cells for this selection", legalCount), 648, 656, 20, LegalBorder);
            Text("Layers read bottom (1) to top (3).", 648, 684, 18, Muted);
        }
        Text("Tab: focus   Enter: choose   R: new game", 648, 724, 16, Muted);
        Text(notice, 48, 772, 18, Ink);

        if (modal != Modal::None) {
            DrawRectangleRec({0, 0, CanvasWidth, CanvasHeight}, Fade(Ink, 0.6f));
            if (modal == Modal::NewGame) {
                Panel({320, 280, 480, 264}, Surface);
                Text("Start a new game?", 352, 312, 28);
                Text("This board will be discarded.", 352, 368, 20, Muted);
                Text("The other player will start next.", 352, 400, 20, Muted);
                Button(CancelButton, "Keep playing / N", mouse, !confirmFocused);
                Button(ConfirmButton, "New game / Y", mouse, confirmFocused);
            } else {
                Panel({144, 112, 832, 624}, Surface);
                Text("HOW TO PLAY", 176, 144, 28);
                Text("Each player has 5 squares, 5 triangles and 5 cylinders.", 176, 200, 20);
                Text("Place on a 4 x 4 board. Each cell holds at most 3 pieces:", 176, 232, 20);
                Text("one of each shape, stacked in the order they are played.", 176, 264, 20);
                Text("The last piece tells the next player where to play:", 176, 312, 20);
                Text("Square: orthogonally adjacent cell. Cylinder: the same cell.", 176, 344, 20);
                Text("Triangle: any distance in its arrow direction (no blocking).", 176, 376, 20);
                Text("A triangle must point toward at least one board cell.", 176, 408, 20);
                Text("If NO piece can follow the restriction, use an EMPTY cell.", 176, 456, 20);
                Text("First player: a square opening forbids a square on turn two.", 176, 488, 20);
                Text("Control 3 full towers to win immediately (majority color).", 176, 536, 20);
                Text("If no move remains, compare full, same-color double, then", 176, 568, 20);
                Text("single towers, in that order. Equal scores mean a draw.", 176, 600, 20);
                Button(CloseHelpButton, "Back / Esc", mouse, true);
            }
        }
        EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
