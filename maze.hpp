#pragma once

#include <raylib.h>
#include <array>
#include <cstdint>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

#include "game_settings.hpp"

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

class Maze {
 public:
  Maze();

  void move_player(Direction d);
  void update(std::uint16_t y, std::uint16_t x);
  void print();  // for debugging purposes
  void draw(int top_y, int top_x, const Camera& camera);

 private:
  /* variables */
  std::mt19937 rand;
  std::array<std::unordered_set<std::uint32_t>, COLS * COLS> maze{};
  Vector3 pc{0.0, 0.0, 0.0};
  uint32_t maze_start{};
  uint32_t maze_end{};
  std::array<bool, COLS * COLS> visited{};
  const float grid_size_2d{static_cast<float>(MAZE_SIZE_2D) / COLS};
  const float grid_size_3d{static_cast<float>(MAZE_SIZE_3D) / COLS};

  /* general helpers */
  std::vector<std::uint32_t> adjacent_cells(std::uint32_t cell);
  std::pair<int, int> cell_starting_coords(std::uint32_t cell);
  std::pair<int, int> cell_ending_coords(std::uint32_t cell, Direction d);
  std::pair<int, int> exit_coords(std::uint32_t cell, bool start);
  void draw_2d();
  void draw_3d(const Camera& camera);

  /* interaction with maze */
  void add_edge(std::uint32_t from, std::uint32_t to);

  /* DFS */
  void DFS_Helper(std::uint32_t cell, std::array<bool, COLS * COLS>& visited);
  void DFS(std::uint32_t starting_cell);
};
