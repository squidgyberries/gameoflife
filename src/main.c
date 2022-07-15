#include "colors.h"

#include "raylib.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000

#define BOARD_WIDTH  1000
#define BOARD_HEIGHT 1000

int logLevel = LOG_INFO;
int color = 1;

int play = 0;
int tileSize = 1;

typedef unsigned char Board[BOARD_HEIGHT][BOARD_WIDTH];

Board board1;
Board board2;

Board *currentBoard = &board1;
Board *otherBoard = &board2;

// log function for raylib to use
void logRaylib(int msgType, const char *text, va_list args) {
  if (msgType < logLevel)
    return;

  switch (msgType) {
  case LOG_TRACE:
    if (color)
      printf(ANSI_COLOR_MAGENTA "[TRACE]" ANSI_COLOR_RESET ": ");
    else
      printf("[TRACE]: ");
    break;
  case LOG_DEBUG:
    if (color)
      printf(ANSI_COLOR_MAGENTA "[DEBUG]" ANSI_COLOR_RESET ": ");
    else
      printf("[DEBUG]: ");
    break;
  case LOG_INFO:
    if (color)
      printf(ANSI_COLOR_BLUE "[INFO]" ANSI_COLOR_RESET ": ");
    else
      printf("[INFO]: ");
    break;
  case LOG_WARNING:
    if (color)
      printf(ANSI_COLOR_YELLOW "[WARN]" ANSI_COLOR_RESET ": ");
    else
      printf("[WARN]: ");
    break;
  case LOG_ERROR:
    if (color)
      printf(ANSI_COLOR_RED "[ERROR]" ANSI_COLOR_RESET ": ");
    else
      printf("[ERROR]: ");
    break;
  case LOG_FATAL:
    if (color)
      printf(ANSI_COLOR_RED "[LOG]" ANSI_COLOR_RESET ": ");
    else
      printf("[LOG]: ");
    break;
  default:
    break;
  }

  vprintf(text, args);
  printf("\n");
}

// wrapper around raylib log function for easier use
void logMsg(TraceLogLevel msgType, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logRaylib(msgType, fmt, args);
  va_end(args);
}

// write board to file
void writeBoard(FILE *file, Board *board) {
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH - 1; j++) {
      fprintf(file, "%hhu ", (*board)[i][j]);
    }
    fprintf(file, "%hhu\n", (*board)[i][BOARD_WIDTH - 1]);
  }
}

// load board from file to board
void loadBoard(FILE *file, Board *board) {
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH; j++) {
      fscanf(file, "%hhu", &(*board)[i][j]);
    }
  }
}

void updateBoards() {
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH; j++) {
      int neighbours = 0;
      // up
      if (i > 0 && (*currentBoard)[i - 1][j]) {
        neighbours++;
      }
      // up and right
      if (i > 0 && j < BOARD_WIDTH - 1 && (*currentBoard)[i - 1][j + 1]) {
        neighbours++;
      }
      // right
      if (j < BOARD_WIDTH - 1 && (*currentBoard)[i][j + 1]) {
        neighbours++;
      }
      // down and right
      if (i < BOARD_HEIGHT - 1 && j < BOARD_WIDTH - 1 && (*currentBoard)[i + 1][j + 1]) {
        neighbours++;
      }
      // down
      if (i < BOARD_HEIGHT - 1 && (*currentBoard)[i + 1][j]) {
        neighbours++;
      }
      // down and left
      if (i < BOARD_HEIGHT - 1 && j > 0 && (*currentBoard)[i + 1][j - 1]) {
        neighbours++;
      }
      // left
      if (j > 0 && (*currentBoard)[i][j - 1]) {
        neighbours++;
      }
      // up and left
      if (i > 0 && j > 0 && (*currentBoard)[i - 1][j - 1]) {
        neighbours++;
      }

      if ((*currentBoard)[i][j]) {
        if (neighbours < 2 || neighbours > 3) {
          (*otherBoard)[i][j] = 0;
        } else {
          (*otherBoard)[i][j] = 1;
        }
      } else {
        if (neighbours == 3) {
          (*otherBoard)[i][j] = 1;
        } else {
          (*otherBoard)[i][j] = 0;
        }
      }
    }
  }

  // swap boards
  Board *temp = currentBoard;
  currentBoard = otherBoard;
  otherBoard = temp;
}

int main(int argc, char **argv) {
  // command line arguments
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--logLevel")) {
      if (i < argc - 1) {
        if (sscanf(argv[i + 1], "%d", &logLevel) == 1) {
          i++;
        } else {
          logMsg(LOG_WARNING, "No value specified for logLevel, defaulting to %d", logLevel);
        }
      } else {
        logMsg(LOG_WARNING, "No value specified for logLevel, defaulting to %d", logLevel);
      }
    } else if (!strcmp(argv[i], "--no-color")) {
      color = 0;
    } else {
      logMsg(LOG_WARNING, "Unrecognized argument \"%s\", skipping", argv[i]);
    }
  }

  // raylib setup
  SetTraceLogCallback(logRaylib);

  // window
  SetConfigFlags(FLAG_VSYNC_HINT);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game of Life");

  // load board
  FILE *boardFile = fopen("board.txt", "r");
  if (boardFile == NULL) {
    logMsg(LOG_ERROR, "Opening board failed");
    CloseWindow();
    return EXIT_FAILURE;
  }
  loadBoard(boardFile, currentBoard);
  fclose(boardFile);

  // tick stuff
  double tickRate = 5.0;
  const double tickTime = 1.0 / tickRate;
  double tickTimer = 0.0;

  // game loop
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_SPACE)) {
      play = !play;
    }

    // tick
    tickTimer += GetFrameTime();
    while (tickTimer >= tickTime) {
      if (play) {
        updateBoards();
      }
      tickTimer -= tickTime;
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);

    // draw board
    for (int i = 0; i < BOARD_HEIGHT; i++) {
      for (int j = 0; j < BOARD_WIDTH; j++) {
        if ((*currentBoard)[i][j]) {
          DrawRectangle(j * tileSize, i * tileSize, tileSize, tileSize, BLACK);
        }
      }
    }

    EndDrawing();
  }

  CloseWindow();

  return EXIT_SUCCESS;
}