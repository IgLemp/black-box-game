// Igor Łempicki 200449
// EiT gr.1 gra Black Box

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if defined __cplusplus
    #include <thread>
    #include <chrono>
    #define SLEEP(n) std::this_thread::sleep_for(std::chrono::seconds(n))
    #define __cls system("cls") // clear screen
#else
    #include <unistd.h>
    #define SLEEP(n) sleep(n)
    #define __cls printf("\033[2J\033[1;1H"); // clear screen
#endif

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



// #define __cls ""
#define B_LEFT_UP    (char)(201)
#define B_RIGHT_UP   (char)(187)
#define B_LEFT_DOWN  (char)(200)
#define B_RIGHT_DOWN (char)(188)
#define B_HBEAM      (char)(205)
#define B_VBEAM      (char)(186)
#define B_BORDER     (char)(177)
#define B_FILL       (char)(176)

#define BOLD(s) "\33[1m" s "\33[0m"
#define RED(s) "\33[31m" s "\33[0m"
#define GREEN(s) "\33[32m" s "\33[0m"
#define BLUE(s) "\33[34m" s "\33[0m"


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
    SNAKE,
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

typedef enum {
    INSERT,
    RETREIVE,
} HistoryManagerOptions;

typedef struct {
    MarkerType type;
    uint8_t number;
} Marker;

typedef struct {
    uint8_t x, y;
} Point;

typedef struct {
    MarkerType type;
    union {
        struct {Point f; Point l;} two_point;
        Point point;
    } data;
} MarkerAtom;

typedef struct {
    char move;
    MarkerAtom data;
} HistoryEntry;

// circular buffer
typedef struct {
    HistoryEntry moves[5];
    int position;
    int depth;
} MoveHistory;


MarkerAtom check_hit(Point cursor, uint8_t last_index, bool atoms[12][12]);
void display_board(Marker board[12][12], bool atoms[12][12], uint8_t last_index, Point cursor, BoardPrinterOptions opt);
void run_menu (GameState *game_state, uint8_t *last_board_index);
void run_game (GameState *game_state, uint8_t last_board_index, Marker board[12][12], bool atoms[12][12]);
void run_check(GameState *game_state, uint8_t last_board_index, Marker board[12][12], bool atoms[12][12]);
void run_end  (GameState *game_state, uint8_t last_board_index, Marker board[12][12], bool atoms[12][12]);

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

// TODO: History depth 5 with dynamic undo redo

int main() {
    GameState game_state = MENU;
    uint8_t last_board_index; // that is including borders!
    Marker board[12][12];
    memset(board, 0, 12 * 12 * sizeof(Marker));
    bool atoms[12][12] = {false};

    __cls;
    do {
        switch (game_state) {
        case MENU:    run_menu (&game_state, &last_board_index); break;
        case RUNNING: run_game (&game_state, last_board_index, board, atoms); break;
        case CHECK:   run_check(&game_state, last_board_index, board, atoms); break;
        case END:     run_end  (&game_state, last_board_index, board, atoms); break;
        }
    } while (game_state != QUIT);

    __cls;
    return 0;
}


