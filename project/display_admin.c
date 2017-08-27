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
*  store screen output sequence
**/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <simpl.h>
#include "message.h"

/* data */
typedef struct _node
{
    MESSAGE reply;
    struct _node* next;
} fifo;
fifo *head, *tail;
char *fromw = NULL;
MESSAGE msg, reply;
int winner, flag = 0;
char *waitp = NULL;  // store the waiting painter
/* function */
void game(void);
void push_msg(void);
void pop_msg(void);
void myreply(char*, MESSAGE*);
void die(void);
/* main */
int main(void)
{
    if (name_attach("Display_Admin", NULL) == -1)  die();
    game();
    if (name_detach() == -1)  die();
    return 0;
}

/* implementations */
void game(void)
{
    while (!(flag && head == NULL))  // while !(end AND fifo empty)
    {
        if (Receive(&fromw, &msg, sizeof(msg)) == -1)  die();
        switch (msg.type)
        {
            case DISPLAY_ARENA:
                reply.type = OKAY;
                myreply(fromw, &reply);
                push_msg();  // create new node
                break;
            case PAINTER_READY:
                waitp = fromw;  // to be handled later
                break;
            case END:
                flag = 1;  // end
                winner = msg.cycleId;
                push_msg();
                reply.type = OKAY;  myreply(fromw, &reply);  // need to ACK
                break;
            default:
                break;
        }
        pop_msg();
    }
}

void myreply(char* fromw, MESSAGE *reply)
{
    if (Reply(fromw, reply, sizeof(MESSAGE)) == -1)  die();
}

void push_msg(void)
{
    if (head == NULL)
    {
        head = (fifo*) malloc(sizeof(fifo));  head->next = NULL;
        tail = head;  head->reply.arena = msg.arena;
    }
    else
    {
        tail->next = (fifo*) malloc(sizeof(fifo)); tail = tail->next;
        tail->next = NULL;  tail->reply.arena = msg.arena;
    }
}

void pop_msg(void)
{
    if (head != NULL && waitp != NULL)  // can print
    {
        if (flag && head->next == NULL)  // last one to print
        {
            head->reply.type = END; head->reply.cycleId = winner;
        }
        else    head->reply.type = PAINT;
        myreply(waitp, &(head->reply));
        fifo* tmp = head;  head = head->next;  free(tmp);  // del head
        waitp = NULL;  // handled
    }
}

void die(void)
{
    fprintf(stderr, "%s\n", whatsMyError()); exit(-1);
}
