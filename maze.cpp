#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "maze.hpp"

namespace {
int upper_y = 0;
int upper_x = 0;

Direction border_direction(std::uint32_t cell) {
  if (cell % COLS == 0)
    return LEFT;
  if ((cell + 1) % COLS == 0)
    return RIGHT;
  if (cell < COLS)
    return UP;
  return DOWN;
}

Direction direction_between(std::uint32_t from, std::uint32_t to) {
  int diff = static_cast<int>(to) - static_cast<int>(from);
  if (diff == -static_cast<int>(COLS))
    return UP;
  if (diff == static_cast<int>(COLS))
    return DOWN;
  if (diff == -1)
    return LEFT;
  if (diff == 1)
    return RIGHT;

  throw std::runtime_error("Invalid Direction");
}
}  // namespace

/*
 * constructor
 */
Maze::Maze() : rand(std::random_device{}()) {}

/*
 * public functions
 */

void Maze::move_player(Direction d, Camera& camera) {
  Vector3 forward = {
      camera.target.x - camera.position.x,
      0.0f,
      camera.target.z - camera.position.z,
  };
  float forward_len = sqrtf(forward.x * forward.x + forward.z * forward.z);

  if (forward_len < 0.0001f)
    return;

  forward.x /= forward_len;
  forward.z /= forward_len;

  Vector3 right{forward.z, 0.0f, -forward.x};

  Vector3 movement{};
  switch (d) {
    case UP:
      movement = forward;
      break;
    case DOWN:
      movement = Vector3{-forward.x, 0.0f, -forward.z};
      break;
    case LEFT:
      movement = Vector3{-right.x, 0.0f, -right.z};
      break;
    case RIGHT:
      movement = right;
      break;
    default:
      throw std::runtime_error("Invalid movement direction");
  }

  float step = PLAYER_GRID_SPEED;
  const Vector3 old_pc{pc};

  // Moves one axis at a time in order to prevent diagonal corner clipping. Also
  // allows player to slide along the walls.
  auto try_axis_move = [this](float delta, bool x_axis) {
    if (std::fabs(delta) <= 0.0001f)
      return;

    const std::uint32_t before = player_cell();
    Vector3 candidate = pc;

    if (x_axis)
      candidate.x += delta;
    else
      candidate.z += delta;

    float epsilon = 0.001f;
    candidate.x = std::clamp(candidate.x, 0.0f, MAZE_SIZE_3D - epsilon);
    candidate.z = std::clamp(candidate.z, 0.0f, MAZE_SIZE_3D - epsilon);

    int row =
        std::clamp(static_cast<int>(candidate.z / grid_size_3d), 0, COLS - 1);
    int col =
        std::clamp(static_cast<int>(candidate.x / grid_size_3d), 0, COLS - 1);
    std::uint32_t after = static_cast<std::uint32_t>(row * COLS + col);

    if (after == before || can_cross(before, after))
      pc = candidate;
  };

  try_axis_move(movement.x * step, true);
  try_axis_move(movement.z * step, false);
  (void)old_pc;
  sync_camera(camera);
}

void Maze::update(std::uint16_t y, std::uint16_t x) {
  visited.fill(false);
  for (auto& edges : maze)
    edges.clear();

  if (y >= COLS)
    throw std::runtime_error("Maze update: coordinates out of bounds.");

  if (x >= COLS)
    throw std::runtime_error("Maze update: coordinates out of bounds.");

  if (y != 0 && y != COLS - 1 && x != 0 && x != COLS - 1)
    throw std::runtime_error(
        "Maze update: invalid coordinates for start - they do not fall along "
        "the maze wall");

  std::uint32_t starting_cell = (y * COLS) + x;
  DFS(starting_cell);

  maze_start = starting_cell;

  // besides the starting cell, all edge cells are valid ending cells
  std::vector<std::uint32_t> possible_ending_cells{};
  possible_ending_cells.reserve(4 * COLS);

  for (std::uint32_t c = 0; c < COLS; ++c) {
    possible_ending_cells.push_back(c);
    possible_ending_cells.push_back((COLS - 1) * COLS + c);
  }

  for (std::uint32_t r = 1; r + 1 < COLS; ++r) {
    possible_ending_cells.push_back(r * COLS);
    possible_ending_cells.push_back(r * COLS + COLS - 1);
  }

  std::erase(possible_ending_cells, starting_cell);

  std::uniform_int_distribution<std::size_t> dist(
      0, possible_ending_cells.size() - 1);

  maze_end = possible_ending_cells[dist(rand)];

  // Spawn player at center of requested starting cell
  pc.x = (static_cast<float>(x) + 0.5f) * grid_size_3d;
  pc.y = 0.05f;
  pc.z = (static_cast<float>(y) + 0.5f) * grid_size_3d;
}