MarkerAtom check_hit(Point cursor, uint8_t last_index, bool atoms[12][12]) {
    Direction ray_direction;
    Point ray_position = cursor;
    bool was_reflected = false;

    /*
        We have 4 possibilities
            direct hit
            left hit
            right hit
            left and right hit
    */

   // set initial rotation
    if      (cursor.x == 0)          { ray_direction = RIGHT; }
    else if (cursor.x == last_index) { ray_direction = LEFT;  }
    else if (cursor.y == 0)          { ray_direction = DOWN;  }
    else if (cursor.y == last_index) { ray_direction = UP;    }

    while (true) {
        // check for border collision but avoiding cursor
        // if it isn't performed first later checks will cause
        // checking values outside arrays
        if (((ray_position.x == 0) || (ray_position.x == last_index) || 
             (ray_position.y == 0) || (ray_position.y == last_index)) &&
             !((ray_position.x == cursor.x) && (ray_position.y == cursor.y)))
            {
                // since we already know it didn't hit anything at the start
                #ifndef __cplusplus
                    return (MarkerAtom){ SNAKE, {(Point){cursor.x, cursor.y}, (Point){ray_position.x, ray_position.y}} };
                #else
                    // MarkerAtom mark = { SNAKE, { {cursor.x, cursor.y}, {ray_position.x, ray_position.y}} };
                    MarkerAtom mark;
                    mark.type = SNAKE;
                    mark.data.two_point.f = {cursor.x, cursor.y};
                    mark.data.two_point.l = {ray_position.x, ray_position.y};
                    return mark;
                #endif
            }

        // check at borders to prevent out of boundry array acceses later
        if (((ray_position.x == cursor.x) && (ray_position.y == cursor.y)) && was_reflected)
            {
                #ifndef __cplusplus
                    return (MarkerAtom){ REFLECTION, {cursor.x, cursor.y} };
                #else
                    MarkerAtom mark;
                    mark.type = REFLECTION;
                    mark.data.point = {cursor.x, cursor.y};
                    return mark;
                #endif
            }

        // check for direct hits and reflections
        // rotate accordingly
        if      (ray_direction == RIGHT) {
            if (atoms[ray_position.y][ray_position.x + 1]) // D hit
                {
                    #ifndef __cplusplus
                        return (MarkerAtom){ HIT, {cursor.x, cursor.y} };
                    #else
                        MarkerAtom mark;
                        mark.type = HIT;
                        mark.data.point = {cursor.x, cursor.y};
                        return mark;
                    #endif
                }
            else if (atoms[ray_position.y - 1][ray_position.x + 1] && atoms[ray_position.y + 1][ray_position.x + 1]) // LR hit
                { ray_direction = LEFT; was_reflected = true; }
            else if (atoms[ray_position.y - 1][ray_position.x + 1]) // L hit
                { ray_direction = DOWN; was_reflected = true; }
            else if (atoms[ray_position.y + 1][ray_position.x + 1]) // R hit
                { ray_direction = UP;   was_reflected = true; }
        }
        else if (ray_direction == LEFT) {
            if (atoms[ray_position.y][ray_position.x - 1]) // D hit
                {
                    #ifndef __cplusplus
                        return (MarkerAtom){ HIT, {cursor.x, cursor.y} };
                    #else
                        MarkerAtom mark;
                        mark.type = HIT;
                        mark.data.point = {cursor.x, cursor.y};
                        return mark;
                    #endif
                }
            else if (atoms[ray_position.y - 1][ray_position.x - 1] && atoms[ray_position.y + 1][ray_position.x - 1]) // LR hit
                { ray_direction = RIGHT; was_reflected = true; }
            else if (atoms[ray_position.y - 1][ray_position.x - 1]) // R hit
                { ray_direction = DOWN;  was_reflected = true; }
            else if (atoms[ray_position.y + 1][ray_position.x - 1]) // L hit
                { ray_direction = UP;    was_reflected = true; }
        }
        else if (ray_direction == DOWN) {
            if (atoms[ray_position.y + 1][ray_position.x]) // D hit
                {
                    #ifndef __cplusplus
                        return (MarkerAtom){ HIT, {cursor.x, cursor.y} };
                    #else
                        MarkerAtom mark;
                        mark.type = HIT;
                        mark.data.point = {cursor.x, cursor.y};
                        return mark;
                    #endif
                }
            else if (atoms[ray_position.y + 1][ray_position.x - 1] && atoms[ray_position.y + 1][ray_position.x + 1]) // LR hit
                { ray_direction = UP;    was_reflected = true; }
            else if (atoms[ray_position.y + 1][ray_position.x - 1]) // L hit
                { ray_direction = RIGHT; was_reflected = true; }
            else if (atoms[ray_position.y + 1][ray_position.x + 1]) // R hit
                { ray_direction = LEFT;  was_reflected = true; }
        }
        else if (ray_direction == UP) {
            if (atoms[ray_position.y - 1][ray_position.x]) // D hit
                {
                    #ifndef __cplusplus
                        return (MarkerAtom){ HIT, {cursor.x, cursor.y} };
                    #else
                        MarkerAtom mark;
                        mark.type = HIT;
                        mark.data.point = {cursor.x, cursor.y};
                        return mark;
                    #endif
                }
            else if (atoms[ray_position.y - 1][ray_position.x - 1] && atoms[ray_position.y - 1][ray_position.x + 1]) // LR hit
                { ray_direction = DOWN;  was_reflected = true; }
            else if (atoms[ray_position.y - 1][ray_position.x - 1]) // L hit
                { ray_direction = RIGHT; was_reflected = true; }
            else if (atoms[ray_position.y - 1][ray_position.x + 1]) // R hit
                { ray_direction = LEFT;  was_reflected = true; }
        }

        // because of a previous check and set for D collision only R and L check is needed
        // to detect direct reflections
        if (((ray_position.x == cursor.x) && (ray_position.y == cursor.y)) && was_reflected)
            {
                #ifndef __cplusplus
                    return (MarkerAtom){ REFLECTION, {cursor.x, cursor.y} };
                #else
                    MarkerAtom mark;
                    mark.type = REFLECTION;
                    mark.data.point = {cursor.x, cursor.y};
                    return mark;
                #endif
            }
        
        // move the ray
        switch (ray_direction) {
        case RIGHT: ray_position.x++; break;
        case LEFT:  ray_position.x--; break;
        case UP:    ray_position.y--; break;
        case DOWN:  ray_position.y++; break;
        }
    }

}


