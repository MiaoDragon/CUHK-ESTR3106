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
    DIRECTION dir;
    BOOST boost;
    struct _node* next;
} fifo;
fifo *head[2] = {NULL, NULL}, *tail[2] = {NULL, NULL};
char *fromw = NULL;
MESSAGE msg, reply;
int hId;
int cour_num = 0, k_num = 0;
/* functions */
void game(void);
void push(void);
void pop(int);
void die(void);
void myreply(void);
/* main program */
int main(void)
{
    if (name_attach("Input_Admin", NULL) == -1)  die();
    game();
    if (name_detach() == -1)  die();
    return 0;
}
/* implementations */
void game(void)
{
    int flag = 0;
    DIRECTION dir[2] = {WEST, EAST};
    while (!flag || cour_num || k_num)
    {
        if (Receive(&fromw, &msg, sizeof(msg)) == -1)  die();
        switch (msg.type)
        {
            // courier
            case REGISTER_COURIER:
                if (cour_num == 2)  //fail
                {
                    reply.type = FAIL;  myreply();
                }
                else
                {
                    reply.type = INIT; myreply(); cour_num++;
                }
                break;
            case COURIER_READY:
                reply.type = REGISTER_HUMAN; myreply();
                break;
            case INIT:
                reply.type = HUMAN_READY; reply.humanId = msg.humanId; myreply();
                break;
            case FAIL:  // maximum number
                break;
            case START:
            case UPDATE:
                reply.type = HUMAN_MOVE; reply.humanId = msg.humanId;
                if (head[reply.humanId] == NULL)
                {
                    reply.dir = (DIRECTION) (reply.humanId * 2);
                    reply.boost = NO;
                }
                else
                {
                    reply.boost = head[reply.humanId]->boost;
                    if (reply.boost == YES)
                        reply.dir = dir[reply.humanId];
                    else  reply.dir = head[reply.humanId]->dir;
                }
                myreply();  dir[reply.humanId] = reply.dir;
                pop(reply.humanId);
                break;
            // keyboards
            case REGISTER_KEYBOARD:
                if (k_num)
                {
                    reply.type = FAIL;  myreply();
                }
                else
                {
                    reply.type = INIT;  myreply();  k_num++;
                }
                break;
            case KEYBOARD_READY:
                reply.type = START;  myreply();
                break;
            case KEYBOARD_INPUT:
                if (flag)
                {
                    reply.type = END; myreply(); k_num--;
                }
                else
                {
                    reply.type = OKAY;  myreply();
                    push();
                }
                break;
            case END:
                reply.type = OKAY;  myreply();
                flag = 1;  cour_num--;
                break;
            default:    break;
        }
    }
}

void pop(int hId)
{
    if (head[hId] == NULL)  return;
    if (head[hId] == tail[hId])  return;
    fifo *tmp = head[hId];  head[hId] = head[hId]->next;  free(tmp);
}

void push(void)
{
    int i;
    for (i = 0; i < MAX_KEYS; i++)
    {
        if (msg.key[i] == KEY_UP || msg.key[i] == KEY_DOWN || \
            msg.key[i] == KEY_LEFT || msg.key[i] == KEY_RIGHT || \
            msg.key[i] == 'p')
        {
            if (head[0] == NULL)  // create new
            {
                head[0]= (fifo*) malloc(sizeof(fifo));
                head[0]->next = NULL;  tail[0] = head[0];
            }
            else
            {
                tail[0]->next = (fifo*) malloc(sizeof(fifo));
                tail[0] = tail[0]->next;  tail[0]->next = NULL;
            }
            switch (msg.key[i])
            {
                case KEY_UP:  tail[0]->dir = NORTH; tail[0]->boost = NO; break;
                case KEY_DOWN:  tail[0]->dir = SOUTH; tail[0]->boost = NO; break;
                case KEY_LEFT:  tail[0]->dir = WEST; tail[0]->boost = NO; break;
                case KEY_RIGHT:  tail[0]->dir = EAST; tail[0]->boost = NO; break;
                case 'p':  tail[0]->dir = EAST; tail[0]->boost = YES; break;
                default:  break;
            }
        }
        if (msg.key[i] == 'w' || msg.key[i] == 's' || \
            msg.key[i] == 'a' || msg.key[i] == 'd' || \
            msg.key[i] == 'q')
        {
            if (head[1] == NULL)  // create new
            {
                head[1] = (fifo*) malloc(sizeof(fifo));
                head[1]->next = NULL;  tail[1] = head[1];
            }
            else
            {
                tail[1]->next = (fifo*) malloc(sizeof(fifo));
                tail[1] = tail[1]->next;  tail[1]->next = NULL;
            }
            switch (msg.key[i])
            {
                case 'w':  tail[1]->dir = NORTH; tail[1]->boost = NO; break;
                case 's':  tail[1]->dir = SOUTH; tail[1]->boost = NO; break;
                case 'a':  tail[1]->dir = WEST; tail[1]->boost = NO; break;
                case 'd':  tail[1]->dir = EAST; tail[1]->boost = NO; break;
                case 'q':  tail[1]->dir = EAST; tail[1]->boost = YES; break;
                default:  break;
            }
        }
    }
}

void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}

void myreply(void)
{
    if (Reply(fromw, &reply, sizeof(MESSAGE)) == -1)  die();
}
