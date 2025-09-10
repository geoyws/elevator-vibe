#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define FLOORS 4
#define FLOOR_HEIGHT 120
#define ELEVATOR_WIDTH 80
#define ELEVATOR_HEIGHT 100
#define BUTTON_SIZE 30
#define VIRTUAL_WIDTH 800
#define VIRTUAL_HEIGHT 600
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PAUSE_TIME 5.0f

typedef struct
{
  int floor;
  int direction; // 1 up, -1 down
} Request;

typedef struct
{
  int floor;
  float y;
  bool moving;
  int direction;
  float speed;
  float pause_timer;
  bool paused;
  float door_position; // 0 closed, 1 open
} Elevator;

Request request_queue[100];
int request_queue_size = 0;

int target_floors[5] = {0};        // 1 to 4
bool floor_buttons[FLOORS + 1][2]; // floor 1-4, 0 up 1 down

Elevator elevator;

void init_elevator()
{
  elevator.floor = 1;
  elevator.y = WINDOW_HEIGHT - FLOOR_HEIGHT - ELEVATOR_HEIGHT;
  elevator.moving = false;
  elevator.direction = 0;
  elevator.speed = 2.0f;
  elevator.pause_timer = 0.0f;
  elevator.paused = false;
  elevator.door_position = 0.0f;
}

void add_request(int floor, int dir)
{
  // Check if already in queue
  for (int i = 0; i < request_queue_size; i++)
  {
    if (request_queue[i].floor == floor && request_queue[i].direction == dir)
      return;
  }
  request_queue[request_queue_size].floor = floor;
  request_queue[request_queue_size].direction = dir;
  request_queue_size++;
}

void add_target(int floor)
{
  if (target_floors[floor] == 0)
  {
    target_floors[floor] = 1;
  }
}

void remove_request(int floor, int dir)
{
  for (int i = 0; i < request_queue_size; i++)
  {
    if (request_queue[i].floor == floor && request_queue[i].direction == dir)
    {
      for (int j = i; j < request_queue_size - 1; j++)
      {
        request_queue[j] = request_queue[j + 1];
      }
      request_queue_size--;
      break;
    }
  }
}

void update_elevator()
{
  if (elevator.paused)
  {
    // Check if any inside button pressed
    bool has_target = false;
    for (int i = 1; i <= FLOORS; i++)
    {
      if (target_floors[i])
      {
        has_target = true;
        break;
      }
    }
    if (has_target)
    {
      elevator.paused = false;
      elevator.pause_timer = 0.0f;
      elevator.door_position = 0.0f; // Close doors when inside button pressed
    }
    else
    {
      // Animate doors opening
      elevator.door_position = fmin(1.0f, elevator.door_position + 0.05f);
      elevator.pause_timer += GetFrameTime();
      if (elevator.pause_timer >= PAUSE_TIME)
      {
        elevator.paused = false;
        elevator.pause_timer = 0.0f;
        // Process next request
        if (request_queue_size > 0)
        {
          int next_floor = request_queue[0].floor;
          add_target(next_floor);
          remove_request(next_floor, request_queue[0].direction);
        }
      }
    }
    return;
  }

  // Process queue if idle
  if (!elevator.moving && !elevator.paused && request_queue_size > 0)
  {
    int next_floor = request_queue[0].floor;
    add_target(next_floor);
    remove_request(next_floor, request_queue[0].direction);
  }

  // Check if any target floors
  int next_target = 0;
  for (int i = 1; i <= FLOORS; i++)
  {
    if (target_floors[i])
    {
      next_target = i;
      break;
    }
  }

  if (next_target == 0)
  {
    // No targets, go to floor 1
    if (elevator.floor != 1)
    {
      next_target = 1;
      add_target(1);
    }
    else
    {
      elevator.moving = false;
      return;
    }
  }

  if (next_target != elevator.floor)
  {
    elevator.moving = true;
    elevator.door_position = 0.0f; // Close doors when moving
    float target_y = WINDOW_HEIGHT - (next_target * FLOOR_HEIGHT) - ELEVATOR_HEIGHT;
    elevator.direction = (target_y < elevator.y) ? -1 : 1;
    elevator.y += elevator.direction * elevator.speed;
    if ((elevator.direction == -1 && elevator.y <= target_y) ||
        (elevator.direction == 1 && elevator.y >= target_y))
    {
      elevator.y = target_y;
      elevator.floor = next_target;
      elevator.moving = false;
      elevator.paused = true;
      elevator.door_position = 0.0f;
      target_floors[next_target] = 0;
      // Clear floor button
      if (next_target == 1)
        floor_buttons[1][0] = false;
      else if (next_target == FLOORS)
        floor_buttons[FLOORS][1] = false;
      else
      {
        floor_buttons[next_target][0] = false;
        floor_buttons[next_target][1] = false;
      }
    }
  }
  else
  {
    elevator.moving = false;
  }
}

