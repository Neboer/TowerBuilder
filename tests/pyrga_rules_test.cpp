#include "pyrga_game.h"

#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using Type = PyrgaPieceType;
using Positions = std::set<std::pair<int, int>>;

void Require(bool condition, const std::string &message)
{
    if (!condition) throw std::runtime_error(message);
}

PyrgaPiece Piece(Type type, int owner, Orientation direction = Orientation::None)
{
    return {false, type, owner, direction};
}

void Move(PyrgaGame &game, Point position, Type type, Orientation direction = Orientation::None)
{
    Require(game.MakeMove(position, Piece(type, game.currentPlayer, direction)), "expected legal move was rejected");
}

Positions ValidPositions(const PyrgaGame &game)
{
    Positions result;
    for (const auto &p : game.currentValidMovePositions) result.emplace(p.x, p.y);
    Require(result.size() == game.currentValidMovePositions.size(), "duplicate legal positions");
    return result;
}

void ExpectPositions(const PyrgaGame &game, const Positions &expected)
{
    Require(ValidPositions(game) == expected, "legal position set differs from expected restriction");
}

std::string Snapshot(const PyrgaGame &game)
{
    std::ostringstream out;
    auto point = [&out](Point p) { out << p.x << ',' << p.y << ';'; };
    auto piece = [&out](const PyrgaPiece &p) {
        out << p.Empty << ',' << static_cast<int>(p.type) << ',' << p.owner << ','
            << static_cast<int>(p.orientation) << ';';
    };
    out << game.currentPlayer << ';' << game.gameOver << ';' << game.winner << ';'
        << game.nextMoveIsFirstMove << ';' << game.currentValidMovePositionsIsAllGrid << ';'
        << game.board.BoardWidth << ';' << game.board.BoardHeight << ';';
    point(game.lastMovePosition);
    piece(game.lastMovePiece);
    for (const auto &column : game.board.grid)
        for (const auto &box : column) {
            point(box.position);
            for (const auto &p : box.pieces) piece(p);
        }
    for (const auto &player : game.players) {
        out << player.id << ';' << player.name.size() << ':' << player.name << ';';
        for (const auto &stock : player.piecesCount)
            out << static_cast<int>(stock.first) << ',' << stock.second << ';';
        out << '|';
    }
    for (const auto &p : game.currentValidMovePositions) point(p);
    return out.str();
}

void RejectUnchanged(PyrgaGame &game, Point position, const PyrgaPiece &piece)
{
    const auto before = Snapshot(game);
    Require(!game.CheckPlaceValid(position, piece), "illegal move reported valid");
    Require(!game.MakeMove(position, piece), "illegal move accepted");
    Require(Snapshot(game) == before, "rejected move changed game state");
}

// Fixture towers have distinct shapes and inward-facing triangles, just like real moves.
void PlaceFixture(PyrgaGame &game, Point position, Type type, int owner)
{
    const auto direction = type == Type::Triangle
        ? (position.y == 0 ? Orientation::Down : Orientation::Up) : Orientation::None;
    Require(game.board.placePiece(position, Piece(type, owner, direction)), "invalid fixture placement");
    Require(game.players[owner].piecesCount[type] > 0, "fixture exceeded starting inventory");
    --game.players[owner].piecesCount[type];
}

void Tower(PyrgaGame &game, Point position, const std::vector<int> &owners)
{
    const Type shapes[] = {Type::Square, Type::Cylinder, Type::Triangle};
    for (std::size_t i = 0; i < owners.size(); ++i) PlaceFixture(game, position, shapes[i], owners[i]);
}

void Restrict(PyrgaGame &game, Point position, Type type, Orientation direction = Orientation::None)
{
    game.nextMoveIsFirstMove = false;
    game.lastMovePosition = position;
    game.lastMovePiece = Piece(type, 1 - game.currentPlayer, direction);
}

