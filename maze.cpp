#include <raylib.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "maze.hpp"

int upper_y, upper_x;

/*
 * constructor
 */
Maze::Maze() : rand(std::random_device{}()) {}

/*
 * public functions
 */

void Maze::move_player(Direction d) {
  // TODO using the players angle get a movement for all directions (i.e. if
  // player is facing 45 degrees then w should move then equally forwards and
  // sideways)
  // then for depicting where the character is, multiply the pc location by the
  // grid size of that dimension
  // the logic of this needs to be totally rethought out. First calculate the x
  // and y displacment based on the angle and the direction of movement. Then
  // clamp the x and y directions (or x, z directions?) based on where there are
  // walls
  // I think that the camera "angle" is probably found using the camera target?
  // (I think it just changes the x position) (between 3 and -3?)
  // also need to decide how thick each wall will be and put that into the
  // calcuation somewhere as well

  // calculate player angle. calculate x, y displacement based on angle (i think
  // cos?). calculate new x,y coordinates. clamp x,y coordinates based on grid
  // borders

  // talking to claude; get the direction vector (relation between target and
  // position), and normalize this. Then scalar multiply each by speed and add
  // it to pc. After that, check whether there is a border at either direction
  // and clamp.
  Vector3 original_pc{pc};

  std::uint32_t init_cell =
      (static_cast<std::uint32_t>(pc.y / grid_size_2d) * COLS) +
      static_cast<std::uint32_t>(pc.x / grid_size_2d);

  int speed_y = static_cast<int>(PLAYER_GRID_SPEED * grid_size_2d);
  int speed_x = static_cast<int>(PLAYER_GRID_SPEED * grid_size_2d);

  switch (d) {
    case UP:
      pc.y = (pc.y - speed_y < 0) ? 0 : pc.y - speed_y;
      break;
    case DOWN:
      pc.y = (pc.y + speed_y >= MAZE_SIZE_2D - grid_size_2d)
                 ? MAZE_SIZE_2D - grid_size_2d
                 : pc.y + speed_y;
      break;
    case LEFT:
      pc.x = (pc.x - speed_x < 0) ? 0 : pc.x - speed_x;
      break;
    case RIGHT:
      pc.x = (pc.x + speed_x >= MAZE_SIZE_2D - grid_size_2d)
                 ? MAZE_SIZE_2D - grid_size_2d
                 : pc.x + speed_x;
      break;
    default:
      std::runtime_error("Invalid movement");
  }

  std::uint32_t end_cell =
      (static_cast<std::uint32_t>(pc.y / grid_size_2d) * COLS) +
      static_cast<std::uint32_t>(pc.x / grid_size_2d);

  if (end_cell != init_cell && !(maze.at(init_cell).count(end_cell)))
    pc = original_pc;
}

void Maze::update(std::uint16_t y, std::uint16_t x) {
  pc = Vector3{0.0f, 0.0f, 0.0f};
  visited.fill(false);
  for (size_t i = 0; i < maze.size(); ++i)
    maze[i].clear();

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
  possible_ending_cells.resize(COLS);

  std::iota(possible_ending_cells.begin(), possible_ending_cells.end(), 0u);
  possible_ending_cells.resize(2 * COLS);
  std::iota(possible_ending_cells.begin() + COLS, possible_ending_cells.end(),
            static_cast<std::uint32_t>((COLS - 1) * COLS));

  possible_ending_cells.reserve(possible_ending_cells.size() + (2 * COLS));
  for (std::uint32_t r = 0; r < COLS; ++r) {
    possible_ending_cells.push_back(r * COLS);
    possible_ending_cells.push_back((r * COLS) + COLS - 1);
  }

  std::erase(possible_ending_cells, starting_cell);

  std::uniform_int_distribution<std::size_t> dist(
      0, possible_ending_cells.size() - 1);

  maze_end = possible_ending_cells[dist(rand)];
}

void Maze::print() {
  std::cout << "Format:\n Cellno: Neighbors" << std::endl;
  for (std::uint32_t i = 0; i < COLS * COLS; ++i) {
    std::cout << i << " : ";
    for (const auto& edge : maze.at(i)) {
      std::cout << edge << " ";
    }
    std::cout << std::endl;
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
  if (static_cast<std::int64_t>(cell) - 1 >= 0 && cell % COLS != 0)
    adj_cells.push_back(cell - 1);
  if (static_cast<std::int64_t>(cell) + 1 < COLS * COLS &&
      (cell + 1) % COLS != 0)
    adj_cells.push_back(cell + 1);

  // up, down neighbors
  if (static_cast<std::int64_t>(cell) - COLS >= 0)
    adj_cells.push_back(cell - COLS);
  if (static_cast<std::int64_t>(cell) + COLS < COLS * COLS)
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
  // RLAPI void DrawRectangleRecV(Vector2 start, Vector2 width, Color color);
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

  int rounded_x =
      upper_x + (static_cast<int>(pc.x) -
                 (static_cast<int>(pc.x) % static_cast<int>(grid_size_2d)));
  int rounded_y =
      upper_y + (static_cast<int>(pc.y) -
                 (static_cast<int>(pc.y) % static_cast<int>(grid_size_2d)));
  int radius = grid_size_2d / 2;

  // draw the player location as a circle snapped to grid
  DrawCircle(rounded_x + radius, rounded_y + radius, radius, RED);
}

void Maze::draw_3d(const Camera& camera) {
  BeginMode3D(camera);

  DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{MAZE_SIZE_3D, MAZE_SIZE_3D},
            LIGHTGRAY);

  EndMode3D();
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
