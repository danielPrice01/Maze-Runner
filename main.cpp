#include <fcntl.h>
#include <raylib.h>
#include <unistd.h>
#include <cstdio>

#include "game_settings.hpp"
#include "maze.hpp"

int main(int argc, char** argv) {
  Maze m{};
  m.update(0, 0);

  bool verbose{false};
  int flag;
  while ((flag = getopt(argc, argv, "v")) != -1) {
    if (flag == 'v') {
      verbose = 1;
    }
  }

  if (!verbose) {
    int devnull_fd = open("/dev/null", O_WRONLY);
    if (devnull_fd == -1) {
      std::perror("open");
      goto done;
    }

    dup2(devnull_fd, STDOUT_FILENO);
    dup2(devnull_fd, STDERR_FILENO);
    close(devnull_fd);
  }

  InitWindow(WIDTH, HEIGHT, "Maze");
  SetTargetFPS(TARGET_FPS);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (IsKeyPressed(KEY_Q))
      goto done;

    if (IsKeyPressed(KEY_UP) || IsKeyDown(KEY_UP))
      m.move_player(Direction::UP);
    if (IsKeyPressed(KEY_DOWN) || IsKeyDown(KEY_DOWN))
      m.move_player(Direction::DOWN);
    if (IsKeyPressed(KEY_LEFT) || IsKeyDown(KEY_LEFT))
      m.move_player(Direction::LEFT);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyDown(KEY_RIGHT))
      m.move_player(Direction::RIGHT);

    m.draw(300, 400);

    EndDrawing();
  }

done:
  CloseWindow();

  return 0;
}
