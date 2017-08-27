/*
 * CSCI3180 Principles of Programming Languages
 *
 * --- Declaration ---
 *
 * I declare that the assignment here submitted is original except for source
 * material explicitly acknowledged. I also acknowledge that I am aware of
 * University policy and regulations on honesty in academic work, and of the
 * disciplinary guidelines and procedures applicable to breaches of such policy
 * and regulations, as contained in the website
 * http://www.cuhk.edu.hk/policy/academichonesty/
 *
 * Assignment 5
 * Name : Miao Ying Long
 * Student ID : 1155046924
 * Email Addr : ylmiao4@cse.cuhk.edu.hk
 */
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "message.h"
/* data */
ARENA arena;
/* functions */
void init_arena(void);
void ai(DIRECTION*);
int get_startr(COORDINATE, int, int);  // get the start row of screen in arena
                                    // given cycle pos
int get_startc(COORDINATE, int, int);
COORDINATE get_nextpos(COORDINATE, DIRECTION);
void get_safedir(void);
int main(void)
{
    // init screen
    int startr, startc;
    int maxr, maxc;
    init_arena();  // init arena
    initscr(); // init screen
    cbreak();  // disable line buffering, signal still pass
    noecho();  // no echo to screen when input
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, maxr, maxc);
    curs_set(0);  // set cursor invisable
    start_color();
    // define color pair
    init_pair(REDCYCLE, COLOR_RED, COLOR_BLACK);  // head: @, body: ' '
    init_pair(GREENCYCLE, COLOR_GREEN, COLOR_BLACK);
    init_pair(BOUNDARY, COLOR_WHITE, COLOR_BLACK);
    clear();
    //get input and handle, every 0.05 sec
    while (1)
    {
        //get current start pos & print
        startr = get_startr(arena.cycle[0].pos, maxr, maxc);
        startc = get_startc(arena.cycle[0].pos, maxr, maxc);
        move(0, 0);  // move cursor
        int i, j;
        for (i = 0; i < maxr; i++)
            for (j = 0; j < maxc; j++)
            {
                if (i + startr < 0 || i + startr >= MAX_HEIGHT || \
                    j + startc < 0 || j + startc >= MAX_WIDTH)
                {
                    // out of boundary
                    addch(' ');  continue;
                }
                if (i+startr == arena.cycle[0].pos.y && \
                    j+startc == arena.cycle[0].pos.x)
                {
                    addch('@' | COLOR_PAIR(REDCYCLE));
                    continue;
                }
                switch (arena.wall[j+startc][i+startr])
                {
                    case NONE:       addch(' '); break;
                    case REDCYCLE:   addch(' ' | COLOR_PAIR(REDCYCLE) | A_REVERSE); break;
                    case GREENCYCLE: addch(' ' | COLOR_PAIR(GREENCYCLE) | A_REVERSE); break;
                    case BOUNDARY:   addch(' ' | COLOR_PAIR(BOUNDARY) | A_REVERSE); break;
                    default:         break;
                }
            }
        refresh();
        // wait 0.05 sec  (50000 microsecond)
        usleep(50000);
        // move & turn
        int cyr, cyc, row, col;
        COORDINATE next = get_nextpos(arena.cycle[0].pos, arena.cycle[0].dir);
        row = next.y; col = next.x;
        if (arena.wall[col][row] != NONE)   break;   // error
        arena.wall[col][row] = REDCYCLE;
        arena.cycle[0].pos = (COORDINATE) {row, col};
        if (rand() % 100 < 85)  ai(&(arena.cycle[0].dir));
        else get_safedir();
    }
    // destroy when collide
    curs_set(1);
    endwin();
    return 0;
}

