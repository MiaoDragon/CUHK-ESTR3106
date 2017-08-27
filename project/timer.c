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
*  sleep
*  Workers for game_admin
**/
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <simpl.h>
#include <time.h>
#include "message.h"
/* data */
int admin;
MESSAGE msg, reply;
/* function */
void mysend(void);
void registration(void);
void game(void);
void die(void);
/* main */
int main(void)
{
    if (name_attach("Timer", NULL) == -1)  die();
    if ((admin = name_locate("Game_Admin")) == -1)  die();
    registration();
    game();
    name_detach();
    return 0;
}

void registration(void)
{
    msg.type = REGISTER_TIMER;
    mysend();
    if (reply.type == FAIL)
    {
        fprintf(stderr, "registration failed.\n");
        exit(-1);
    }
}

void game(void)
{
    while (1)
    {
        msg.type = TIMER_READY;
        mysend();
        if (reply.type == SLEEP)  usleep(reply.interval);
        else                      break;    // end of game
    }
}

void mysend(void)
{
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1)  die();
}
void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}
