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

    Vector3 old_position = camera.position;
    UpdateCamera(&camera, camera_mode);
    Vector3 camera_movement{
        camera.position.x - old_position.x,
        camera.position.y - old_position.y,
        camera.position.z - old_position.z,
    };
    camera.position = old_position;
    camera.target.x -= camera_movement.x;
    camera.target.y -= camera_movement.y;
    camera.target.z -= camera_movement.z;

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
      m.move_player(Direction::UP, camera);
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
      m.move_player(Direction::DOWN, camera);
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
      m.move_player(Direction::LEFT, camera);
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
      m.move_player(Direction::RIGHT, camera);

    if (IsKeyPressed(KEY_R)) {
      m.update(0, 0);
      m.sync_camera(camera);
    }

    m.draw(10, 10, camera);

    EndDrawing();
  }

done:
  CloseWindow();

  return 0;
}
