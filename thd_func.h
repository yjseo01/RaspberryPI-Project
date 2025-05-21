#ifndef __THD_FUNC_H__
#define __THD_FUNC_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"
#include "ctrl_func.h"

void* worker_thd_func(void*);
void* handle_thd_func(void*);

#endif // __THD_FUNC_H__