void Maze::print() {
  std::cout << "Format:\n Cellno: Neighbors\n";
  for (std::uint32_t i = 0; i < COLS * COLS; ++i) {
    std::cout << i << " : ";
    for (const auto& edge : maze.at(i)) {
      std::cout << edge << " ";
    }
    std::cout << "\n";
  }

  std::cout << "Starting cell: " << maze_start << std::endl;
  std::cout << "Ending cell: " << maze_end << std::endl;
}

void Maze::draw(int top_y, int top_x, const Camera& camera) {
  upper_y = top_y;
  upper_x = top_x;

  draw_3d(camera);
  draw_2d();
}

/*
 * private functions
 */

std::vector<std::uint32_t> Maze::adjacent_cells(std::uint32_t cell) {
  std::vector<std::uint32_t> adj_cells{};

  if (cell % COLS != 0)
    adj_cells.push_back(cell - 1);
  if ((cell + 1) % COLS != 0)
    adj_cells.push_back(cell + 1);
  if (cell >= COLS)
    adj_cells.push_back(cell - COLS);
  if (cell + COLS < COLS * COLS)
    adj_cells.push_back(cell + COLS);

  return adj_cells;
}

std::pair<int, int> Maze::cell_starting_coords(std::uint32_t cell) {
  return {upper_x + ((cell % COLS) * static_cast<int>(grid_size_2d)),
          upper_y + ((cell / COLS) * static_cast<int>(grid_size_2d))};
}

std::pair<int, int> Maze::cell_ending_coords(std::uint32_t cell, Direction d) {
  auto [x, y] = cell_starting_coords(cell);

  switch (d) {
    case UP:
    case DOWN:
      x += grid_size_2d;
      break;
    case LEFT:
    case RIGHT:
      y += grid_size_2d;
      break;
    default:
      throw std::runtime_error("Invalid direction");
  }

  return std::pair{x, y};
}

std::pair<int, int> Maze::exit_coords(std::uint32_t cell, bool start) {
  Direction d = border_direction(cell);
  auto [start_x, start_y] = cell_starting_coords(cell);
  auto [end_x, end_y] = cell_ending_coords(cell, d);

  if (start) {
    if (d == UP)
      return std::pair{start_x, start_y + 1};
    if (d == DOWN)
      return std::pair{start_x, end_y + grid_size_2d};
    if (d == LEFT)
      return std::pair{start_x + 1, start_y};
    if (d == RIGHT)
      return std::pair{end_x + grid_size_2d, start_y};
  }

  if (d == UP)
    return std::pair{end_x, end_y + 1};
  if (d == DOWN)
    return std::pair{end_x, end_y + grid_size_2d};
  if (d == LEFT)
    return std::pair{end_x, end_y};
  if (d == RIGHT)
    return std::pair{end_x + grid_size_2d, end_y};

  throw std::runtime_error("Invalid parameters");
}

void Maze::draw_2d() {
  // draw black box for the 2d map
  DrawRectangleV(Vector2{.x = static_cast<float>(upper_x),
                         .y = static_cast<float>(upper_y)},
                 Vector2{.x = static_cast<float>(MAZE_SIZE_2D),
                         .y = static_cast<float>(MAZE_SIZE_2D)},
                 BLACK);

  // draw outer box
  DrawRectangleLines(upper_x, upper_y, MAZE_SIZE_2D, MAZE_SIZE_2D, WHITE);

  // iterate through each cell and for each of its adjacent cells if there is
  // no edge between them draw a line
  for (std::uint32_t cell = 0; cell < maze.size(); ++cell) {
    for (const std::uint32_t adj_cell : adjacent_cells(cell)) {
      // skips cells that are less than current cell to avoid redrawing lines
      if (adj_cell > cell && !(maze[cell].count(adj_cell))) {
        auto [start_x, start_y] = cell_starting_coords(adj_cell);

        auto [end_x, end_y] =
            cell_ending_coords(adj_cell, direction_between(cell, adj_cell));

        DrawLine(start_x, start_y, end_x, end_y, WHITE);
      }
    }
  }

  // draw a black line over start and end
  auto [start_x1, start_y1] = exit_coords(maze_start, true);
  auto [start_x2, start_y2] = exit_coords(maze_start, false);
  DrawLine(start_x1, start_y1, start_x2, start_y2, BLACK);

  auto [end_x1, end_y1] = exit_coords(maze_end, true);
  auto [end_x2, end_y2] = exit_coords(maze_end, false);
  DrawLine(end_x1, end_y1, end_x2, end_y2, BLACK);

  int col = std::clamp(static_cast<int>(pc.x / grid_size_3d), 0, COLS - 1);
  int row = std::clamp(static_cast<int>(pc.z / grid_size_3d), 0, COLS - 1);

  float map_x = static_cast<float>(upper_x) +
                (static_cast<float>(col) + 0.5f) * grid_size_2d;
  float map_y = static_cast<float>(upper_y) +
                (static_cast<float>(row) + 0.5f) * grid_size_2d;

  // draw the player location as a circle snapped to grid
  DrawCircleV(Vector2{map_x, map_y}, grid_size_2d * 0.3f, RED);
}

