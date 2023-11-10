#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

typedef enum {
    MENU,
    RUNNING,
    END,
    QUIT,
} GameState;

typedef enum {
    EMPTY,
    HIT,
    MARK,
    REFLECTION,
} MarkerType;

typedef struct {
    MarkerType type;
    size_t number;
} Marker;

typedef struct {
    size_t x, y;
} Cursor;


Marker check_hit(Cursor c, Marker **board);
void display_board();
void run_menu();
void run_game();
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
*/


int main() {
    GameState game_state = MENU;
    size_t last_board_index; // that is including borders!
    Marker board[12][12];
    memset(board, 0, 12 * 12 * sizeof(Marker));
    bool atoms[12][12] = {false};

    do {
        switch (game_state) {
            case MENU:    run_menu(&game_state, &last_board_index);
            case RUNNING: run_game(&game_state, &last_board_index, board, atoms);
            case END:     run_end();
        }
    } while (game_state != QUIT);

    return 0;
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
    case 'B': case 'b':
        *last_board_index = 10 + 1;
        *game_state = RUNNING;
        break;
    case 'Q': case 'q':
        *game_state = END;
        break;
    default: break;
    }
}

void run_game(GameState *game_state, size_t *last_board_index, Marker board[12][12], bool atoms[12][12]) {
    Cursor cursor = {0, 0};
    char input;
    char history[5];

    // setup
    // for (size_t i = 1; i < *last_board_index, )

    while (*game_state == RUNNING) {
        display_board(board, *last_board_index, cursor);
        printf("> "); // Prompt
        scanf("%c%*c", &input);

        switch (input) {
        case 'W': case 'w': if (cursor.y > 0)                 { cursor.y -= 1; } /*last_move = 'w';*/ break;
        case 'S': case 's': if (cursor.y < *last_board_index) { cursor.y += 1; } /*last_move = 's';*/ break;
        case 'A': case 'a': if (cursor.x > 0)                 { cursor.x -= 1; } /*last_move = 'a';*/ break;
        case 'D': case 'd': if (cursor.x < *last_board_index) { cursor.x += 1; } /*last_move = 'd';*/ break;
        case 'U': case 'u':
            // if (last_move != 0) {
            //     switch (last_move) {
            //         // Yes it undoes move into wall by moving one back
            //         // I'm not gonna bother
            //         case 'W': case 'w': if (cursor.y < *last_board_index) { cursor.y += 1; } break;
            //         case 'S': case 's': if (cursor.y > 0)                 { cursor.y -= 1; } break;
            //         case 'A': case 'a': if (cursor.x < *last_board_index) { cursor.x += 1; } break;
            //         case 'D': case 'd': if (cursor.x > 0)                 { cursor.x -= 1; } break;
            //         case ' ': break;
            //         case 'o': break;
            //     }
            //     last_move = '\0';
            // }
            break;
        case 'R': case 'r':
            // if (last_move != 0) {
            //     switch (last_move) {
            //         case 'W': case 'w': if (cursor.y < *last_board_index) { cursor.y += 1; } break;
            //         case 'S': case 's': if (cursor.y > 0)                 { cursor.y -= 1; } break;
            //         case 'A': case 'a': if (cursor.x < *last_board_index) { cursor.x += 1; } break;
            //         case 'D': case 'd': if (cursor.x > 0)                 { cursor.x -= 1; } break;
            //         case ' ': break;
            //         case 'o': break;
            //     }
            //     last_move = '\0';
            // }
            break;
        case ' ':
            if ( (cursor.x == 0) || (cursor.x == *last_board_index) ||
                 (cursor.y == 0) || (cursor.y == *last_board_index) ) {
                    // check_hit(cursor, board);
                 }
            break;
        case 'o':
            if ( !((cursor.x == 0) || (cursor.x == *last_board_index) ||
                   (cursor.y == 0) || (cursor.y == *last_board_index))) {
                    if (board[cursor.y][cursor.x].type != MARK) {
                        board[cursor.y][cursor.x] = (Marker){MARK, 0};
                    } else {
                        board[cursor.y][cursor.x] = (Marker){EMPTY, 0};
                    }
                 }
            break;
        case 'k': break;
        case 'p': break;
        case 'h': break;
        default: break;
        }

        printf("x:%i y:%i\n", cursor.x, cursor.y);
        printf("\n");
    }
}

void run_end(GameState *game_state, size_t *last_board_index, Marker board[12][12], bool atoms[12][12]) {
    size_t score = 0;
    
    printf("╔");
    for (size_t i = 0; i <= *last_board_index; i++) { printf("═"); }
    printf("╗\n");

    for (size_t i = 0; i <= *last_board_index; i++) { 
        printf("║");
        for (size_t j = 0; j <= *last_board_index; j++) {
            if ((i == 0) || (i == *last_board_index) || (j == 0) || (j == *last_board_index)) {
                printf("▓");                
            } else {
                switch (board[i][j].type) {
                case EMPTY: printf("░"); break;
                case MARK: 
                    if (atoms[i][j]) { printf("O"); }
                    else             { printf("X"); }
                    break;
                case HIT:        printf("%i", board[i][j].number); break;
                case REFLECTION: printf("%i", board[i][j].number); break;
                default: break;
                }
            }
        }
        printf("║\n");
    }

    printf("╚");
    for (size_t i = 0; i <= *last_board_index; i++) { printf("═"); }
    printf("╝\n");

}