/* implementation of subroutines */
void init_arena()
{
    // boundary
    int i, j;
    for (i = 0; i < MAX_HEIGHT; i++)
        for (j = 0; j < MAX_WIDTH; j++)
            arena.wall[j][i] = NONE;
    for (j = 0; j < MAX_WIDTH; j++)
    {
        arena.wall[j][0] = BOUNDARY;
        arena.wall[j][MAX_HEIGHT-1] = BOUNDARY;
    }
    for (i = 0; i < MAX_HEIGHT; i++)
    {
        arena.wall[0][i] = BOUNDARY;
        arena.wall[MAX_WIDTH-1][i] = BOUNDARY;
    }

    // set cycle
    srand(time(NULL));
    arena.cycle[0].pos = (COORDINATE){ rand() % (MAX_HEIGHT-1), rand() % (MAX_WIDTH-1) }; // y, x
    arena.cycle[0].dir = (DIRECTION) (rand() % 4);
    arena.wall[arena.cycle[0].pos.x][arena.cycle[0].pos.y] = REDCYCLE;
}

int get_startr(COORDINATE cor, int maxr, int maxc)
{
    return cor.y-maxr/2;
}

int get_startc(COORDINATE cor, int maxr, int maxc)
{
    return cor.x-maxc/2;
}

COORDINATE get_nextpos(COORDINATE pos, DIRECTION dir)
{
    int row = pos.y, col = pos.x;
    switch (dir)
    {
        case EAST:   col++; break;
        case WEST:   col--; break;
        case SOUTH:  row++; break;
        case NORTH:  row--; break;
    }
    return (COORDINATE){row, col};
}

void ai(DIRECTION *dir)
{
    // three directions possible
    // cal the nearest alive area within one window
    // maximize the above value
#define WIDTH 10
    const int mover[4] = {0, 1, 0, -1};
    const int movec[4] = {1, 0, -1, 0};
    DIRECTION drev = (arena.cycle[0].dir+2) % 4;  // reverse dir
    DIRECTION i;
    int pr = arena.cycle[0].pos.y, pc = arena.cycle[0].pos.x;
    int row, col, max = -10, area, maxdir = EAST, count;
    for (i = EAST; i < 4; i++)
    {
        count = 0;
        if (i == drev)  continue;
        // check the shortest distance
        area = 0;  // the minimum distance of this dir
        if (mover[i])  // vertical move
        {
            for (col = pc - WIDTH/2; col <= pc + WIDTH/2; col++)
            {
                if (col < 0 || col >= MAX_WIDTH)  continue;
                count = 0;
                for (row = pr+mover[i]; row >= 0 && row < MAX_HEIGHT && count <= WIDTH; row += mover[i])
                {
                    // find the first BOUNDARY
                    if (arena.wall[col][row] != NONE)    break;
                    count++;
                }
                area += count;
            }
        }
        else
        {
            for (row = pr - WIDTH/2; row <= pr + WIDTH/2; row++)
            {
                if (row < 0 || row >= MAX_HEIGHT)  continue;
                count = 0;
                for (col = pc+movec[i]; col >= 0 && col < MAX_WIDTH && count <= WIDTH; col += movec[i])
                {
                    // find the first BOUNDARY
                    if (arena.wall[col][row] != NONE)    break;
                    count++;
                }
                area += count;
            }
        }
        if (area > max)
        {
            max = area;  maxdir = i;
        }
    }
    *dir = maxdir;  //*boost = NO;
}

void get_safedir()
{
    // can safely assume not out of bound
    int num = 0;
    DIRECTION safe[4];  // store the safe move
    int row, col;
    DIRECTION i;
    for (i = EAST; i < 4; i++)
    {
        if (i == arena.cycle[0].dir - 2 || i == arena.cycle[0].dir + 2)
            continue;  // cannot turn the opposite dir
        COORDINATE next = get_nextpos(arena.cycle[0].pos, i);
        row = next.y; col = next.x;
        if (arena.wall[col][row] == NONE)  safe[num++] = i;
    }
    if (num == 0)  arena.cycle[0].dir = rand() % 4;
    else           arena.cycle[0].dir = safe[rand() % num];  // random get one
}
