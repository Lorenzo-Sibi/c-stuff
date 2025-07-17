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
    long int generation;
    char grid[CELL_NUM];
    char next_grid[CELL_NUM];
    bool running;
} Game;