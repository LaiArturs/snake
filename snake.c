#include <curses.h>  // for display
#include <unistd.h>  // for sleep()
#include <stdlib.h>  // for srand()
#include <time.h>  // for time()

#define START_LENGTH 5
#define MIN_WINDOW_WIDTH 58
#define MIN_WINDOW_HEIGHT 14

char getDirection(char cd, WINDOW *win);

typedef struct snakeSection {
    short int y;
    short int x;
    struct snakeSection *next;
} snakeSection_t;


typedef struct {
    unsigned int length;
    snakeSection_t *head;
    char direction;  // 'U' - UP, 'D' - down, 'L' - left, 'R' - right
} snake_t;

typedef struct {
    unsigned int y;
    unsigned int x;
    char foodDisplayed;  // 0 - not displayed, 1 - is displayed
} food_t;

snake_t initializeSnake(void);
void removeSnakeEndFromDisplay(snake_t snake, WINDOW *win);
void drawSnake(snake_t snake, WINDOW *win);
void advanceSnakeForward(snake_t *snake);
snakeSection_t *returnTail(snake_t snake);
char detectCollisions(snake_t snake);

char initNewGame(snake_t *gameSnake, WINDOW *win, unsigned int *score, food_t *food);
char looseMenu(WINDOW *win, unsigned int score);
char playSnake(snake_t *gameSnake, WINDOW *win, unsigned int *score, food_t *food);
char pauseMenu(WINDOW *win, unsigned int score);
char mainMenu(WINDOW *win);
char aboutMenu(WINDOW *win);

int windowWidth;
int windowHeight;
int input;  // to read keyboard input

