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
*  Computer Cycle
*  Workers for game_admin
**/
#include <sys/types.h>
#include <simpl.h>
#include <stdio.h>
#include <stdlib.h>
#include "message.h"
/* data */
MESSAGE msg, reply;
CYCLE cycle;
ARENA arena;
int cid;  // id
int admin;
int bnum = 3;  // remain boost
//test

/* functions */
void ai(DIRECTION*, BOOST*);
void die(void);
void game(void);
void mysend(void);
void registration(void);
/* main */
int main(int argc, char* argv[])
{
    char name[20];

    if (argv[1][1])  { fprintf(stderr, "argument wrong!\n"); exit(-1); }
    if (argv[1][0] != '0' && argv[1][0] != '1')
    {
        fprintf(stderr, "argument wrong!\n");
        exit(-1);
    }
    sprintf(name, "Cycle %d", argv[1][0] - '0');
    if (name_attach(name, NULL) == -1)  die();
    if ((admin = name_locate("Game_Admin")) == -1)  die();
    registration();
    game();
    if (name_detach())  die();
    return 0;
}

void registration(void)
{
    msg.type = REGISTER_CYCLE;
    mysend();
    if (reply.type == FAIL)
    {
        fprintf(stderr, "registration failed.\n");
        exit(-1);
    }
    cid = reply.cycleId;
}

void game(void)
{
    msg.type = CYCLE_READY;  msg.cycleId = cid;
    mysend();
    if (reply.type != START)  exit(0);  // game finished
    arena = reply.arena;

    while (1)   // not end
    {
        ai(&msg.dir, &msg.boost);
        msg.type = MOVE; msg.cycleId = cid;
        mysend();
        if (reply.type == UPDATE)   arena = reply.arena;
        else                        break;   // end of game
    }
}

void ai(DIRECTION *dir, BOOST *boost)
{
    // three directions possible
    // cal the nearest alive area within one window
    // maximize the above value
#define WIDTH 10
    const int mover[4] = {0, 1, 0, -1};
    const int movec[4] = {1, 0, -1, 0};
    DIRECTION drev = (arena.cycle[cid].dir+2) % 4;  // reverse dir
    DIRECTION i;
    int pr = arena.cycle[cid].pos.y, pc = arena.cycle[cid].pos.x;
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
    *dir = maxdir;  *boost = NO;
}

void mysend(void)
{
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1)  die();
}
void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}