void display_board(Marker board[12][12], bool atoms[12][12], uint8_t last_index, Point cursor, BoardPrinterOptions opt) {
    printf("%c", B_LEFT_UP);
    for (uint8_t i = 0; i <= last_index; i++) { printf("%c", B_HBEAM); }
    printf("%c\n" ,B_RIGHT_UP);

    // I know it's bearly readable
    // I couldn't find a better way to do this
    // NOTICE: if statements first check for indexes then for tile type!
    for (uint8_t i = 0; i <= last_index; i++) {
        printf("%c", B_VBEAM);
        for (uint8_t j = 0; j <= last_index; j++) {
            if (((i == cursor.y) && (j == cursor.x)) && opt & SHOW_CURSOR) { printf("@"); }
            else if (atoms[i][j]) { 
                if (opt & SHOW_ATOMS) { printf("o"); }
                else if (opt & SHOW_CORRECT_HITS) { if (board[i][j].type == MARK) { printf("O"); } else { printf("o"); } }
                else if (opt & SHOW_MARKERS) { if (board[i][j].type == MARK) { printf("o"); } else { printf("%c", B_FILL); } }
                else { printf("%c", B_FILL); }
            }
            else if (board[i][j].type == HIT)        { printf(GREEN("H")); }
            else if (board[i][j].type == REFLECTION) { printf(RED("R")); } 
            else if (board[i][j].type == SNAKE)      { printf(BLUE("%01x"), board[i][j].number); } 
            else if (board[i][j].type == MARK) {
                if (opt & SHOW_MARKERS) { printf("o"); }
                else if (opt & SHOW_CORRECT_HITS) { printf("X"); }
                else { printf("%c", B_FILL); }
            } 
            else if ((i == 0) || (i == last_index) || (j == 0) || (j == last_index)) { printf("%c" ,B_BORDER); }
            else { printf("%c", B_FILL); }
        }
        printf("%c\n", B_VBEAM);
    }

    printf("%c", B_LEFT_DOWN);
    for (uint8_t i = 0; i <= last_index; i++) { printf("%c", B_HBEAM); }
    printf("%c\n", B_RIGHT_DOWN);
}