void OpeningPlayersAndSecondTurn()
{
    for (int starter : {0, 1}) {
        PyrgaGame game(starter);
        Require(game.currentPlayer == starter && game.GetNextPlayer() == 1 - starter, "wrong starting player");
        Require(game.nextMoveIsFirstMove && game.currentValidMovePositionsIsAllGrid, "opening flags incorrect");
        Require(ValidPositions(game).size() == BOARD_SIZE * BOARD_SIZE, "opening does not cover board");
        Move(game, {1, 1}, Type::Square);
        Require(game.players[starter].piecesCount[Type::Square] == 4, "opening stock not decremented");
        Require(game.currentPlayer == 1 - starter && !game.nextMoveIsFirstMove, "opening did not pass turn");
        Move(game, {2, 1}, Type::Square); // The other player's first square remains legal.
        RejectUnchanged(game, {2, 2}, Piece(Type::Square, starter));
        Move(game, {2, 2}, Type::Cylinder);
        Move(game, {2, 2}, Type::Triangle, Orientation::Up);
        Move(game, {2, 0}, Type::Square); // Ban expires after the starter's second move.

        PyrgaGame nonSquare(starter);
        Move(nonSquare, {1, 1}, Type::Cylinder);
        Move(nonSquare, {1, 1}, Type::Triangle, Orientation::Right);
        Move(nonSquare, {2, 1}, Type::Square); // No ban if the opening was not a square.
    }
}

void SquareAndTriangleRestrictions()
{
    PyrgaGame square;
    Move(square, {1, 1}, Type::Square);
    ExpectPositions(square, {{0, 1}, {2, 1}, {1, 0}, {1, 2}});
    Require(!square.currentValidMovePositionsIsAllGrid, "square restriction released prematurely");
    RejectUnchanged(square, {2, 2}, Piece(Type::Cylinder, square.currentPlayer));
    PyrgaGame corner;
    Move(corner, {0, 0}, Type::Square);
    ExpectPositions(corner, {{1, 0}, {0, 1}});

    const std::vector<std::pair<Orientation, Positions>> rays = {
        {Orientation::Up, {{1, 0}}}, {Orientation::Down, {{1, 2}, {1, 3}}},
        {Orientation::Left, {{0, 1}}}, {Orientation::Right, {{2, 1}, {3, 1}}}
    };
    for (const auto &ray : rays) {
        PyrgaGame game;
        Move(game, {1, 1}, Type::Triangle, ray.first);
        ExpectPositions(game, ray.second);
        Require(!game.currentValidMovePositionsIsAllGrid, "triangle restriction released prematurely");
    }
    PyrgaGame beyondFull;
    Tower(beyondFull, {1, 2}, {0, 1, 0});
    Move(beyondFull, {1, 1}, Type::Triangle, Orientation::Down);
    ExpectPositions(beyondFull, {{1, 3}}); // A filled square does not stop a ray.
}

void TriangleEdgeOrientations()
{
    for (int x = 0; x < BOARD_SIZE; ++x)
        for (int y = 0; y < BOARD_SIZE; ++y)
            for (auto direction : {Orientation::Up, Orientation::Down, Orientation::Left, Orientation::Right}) {
                PyrgaGame game;
                const bool inward = !(direction == Orientation::Up && y == 0)
                    && !(direction == Orientation::Down && y == BOARD_SIZE - 1)
                    && !(direction == Orientation::Left && x == 0)
                    && !(direction == Orientation::Right && x == BOARD_SIZE - 1);
                const auto p = Piece(Type::Triangle, 0, direction);
                Require(game.CheckPlaceValid({x, y}, p) == inward, "triangle edge orientation validity incorrect");
                if (inward) Require(game.MakeMove({x, y}, p), "inward triangle rejected");
                else RejectUnchanged(game, {x, y}, p);
            }
}

