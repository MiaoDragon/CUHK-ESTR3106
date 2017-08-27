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
#include <unistd.h>
#include <ncurses.h>
#include <simpl.h>
#include "message.h"
/* data */
int admin, flag;
MESSAGE msg, reply;
/* functions */
void registration(void);
void game(void);
void mysend(void);
void die(void);
/* main program */
int main(void)
{
    if (name_attach("Keyboard", NULL) == -1)  die();
    if ((admin = name_locate("Input_Admin")) == -1)  die();
    registration();
    game();
    if (name_detach() == -1)  die();
    return 0;
}
/* implementations */

void registration(void)
{
    msg.type = REGISTER_KEYBOARD;
    mysend();
    if (reply.type == FAIL)
    {
        fprintf(stderr, "registration failed.\n");
        exit(-1);
    }
}

void game(void)
{
    msg.type = KEYBOARD_READY;  mysend();  // start
    if (reply.type != START)
    {
        fprintf(stderr, "start failed.\n");
        exit(-1);
    }
    // main loop
    initscr();
    WINDOW *win = newwin(0, 0, 0, 0); wtimeout(win, 10); noecho(); keypad(win, TRUE);
    while (!flag)
    {
        //get input from user
        int i, last, c;
        msg.type = KEYBOARD_INPUT;  last = ' ';
        for (i = 0; i < MAX_KEYS; i++)
        {
            c = ' ';
            if ((c = wgetch(win)) == ERR)  msg.key[i] = last;
            else
            {
                last = c;  msg.key[i] = c;
            }
        }
        mysend();
        if (reply.type != OKAY)  flag = 1;  // end
    }
    delwin(win); sleep(5); endwin();
}

void mysend(void)
{
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1) die();
}
void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}