int main(void)
{   
    unsigned int score = 0;
    food_t food;  // Hold information about food;
    

    /*      gameControl
    *   This variable determines what game should do next
    *   i - initialize new game;
    *   g - play/continue game;
    *   l - lose menu;
    *   m - main menu;
    *   p - pause menu;
    *   m - main menu
    */
    char gameControl = 'i';

    initscr();  // initialize curses
    windowWidth = COLS;
    windowHeight = LINES;
    if (windowHeight < MIN_WINDOW_HEIGHT || windowWidth < MIN_WINDOW_WIDTH) {
        printf("Min window size is %ix%i. Please resize your terminal window!\n", MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
        delwin(stdscr);
        endwin();
        refresh();
        return 1;
    }

    WINDOW *win = newwin(windowHeight-2, windowWidth-2, 1, 1);
    WINDOW *looseWindow;
    cbreak();  // return key immediately
    noecho();  // dont print which key is pressed
    nodelay(win, TRUE);  // use non blocking for getch()
    keypad(win, TRUE);  // allow special keys
    curs_set(0);  // hide cursor
    snake_t gameSnake = initializeSnake();
    
    //box(win, ACS_VLINE, ACS_HLINE );
    box(win, '|', ACS_HLINE );
    
    //mvwprintw( win, 10, 15, "%c", 64 );wrefresh(win);

    // draw first snake
    drawSnake(gameSnake, win);

    wrefresh(win);

    // Main Game loop
    while (1)
    {

        switch (gameControl) {
        
            // Initialize new game
            case 'i':
                gameControl = initNewGame(&gameSnake, win, &score, &food);
                break;
            
            // Play/continue snake game
            case 'g':
                gameControl = playSnake(&gameSnake, win, &score, &food);
                break;

            // Loose game menu;
            case 'l':
                gameControl = looseMenu(looseWindow, score);
                break;
            
            // Pause menu
            case 'p':
                gameControl = pauseMenu(looseWindow, score);
                break;
            
            // Main menu
            case 'm':
                gameControl = mainMenu(win);
                break;
            
            // About
            case 'a':
                gameControl = aboutMenu(win);
                break;

            // Quit game
            case 'q':
                delwin(win);
                delwin(looseWindow);
                delwin(stdscr);
                endwin();
                refresh();
                return 0;

            default:
                break;
        }

    }
    
    return 0;
}
snake_t initializeSnake(void) {
    snake_t gameSnake;
    gameSnake.length = 0;
    gameSnake.direction = 'R';
    snakeSection_t *currentSection = NULL;
    currentSection = malloc(sizeof(snakeSection_t));
    if (currentSection==NULL) {
        // could not allocate memory for snake section
        return gameSnake;
    }
    gameSnake.head = currentSection;
    
    for (int i = 0; i < 5; i++) {
        currentSection->y = windowHeight/2;
        currentSection->x = (windowWidth/2)-i;
        currentSection->next = NULL;
        gameSnake.length++;
        if (i != 5-1){
            currentSection->next = malloc(sizeof(snakeSection_t));
            if (currentSection==NULL) {
                // could not allocate memory for snake section
                return gameSnake;
            }
            currentSection = currentSection->next;    
        }
    }
    
    return gameSnake;
}

void removeSnakeEndFromDisplay(snake_t snake, WINDOW *win) {
    snakeSection_t *currentSection = snake.head;
    while (currentSection->next!=NULL) {
        currentSection = currentSection->next;
    }
    mvwaddch(win, currentSection->y, currentSection->x, ' ');
}

void drawSnake(snake_t snake, WINDOW *win) {
    snakeSection_t *currentSection = snake.head;
    mvwaddch(win, currentSection->y, currentSection->x, '&');
    while (currentSection->next!=NULL)
    {
        currentSection = currentSection->next;
        mvwaddch(win, currentSection->y, currentSection->x, '@');
    }
}

void advanceSnakeForward(snake_t *snake) {
    snakeSection_t *newHead = NULL;
    newHead = malloc(sizeof(snakeSection_t));
    newHead->x = snake->head->x;
    newHead->y = snake->head->y;
    newHead->next = snake->head;
    snake->head = newHead;
    
    switch (snake->direction)
        {
        case 'U':
            newHead->y--;
            break;
        case 'D':
            newHead->y++;
            break;
        case 'R':
            newHead->x++;
            break;
        case 'L':
            newHead->x--;
            break;
        
        default:
            // Error case
            break;
        }

    snakeSection_t *currentSection = snake->head;
    for (size_t i = 1; i < snake->length; i++)
    {
        currentSection = currentSection->next;
    }

    if (currentSection->next != NULL) {
        free(currentSection->next);
        currentSection->next=NULL;
    } 
}

snakeSection_t *returnTail(snake_t snake) {
    snakeSection_t *currentSection = snake.head;
    while (currentSection->next!=NULL)
    {
        currentSection = currentSection->next;
    }
    return currentSection;    
}

char detectCollisions(snake_t snake) {
    // Detect wall collisions
    if (snake.head->y == 0 || snake.head->x == 0 || snake.head->y == windowHeight-3 || snake.head->x == windowWidth-3){
        return 1;
    }
    // Detect self biting
    snakeSection_t *currentSection = snake.head;
    while (currentSection->next != NULL) {
        currentSection = currentSection->next;
        if(currentSection->x == snake.head->x && currentSection->y == snake.head->y) {
            return 1;
        }
    }
    return 0;    
}


char getDirection(char cd, WINDOW *win) {
    int ch;
    char key;
    if ((ch = wgetch(win)) == ERR) {
        return 0;
    }
    else {
        switch (ch)
        {
        case KEY_DOWN:
            key = 'D';
            break;
        
        case KEY_UP:
            key = 'U';
            break;

        case KEY_LEFT:
            key = 'L';
            break;

        case KEY_RIGHT:
            key = 'R';
            break;
        
        case 'q': // ESC
            key = 'p';
            return key;
            break;
        
        default:
            return 0;
        }
    }
    if ((key == 'U' && cd == 'D') || (key == 'D' && cd == 'U') ||
        (key == 'R' && cd == 'L') || (key == 'L' && cd == 'R') ) {
        return 0;
    } else
    {
        return key;
    }
    
    
}
char looseMenu(WINDOW *win, unsigned int score){
    win = newwin(10, 20, (windowHeight-10)/2, (windowWidth-20)/2);
    nodelay(win, TRUE);  // use non blocking for getch()
    keypad(win, TRUE);  // allow special keys
    box(win, '|', ACS_HLINE );
    mvwprintw(win, 1, 5, "GAME OVER!");
    mvwprintw(win, 3, 1, "Score: %d", score*10);
    mvwprintw(win, 5, 1, "Restart - Enter");
    mvwprintw(win, 6, 1, "Main menu - q");
    wrefresh(win);
    while (1)
    {   
        usleep(100000);
        if ((input = wgetch(win)) != ERR) {
            if (input == 10) {
                return 'i';
            } else if (input == 'q') {
                return 'm';
            }
        }
    }
}

char pauseMenu(WINDOW *win, unsigned int score) {
    win = newwin(10, 20, (windowHeight-10)/2, (windowWidth-20)/2);
    nodelay(win, TRUE);  // use non blocking for getch()
    keypad(win, TRUE);  // allow special keys
    box(win, '|', ACS_HLINE );
    mvwprintw(win, 1, 5, "PAUSE MENU");
    mvwprintw(win, 3, 1, "Score: %d", score*10);
    mvwprintw(win, 5, 1, "Return - Enter");
    mvwprintw(win, 6, 1, "Restart - Space");
    mvwprintw(win, 7, 1, "Main menu - q");
    wrefresh(win);
    while (1)
    {   
        usleep(100000);
        if ((input = wgetch(win)) != ERR) {
            if (input == 10) {
                return 'g';
            } else if (input == ' ') {
                return 'i';
            } else if (input == 'q') {
                return 'm';
            }
        }
    }
}

char mainMenu(WINDOW *win) {
    wclear(win);
    box(win, '|', ACS_HLINE );
    mvwprintw(win, 2, (windowWidth/2)-5, "__SNAKE__");
    mvwprintw(win, 3, (windowWidth/2)-5, "@@@@@@@&   *");
    mvwprintw(win, 5, 5, "New game - Enter");
    mvwprintw(win, 6, 5, "Quit - q");
    mvwprintw(win, 7, 5, "About - a");
    wrefresh(win);
    while (1)
    {   
        usleep(100000);
        if ((input = wgetch(win)) != ERR) {
            if (input == 10) {
                return 'i';
            } else if (input == 'q') {
                return 'q';
            } else if (input == 'a') {
                return 'a';
            }
        }
    }
}

char aboutMenu(WINDOW *win) {
    wclear(win);
    box(win, '|', ACS_HLINE );
    mvwprintw(win, 2, (windowWidth/2)-5, "ABOUT");
    mvwprintw(win, 4, 5, "This game is dedicated to my friend, APE.");
    mvwprintw(win, 5, 5, "Even if you feel as chasing impossible goals,");
    mvwprintw(win, 6, 5, "smashing your head against the wall and biting");
    mvwprintw(win, 7, 5, "yourself, remember that you have another chance");
    mvwprintw(win, 8, 5, "to be better for yourself.");
    mvwprintw(win, 9, 45, "-Arturs");
    mvwprintw(win, windowHeight-4, 5, "Quit - q");
    wrefresh(win);
    while (1)
    {   
        usleep(100000);
        if ((input = wgetch(win)) != ERR) {
            if (input == 'q') {
                return 'm';
            }
        }
    }
}

char initNewGame(snake_t *gameSnake, WINDOW *win, unsigned int *score, food_t *food) {
    *score = 0;
    food->foodDisplayed = 0;
    wclear(win);
    win = newwin(windowHeight-2, windowWidth-2, 1, 1);
    nodelay(win, TRUE);
    keypad(win, TRUE);
    curs_set(0);
    *gameSnake = initializeSnake();
    wrefresh(win);
    return 'g';
}

char playSnake(snake_t *gameSnake, WINDOW *win, unsigned int *score, food_t *food) {
    box(win, '|', ACS_HLINE );
    drawSnake(*gameSnake, win);
    wrefresh(win);
    while (1) {
        usleep(100000);
        // check if food eaten
        if (food->foodDisplayed) {
            if (gameSnake->head->y == food->y && gameSnake->head->x == food->x) {
                food->foodDisplayed = 0;
                gameSnake->length++;
                *score += 1;
            }
        }
        // remove last bit of snake
        removeSnakeEndFromDisplay(*gameSnake, win);
        
        // Get dirrection from user
        char newDirrection;
        newDirrection = getDirection(gameSnake->direction, win);
        if (newDirrection != 0) {
            if (newDirrection == 'p') {
                return 'p';
            }
            gameSnake->direction = newDirrection;
        }
        
        // move snake head forward
        advanceSnakeForward(gameSnake);
        mvwaddch(win, gameSnake->head->y, gameSnake->head->x, '&'); //head
        mvwaddch(win, gameSnake->head->next->y, gameSnake->head->next->x, '@'); //first part of neck
        snakeSection_t *tail = returnTail(*gameSnake);
        // calculate food location
        snakeSection_t *currentSection;  // to iterate over snake
        if (!food->foodDisplayed) {
            unsigned int r;
            srand ( time(NULL) );
            char foodOnSnake = 0;
            do {
                r = rand();
                food->y = (r % (windowHeight - 4)) + 1;
                food->x = (r % (windowWidth - 4)) + 1;
                foodOnSnake = 0;
                currentSection = gameSnake->head;
                while (currentSection!=NULL) {
                    if (currentSection->y == food->y && currentSection->x == food->x) {
                        foodOnSnake = 1;
                        break;
                    }
                    currentSection = currentSection->next;
                }    
            } while (foodOnSnake);
            mvwaddch(win, food->y, food->x, '*');
            food->foodDisplayed = 1;
        }

        // Check if snake hits walls or itself
        if (detectCollisions(*gameSnake)) {
            wrefresh(win);
            return 'l';
            break;
        }

        wrefresh(win);
    }
}
