#include "raylib.h"
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

struct Cell {
  bool isStart = false;
  bool isbarrier = false;
  bool isEnd = false;
  bool isVisidted = false;
  bool isOpen = false;
  bool isPath = false;
  Vector2 position = {0.f, 0.f};
  int row = 0;
  int col = 0;
  int gridSize = 0;

  int hScore = std::numeric_limits<int>::max();
  int gScore = std::numeric_limits<int>::max();

  Cell *parent = nullptr;

  std::vector<Cell *> neighbours;
  Cell(Vector2 position, int row, int col, int gridSize)
      : position(position), row(row), col(col), gridSize(gridSize) {}
  void UpdateNeigbours(std::vector<Cell> &grid) {
    if (row < gridSize - 1) {
      Cell &curr = (grid[(gridSize * (row + 1)) + col]);
      if (!curr.isbarrier) {
        neighbours.push_back(&curr);
      }
    }
    if (row > 0) {
      Cell &curr = (grid[(gridSize * (row - 1)) + col]);
      if (!curr.isbarrier) {
        neighbours.push_back(&curr);
      }
    }
    if (col < gridSize - 1) {
      Cell &curr = (grid[(gridSize * row) + (col + 1)]);
      if (!curr.isbarrier) {
        neighbours.push_back(&curr);
      }
    }
    if (col > 0) {
      Cell &curr = (grid[(gridSize * row) + col - 1]);
      if (!curr.isbarrier) {
        neighbours.push_back(&curr);
      }
    }
  }

  void Update() { _fScore = gScore + hScore; }
  int GetFScore() const { return _fScore; }

private:
  int _fScore = std::numeric_limits<int>::max();
};

struct CompareNode {
  bool operator()(const Cell *a, const Cell *b) {
    if (a->GetFScore() == b->GetFScore()) {
      return a->GetFScore() > b->GetFScore();
    }
    return a->GetFScore() > b->GetFScore();
  }
};

int Heuristic(Cell *start, Cell *end) {

  int x1 = start->position.x;
  int y1 = start->position.y;
  int x2 = end->position.x;
  int y2 = end->position.y;

  return abs(x1 - x2) + abs(y1 - y2);
}

float CreateGrid(std::vector<Cell> &grid, int gridSize, int windowWidth) {
  int cellWidth = windowWidth / gridSize;

  for (int row = 0; row < gridSize; row++) {
    for (int col = 0; col < gridSize; col++) {
      grid.emplace_back(Cell({static_cast<float>(col * cellWidth),
                              static_cast<float>(row * cellWidth)},
                             row, col, gridSize));
    }
  }
  return cellWidth;
}

bool FillGridFromFile(const std::string &path, std::vector<Cell> &grid,
                      int gridSize) {
  std::cout << path << "\n";

  if (!std::filesystem::exists(path)) {
    std::cerr << "File does not exist: " << path << "\n";
    return false;
  }
  std::ifstream mazeFile(path);
  if (!mazeFile.is_open()) {
    return false;
  }

  std::vector<std::string> maze = {};
  std::string line;
  while (std::getline(mazeFile, line)) {
    maze.push_back(line);
  }

  for (size_t row = 0; row < maze.size(); row++) {
    for (size_t col = 0; col < maze[row].length(); col++) {
      std::string &string = maze[row];
      Cell &currCell = grid[(row * gridSize) + col];

      if (string[col] == '#') {
        currCell.isbarrier = true;
      } else if (string[col] == 's') {
        currCell.isStart = true;
      } else if (string[col] == 'e') {
        currCell.isEnd = true;
      }
    }
  }
  return true;
}
void ReconstructPath(Cell *end, Cell *start, std::function<void()> &render) {
  Cell *curr = end->parent;

  while (curr != start) {
    curr->isPath = true;
    curr = curr->parent;
    render();
  }
}

bool PathFind(std::vector<Cell> &grid, Cell *start, Cell *end,
              std::function<void()> &render) {

  if (!start || !end) {
    return false;
  }

  std::priority_queue<Cell *, std::vector<Cell *>, CompareNode> openSet;
  start->hScore = Heuristic(start, end);
  start->gScore = 0;
  start->Update();
  openSet.push(start);

  while (!openSet.empty()) {
    Cell *current = openSet.top();
    openSet.pop();
    if (current == end) {
      ReconstructPath(end, start, render);
      return true;
    }

    for (Cell *neighbour : current->neighbours) {

      if (neighbour->isVisidted) {
        continue;
      }

      int pendingGScore = current->gScore + 1;
      if (neighbour->gScore > pendingGScore) {

        neighbour->parent = current;
        neighbour->hScore = Heuristic(neighbour, end);
        neighbour->gScore = pendingGScore;
        neighbour->Update();

        if (!neighbour->isVisidted) {
          neighbour->isOpen = true;
          openSet.push(neighbour);
        }
      }
    }

    render();
    if (current != start)
      current->isVisidted = true;
  }
  return false;
}