void run_menu(GameState *game_state, uint8_t *last_board_index) {
    char input;
    puts(STARTING_SCREEN);
    printf("> "); // Prompt
    scanf("%c", &input);

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
        *game_state = QUIT;
        break;
    default: break;
    }
    
    __cls;
}


void run_game(GameState *game_state, uint8_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    Point cursor = {0, 0};
    char input;
    uint8_t number_of_atoms;
    uint8_t check_number = 0;
    uint8_t marker_count = 0;
    memset(board, 0, 12 * 12 * sizeof(Marker));
    memset(atoms, 0, 12 * 12 * sizeof(bool));
    MoveHistory history = { {'\0'}, 0, 0};

    // setup
    switch (last_board_index) {
    case 6:  number_of_atoms = 3; break;
    case 9:  number_of_atoms = 5; break;
    case 11: number_of_atoms = 8; break;
    }

    for (uint8_t i = 0; i < number_of_atoms;) {
        uint8_t x = ((uint8_t)rand() % (last_board_index - 1)) + 1;
        uint8_t y = ((uint8_t)rand() % (last_board_index - 1)) + 1;

        // check if atom exists
        if (!atoms[y][x]) { atoms[y][x] = true; i++; }
    }

    // main game loop
    while (*game_state == RUNNING) {
        display_board(board, atoms, last_board_index, cursor, (BoardPrinterOptions)(SHOW_CURSOR | SHOW_MARKERS));
        #ifdef DEBUG
            printf("x:%i y:%i\n", cursor.x, cursor.y);
            printf("pos: %i depth: %i\n", history.position, history.depth);
            for (int i = 0; i < 5; i++) { printf("%c ", history.moves[i].move); } printf("\n");
            for (int i = 0; i < 5; i++) { if (i == (((history.position - history.depth) % 5) + 5) % 5) { printf("^ "); } else { printf("  "); } } printf("\n");
            printf("history index: %i\n", (history.position - history.depth) % 5);
            printf("unclamped: %i\n", history.position - history.depth);
        #endif
        printf("markers left: %i\n", number_of_atoms - marker_count);
        printf("> "); // Prompt
        scanf("%c", &input);

        // C++ compiler screams at me if I initialize it more than one time
        // C compiler doesn't and I don't know why
        // anyhow I'm initializing it here
        MarkerAtom marker_atom;
        int history_cursor;

        switch (input) {
        case 'W': case 'w':
            if (cursor.y > 0)                { cursor.y--; }
            history.moves[(((history.position - history.depth) % 5) + 5) % 5].move = 'w';
            if (history.depth > 0) { history.depth--; }
            else                   { history.position = (history.position + 1 + 5) % 5; }
            break;
        case 'S': case 's':
            if (cursor.y < last_board_index) { cursor.y++; }
            history.moves[(((history.position - history.depth) % 5) + 5) % 5].move = 's';
            if (history.depth > 0) { history.depth--; }
            else                   { history.position = (history.position + 1 + 5) % 5; }
            break;
        case 'A': case 'a':
            if (cursor.x > 0)                { cursor.x--; }
            history.moves[(((history.position - history.depth) % 5) + 5) % 5].move = 'a';
            if (history.depth > 0) { history.depth--; }
            else                   { history.position = (history.position + 1 + 5) % 5; }
            break;
        case 'D': case 'd':
            if (cursor.x < last_board_index) { cursor.x++; }
            history.moves[(((history.position - history.depth) % 5) + 5) % 5].move = 'd';
            if (history.depth > 0) { history.depth--; }
            else                   { history.position = (history.position + 1 + 5) % 5; }
            break;
        case 'U': case 'u':
            if (history.depth < 4) {
                history.depth++;
                history_cursor = (((history.position - history.depth) % 5) + 5) % 5;
                
                switch (history.moves[history_cursor].move) {
                case 'w': if (cursor.y < last_board_index) { cursor.y++; } break;
                case 's': if (cursor.y > 0)                { cursor.y--; } break;
                case 'a': if (cursor.x < last_board_index) { cursor.x++; } break;
                case 'd': if (cursor.x > 0)                { cursor.x--; } break;
                case 'o':
                    if ( !((cursor.x == 0) || (cursor.x == last_board_index) ||
                           (cursor.y == 0) || (cursor.y == last_board_index)))
                        {
                            if (board[cursor.y][cursor.x].type != MARK) {
                                #ifndef __cplusplus
                                    board[cursor.y][cursor.x] = (Marker){MARK, 0};
                                #else
                                    board[cursor.y][cursor.x] = {MARK, 0};
                                #endif
                            } else {
                                #ifndef __cplusplus
                                    board[cursor.y][cursor.x] = (Marker){EMPTY, 0};
                                #else
                                    board[cursor.y][cursor.x] = {EMPTY, 0};
                                #endif
                            }
                        }
                    break;
                case ' ':
                    marker_atom = history.moves[history_cursor].data;
                    if (history.moves[history_cursor].data.type == SNAKE) {
                        board[marker_atom.data.two_point.f.y][marker_atom.data.two_point.f.x].type = EMPTY;
                        board[marker_atom.data.two_point.l.y][marker_atom.data.two_point.l.x].type = EMPTY;
                        board[marker_atom.data.two_point.f.y][marker_atom.data.two_point.f.x].number = 0;
                        board[marker_atom.data.two_point.l.y][marker_atom.data.two_point.l.x].number = 0;
                        check_number--;
                    } else {
                        board[marker_atom.data.point.y][marker_atom.data.point.x].type = EMPTY;
                    }
                    break;

                default: break;
                }
            }
            break;
        case 'R': case 'r':
            if (history.depth > 0) {
                switch (history.moves[(((history.position - history.depth) % 5) + 5) % 5].move) {
                case 'w': if (cursor.y > 0)                { cursor.y--; } break;
                case 's': if (cursor.y < last_board_index) { cursor.y++; } break;
                case 'a': if (cursor.x > 0)                { cursor.x--; } break;
                case 'd': if (cursor.x < last_board_index) { cursor.x++; } break;
                case 'o':
                    if ( !((cursor.x == 0) || (cursor.x == last_board_index) ||
                           (cursor.y == 0) || (cursor.y == last_board_index)))
                        {
                            if (board[cursor.y][cursor.x].type != MARK) {
                                #ifndef __cplusplus
                                    board[cursor.y][cursor.x] = (Marker){MARK, 0};
                                #else
                                    board[cursor.y][cursor.x] = {MARK, 0};
                                #endif
                            } else {
                                #ifndef __cplusplus
                                    board[cursor.y][cursor.x] = (Marker){EMPTY, 0};
                                #else
                                    board[cursor.y][cursor.x] = {EMPTY, 0};
                                #endif
                            }
                        }
                    break;
                case ' ':
                    history_cursor = (((history.position - history.depth) % 5) + 5) % 5;
                    marker_atom = history.moves[history_cursor].data;
                    if (history.moves[history_cursor].data.type == SNAKE) {
                        board[marker_atom.data.two_point.f.y][marker_atom.data.two_point.f.x].type = marker_atom.type;
                        board[marker_atom.data.two_point.l.y][marker_atom.data.two_point.l.x].type = marker_atom.type;
                        board[marker_atom.data.two_point.f.y][marker_atom.data.two_point.f.x].number = check_number;
                        board[marker_atom.data.two_point.l.y][marker_atom.data.two_point.l.x].number = check_number;
                        check_number++;
                    } else {
                        board[marker_atom.data.point.y][marker_atom.data.point.x].type = history.moves[history_cursor].data.type;
                    }
                default: break;
                }
                history.depth--;
            }
            break;
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
                if (board[cursor.y][cursor.x].type == EMPTY) {
                        history_cursor = (((history.position - history.depth) % 5) + 5) % 5;
                        MarkerAtom mark = check_hit(cursor, last_board_index, atoms);
                        history.moves[history_cursor].move = ' ';
                        if (mark.type == SNAKE) {
                            board[mark.data.two_point.f.y][mark.data.two_point.f.x].type = mark.type;
                            board[mark.data.two_point.l.y][mark.data.two_point.l.x].type = mark.type;
                            board[mark.data.two_point.f.y][mark.data.two_point.f.x].number = check_number;
                            board[mark.data.two_point.l.y][mark.data.two_point.l.x].number = check_number;
                            history.moves[history_cursor].data.type = SNAKE;
                            history.moves[history_cursor].data.data.two_point = mark.data.two_point;
                            check_number++;
                        } else {
                            history.moves[history_cursor].data.type = mark.type;
                            history.moves[history_cursor].data.data.point = mark.data.point;
                            board[mark.data.point.y][mark.data.point.x].type = mark.type;
                        }
                        if (history.depth > 0) { history.depth--; }
                        else                   { history.position = (history.position + 1 + 5) % 5; }
                    }
            }
            break;
        case 'o':
            if ( !((cursor.x == 0) || (cursor.x == last_board_index) ||
                   (cursor.y == 0) || (cursor.y == last_board_index))) {
                    if (board[cursor.y][cursor.x].type != MARK) {
                        if (marker_count < number_of_atoms) {
                            #ifndef __cplusplus
                                board[cursor.y][cursor.x] = (Marker){MARK, 0};
                            #else
                                board[cursor.y][cursor.x] = {MARK, 0};
                            #endif
                            marker_count++;
                        }
                    } else {
                        #ifndef __cplusplus
                            board[cursor.y][cursor.x] = (Marker){EMPTY, 0};
                        #else
                            board[cursor.y][cursor.x] = {EMPTY, 0};
                        #endif
                        marker_count--;
                    }
                    history.moves[(((history.position - history.depth) % 5) + 5) % 5].move = 'o';
                    if (history.depth > 0) { history.depth--; }
                    else                   { history.position = (history.position + 1 + 5) % 5; }
            }
            break;
        case 'k': scanf("%*c"); *game_state = END;   break;
        case 'p': scanf("%*c"); *game_state = CHECK; break;
        case 'h': case 'H':
            __cls;
            display_board(board, atoms, last_board_index, cursor, SHOW_ATOMS);
            SLEEP(3);
            break;
        case 'Q': case 'q': scanf("%*c"); *game_state = MENU; break;
        default: break;
        }

        
        __cls;
    }
}