void CylinderAndRepeatedShapes()
{
    PyrgaGame game;
    Move(game, {1, 1}, Type::Cylinder);
    ExpectPositions(game, {{1, 1}});
    RejectUnchanged(game, {1, 1}, Piece(Type::Cylinder, game.currentPlayer));
    RejectUnchanged(game, {1, 2}, Piece(Type::Square, game.currentPlayer));
    Move(game, {1, 1}, Type::Square);
    for (auto shape : {Type::Square, Type::Triangle, Type::Cylinder}) {
        PyrgaGame duplicate;
        PlaceFixture(duplicate, {1, 1}, shape, 1);
        const auto direction = shape == Type::Triangle ? Orientation::Down : Orientation::None;
        RejectUnchanged(duplicate, {1, 1}, Piece(shape, 0, direction));
    }
}

void ReleaseRequiresNoActionAndEmptySquare()
{
    PyrgaGame available;
    Tower(available, {1, 1}, {1});
    Restrict(available, {1, 1}, Type::Cylinder);
    Require(available.UpdateValidMoves(), "restricted stacking move missing");
    ExpectPositions(available, {{1, 1}});
    Require(!available.currentValidMovePositionsIsAllGrid, "available restriction incorrectly released");
    RejectUnchanged(available, {0, 0}, Piece(Type::Square, 0));

    PyrgaGame full;
    Tower(full, {1, 1}, {1, 0, 1});
    Tower(full, {2, 2}, {0});
    Restrict(full, {1, 1}, Type::Cylinder);
    Require(full.UpdateValidMoves() && full.currentValidMovePositionsIsAllGrid, "full target did not release");
    Require(ValidPositions(full).size() == 14, "release did not select exactly empty squares");
    RejectUnchanged(full, {2, 2}, Piece(Type::Cylinder, 0));
    Move(full, {0, 0}, Type::Square);

    PyrgaGame stock;
    Tower(stock, {1, 1}, {1});
    Tower(stock, {2, 2}, {1});
    stock.players[0].piecesCount[Type::Triangle] = 0;
    stock.players[0].piecesCount[Type::Cylinder] = 0;
    Restrict(stock, {1, 1}, Type::Cylinder);
    Require(stock.UpdateValidMoves() && stock.currentValidMovePositionsIsAllGrid, "stock exhaustion did not release restriction");
    Require(ValidPositions(stock).size() == 14, "inventory release included occupied squares");
    RejectUnchanged(stock, {2, 2}, Piece(Type::Square, 0));
    Move(stock, {0, 0}, Type::Square);

    PyrgaGame noEmpty;
    PlaceFixture(noEmpty, {0, 0}, Type::Square, 0);
    PlaceFixture(noEmpty, {0, 0}, Type::Triangle, 1);
    const Type shapes[] = {Type::Square, Type::Triangle, Type::Cylinder};
    for (int i = 1; i < BOARD_SIZE * BOARD_SIZE; ++i)
        PlaceFixture(noEmpty, {i / BOARD_SIZE, i % BOARD_SIZE}, shapes[i % 3], i % 2);
    Require(noEmpty.UpdateValidMoves(), "last stacking move unavailable");
    Move(noEmpty, {0, 0}, Type::Cylinder);
    Require(noEmpty.gameOver && noEmpty.currentValidMovePositions.empty(),
            "blocked cylinder with no empty square failed to end play");
    Require(!noEmpty.players[noEmpty.currentPlayer].PiecesPlayerHave().empty(),
            "board-exhaustion fixture accidentally exhausted inventory");
    RejectUnchanged(noEmpty, {1, 1}, Piece(Type::Triangle, noEmpty.currentPlayer, Orientation::Up));
}