void RenderGrid(std::vector<Cell> &grid, float cellWidth, int gridSize) {

  for (Cell &cell : grid) {
    Color color = WHITE;
    if ((cell.isStart)) {
      color = BLUE;
    } else if (cell.isEnd) {
      color = ORANGE;
    } else if (cell.isbarrier) {
      color = BLACK;
    } else if (cell.isVisidted) {
      color = RED;
    } else if (cell.isOpen) {
      color = GREEN;
    }
    if (cell.isPath) {
      color = PURPLE;
    }

    DrawRectangleV(cell.position, {cellWidth, cellWidth}, color);
  }

  // Vertical Lines
  for (int x = 0; x < gridSize; x++) {
    Vector2 start = {x * cellWidth, 0};
    Vector2 end = {x * cellWidth, float(GetScreenHeight())};
    DrawLineEx(start, end, 1, BLACK);
  }

  for (int y = 0; y < gridSize; y++) {
    Vector2 start = {0, y * cellWidth};
    Vector2 end = {float(GetScreenWidth()), y * cellWidth};
    DrawLineEx(start, end, 1, BLACK);
  }
}

void GetInput(std::vector<Cell> &grid, float cellWidth, int gridSize,
              int &inputState, bool &gridCleared,
              std::function<void()> &render) {
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && gridCleared) {
    Vector2 mousePos = GetMousePosition();

    int row = int(mousePos.y / cellWidth);
    int col = int(mousePos.x / cellWidth);

    Cell &current = grid[(row * gridSize) + col];

    if (inputState == 0 && !current.isStart && !current.isEnd &&
        !current.isbarrier) {
      current.isStart = true;
      inputState = 1; // Next click should set the end
    } else if (inputState == 1 && !current.isStart && !current.isEnd &&
               !current.isbarrier) {
      current.isEnd = true;
      inputState = 2; // Next clicks should set barriers
    } else if (inputState == 2 && !current.isStart && !current.isEnd &&
               !current.isbarrier) {
      current.isbarrier = true;
    }
  } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && gridCleared) {
    Vector2 mousePos = GetMousePosition();

    int row = int(mousePos.y / cellWidth);
    int col = int(mousePos.x / cellWidth);

    Cell &current = grid[(row * gridSize) + col];

    if (current.isStart) {
      current.isStart = false;
      inputState = 0; // Reset inputState to allow setting start again
    } else if (current.isEnd) {
      current.isEnd = false;
      inputState = 1; // Allow setting end again
    } else if (current.isbarrier) {
      current.isbarrier = false;
      inputState = 2; // Continue with barriers removal
    }
  }

  if (IsKeyPressed(KEY_SPACE)) {

    gridCleared = false;
    Cell *start = nullptr;
    Cell *end = nullptr;

    for (Cell &cell : grid) {
      cell.UpdateNeigbours(grid);

      if (cell.isStart) {
        start = &cell;
      }
      if (cell.isEnd) {
        end = &cell;
      }
    }
    PathFind(grid, start, end, render);
  }

  if (IsKeyPressed(KEY_LEFT_SHIFT)) {
    grid.clear();
    CreateGrid(grid, gridSize, GetScreenWidth());
    inputState = 0;
    gridCleared = true;
  }
}

int main(int argc, char **argv) {
  std::string file = "";
  bool hasFile = false;

  if (argc == 2) {
    std::string arg = argv[1];
    if (arg.substr(0, 7) != "--maze=") {
      std::cout << "invalid option\n";
      std::cout << "usage: \"" << argv[0] << "\" for blank map\n";
      std::cout << "or " << argv[0]
                << " --maze=path_to_maze_file.txt fto read map file\n";
      return -1;
    } else {
      file = arg.substr(7);
      hasFile = true;
    }
  } else if (argc > 2) {
    std::cout << "too many argumnets!\n";
    std::cout << "usage: " << argv[0] << " for blank map\n";
    std::cout << "or " << argv[0]
              << " --maze=path_to_maze_file.txt fto read map file\n";
    return -1;
  }

  InitWindow(800, 800, "A* Pathfinding");

  // Grid
  std::vector<Cell> grid = {};
  int gridSize = 100;

  float cellSize = CreateGrid(grid, gridSize, GetScreenWidth());

  if (hasFile) {
    if (!FillGridFromFile(file, grid, gridSize)) {
      std::cout << "Failed to read maze, file\n";
      return -1;
    }
  }


  int inputState = 0;
  bool gridCleared = true;


  std::function<void()> render = [&]() -> void {
    BeginDrawing();
    ClearBackground(BLACK);
    RenderGrid(grid, cellSize, gridSize);
    EndDrawing();
  };

  while (!WindowShouldClose()) {
    GetInput(grid, cellSize, gridSize, inputState, gridCleared, render);
    render();
  }

  CloseWindow();
}