void run_end(GameState *game_state, uint8_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    uint8_t score = 0;
    uint8_t number_of_atoms;

    #ifndef __cplusplus
        display_board(board, atoms, last_board_index, (Point){0, 0}, SHOW_CORRECT_HITS);
    #else
        display_board(board, atoms, last_board_index, {0, 0}, SHOW_CORRECT_HITS);
    #endif
    for (uint8_t i = 0; i < last_board_index; i++) {
        for (uint8_t j = 0; j < last_board_index; j++) {
            if ((board[i][j].type == MARK) && atoms[i][j]) { score++; }
        }
    }
    
    switch (last_board_index) {
    case 6:  number_of_atoms = 3; break;
    case 9:  number_of_atoms = 5; break;
    case 11: number_of_atoms = 8; break;
    }

    printf("Your score is: " GREEN(BOLD("%i")) "\n", score);
    if (score == number_of_atoms) { printf(BOLD("Congratulations! You've marked all of the atoms!\n")); }
    puts("press ENTER to continue...");
    scanf("%*c");
    __cls;
    *game_state = MENU;
}


void run_check(GameState *game_state, uint8_t last_board_index, Marker board[12][12], bool atoms[12][12]) {
    #ifndef __cplusplus
        display_board(board, atoms, last_board_index, (Point){0, 0}, SHOW_ATOMS);
    #else
        display_board(board, atoms, last_board_index, {0, 0}, SHOW_ATOMS);
    #endif
    puts("press ENTER to continue...");
    scanf("%*c");
    __cls;
    *game_state = MENU;
}
