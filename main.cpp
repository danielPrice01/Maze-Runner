#include <fcntl.h>
#include <raylib.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

#include "game_settings.hpp"
#include "maze.hpp"

int main(int argc, char** argv) {
  Maze m{};

  Camera camera{.position = Vector3{0.0f, 2.0f, 1.0f},
                .target = Vector3{0.0f, 2.0f, 0.0f},
                .up = Vector3{0.0f, 1.0f, 0.0f},
                .fovy = 60.0f,
                .projection = CAMERA_PERSPECTIVE};

  int camera_mode = CAMERA_FIRST_PERSON;

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
  DisableCursor();

  m.update(0, 0);

  if (verbose)
    m.print();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (IsKeyPressed(KEY_Q))
      goto done;

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
      m.move_player(Direction::UP);
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
      m.move_player(Direction::DOWN);
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
      m.move_player(Direction::LEFT);
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
      m.move_player(Direction::RIGHT);
    if (IsKeyPressed(KEY_R))
      m.update(0, 0);

    // update Camera.position to be equal to pc.x, pc.y, pc.z scaled by 3d grid
    // size
    UpdateCamera(&camera, camera_mode);

    std::cout << camera.target.x << " " << camera.target.y << " "
              << camera.target.z << std::endl;

    m.draw(10, 10, camera);

    EndDrawing();
  }

done:
  CloseWindow();

  return 0;
}
