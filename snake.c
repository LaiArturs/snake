#include <curses.h>  // for display
#include <unistd.h>  // for sleep()
#include <stdlib.h>  // for srand()
#include <time.h>  // for time()

char getDirection(char cd, WINDOW *win);

int main(void)
{   
    int maxSnakeLength = 100;
    int snake[maxSnakeLength][2];
    int snakeLength = 5;
    char dirrection = 'R'; // 'U' - UP, 'D' - down, 'L' - left, 'R' - right
    char newDirrection;
    char foodDisplayed = 0;  // 0 - not displayed, 1 - is displayed
    unsigned int food[2];
    unsigned int score = 0;

    initscr();  // initialize curses
    WINDOW *win = newwin(LINES-2, COLS-2, 1, 1);
    cbreak();  // return key immediately
    //noecho();  // dont print which key is pressed
    nodelay(win, TRUE);  // use non blocking for getch()
    keypad(win, TRUE);  // allow special keys
    curs_set(0);  // hide cursor
    for (size_t i = 0; i < snakeLength; i++)
    {
        snake[i][0] = LINES/2;
        snake[i][1] = (COLS/2)-i;
    }
    
    //box(win, ACS_VLINE, ACS_HLINE );
    box(win, '|', ACS_HLINE );
    
    //mvwprintw( win, 10, 15, "%c", 64 );wrefresh(win);

    // draw first snake
    mvwaddch( win, snake[0][0], snake[0][1], '&');
    for (size_t i = 1; i < snakeLength; i++) {   
        mvwaddch( win, snake[i][0], snake[i][1], '@');
    }
    wrefresh(win);
    while (1)
    {
        usleep(100000);

        // check if food eaten
        if (foodDisplayed) {
            if (snake[0][0] == food[0] && snake[0][1] == food[1]){
                foodDisplayed = 0;
                snakeLength++;
                score++;
            }
        }


        // remove last bit of snake
        mvwaddch(win, snake[snakeLength - 1][0], snake[snakeLength - 1][1], ' ');
        // Get dirrection from user
        newDirrection = getDirection(dirrection, win);
        if (newDirrection != 0) {
            dirrection = newDirrection;
        }
        // recalculate snake
        for (size_t i = snakeLength; i > 0; i--) {
            snake[i][0] = snake[i-1][0];
            snake[i][1] = snake[i-1][1];
        }
        switch (dirrection)
        {
        case 'U':
            snake[0][0]--;
            break;
        case 'D':
            snake[0][0]++;
            break;
        case 'R':
            snake[0][1]++;
            break;
        case 'L':
            snake[0][1]--;
            break;
        
        default:
            // Error case
            break;
        }

        // move snake head forward
        mvwaddch(win, snake[0][0], snake[0][1], '&'); //head
        mvwaddch(win, snake[1][0], snake[1][1], '@'); //first part of neck

        // calculate food location
        if (!foodDisplayed) {
            unsigned int r;
            srand ( time(NULL) );
            char foodOnSnake = 0;
            do {
                r = rand();
                food[0] = (r % (LINES - 4)) + 1;
                food[1] = (r % (COLS - 4)) + 1;
                foodOnSnake = 0;
                for (size_t i = 0; i < snakeLength; i++)
                {
                    if (snake[i][0] == food[0] && snake[i][1] == food[1]) {
                        foodOnSnake = 1;
                        break;
                    }
                }    
            } while (foodOnSnake);
            mvwaddch(win, food[0], food[1], '*');
            mvwprintw( win, 2, 2, "%d:%d\t%d", food[0], food[1], score*10);
            foodDisplayed = 1;
        }

        wrefresh(win);
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