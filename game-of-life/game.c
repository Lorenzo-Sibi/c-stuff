#include <stdio.h>
#include <stdlib.h>
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

#define ALIVE 'O'
#define DEAD '.'
#define ALIVE_COLOR (Color)WHITE
#define DEAD_COLOR (Color)BLACK

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    char grid[CELL_NUM];
    char next_grid[CELL_NUM];
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

void start_game(Game* game) {
    while(1) {
        update(game); // Update the game state
        render(game); // Render the game state
        usleep(FRAME_TIME); // Sleep for the frame time
    }
}
int main(void) {


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life. Lorenzo Sibi's Implementation");
    SetTargetFPS(60);

    Vector2 mousePosition = GetMousePosition();


    Game game;
    init_game(&game);



/*  
    set_cell_state((char*)game.grid, (Position){25, 25}, ALIVE);
    set_cell_state((char*)game.grid, (Position){26, 25}, ALIVE);
    set_cell_state((char*)game.grid, (Position){27, 25}, ALIVE);
    set_cell_state((char*)game.grid, (Position){26, 24}, ALIVE);
    set_cell_state((char*)game.grid, (Position){27, 26}, ALIVE);
    
    set_cell_state((char*)game.grid, (Position){25, 25}, ALIVE);
    set_cell_state((char*)game.grid, (Position){26, 25}, ALIVE);
    set_cell_state((char*)game.grid, (Position){27, 25}, ALIVE);
    set_cell_state((char*)game.grid, (Position){27, 24}, ALIVE);
    set_cell_state((char*)game.grid, (Position){26, 23}, ALIVE);
    
    
    
    render(&game); // Initial renders
    sleep(2);
    start_game(&game);
    
    */
    return 0;
}