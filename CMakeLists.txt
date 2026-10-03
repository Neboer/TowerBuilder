cmake_minimum_required(VERSION 3.16)
project(TowerBuilder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib 5.0 QUIET)
if(TARGET raylib)
    set(TOWERBUILDER_RAYLIB raylib)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib>=5.0)
    set(TOWERBUILDER_RAYLIB PkgConfig::RAYLIB)
endif()

add_library(pyrga_rules
    src/pyrga_base.cpp
    src/pyrga_game_game_turns.cpp
    src/pyrga_game_update_valid_moves.cpp
    src/pyrga_game_get_winner.cpp
    src/pyrga_game_player.cpp
)
target_include_directories(pyrga_rules PUBLIC src)

add_executable(TowerBuilder src/main.cpp)
target_link_libraries(TowerBuilder PRIVATE pyrga_rules ${TOWERBUILDER_RAYLIB})

include(CTest)
if(BUILD_TESTING)
    add_executable(pyrga_rules_test tests/pyrga_rules_test.cpp)
    target_link_libraries(pyrga_rules_test PRIVATE pyrga_rules)
    add_test(NAME pyrga_rules COMMAND pyrga_rules_test)
    add_test(NAME hotseat_smoke COMMAND TowerBuilder --smoke-test)
endif()
