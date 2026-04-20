#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ROWS 32
#define MAX_COLS 32
#define STATUS_PLAYING 0
#define STATUS_WIN 1
#define STATUS_LOSE 2
#define CELL_MINE -1

typedef struct {
    int rows;
    int cols;
    int mines;
    int status;
    int revealed_count;
    int board[MAX_ROWS][MAX_COLS];
    unsigned char visible[MAX_ROWS][MAX_COLS];
    unsigned char flags[MAX_ROWS][MAX_COLS];
} Game;

static int in_bounds(const Game *g, int r, int c) {
    return r >= 0 && r < g->rows && c >= 0 && c < g->cols;
}

static int save_game(const char *path, const Game *g) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(g, sizeof(Game), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int load_game(const char *path, Game *g) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(g, sizeof(Game), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static void place_mines(Game *g, unsigned int seed) {
    int placed = 0;
    srand(seed);
    while (placed < g->mines) {
        int r = rand() % g->rows;
        int c = rand() % g->cols;
        if (g->board[r][c] == CELL_MINE) continue;
        g->board[r][c] = CELL_MINE;
        placed++;
    }
}

static int count_adjacent_mines(const Game *g, int r, int c) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr;
            int nc = c + dc;
            if (in_bounds(g, nr, nc) && g->board[nr][nc] == CELL_MINE) {
                count++;
            }
        }
    }
    return count;
}

static void fill_numbers(Game *g) {
    for (int r = 0; r < g->rows; r++) {
        for (int c = 0; c < g->cols; c++) {
            if (g->board[r][c] == CELL_MINE) continue;
            g->board[r][c] = count_adjacent_mines(g, r, c);
        }
    }
}

static void flood_reveal(Game *g, int r, int c) {
    if (!in_bounds(g, r, c)) return;
    if (g->visible[r][c] || g->flags[r][c]) return;
    g->visible[r][c] = 1;
    g->revealed_count++;
    if (g->board[r][c] != 0) return;

    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            flood_reveal(g, r + dr, c + dc);
        }
    }
}

static void reveal_all_mines(Game *g) {
    for (int r = 0; r < g->rows; r++) {
        for (int c = 0; c < g->cols; c++) {
            if (g->board[r][c] == CELL_MINE) g->visible[r][c] = 1;
        }
    }
}

static void check_win(Game *g) {
    int safe_cells = g->rows * g->cols - g->mines;
    if (g->revealed_count >= safe_cells && g->status == STATUS_PLAYING) {
        g->status = STATUS_WIN;
    }
}

static void print_state_json(const Game *g) {
    printf("{");
    printf("\"rows\":%d,", g->rows);
    printf("\"cols\":%d,", g->cols);
    printf("\"mines\":%d,", g->mines);
    printf("\"status\":\"%s\",", g->status == STATUS_WIN ? "win" : (g->status == STATUS_LOSE ? "lose" : "playing"));
    printf("\"revealed_count\":%d,", g->revealed_count);

    printf("\"board\":[");
    for (int r = 0; r < g->rows; r++) {
        printf("[");
        for (int c = 0; c < g->cols; c++) {
            printf("%d", g->board[r][c]);
            if (c + 1 < g->cols) printf(",");
        }
        printf("]");
        if (r + 1 < g->rows) printf(",");
    }
    printf("],");

    printf("\"visible\":[");
    for (int r = 0; r < g->rows; r++) {
        printf("[");
        for (int c = 0; c < g->cols; c++) {
            printf("%d", g->visible[r][c] ? 1 : 0);
            if (c + 1 < g->cols) printf(",");
        }
        printf("]");
        if (r + 1 < g->rows) printf(",");
    }
    printf("],");

    printf("\"flags\":[");
    for (int r = 0; r < g->rows; r++) {
        printf("[");
        for (int c = 0; c < g->cols; c++) {
            printf("%d", g->flags[r][c] ? 1 : 0);
            if (c + 1 < g->cols) printf(",");
        }
        printf("]");
        if (r + 1 < g->rows) printf(",");
    }
    printf("]");

    printf("}\n");
}

