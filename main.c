#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define STARTING_SCREEN \
"\
    ____  __           __      ____\n\
   / __ )/ /___ ______/ /__   / __ )____  _  __\n\
  / __  / / __ `/ ___/ //_/  / __  / __ \\| |/_/\n\
 / /_/ / / /_/ / /__/ ,<    / /_/ / /_/ />  <\n\
/_____/_/\\__,_/\\___/_/|_|  /_____/\\____/_/|_|\n\
\n\
Choose board size:\n\
    (S) SMALL  5x5\n\
    (M) MEDIUM 8x8\n\
    (L) LARGE  10x10\n\
"

#define UTF8

#ifdef UTF8
    #define CLS "\e[1;1H\e[2J" // clear screen
    #define B_LEFT_UP "╔"
    #define B_RIGHT_UP "╗"
    #define B_LEFT_DOWN "╚"
    #define B_RIGHT_DOWN "╝"
    #define B_HBEAM "═"
    #define B_VBEAM "║"
    #define B_BORDER "▓"
    #define B_FILL "░"
#else
    #define CLS "\n\n\n\n\n\n\n"
    #define B_LEFT_UP "/"
    #define B_RIGHT_UP "\\"
    #define B_LEFT_DOWN "\\"
    #define B_RIGHT_DOWN "/"
    #define B_HBEAM "-"
    #define B_VBEAM "|"
    #define B_BORDER "#"
    #define B_FILL "."
#endif

// #define CLS

typedef enum {
    MENU,
    RUNNING,
    END,
    CHECK,
    QUIT,
} GameState;

typedef enum {
    EMPTY,
    HIT,
    MARK,
    REFLECTION,
    MISS,
} MarkerType;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
} Direction;

typedef enum {
    SHOW_CURSOR  = 1,
    SHOW_ATOMS   = 2,
    SHOW_MARKERS = 4,
    SHOW_CORRECT_HITS = 8,
} BoardPrinterOptions;

typedef struct {
    MarkerType type;
    size_t number;
} Marker;

typedef struct {
    size_t x, y;
} Cursor, Ray;


void check_hit(Cursor cursor, size_t last_index, Marker board[12][12], bool atoms[12][12]);
void display_board(Marker board[12][12], bool atoms[12][12], size_t last_index, Cursor cursor, BoardPrinterOptions opt);
void run_menu();
void run_game();
void run_check();
void run_end();

/*
    Board starts at x:1 y:1
    so borders start at x:0 y:0
    and end at x:{size} y:{size}

     0123456
    0 #####
    1#*****#
    2#*****#
    3#*****#
    4#*****#
    5#*****#
    6 #####

    NOTICE: coordinate system starts at top left!
*/

// TODO: Make printing a board less of an ass pain
//       probably will have to make some fat function
//       confugured with enum or something
//       could be a good idea actually

int main() {
    GameState game_state = MENU;
    size_t last_board_index; // that is including borders!
    Marker board[12][12];
    memset(board, 0, 12 * 12 * sizeof(Marker));
    bool atoms[12][12] = {false};

    printf(CLS);
    do {
        switch (game_state) {
            case MENU:    run_menu (&game_state, &last_board_index); break;
            case RUNNING: run_game (&game_state, last_board_index, board, atoms); break;
            case CHECK:   run_check(&game_state, last_board_index, board, atoms); break;
            case END:     run_end  (&game_state, last_board_index, board, atoms); break;
        }
    } while (game_state != QUIT);

    printf(CLS);
    return 0;
}


void check_hit(Cursor cursor, size_t last_index, Marker board[12][12], bool atoms[12][12]) {
    Direction ray_direction;
    Ray ray_position = cursor;

    if      (cursor.x == 0)          { ray_direction = RIGHT; }
    else if (cursor.x == last_index) { ray_direction = LEFT;  }
    else if (cursor.y == 0)          { ray_direction = DOWN;  }
    else if (cursor.y == last_index) { ray_direction = UP;    }

    if      (ray_direction = RIGHT) {
        // if (
        //     // atoms[ray_position.y][ray_position.x + 1]
        // )
    }
    else if (ray_direction = LEFT ) {}
    else if (ray_direction = DOWN ) {}
    else if (ray_direction = UP   ) {}
}


void display_board(Marker board[12][12], bool atoms[12][12], size_t last_index, Cursor cursor, BoardPrinterOptions opt) {
    printf(B_LEFT_UP);
    for (size_t i = 0; i <= last_index; i++) { printf(B_HBEAM); }
    printf(B_RIGHT_UP "\n");

    // I know it's bearly readable
    // I couldn't find a better way to do this
    // NOTICE: if statements first check for indexes then for tile tipe!
    for (size_t i = 0; i <= last_index; i++) {
        printf(B_VBEAM);
        for (size_t j = 0; j <= last_index; j++) {
            if (((i == cursor.y) && (j == cursor.x)) && opt & SHOW_CURSOR) { printf("@"); }
            else if (atoms[i][j]) { 
                if (opt & SHOW_ATOMS) { printf("o"); }
                else if (opt & SHOW_CORRECT_HITS) { if (board[i][j].type == MARK) { printf("O"); } else { printf("o"); } }
                else { printf(B_FILL); }
            }
            else if ((board[i][j].type == HIT) || 
                     (board[i][j].type == REFLECTION) || 
                     (board[i][j].type == MISS)) { printf("%01x", board[i][j].number); } 
            else if ((board[i][j].type == MARK)) {
                if (opt & SHOW_MARKERS) { printf("o"); }
                else if (opt & SHOW_CORRECT_HITS) { printf("X"); }
            } 
            else if ((i == 0) || (i == last_index) || (j == 0) || (j == last_index)) { printf(B_BORDER); }
            else { printf(B_FILL); }            
        }
        printf(B_VBEAM "\n");
    }

    printf(B_LEFT_DOWN);
    for (size_t i = 0; i <= last_index; i++) { printf(B_HBEAM); }
    printf(B_RIGHT_DOWN "\n");
}


