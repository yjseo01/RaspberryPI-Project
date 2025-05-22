#ifndef __THD_FUNC_H__
#define __THD_FUNC_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <syslog.h>
#include <wiringPiI2C.h>

#include "ctrl_func.h"
#include "librasp.h"

#define THRESHOLD 180

void* handle_thd_func(void*);

void* handle_buzzer_on(void*);
void* handle_countdown(void*);

void cleanup_buzzer(void*);

extern pthread_t g_buzzer_tid;
extern atomic_bool g_buzzer_running;
extern int a2dVal;
extern pthread_mutex_t acdmtx2;

typedef struct 
{
    int clnt_sock;
    int acd_fd;
} thd_arg_t;

#endif // __THD_FUNC_H__
