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

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <simpl.h>
#include <ncurses.h>
#include "message.h"

/* data */
typedef struct _node
{
    MESSAGE reply;
    struct _node* next;
} fifo;
fifo *head, *tail;
ARENA arena;  // store current arena
char *fromw = NULL;
MESSAGE msg, reply;
int maxr, maxc;  // screen size
int cnum = 0, tnum = 0, cournum = 0;
int flag = 0;  // 0: not finish; 1: 0 win;  2: 1 win;  3: draw
/* functions */
void registration(void);
void game(void);
void finish(void);  // notify & !!!clear memory!!!
void myreply(char*, MESSAGE*);  // need to take para, as there may be waiting worker
void step(int, int);  // avaialble or not
int check(void);  // check the status & change color
void init_arena(void);
void die(void);
void push(void);
void pop(void);
COORDINATE get_nextpos(COORDINATE, DIRECTION);

/* main program */
int main(void)
{
    // init
    if (name_attach("Game_Admin", NULL) == -1)  die();
    registration();
    game();
    finish();
    if (name_detach() == -1)  die();
    return 0;
}

/* implementations */
void registration(void)
{
    // store the fromw of ready component
    char *cy[2] = {NULL, NULL}, *cour = NULL, *timer = NULL;
    // init arena
    init_arena();
    /* registration */
    while (cy[0] == NULL || cy[1] == NULL || cour == NULL || timer == NULL)  //when someone is not ready
    {
        if (Receive(&fromw, &msg, sizeof(msg)) == -1)  die();
        switch (msg.type)
        {
            case REGISTER_CYCLE:
                if (cnum == 2)  {reply.type = FAIL; myreply(fromw, &reply);}
                else            {reply.type = INIT; reply.cycleId = cnum++; myreply(fromw, &reply);}
                break;
            case REGISTER_TIMER:
                if (tnum)    {reply.type = FAIL; myreply(fromw, &reply);}
                else          {tnum++; reply.type = INIT; myreply(fromw, &reply);}
                break;
            case REGISTER_COURIER:
                if (cournum)     {reply.type = FAIL; myreply(fromw, &reply);}
                else          {cournum++; reply.type = INIT; myreply(fromw, &reply);}
                break;
            case REGISTER_HUMAN:
                if (cnum == 2)  {reply.type = FAIL; myreply(fromw, &reply);}
                else              {reply.type = INIT; reply.humanId = cnum++; myreply(fromw, &reply);}
                break;
            // store in fifo
            case CYCLE_READY:
                push();  tail->reply.type = START;
                if (cy[0] == NULL)  cy[0] = fromw;
                else                cy[1] = fromw;
                break;
            case HUMAN_READY:
                push();  tail->reply.type = START;  tail->reply.humanId = msg.humanId;
                if (cy[0] == NULL)  cy[0] = fromw;
                else                cy[1] = fromw;
                break;
            case TIMER_READY:
                timer = fromw;  break;
            case COURIER_READY:
                cour = fromw;  break;
            default: break;
        }
    }
    myreply(cy[0], &(head->reply)); pop();  myreply(cy[1], &(head->reply));  pop();
    reply.type = DISPLAY_ARENA;  reply.arena = arena;  myreply(cour, &reply);
    reply.type = SLEEP;  reply.interval = 50000;  myreply(timer, &reply);
}

void game(void)
{
    int cid;  // cycle id
    int bting[2] = {0, 0}, avail[2] = {3, 3};  // booting time
    int btimer = 0;
    push();
    while (!flag)
    {
        if (Receive(&fromw, &msg, sizeof(msg)) == -1)  die();
        switch (msg.type)
        {
            case MOVE:
                cid = msg.cycleId;
            case HUMAN_MOVE:
                if (msg.type == HUMAN_MOVE)
                {
                    cid = msg.humanId;  tail->reply.humanId = cid;
                }
                tail->reply.arena.cycle[cid].dir = msg.dir;  // edit current arena
                if (msg.boost == YES && avail[cid])  // valid if available boost exist
                {
                    avail[cid]--;  bting[cid] += 5;
                }
                tail->reply.type = UPDATE;  myreply(fromw, &(tail->reply));
                break;
            case TIMER_READY:
                // move, create new arena
                if (bting[0] || bting[1])  // boost time
                {
                    if ((bting[0] || !btimer) && (bting[1] || !btimer))  step(1, 1);
                    else if (bting[0] || !btimer)  step(1, 0);  // boost, or timer % 5 == 0
                    else if (bting[1] || !btimer)  step(0, 1);
                    if (bting[0])  bting[0]--;
                    if (bting[1])  bting[1]--;
                    // update
                    btimer = (btimer+1) % 5;
                    reply.interval = 10000;
                }
                else
                {
                    step(1, 1);  reply.interval = 50000;
                }
                reply.type = SLEEP;  myreply(fromw, &reply);
                break;
            case OKAY:
                head->reply.type = DISPLAY_ARENA;  myreply(fromw, &(head->reply));
                if (head != tail)  pop();
                break;
            default: break;
        }
    }
}