void run_menu(GameState *game_state, size_t *last_board_index) {
    char input;
    puts(STARTING_SCREEN);
    printf("> "); // Prompt
    scanf("%c%*c", &input);

    switch (input) {
    case 'S': case 's':
        *last_board_index = 5 + 1;
        *game_state = RUNNING;
        break;
    case 'M': case 'm':
        *last_board_index = 8 + 1;
        *game_state = RUNNING;
        break;
    case 'L': case 'l':
        *last_board_index = 10 + 1;
        *game_state = RUNNING;
        break;
    case 'Q': case 'q':
        *game_state = END;
        break;
    default: break;
    }
    
    printf(CLS);
}


void run_game(GameState *game_state, size_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    Cursor cursor = {0, 0};
    char input;
    char history[5];
    size_t number_of_atoms;
    memset(board, 0, 12 * 12 * sizeof(Marker));
    memset(atoms, 0, 12 * 12 * sizeof(bool));

    // setup
    switch (last_board_index) {
        case 6:  number_of_atoms = 3; break;
        case 9:  number_of_atoms = 5; break;
        case 11: number_of_atoms = 8; break;
    }

    for (size_t i = 0; i < number_of_atoms;) {
        size_t x = ((size_t)rand() % (last_board_index - 1)) + 1;
        size_t y = ((size_t)rand() % (last_board_index - 1)) + 1;
        if (// two atoms next to each other make ambiguous positions possible
            !(atoms[y - 1][x - 1] || atoms[y - 1][x] || atoms[y - 1][x + 1])
        ) {
            atoms[y][x] = true;
            i++;
        }
    }

    // main game loop
    while (*game_state == RUNNING) {
        display_board(board, atoms, last_board_index, cursor, SHOW_CURSOR | SHOW_MARKERS);
        printf("x:%i y:%i\n", cursor.x, cursor.y);
        printf("> "); // Prompt
        scanf("%c%*c", &input);

        switch (input) {
        case 'W': case 'w': if (cursor.y > 0)                { cursor.y--; } /*last_move = 'w';*/ break;
        case 'S': case 's': if (cursor.y < last_board_index) { cursor.y++; } /*last_move = 's';*/ break;
        case 'A': case 'a': if (cursor.x > 0)                { cursor.x--; } /*last_move = 'a';*/ break;
        case 'D': case 'd': if (cursor.x < last_board_index) { cursor.x++; } /*last_move = 'd';*/ break;
        case 'U': case 'u': break;
        case 'R': case 'r': break;
        case ' ':
            if (// check if on border...
                (((cursor.x == 0) || (cursor.x == last_board_index)) ||
                ((cursor.y == 0) || (cursor.y == last_board_index))) &&
                // ...but not in a corner
                !(((cursor.x == 0) && (cursor.y == 0)) ||
                ((cursor.x == last_board_index) && (cursor.y == 0)) ||
                ((cursor.x == 0) && (cursor.y == last_board_index)) ||
                ((cursor.x == last_board_index) && (cursor.y == last_board_index)))
            ) {
                check_hit(cursor, last_board_index, board, atoms);
            }
            break;
        case 'o':
            if ( !((cursor.x == 0) || (cursor.x == last_board_index) ||
                   (cursor.y == 0) || (cursor.y == last_board_index))) {
                    if (board[cursor.y][cursor.x].type != MARK) {
                        board[cursor.y][cursor.x] = (Marker){MARK, 0};
                    } else {
                        board[cursor.y][cursor.x] = (Marker){EMPTY, 0};
                    }
                 }
            break;
        case 'k': *game_state = END;   break;
        case 'p': *game_state = CHECK; break;
        case 'h': case 'H':
            printf(CLS);
            display_board(board, atoms, last_board_index, cursor, SHOW_ATOMS);
            sleep(3);
            break;
        case 'Q': case 'q': *game_state = MENU; break;
        default: break;
        }

        
        printf(CLS);
    }
}


void run_end(GameState *game_state, size_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    size_t score = 0;
    display_board(board, atoms, last_board_index, (Cursor){0, 0}, SHOW_CORRECT_HITS);
    printf("Your score is: \e[1m%i\e[0m\n", score);
    puts("press ENTER to continue...");
    scanf("%*c");
    printf(CLS);
    *game_state = MENU;
}


void run_check(GameState *game_state, size_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    display_board(board, atoms, last_board_index, (Cursor){0, 0}, SHOW_ATOMS);
    puts("press ENTER to continue...");
    scanf("%*c");
    printf(CLS);
    *game_state = MENU;
}
