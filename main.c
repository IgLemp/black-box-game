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

// #define CLS printf("\e[1;1H\e[2J"); // clear screen
#define CLS

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
} MarkerType;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
} Direction;

typedef struct {
    MarkerType type;
    size_t number;
} Marker;

typedef struct {
    size_t x, y;
} Cursor, Ray;


void check_hit(Cursor cursor, size_t last_index, Marker board[12][12], bool atoms[12][12]);
void display_board(Marker board[12][12], size_t last_index, Cursor cursor);
void show_atoms(Marker board[12][12], size_t last_index, bool atoms[12][12]);
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

    CLS
    do {
        switch (game_state) {
            case MENU:    run_menu (&game_state, &last_board_index); break;
            case RUNNING: run_game (&game_state, last_board_index, board, atoms); break;
            case CHECK:   run_check(&game_state, last_board_index, board, atoms); break;
            case END:     run_end  (&game_state, last_board_index, board, atoms); break;
        }
    } while (game_state != QUIT);

    CLS
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


void display_board(Marker board[12][12], size_t last_index, Cursor cursor) {
    printf("╔");
    for (size_t i = 0; i <= last_index; i++) { printf("═"); }
    printf("╗\n");

    for (size_t i = 0; i <= last_index; i++) { 
        printf("║");
        for (size_t j = 0; j <= last_index; j++) {
            if ( (i == cursor.y) && (j == cursor.x) ) { printf("@"); 
            } else if ((i == 0) || (i == last_index) || (j == 0) || (j == last_index)) {
                printf("▓");                
            } else {
                switch (board[i][j].type) {
                case EMPTY:      printf("░"); break;
                case MARK:       printf("o"); break;
                case HIT:        printf("%i", board[i][j].number); break;
                case REFLECTION: printf("%i", board[i][j].number); break;
                default: break;
                }
            }
        }
        printf("║\n");
    }

    printf("╚");
    for (size_t i = 0; i <= last_index; i++) { printf("═"); }
    printf("╝\n");
}


void show_atoms(Marker board[12][12], size_t last_index, bool atoms[12][12]) {
    printf("╔");
    for (size_t i = 0; i <= last_index; i++) { printf("═"); }
    printf("╗\n");

    for (size_t i = 0; i <= last_index; i++) { 
        printf("║");
        for (size_t j = 0; j <= last_index; j++) {
            if ((i == 0) || (i == last_index) || (j == 0) || (j == last_index)) {
                printf("▓");
            } else if (atoms[i][j]) {
                printf("O");
            } else {
                switch (board[i][j].type) {
                case EMPTY:      printf("░"); break;
                case HIT:        printf("%i", board[i][j].number); break;
                case REFLECTION: printf("%i", board[i][j].number); break;
                default: break;
                }
            }
        }
        printf("║\n");
    }

    printf("╚");
    for (size_t i = 0; i <= last_index; i++) { printf("═"); }
    printf("╝\n");
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
    
    CLS
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
            !((atoms[x - 1][y - 1]) || (atoms[x - 1][y]) || (atoms[x - 1][y + 1]) ||
              (atoms[x    ][y - 1]) || (atoms[x    ][y]) || (atoms[x    ][y + 1]) ||
              (atoms[x + 1][y - 1]) || (atoms[x - 1][y]) || (atoms[x + 1][y + 1]))
        ) {
            atoms[x][y] = true;
            i++;
        }
        // else {puts("false");}
    }

    // main game loop
    while (*game_state == RUNNING) {
        display_board(board, last_board_index, cursor);
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
            CLS
            show_atoms(board, last_board_index, atoms);
            sleep(3);
            break;
        case 'Q': case 'q': *game_state = MENU; break;
        default: break;
        }

        
        CLS
    }
}


void run_end(GameState *game_state, size_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    size_t score = 0;
    
    printf("╔");
    for (size_t i = 0; i <= last_board_index; i++) { printf("═"); }
    printf("╗\n");

    for (size_t i = 0; i <= last_board_index; i++) { 
        printf("║");
        for (size_t j = 0; j <= last_board_index; j++) {
            if ((i == 0) || (i == last_board_index) || (j == 0) || (j == last_board_index)) {
                printf("▓");                
            } else {
                switch (board[i][j].type) {
                case EMPTY: printf("░"); break;
                case MARK: 
                    if (atoms[i][j]) { printf("O"); score++; }
                    else             { printf("X"); }
                    break;
                case HIT:        printf("%01X", board[i][j].number); break;
                case REFLECTION: printf("%01X", board[i][j].number); break;
                default: break;
                }
            }
        }
        printf("║\n");
    }

    printf("╚");
    for (size_t i = 0; i <= last_board_index; i++) { printf("═"); }
    printf("╝\n");

    printf("Your score is: \e[1m%i\e[0m\n", score);
    puts("press ENTER to continue...");
    scanf("%*c");
    CLS
    *game_state = MENU;
}


void run_check(GameState *game_state, size_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    printf("╔");
    for (size_t i = 0; i <= last_board_index; i++) { printf("═"); }
    printf("╗\n");

    for (size_t i = 0; i <= last_board_index; i++) { 
        printf("║");
        for (size_t j = 0; j <= last_board_index; j++) {
            if ((i == 0) || (i == last_board_index) || (j == 0) || (j == last_board_index)) {
                printf("▓");                
            } else if (atoms[i][j]) {
                printf("O");
            } else {
                switch (board[i][j].type) {
                case EMPTY: printf("░"); break;
                case HIT:        printf("%01X", board[i][j].number); break;
                case REFLECTION: printf("%01X", board[i][j].number); break;
                default: break;
                }
            }
        }
        printf("║\n");
    }

    printf("╚");
    for (size_t i = 0; i <= last_board_index; i++) { printf("═"); }
    printf("╝\n");
    puts("press ENTER to continue...");
    scanf("%*c");
    CLS
    *game_state = MENU;
}