void finish(void)
{
    int printed = 0;
    if (flag == 3)  flag = -1;
    else  flag--;
    while (cnum || tnum || cournum)
    {
        if (Receive(&fromw, &msg, sizeof(msg)) == -1) die();
        switch (msg.type)
        {
            case HUMAN_MOVE:
            case MOVE:
                reply.type = END;  myreply(fromw, &reply);  cnum--;
                break;
            case TIMER_READY:
                reply.type = END;  myreply(fromw, &reply);  tnum--;
                break;
            case COURIER_READY:
            case OKAY:
                if (head != tail)  // more to send
                {
                    head->reply.type = DISPLAY_ARENA;
                    myreply(fromw, &(head->reply));  pop();
                    break;
                }
                else if (printed < 350)
                {
                    head->reply.type = DISPLAY_ARENA; printed++;
                }
                else
                {
                    head->reply.type = END;  head->reply.cycleId = flag; cournum--;
                }
                myreply(fromw, &(head->reply));
                break;
            default:  break;
        }
    }
}

void init_arena(void)
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
    initscr(); getmaxyx(stdscr, maxr, maxc); endwin();
    arena.cycle[0].pos = (COORDINATE) { MAX_HEIGHT/2-1, (MAX_WIDTH-maxc+12)/2 };
    arena.cycle[1].pos = (COORDINATE) { MAX_HEIGHT/2, MAX_WIDTH-(MAX_WIDTH-maxc+12)/2};
    arena.cycle[0].dir = EAST;
    arena.cycle[1].dir = WEST;
}

void myreply(char *fromw, MESSAGE* reply)
{
    if (Reply(fromw, reply, sizeof(MESSAGE)) == -1)  die();
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

void step(int s0, int s1)
{
    int row0, col0, row1, col1;
    COORDINATE p0, p1;
    DIRECTION d0, d1;
    row0 = tail->reply.arena.cycle[0].pos.y, row1 = tail->reply.arena.cycle[1].pos.y;
    col0 = tail->reply.arena.cycle[0].pos.x, col1 = tail->reply.arena.cycle[1].pos.x;
    if (s0)
    {
        p0 = tail->reply.arena.cycle[0].pos;
        d0 = tail->reply.arena.cycle[0].dir;
        p0 = get_nextpos(p0, d0);
        // check if can move to that pos
        if (p0.y - row1 > maxr-6 || row1 - p0.y > maxr-6 || \
            p0.x - col1 > maxc-6  || col1 - p0.x > maxc-6)  s0 = 0;  // can't move
    }
    if (s1)
    {
        p1 = tail->reply.arena.cycle[1].pos;
        d1 = tail->reply.arena.cycle[1].dir;
        p1 = get_nextpos(p1, d1);
        // check if can move to that pos
        if (row0 - p1.y > maxr-6 || p1.y - row0 > maxr-6 || \
            col0 - p1.x > maxc-6  || p1.x - col0 > maxc-6)  s1 = 0;  // can't move
    }
    if (s0 && s1)
    {
        if (p0.y - p1.y > maxr-6 || p1.y-p0.y > maxr-6 || \
            p0.x - p1.x > maxc-6 || p1.x-p0.x > maxc-6) s1 = 0;
    }
    if (s0 || s1)
    {
        push();
        if (s0)
        {
            tail->reply.arena.wall[col0][row0] = REDCYCLE;  // color position before
            tail->reply.arena.cycle[0].pos = p0;
            tail->reply.arena.cycle[0].dir = d0;
        }
        if (s1)
        {
            tail->reply.arena.wall[col1][row1] = GREENCYCLE;  // color position before
            tail->reply.arena.cycle[1].pos = p1;
            tail->reply.arena.cycle[1].dir = d1;
        }
        flag = check();
    }
}

int check(void)
{
    COORDINATE pos0 = tail->reply.arena.cycle[0].pos;
    COORDINATE pos1 = tail->reply.arena.cycle[1].pos;
    int r0 = pos0.y, r1 = pos1.y, c0 = pos0.x, c1 = pos1.x;
    if (r0 == r1 && c0 == c1)  return 3;
    if (tail->reply.arena.wall[c0][r0] != NONE && tail->reply.arena.wall[c1][r1] != NONE)
        return 3;
    if (tail->reply.arena.wall[c0][r0] != NONE)   return 2;  // collision
    if (tail->reply.arena.wall[c1][r1] != NONE)   return 1;
    return 0;
}

void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}

void push(void)
{
    if (head == NULL)
    {
        head = (fifo*) malloc(sizeof(fifo));  tail = head;  head->next = NULL;  head->reply.arena = arena;
    }
    else
    {
        tail->next = (fifo*) malloc(sizeof(fifo));  tail->next->reply = tail->reply;  tail = tail->next; // pass value to next
        tail->next = NULL;
    }
}

void pop(void)
{
    if (head == NULL)  return;
    fifo* tmp = head;  head = head->next;  free(tmp);
}