void Maze::draw_3d(const Camera& camera) {
  BeginMode3D(camera);

  float half_maze = MAZE_SIZE_3D * 0.5f;

  DrawPlane(Vector3{half_maze, 0.0f, half_maze},
            Vector2{MAZE_SIZE_3D, MAZE_SIZE_3D}, LIGHTGRAY);

  auto draw_wall_x = [this](float x, float z) {
    DrawCube(Vector3{x, WALL_HEIGHT * 0.5f, z}, grid_size_3d + WALL_THICKNESS,
             WALL_HEIGHT, WALL_THICKNESS, DARKGRAY);
  };

  auto draw_wall_z = [this](float x, float z) {
    DrawCube(Vector3{x, WALL_HEIGHT * 0.5f, z}, WALL_THICKNESS, WALL_HEIGHT,
             grid_size_3d + WALL_THICKNESS, DARKGRAY);
  };

  // Interior walls; draw right and down walls to avoid redrawing
  for (std::uint32_t cell = 0; cell < maze.size(); ++cell) {
    const std::uint32_t row = cell / COLS;
    const std::uint32_t col = cell % COLS;

    const float cell_x = static_cast<float>(col) * grid_size_3d;
    const float cell_z = static_cast<float>(row) * grid_size_3d;

    if (col + 1 < COLS) {
      const std::uint32_t right = cell + 1;
      if (!maze[cell].count(right)) {
        draw_wall_z(cell_x + grid_size_3d, cell_z + grid_size_3d * 0.5f);
      }
    }

    if (row + 1 < COLS) {
      const std::uint32_t down = cell + COLS;
      if (!maze[cell].count(down)) {
        draw_wall_x(cell_x + grid_size_3d * 0.5f, cell_z + grid_size_3d);
      }
    }
  }

  // Outer walls, leaving openings for start and exit
  for (std::uint32_t i = 0; i < COLS; ++i) {
    const std::uint32_t top_cell = i;
    const std::uint32_t bottom_cell = (COLS - 1) * COLS + i;
    const std::uint32_t left_cell = i * COLS;
    const std::uint32_t right_cell = i * COLS + COLS - 1;

    const float center = (static_cast<float>(i) + 0.5f) * grid_size_3d;

    if (top_cell != maze_start && top_cell != maze_end)
      draw_wall_x(center, 0.0f);
    if (bottom_cell != maze_start && bottom_cell != maze_end)
      draw_wall_x(center, MAZE_SIZE_3D);
    if (left_cell != maze_start && left_cell != maze_end)
      draw_wall_z(0.0f, center);
    if (right_cell != maze_start && right_cell != maze_end)
      draw_wall_z(MAZE_SIZE_3D, center);
  }

  EndMode3D();
}

std::uint32_t Maze::player_cell() {
  int row = std::clamp(static_cast<int>(pc.z / grid_size_3d), 0, COLS - 1);
  int col = std::clamp(static_cast<int>(pc.x / grid_size_3d), 0, COLS - 1);

  return static_cast<std::uint32_t>(row * COLS + col);
}

bool Maze::can_cross(std::uint32_t from, std::uint32_t to) {
  if (from >= maze.size() || to >= maze.size())
    return false;

  int from_row = static_cast<int>(from / COLS);
  int from_col = static_cast<int>(from % COLS);
  int to_row = static_cast<int>(to / COLS);
  int to_col = static_cast<int>(to % COLS);

  if (std::abs(from_row - to_row) + std::abs(from_col - to_col) != 1)
    return false;

  return maze[from].count(to) != 0;
}

void Maze::sync_camera(Camera& camera) {
  const Vector3 old_position = camera.position;
  const Vector3 offset{
      camera.target.x - old_position.x,
      camera.target.y - old_position.y,
      camera.target.z - old_position.z,
  };

  camera.position.x = pc.x;
  camera.position.y = PLAYER_EYE_HEIGHT;
  camera.position.z = pc.z;

  camera.target = Vector3{
      camera.position.x + offset.x,
      camera.position.y + offset.y,
      camera.position.z + offset.z,
  };
}

/* interaction with maze */
void Maze::add_edge(std::uint32_t from, std::uint32_t to) {
  // bidirectional edges
  maze.at(from).insert(to);
  maze.at(to).insert(from);
}

/* DFS */
void Maze::DFS_Helper(std::uint32_t cell,
                      std::array<bool, COLS * COLS>& visited) {
  visited[cell] = true;

  std::vector<std::uint32_t> adj_cells{adjacent_cells(cell)};

  std::shuffle(adj_cells.begin(), adj_cells.end(), rand);

  for (const std::uint32_t adj_cell : adj_cells) {
    if (!visited.at(adj_cell)) {
      add_edge(cell, adj_cell);
      DFS_Helper(adj_cell, visited);
    }
  }
}
void Maze::DFS(std::uint32_t starting_cell) {
  DFS_Helper(starting_cell, visited);
}
