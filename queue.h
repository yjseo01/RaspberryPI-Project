#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef enum {
    CMD_UNKNOWN,
    CMD_LED_ON,
    CMD_LED_OFF,
    CMD_LED_BRIGHTNESS,
    CMD_BUZZER_ON,
    CMD_BUZZER_OFF,
    CMD_AUTO_LED,
    CMD_COUNTDOWN,
    CMD_PLAY_PITCH
} CommandType;

typedef struct Request
{
    CommandType cmd;
    struct Request* next;
    int arg; // 밝기
} Request;

typedef struct
{
    Request* head;
    Request* tail;
    pthread_mutex_t mutex;
    pthread_mutex_t cond;
} RequestQueue;

void init_queue(RequestQueue* q);
void enqueue(RequestQueue* q, CommandType cmd, int arg);
Request* dequeue(RequestQueue* q);
int is_empty(RequestQueue* q);

#endif // __QUEUE_H__