#pragma once

#include <raylib.h>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include "game_settings.hpp"

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

class Maze {
 public:
  inline void move_player(Direction d) {
    Vector3 original_pc{pc};

    std::uint32_t init_cell =
        (static_cast<std::uint32_t>(pc.y / grid_size.y) * COLS) +
        static_cast<std::uint32_t>(pc.x / grid_size.x);

    int speed_y = static_cast<int>(PLAYER_GRID_SPEED * grid_size.y);
    int speed_x = static_cast<int>(PLAYER_GRID_SPEED * grid_size.x);

    switch (d) {
      case UP:
        pc.y = (pc.y - speed_y < 0) ? 0 : pc.y - speed_y;
        break;
      case DOWN:
        pc.y = (pc.y + speed_y >= MAZE_SIZE_2D - grid_size.y)
                   ? MAZE_SIZE_2D - grid_size.y
                   : pc.y + speed_y;
        break;
      case LEFT:
        pc.x = (pc.x - speed_x < 0) ? 0 : pc.x - speed_x;
        break;
      case RIGHT:
        pc.x = (pc.x + speed_x >= MAZE_SIZE_2D - grid_size.x)
                   ? MAZE_SIZE_2D - grid_size.x
                   : pc.x + speed_x;
        break;
      default:
        std::runtime_error("Invalid movement");
    }

    std::uint32_t end_cell =
        (static_cast<std::uint32_t>(pc.y / grid_size.y) * COLS) +
        static_cast<std::uint32_t>(pc.x / grid_size.x);

    if (end_cell != init_cell && !(maze.at(init_cell).count(end_cell)))
      pc = original_pc;
  }

  inline void update(std::uint16_t y, std::uint16_t x) {
    if (y >= ROWS)
      throw std::runtime_error("Maze update: coordinates out of bounds.");

    if (x >= COLS)
      throw std::runtime_error("Maze update: coordinates out of bounds.");

    if (y != 0 && y != ROWS - 1 && x != 0 && x != COLS - 1)
      throw std::runtime_error(
          "Maze update: invalid coordinates for start - they do not fall along "
          "the maze wall");

    std::uint32_t starting_cell = (y * COLS) + x;
    DFS((y * COLS) + x);

    maze_start = starting_cell;
    maze_end = (COLS - 1) * (ROWS - 1);
    // TODO set maze_end, I think just compile a list of all possible edge
    // pieces and choose one?
  }

  void print();  // for debugging purposes
  void draw(int top_y, int top_x);

  Vector3 pc{0.0, 0.0, 0.0};
  Vector2 grid_size{static_cast<float>(MAZE_SIZE_2D) / ROWS,
                    static_cast<float>(MAZE_SIZE_2D) / COLS};

  std::array<std::unordered_set<std::uint32_t>, ROWS * COLS> maze{};

 private:
  /* variables */
  std::array<bool, ROWS * COLS> visited{};
  uint32_t maze_start{};
  uint32_t maze_end{};

  /* translation */
  std::vector<std::uint32_t> adjacent_cells(std::uint32_t cell);
  std::pair<int, int> cell_starting_coords(std::uint32_t cell);

  /* interaction with maze */
  void add_edge(std::uint32_t from, std::uint32_t to);

  /* DFS */
  void DFS_Helper(std::uint32_t cell, std::array<bool, ROWS * COLS>& visited);
  void DFS(std::uint32_t starting_cell);
};
