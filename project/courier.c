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
*  Workers for game_admin
*  2:    game_admin  <--->  display_admin
*  1,0:  game_admin  <--->  input_admin
**/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <simpl.h>
#include "message.h"
/* data */
MESSAGE msg, reply;
int courid;  // id
int gadmin, dadmin, hadmin;
int flag = 0;    // game end or not
int hid;     // human id
/* functions */
void mysend(int);  // send to given admin
void die(void);
void registration_display(void);
void registration_human(void);
void game_display(void);
void game_human(void);
/* main */
int main(int argc, char* argv[])
{
    char name[20];
    if (argv[1][1])  { fprintf(stderr, "argument wrong!\n");  exit(-1); }
    if (argv[1][0] != '0' && argv[1][0] != '1' && argv[1][0] != '2')
    {
        fprintf(stderr, "argument wrong!\n");
        exit(-1);
    }
    courid = argv[1][0] - '0';
    sprintf(name, "Courier %d", courid);
    if (name_attach(name, NULL) == -1)  die();
    if ((gadmin = name_locate("Game_Admin")) == -1)  die();
    if ((dadmin = name_locate("Display_Admin")) == -1)  die();
    if ((hadmin = name_locate("Input_Admin")) == -1)  die();
    if (courid == 2)
    {
        registration_display();
        game_display();
    }
    else
    {
        registration_human();
        game_human();
    }

    if (name_detach() == -1)  die();
    return 0;
}

void registration_display(void)
{
    msg.type = REGISTER_COURIER;
    mysend(gadmin);
    if (reply.type == FAIL)
    {
        fprintf(stderr, "registration failed.\n");
        exit(-1);
    }
}

void registration_human(void)
{
    // register to input_admin
    msg.type = REGISTER_COURIER;
    mysend(hadmin);
    if (reply.type == FAIL)
    {
        fprintf(stderr, "registration failed.\n");
        exit(-1);
    }
    msg.type = COURIER_READY;
    mysend(hadmin);  // reply is REGISTER_HUMAN
    msg.type = REGISTER_HUMAN;  mysend(gadmin);
    // notify input_admin Fail
    if (reply.type == FAIL)
    {
        msg.type = FAIL; mysend(hadmin);
        if (name_detach() == -1)  die();
        exit(0);
    }
    // notify init
    msg.type = INIT;  msg.humanId = reply.humanId;
    mysend(hadmin);
    // human ready, notify game admin
    msg.type = HUMAN_READY; msg.humanId = reply.humanId;
    mysend(gadmin);
    // start
}

void game_display(void)
{
    msg.type = COURIER_READY;
    mysend(gadmin);
    while (!flag)
    {
        switch (reply.type)
        {
            case DISPLAY_ARENA:  // from game admin
                msg.type = DISPLAY_ARENA;
                msg.arena = reply.arena;
                mysend(dadmin);
                break;
            case OKAY: // from display admin
                msg.type = OKAY;
                mysend(gadmin);
                break;
            case END:
                msg.type = END;
                msg.cycleId = reply.cycleId;
                msg.arena = reply.arena;
                mysend(dadmin);
                flag = 1;  // end
                break;
            default:  break;
        }
    }
}

void game_human(void)
{
    // notify input admin start
    msg.type = START; msg.humanId = reply.humanId;
    mysend(hadmin);
    while (!flag)
    {
        switch (reply.type)
        {
            case HUMAN_MOVE:
                msg.type = HUMAN_MOVE; msg.humanId = reply.humanId;
                msg.dir = reply.dir; msg.boost = reply.boost;
                mysend(gadmin);
                break;
            case UPDATE:
                msg.type = UPDATE; msg.humanId = reply.humanId;
                mysend(hadmin);
                break;
            case END:
                msg.type = END; mysend(hadmin);
                break;
            case OKAY:
                flag = 1;
                break;
            default: break;
        }
    }
}
void mysend(int admin)
{
    if (Send(admin, &msg, &reply, sizeof(msg), sizeof(reply)) == -1)  die();
}
void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}
