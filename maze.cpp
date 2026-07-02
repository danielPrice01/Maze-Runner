#include <raylib.h>
#include <algorithm>
#include <iostream>
#include <random>

#include "maze.hpp"

/*
 * public functions
 */

void Maze::print() {
  for (std::uint32_t i = 0; i < ROWS * COLS; ++i) {
    std::cout << i << " : ";
    for (const auto& edge : maze.at(i)) {
      std::cout << edge << " ";
    }
    std::cout << std::endl;
  }
}

void Maze::draw(int top_y, int top_x) {
  // draw outer box
  DrawRectangleLines(top_x, top_y, MAZE_SIZE_2D, MAZE_SIZE_2D, WHITE);

  // iterate through each cell and for each of its adjacent cells if there is no
  // edge between them draw a line
  for (std::uint32_t cell = 0; cell < maze.size(); ++cell) {
    for (const std::uint32_t adj_cell : adjacent_cells(cell)) {
      // skips cells that are less than current cell to avoid redrawing lines
      if (adj_cell > cell && !(maze[cell].count(adj_cell))) {
        int start_y =
            top_y + ((adj_cell / COLS) * static_cast<int>(grid_size.y));
        int start_x =
            top_x + ((adj_cell % COLS) * static_cast<int>(grid_size.x));

        int end_y = start_y;
        int end_x = start_x;

        // cell is to the right, so the line is vertical
        if (adj_cell - cell == 1) {
          end_y += grid_size.y;
        } else {
          end_x += grid_size.x;
        }

        DrawLine(start_x, start_y, end_x, end_y, WHITE);
      }
    }
  }

  int rounded_x =
      top_x + (static_cast<int>(pc.x) -
               (static_cast<int>(pc.x) % static_cast<int>(grid_size.x)));
  int rounded_y =
      top_y + (static_cast<int>(pc.y) -
               (static_cast<int>(pc.y) % static_cast<int>(grid_size.y)));
  int radius = std::min(grid_size.x, grid_size.y) / 2;

  // draw the player location as a circle snapped to grid
  DrawCircle(rounded_x + radius, rounded_y + radius, radius, RED);
}

/*
 * private functions
 */

std::vector<std::uint32_t> Maze::adjacent_cells(std::uint32_t cell) {
  std::vector<std::uint32_t> adj_cells{};
  if (static_cast<std::int64_t>(cell) - 1 >= 0 && cell % COLS != 0)
    adj_cells.push_back(cell - 1);
  if (static_cast<std::int64_t>(cell) + 1 < COLS * ROWS &&
      (cell + 1) % COLS != 0)
    adj_cells.push_back(cell + 1);

  // up, down neighbors
  if (static_cast<std::int64_t>(cell) - COLS >= 0)
    adj_cells.push_back(cell - COLS);
  if (static_cast<std::int64_t>(cell) + COLS < COLS * ROWS)
    adj_cells.push_back(cell + COLS);

  return adj_cells;
}

/* interaction with maze */
void Maze::add_edge(std::uint32_t from, std::uint32_t to) {
  // bidirectional edges
  maze.at(from).insert(to);
  maze.at(to).insert(from);
}

/* DFS */
void Maze::DFS_Helper(std::uint32_t cell,
                      std::array<bool, ROWS * COLS>& visited) {
  visited[cell] = true;

  std::vector<std::uint32_t> adj_cells{adjacent_cells(cell)};

  std::random_device seed;
  std::mt19937 rand(seed());
  std::shuffle(adj_cells.begin(), adj_cells.end(), rand);

  for (const std::uint32_t adj_cell : adj_cells) {
    if (!visited.at(adj_cell)) {
      add_edge(cell, adj_cell);
      DFS_Helper(adj_cell, visited);
    }
  }
}
void Maze::DFS(std::uint32_t starting_cell) {
  visited.fill(false);
  DFS_Helper(starting_cell, visited);
}