void IllegalMovesPreserveState()
{
    PyrgaGame game;
    RejectUnchanged(game, {-1, 0}, Piece(Type::Square, 0));
    RejectUnchanged(game, {BOARD_SIZE, 0}, Piece(Type::Square, 0));
    RejectUnchanged(game, {0, -1}, Piece(Type::Square, 0));
    RejectUnchanged(game, {0, BOARD_SIZE}, Piece(Type::Square, 0));
    RejectUnchanged(game, {1, 1}, Piece(Type::Square, 1));
    RejectUnchanged(game, {1, 1}, PyrgaPiece{});
    RejectUnchanged(game, {1, 1}, Piece(Type::Empty, 0));
    RejectUnchanged(game, {1, 1}, Piece(static_cast<Type>(99), 0));
    RejectUnchanged(game, {1, 1}, Piece(Type::Triangle, 0));
    RejectUnchanged(game, {1, 1}, Piece(Type::Triangle, 0, static_cast<Orientation>(99)));
    RejectUnchanged(game, {1, 1}, Piece(Type::Square, 0, Orientation::Up));
    RejectUnchanged(game, {1, 1}, Piece(Type::Cylinder, 0, Orientation::Down));
    game.players[0].piecesCount[Type::Square] = 0;
    RejectUnchanged(game, {1, 1}, Piece(Type::Square, 0));
    game.players[0].piecesCount.erase(Type::Cylinder);
    RejectUnchanged(game, {1, 1}, Piece(Type::Cylinder, 0));
    Move(game, {1, 1}, Type::Triangle, Orientation::Right);
    Require(game.players[0].piecesCount[Type::Triangle] == 4 && game.currentPlayer == 1,
            "invalid attempts corrupted later valid move");
}

void ExactHeightMajorityCounts()
{
    PyrgaGame game;
    Tower(game, {0, 0}, {0});
    Tower(game, {0, 1}, {1});
    Tower(game, {0, 2}, {0, 0});
    Tower(game, {0, 3}, {1, 1});
    Tower(game, {1, 0}, {0, 1});
    Tower(game, {1, 1}, {0, 1, 0});
    Tower(game, {1, 2}, {1, 0, 1});
    Require(game.TowerCounts(0) == std::array<int, 3>{1, 1, 1}, "player zero exact-height counts incorrect");
    Require(game.TowerCounts(1) == std::array<int, 3>{1, 1, 1}, "player one exact-height counts incorrect");
    Require(game.board[Point{1, 0}].TowerOwner(2) == PLAYER_UNKNOWN, "split two-level tower has an owner");
    Require(game.board[Point{1, 1}].TowerOwner(2) == PLAYER_UNKNOWN, "complete tower counted at lower height");
    Require(game.TowerCounts(PLAYER_UNKNOWN) == std::array<int, 3>{0, 0, 0}, "invalid owner counted towers");
    Require(game.GetWinner() == PLAYER_UNKNOWN, "balanced live board declared a winner");
}

void ImmediateVictoryBeforeNoMoves()
{
    for (int owner : {0, 1}) {
        PyrgaGame game(owner);
        Tower(game, {0, 0}, {owner, 1 - owner, owner});
        Tower(game, {0, 1}, {1 - owner, owner, owner});
        Tower(game, {1, 1}, {owner, 1 - owner});
        for (auto &stock : game.players[1 - owner].piecesCount) stock.second = 0;
        Require(game.UpdateValidMoves(), "winning move unavailable");
        Move(game, {1, 1}, Type::Triangle, Orientation::Up);
        Require(game.gameOver && game.winner == owner, "third complete tower failed to win immediately");
        Require(game.TowerCounts(owner)[2] == 3, "winning majority towers miscounted");
        Require(game.currentValidMovePositions.empty() && !game.currentValidMovePositionsIsAllGrid,
                "immediate win fell through into no-moves release");
        RejectUnchanged(game, {2, 2}, Piece(Type::Square, game.currentPlayer));
        Require(!game.UpdateValidMoves(), "terminal game generated moves");
    }
}

