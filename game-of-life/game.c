#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


#define SCREEN_WIDTH 1500
#define SCREEN_HEIGHT 1200
#define COLS_NUM 70
#define ROWS_NUM 50
#define CELL_NUM (COLS_NUM * ROWS_NUM)
#define FRAME_RATE 10 // Frames per second
#define FRAME_TIME (1000000 / FRAME_RATE) // Milliseconds per frame

#define CELL_SIZE 15 // Size of each cell in pixels

#define ALIVE 'O'
#define DEAD '.'
#define ALIVE_COLOR WHITE
#define DEAD_COLOR BLACK

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    char grid[CELL_NUM];
    char next_grid[CELL_NUM];
    bool running;
} Game;

void clear_terminal() {
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#elif defined(__linux__) || defined(__APPLE__)
    printf("\033[2J\033[H");
#else
    // fallback
    system("clear");
#endif
}

int coord_to_index(Position pos) {
    int row = pos.row % ROWS_NUM;
    int col = pos.col % COLS_NUM;

    if(row < 0)
        row += ROWS_NUM;
    if(col < 0)
        col += COLS_NUM;

    return row * COLS_NUM  + col;
}

Position mouse_to_coord(Vector2 mousePosition) {
    Position pos;
    pos.row = (int)(mousePosition.y);
    pos.col = (int)(mousePosition.x);

    // Ensure the position is within bounds
    if(pos.row < 0) pos.row = 0;
    if(pos.col < 0) pos.col = 0;

    return pos;

}

void init_game(Game* game) {
    memset(game->grid, DEAD, sizeof(game->grid));
    memset(game->next_grid, DEAD, sizeof(game->next_grid));
    game->running = false;
}

char get_cell_state(char* grid, Position pos) {
    return grid[coord_to_index(pos)];
}

/*
Set a specific cell state given a game grid.
*/
void set_cell_state(char* grid, Position pos, char state) {
    grid[coord_to_index(pos)] = state;
}

void toggle_cell_state(char* grid, Position pos) {
    char currentState = get_cell_state(grid, pos);
    char newState = (currentState == ALIVE) ? DEAD : ALIVE;
    set_cell_state(grid, pos, newState);
}

/*
Compute the next state for a given position returning DEAD or ALIVE.
The rules are:
1. Any live cell with fewer than two live neighbors dies (underpopulation).
2. Any live cell with two or three live neighbors lives on to the next generation.
3. Any live cell with more than three live neighbors dies (overpopulation).
4. Any dead cell with exactly three live neighbors becomes a live cell (reproduction).
*/
char compute_next_state(char* grid, Position pos) {
    int alive_neighbors = 0;
    int row = pos.row;
    int col = pos.col;
    for(int drow = -1; drow <= 1; drow++) {
        for(int dcol = -1; dcol <= 1; dcol++) {
            if(drow == 0 && dcol == 0) continue; // Skip the cell itself
            Position neighbor_pos = {row + drow, col + dcol};
            char neighbor_state = get_cell_state(grid, neighbor_pos);
            if(neighbor_state == ALIVE)
                alive_neighbors++;
        }
    }

    char current_state = get_cell_state(grid, pos);
    char next_state = DEAD;
    if(current_state == ALIVE && (alive_neighbors == 3 || alive_neighbors == 2)) 
        next_state = ALIVE;
    else if(current_state == DEAD && alive_neighbors == 3) 
        next_state = ALIVE;

    return next_state;
}

void render(Game* game) {
    char* grid = game->grid; // Render only the 'old' grid
    clear_terminal();
    for(int row = 0; row < ROWS_NUM; row++) {
        for(int col = 0; col < COLS_NUM; col++) {
            printf("%c ", grid[coord_to_index((Position){row, col})]);
        }
        printf("\n");
    }
}

/*
The following function compute the next stato of the game based on the current state.
It should be called periodically, e.g., every FRAME_TIME microseconds.
*/
void update(Game* game) {
    for(int row = 0; row < ROWS_NUM; row++) {
        for(int col = 0; col < COLS_NUM; col++) {
            Position pos = {row, col};
            int next_state = compute_next_state(game->grid, pos);
            set_cell_state(game->next_grid, pos, next_state);
        }
    }
    memcpy(game->grid, game->next_grid, sizeof(game->grid));
}

void game_step(Game* game) {
    game->running = true;
    update(game); // Update the game state
    usleep(FRAME_TIME); // Sleep for the frame time
}

/*
Game's controls:
- reset of the game's cells (DONE)
- start the game loop (DONE), when the game is started it is not possible to edit the cells
- stop the game loop (DONE)
- toggle the state of a cell with the mouse click (DONE)
- toggle the state of multiple cells with the mouse cickhold (TODO - ADVANCED)

*/
int main(void) {


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life. Lorenzo Sibi's Implementation");
    SetTargetFPS(60);

    Game game;
    init_game(&game);

    Vector2 mousePosition = GetMousePosition();
    Vector2 mouseCell = { -1, -1 };

    Rectangle gridRect = { 20, 20, COLS_NUM * CELL_SIZE, ROWS_NUM * CELL_SIZE };
    Rectangle controlPanelRect = { gridRect.width + 30, 20, 200, 300 };
    Rectangle statusRect = { gridRect.width + 30, 340, 200, 100 };

    while(!WindowShouldClose()) {

        mousePosition = GetMousePosition();

        int gridResult = GuiGrid(gridRect, NULL, CELL_SIZE, 1, &mouseCell);
        int controlsResult = GuiGroupBox(controlPanelRect, "Controls");


        // Control Buttons
        int startButton = GuiButton((Rectangle){gridRect.width + 40, 60, 80, 30}, "START");
        int pauseButton = GuiButton((Rectangle){gridRect.width + 40, 100, 80, 30}, "PAUSE");
        int resetButton = GuiButton((Rectangle){gridRect.width + 40, 140, 80, 30}, "RESET");


        if(startButton)
            game.running = true;
        if(pauseButton)
            game.running = false;
        if(resetButton) {
            init_game(&game);
            game.running = false;
        }

        // Status
        GuiGroupBox(statusRect, "Status");
        GuiLabel((Rectangle){gridRect.width + 40, 370, 160, 20}, "Generation: 0");

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePosition, gridRect) && !game.running) {
            Position pos = mouse_to_coord(mouseCell);
            toggle_cell_state(game.grid, pos);
        }

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        BeginDrawing();
        
        // For drawing the cells on the screen
        for(int row = 0; row < ROWS_NUM; row++) {
            for(int col = 0; col < COLS_NUM; col++) {
                Position pos = {row, col};
                char state = get_cell_state(game.grid, pos);
                Rectangle cellRect = (Rectangle){
                    gridRect.x + col * CELL_SIZE,
                    gridRect.y + row * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE
                };
                DrawRectangleRec(cellRect, state == ALIVE ? ALIVE_COLOR : DEAD_COLOR);
                DrawRectangleLinesEx(cellRect, 1.0f ,DARKGRAY);
            }
        }
        
        EndDrawing();

        if(game.running) {
            game_step(&game);
        }
    }

    // Cleanup
    CloseWindow();

    return 0;
}