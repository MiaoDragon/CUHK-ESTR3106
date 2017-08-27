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

/**
*  rules of the game
*  positions of the light cycles
*  admin to cycles, couriers, timer
**/

/**
*  paint
*  Workers for display_admin
**/
#include <sys/types.h>
#include <stdlib.h>
#include <simpl.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include "message.h"
/* data */
int admin;
MESSAGE msg, reply;
CYCLE cycle;
int flag = 0;  // if end
int maxr, maxc;
/* functions */
void display_init(void);
COORDINATE get_start(COORDINATE, COORDINATE);
void game(void);
void paint(void);
void show_result(void);
void mysend(void);
void die(void);
void myclean(void);
/* main */
int main(void)
{
    if (name_attach("Painter", NULL) == -1)  die();
    if ((admin = name_locate("Display_Admin")) == -1)  die();
    display_init();
    game();
    if (name_detach() == -1)  die();
    return 0;
}
/* implementations */
void display_init(void)
{
    initscr();  cbreak();  noecho();
    getmaxyx(stdscr, maxr, maxc);  // boundary
    start_color();
    // define color pair
    init_pair((int)REDCYCLE, COLOR_RED, COLOR_BLACK);  // head: @, body: ' '
    init_pair((int)GREENCYCLE, COLOR_GREEN, COLOR_BLACK);
    init_pair((int)BOUNDARY, COLOR_WHITE, COLOR_BLACK);
    curs_set(0);
}

void game(void)
{
    while (!flag)
    {
        msg.type = PAINTER_READY;
        mysend();
        erase();
        if (reply.type == PAINT)
        {
            erase();
            paint();
            refresh();
        }
        else
        {
            flag = 1;
            paint();  show_result();
            refresh();
            sleep(2);
        }
    }
    curs_set(1);
    endwin();
}

void paint(void)
{
    // directly access reply
    getmaxyx(stdscr, maxr, maxc);
    COORDINATE start = get_start(reply.arena.cycle[0].pos, reply.arena.cycle[1].pos);
    int startr = start.y,  startc = start.x;
    int i, j;
    for (i = 0; i < maxr; i++)
    {
        for (j = 0; j < maxc; j++)
        {
            if (i + startr < 0 || i + startr >= MAX_HEIGHT || \
                j + startc < 0 || j + startc >= MAX_WIDTH)
                // out of boundary
                continue;
            if (i+startr == reply.arena.cycle[0].pos.y && \
                j+startc == reply.arena.cycle[0].pos.x && \
                reply.arena.wall[j+startc][i+startr] == NONE)
            {
                attron(COLOR_PAIR((int)REDCYCLE));  move(i,j);
                addch('@');
                attroff(COLOR_PAIR((int)REDCYCLE));
                continue;
            }
            if (i+startr == reply.arena.cycle[1].pos.y && \
                j+startc == reply.arena.cycle[1].pos.x && \
                reply.arena.wall[j+startc][i+startr] == NONE)
            {
                attron(COLOR_PAIR((int)GREENCYCLE)); move(i,j);
                addch('@');
                attroff(COLOR_PAIR((int)GREENCYCLE));
                continue;
            }
            switch (reply.arena.wall[j+startc][i+startr])
            {
                case REDCYCLE:      attron(COLOR_PAIR((int)REDCYCLE)); move(i,j); addch(' ' | A_REVERSE); attroff(COLOR_PAIR((int)REDCYCLE));  break;
                case GREENCYCLE:    attron(COLOR_PAIR((int)GREENCYCLE)); move(i,j); addch(' ' | A_REVERSE); attroff(COLOR_PAIR((int)GREENCYCLE)); break;
                case BOUNDARY:      attron(COLOR_PAIR((int)BOUNDARY)); move(i,j); addch(' ' | A_REVERSE); attroff(COLOR_PAIR((int)BOUNDARY)); break;
                default: break;
            }
        }
    }
}

void show_result(void)
{
    // directly access reply
    move(0, 0);  // move cursor
    if (reply.cycleId == -1)  // draw
        addstr("*      DRAW!      *");
    else  if (reply.cycleId == 0)
        printw("*      RED WINS!      *");
    else  printw("*      GREEN WINS!      *");
}

COORDINATE get_start(COORDINATE pos1, COORDINATE pos2)
{
    // get the origin of screen in arena
    return (COORDINATE) { (pos1.y+pos2.y)/2-maxr/2, (pos1.x+pos2.x)/2-maxc/2 };
}

void myclean(void)
{
    int i, j;
    for (i = 0; i < maxr; i++)  for (j = 0; j < maxc; j++)  addch(' ');
}

void mysend(void)
{
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1)  die();
}

void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}