static int cmd_init(int argc, char **argv) {
    if (argc != 7) {
        fprintf(stderr, "usage: init rows cols mines seed output_file\n");
        return 1;
    }

    Game g;
    memset(&g, 0, sizeof(Game));
    g.rows = atoi(argv[2]);
    g.cols = atoi(argv[3]);
    g.mines = atoi(argv[4]);
    unsigned int seed = (unsigned int)strtoul(argv[5], NULL, 10);
    const char *out_path = argv[6];

    if (g.rows <= 0 || g.rows > MAX_ROWS || g.cols <= 0 || g.cols > MAX_COLS) {
        fprintf(stderr, "invalid board size\n");
        return 1;
    }
    if (g.mines <= 0 || g.mines >= g.rows * g.cols) {
        fprintf(stderr, "invalid mine count\n");
        return 1;
    }

    g.status = STATUS_PLAYING;
    place_mines(&g, seed);
    fill_numbers(&g);

    if (!save_game(out_path, &g)) {
        fprintf(stderr, "failed to save game\n");
        return 1;
    }
    return 0;
}

static int cmd_reveal(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: reveal state_file row col\n");
        return 1;
    }
    const char *path = argv[2];
    int row = atoi(argv[3]);
    int col = atoi(argv[4]);

    Game g;
    if (!load_game(path, &g)) {
        fprintf(stderr, "failed to load game\n");
        return 1;
    }
    if (g.status != STATUS_PLAYING) {
        fprintf(stderr, "game already finished\n");
        return 1;
    }
    if (!in_bounds(&g, row, col)) {
        fprintf(stderr, "cell out of range\n");
        return 1;
    }
    if (g.flags[row][col] || g.visible[row][col]) {
        if (!save_game(path, &g)) {
            fprintf(stderr, "failed to save game\n");
            return 1;
        }
        return 0;
    }

    if (g.board[row][col] == CELL_MINE) {
        g.status = STATUS_LOSE;
        reveal_all_mines(&g);
    } else {
        flood_reveal(&g, row, col);
        check_win(&g);
    }

    if (!save_game(path, &g)) {
        fprintf(stderr, "failed to save game\n");
        return 1;
    }
    return 0;
}

static int cmd_flag(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: flag state_file row col\n");
        return 1;
    }
    const char *path = argv[2];
    int row = atoi(argv[3]);
    int col = atoi(argv[4]);

    Game g;
    if (!load_game(path, &g)) {
        fprintf(stderr, "failed to load game\n");
        return 1;
    }
    if (g.status != STATUS_PLAYING) {
        fprintf(stderr, "game already finished\n");
        return 1;
    }
    if (!in_bounds(&g, row, col)) {
        fprintf(stderr, "cell out of range\n");
        return 1;
    }
    if (g.visible[row][col]) {
        fprintf(stderr, "cannot flag opened cell\n");
        return 1;
    }

    g.flags[row][col] = g.flags[row][col] ? 0 : 1;
    if (!save_game(path, &g)) {
        fprintf(stderr, "failed to save game\n");
        return 1;
    }
    return 0;
}

static int cmd_dump(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: dump state_file\n");
        return 1;
    }
    Game g;
    if (!load_game(argv[2], &g)) {
        fprintf(stderr, "failed to load game\n");
        return 1;
    }
    print_state_json(&g);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: <init|reveal|flag|dump> ...\n");
        return 1;
    }
    if (strcmp(argv[1], "init") == 0) return cmd_init(argc, argv);
    if (strcmp(argv[1], "reveal") == 0) return cmd_reveal(argc, argv);
    if (strcmp(argv[1], "flag") == 0) return cmd_flag(argc, argv);
    if (strcmp(argv[1], "dump") == 0) return cmd_dump(argc, argv);
    fprintf(stderr, "unknown command: %s\n", argv[1]);
    return 1;
}