void draw_building()
{
  // Draw floors
  for (int i = 1; i <= FLOORS; i++)
  {
    int y = WINDOW_HEIGHT - i * FLOOR_HEIGHT;
    DrawRectangle(200, y, 400, FLOOR_HEIGHT, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(200, y, 400, FLOOR_HEIGHT, BLACK);
    DrawText(TextFormat("Floor %d", i), 450, y + 15, 24, BLACK);
    // Draw buttons
    if (i == 1)
    {
      // Up button
      Color color = floor_buttons[i][0] ? GREEN : ORANGE;
      DrawRectangle(150, y + 40, BUTTON_SIZE, BUTTON_SIZE, color);
      DrawText("UP", 155, y + 45, 10, WHITE);
    }
    else if (i == FLOORS)
    {
      // Down button
      Color color = floor_buttons[i][1] ? GREEN : ORANGE;
      DrawRectangle(150, y + 40, BUTTON_SIZE, BUTTON_SIZE, color);
      DrawText("DN", 155, y + 45, 10, WHITE);
    }
    else
    {
      // Up and Down
      Color color_up = floor_buttons[i][0] ? GREEN : ORANGE;
      DrawRectangle(120, y + 40, BUTTON_SIZE, BUTTON_SIZE, color_up);
      DrawText("UP", 125, y + 45, 10, WHITE);
      Color color_down = floor_buttons[i][1] ? GREEN : ORANGE;
      DrawRectangle(180, y + 40, BUTTON_SIZE, BUTTON_SIZE, color_down);
      DrawText("DN", 185, y + 45, 10, WHITE);
    }
  }
  // Draw elevator shaft
  DrawRectangle(300, 0, ELEVATOR_WIDTH, WINDOW_HEIGHT, GRAY);
  // Draw elevator
  DrawRectangle(300, elevator.y, ELEVATOR_WIDTH, ELEVATOR_HEIGHT, YELLOW);
  // Draw doors
  float door_gap = ELEVATOR_WIDTH / 2 * elevator.door_position;
  DrawRectangle(300, elevator.y, ELEVATOR_WIDTH / 2 - door_gap / 2, ELEVATOR_HEIGHT, DARKGRAY);
  DrawRectangle(300 + ELEVATOR_WIDTH / 2 + door_gap / 2, elevator.y, ELEVATOR_WIDTH / 2 - door_gap / 2, ELEVATOR_HEIGHT, DARKGRAY);
  DrawText("🚪", 315, elevator.y + 30, 30, BLACK);
}

void draw_control_panel()
{
  DrawRectangle(650, 100, 120, 200, Fade(PINK, 0.7f));
  DrawText("Elevator", 660, 110, 20, DARKBLUE);
  for (int i = FLOORS; i >= 1; i--)
  {
    int button_index = FLOORS - i;
    int y = 140 + button_index * 40;
    Color color = target_floors[i] ? GREEN : SKYBLUE;
    DrawRectangle(660, y, 30, 30, color);
    DrawText(TextFormat("%d", i), 670, y + 5, 20, BLACK);
  }
}

void draw_queue()
{
  DrawText("Queue:", 50, 50, 20, DARKBLUE);
  for (int i = 0; i < request_queue_size; i++)
  {
    char dir_str[10];
    if (request_queue[i].direction == 1)
      strcpy(dir_str, "UP");
    else
      strcpy(dir_str, "DOWN");
    DrawText(TextFormat("Floor %d %s", request_queue[i].floor, dir_str), 50, 80 + i * 30, 18, BLACK);
  }
}

void handle_input()
{
  Vector2 mouse = GetMousePosition();
  // Scale mouse position for virtual resolution
  mouse.x = mouse.x * VIRTUAL_WIDTH / GetScreenWidth();
  mouse.y = mouse.y * VIRTUAL_HEIGHT / GetScreenHeight();
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    // Check floor buttons
    for (int i = 1; i <= FLOORS; i++)
    {
      int y = WINDOW_HEIGHT - i * FLOOR_HEIGHT;
      if (i == 1)
      {
        Rectangle rect = {150, y + 40, BUTTON_SIZE, BUTTON_SIZE};
        if (CheckCollisionPointRec(mouse, rect))
        {
          floor_buttons[i][0] = true;
          add_request(i, 1);
        }
      }
      else if (i == FLOORS)
      {
        Rectangle rect = {150, y + 40, BUTTON_SIZE, BUTTON_SIZE};
        if (CheckCollisionPointRec(mouse, rect))
        {
          floor_buttons[i][1] = true;
          add_request(i, -1);
        }
      }
      else
      {
        Rectangle rect_up = {120, y + 40, BUTTON_SIZE, BUTTON_SIZE};
        if (CheckCollisionPointRec(mouse, rect_up))
        {
          floor_buttons[i][0] = true;
          add_request(i, 1);
        }
        Rectangle rect_down = {180, y + 40, BUTTON_SIZE, BUTTON_SIZE};
        if (CheckCollisionPointRec(mouse, rect_down))
        {
          floor_buttons[i][1] = true;
          add_request(i, -1);
        }
      }
    }
    // Check elevator buttons
    for (int i = FLOORS; i >= 1; i--)
    {
      int button_index = FLOORS - i;
      int y = 140 + button_index * 40;
      Rectangle rect = {660, y, 30, 30};
      if (CheckCollisionPointRec(mouse, rect))
      {
        add_target(i);
      }
    }
  }
}

int main()
{
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Elevator Simulation");
  SetTargetFPS(60);
  init_elevator();

  RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

  while (!WindowShouldClose())
  {
    handle_input();
    update_elevator();

    BeginTextureMode(target);
    ClearBackground(RAYWHITE);
    draw_building();
    draw_control_panel();
    draw_queue();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture, (Rectangle){0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT}, (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, (Vector2){0, 0}, 0, WHITE);
    EndDrawing();
  }

  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