// Each case finishes its final tower with a real move, then the opponent's empty
// inventory ends play. The final board, not a mocked winner, determines the result.
void EndgameRanking()
{
    struct Case {
        const char *name;
        std::vector<std::vector<int>> towers;
        int winner;
    };
    const std::vector<Case> cases = {
        {"three-level player zero", {{1, 1}, {1, 1}, {0, 1, 0}}, 0},
        {"three-level player one", {{0, 0}, {0, 0}, {1, 0, 1}}, 1},
        {"two-level player zero", {{0, 1, 0}, {1, 0, 1}, {1}, {1}, {0, 0}}, 0},
        {"two-level player one", {{0, 1, 0}, {1, 0, 1}, {0}, {0}, {1, 1}}, 1},
        {"single-level player zero", {{0, 1, 0}, {1, 0, 1}, {0, 0}, {1, 1}, {0}}, 0},
        {"single-level player one", {{0, 1, 0}, {1, 0, 1}, {0, 0}, {1, 1}, {1}}, 1},
        {"all heights tied", {{0, 1, 0}, {1, 0, 1}, {0, 0}, {1, 1}, {0}, {1}}, PLAYER_UNKNOWN},
        {"only single levels tied", {{0}, {1}}, PLAYER_UNKNOWN},
        {"split two-level tower is neutral", {{0, 1}}, PLAYER_UNKNOWN}
    };
    const Type shapes[] = {Type::Square, Type::Cylinder, Type::Triangle};
    for (const auto &test : cases) {
        try {
            const auto &last = test.towers.back();
            const int finisher = last.back();
            PyrgaGame game(finisher);
            for (std::size_t i = 0; i < test.towers.size(); ++i) {
                const Point position{static_cast<int>(i / BOARD_SIZE), static_cast<int>(i % BOARD_SIZE)};
                auto owners = test.towers[i];
                if (i + 1 == test.towers.size()) owners.pop_back();
                Tower(game, position, owners);
            }
            for (auto &stock : game.players[1 - finisher].piecesCount) stock.second = 0;
            Require(game.GetWinner() == PLAYER_UNKNOWN, "nonterminal fixture ended prematurely");
            Require(game.UpdateValidMoves(), "finishing move unavailable");
            const auto index = test.towers.size() - 1;
            const Point position{static_cast<int>(index / BOARD_SIZE), static_cast<int>(index % BOARD_SIZE)};
            const auto shape = shapes[last.size() - 1];
            Move(game, position, shape, shape == Type::Triangle
                ? (position.y == 0 ? Orientation::Down : Orientation::Up) : Orientation::None);
            Require(game.gameOver, "no-inventory endgame did not finish");
            Require(game.winner == test.winner && game.GetWinner(true) == test.winner, "wrong height-ranked result");
            Require(game.currentValidMovePositions.empty(), "no-move endgame retained legal positions");
            RejectUnchanged(game, {3, 3}, Piece(Type::Square, game.currentPlayer));
        } catch (const std::exception &error) {
            throw std::runtime_error(std::string(test.name) + ": " + error.what());
        }
    }
}
} // namespace

int main()
{
    const std::vector<std::pair<const char *, std::function<void()>>> tests = {
        {"both starters and opening-square second-turn ban", OpeningPlayersAndSecondTurn},
        {"orthogonal squares and triangle rays", SquareAndTriangleRestrictions},
        {"triangle edge orientations", TriangleEdgeOrientations},
        {"cylinder same-square and repeated-shape rejection", CylinderAndRepeatedShapes},
        {"no-action release selects only empty squares", ReleaseRequiresNoActionAndEmptySquare},
        {"illegal moves preserve complete public state", IllegalMovesPreserveState},
        {"exact-height majority tower counts", ExactHeightMajorityCounts},
        {"third complete tower wins before no-moves endgame", ImmediateVictoryBeforeNoMoves},
        {"height-ranked endgames and draws reject further moves", EndgameRanking}
    };
    int failures = 0;
    for (const auto &test : tests) {
        try {
            test.second();
            std::cout << "PASS: " << test.first << '\n';
        } catch (const std::exception &error) {
            ++failures;
            std::cerr << "FAIL: " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - failures << '/' << tests.size() << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
