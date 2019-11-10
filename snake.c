#include <curses.h>  // for display
#include <unistd.h>  // for sleep()
#include <stdlib.h>  // for srand()
#include <time.h>  // for time()

#define START_LENGTH 5

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

snake_t initializeSnake(void);
void removeSnakeEndFromDisplay(snake_t snake, WINDOW *win);
void drawSnake(snake_t snake, WINDOW *win);
void advanceSnakeForward(snake_t *snake);
snakeSection_t *returnTail(snake_t snake);
char detectCollisions(snake_t snake);

int windowWidth;
int windowHeight;

int main(void)
{   
    char newDirrection;
    char foodDisplayed = 0;  // 0 - not displayed, 1 - is displayed
    unsigned int food[2];
    unsigned int score = 0;
    snakeSection_t *currentSection;  // to iterate over snake

    initscr();  // initialize curses
    windowWidth = COLS;
    windowHeight = LINES;
    WINDOW *win = newwin(windowHeight-2, windowWidth-2, 1, 1);
    cbreak();  // return key immediately
    //noecho();  // dont print which key is pressed
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
    while (1)
    {
        usleep(100000);
        // check if food eaten
        if (foodDisplayed) {
            if (gameSnake.head->y == food[0] && gameSnake.head->x == food[1]) {
                foodDisplayed = 0;
                gameSnake.length++;
                score++;
            }
        }
        // remove last bit of snake
        removeSnakeEndFromDisplay(gameSnake, win);
        
        // Get dirrection from user
        newDirrection = getDirection(gameSnake.direction, win);
        if (newDirrection != 0) {
            gameSnake.direction = newDirrection;
        }
        
        // move snake head forward
        advanceSnakeForward(&gameSnake);
        mvwaddch(win, gameSnake.head->y, gameSnake.head->x, '&'); //head
        mvwaddch(win, gameSnake.head->next->y, gameSnake.head->next->x, '@'); //first part of neck
        snakeSection_t *tail = returnTail(gameSnake);
        // calculate food location
        if (!foodDisplayed) {
            unsigned int r;
            srand ( time(NULL) );
            char foodOnSnake = 0;
            do {
                r = rand();
                food[0] = (r % (windowHeight - 4)) + 1;
                food[1] = (r % (windowWidth - 4)) + 1;
                foodOnSnake = 0;
                currentSection = gameSnake.head;
                while (currentSection!=NULL) {
                    if (currentSection->y == food[0] && currentSection->x == food[1]) {
                        foodOnSnake = 1;
                        break;
                    }
                    currentSection = currentSection->next;
                }    
            } while (foodOnSnake);
            mvwaddch(win, food[0], food[1], '*');
            foodDisplayed = 1;
        }
        if (detectCollisions(gameSnake)) {
            wrefresh(win);
            break;
        }

        wrefresh(win);
    }

    WINDOW *looseWindow = newwin(10, 20, (windowHeight-10)/2, (windowWidth-20)/2);
    box(looseWindow, '|', ACS_HLINE );
    mvwprintw(looseWindow, 1, 5, "Game Over!");
    mvwprintw(looseWindow, 3, 1, "Score: %d", score*10);
    wrefresh(looseWindow);
    while (1)
    {
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