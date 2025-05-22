#include "queue.h"

void init_queue(RequestQueue* q)
{
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void enqueue(RequestQueue* q, CommandType cmd, int arg)
{
    Request* new_req = (Request*)malloc(sizeof(Request));
    if (!new_req) {
        perror("malloc");
        return;
    }
    new_req->cmd = cmd;
    new_req->arg = arg;
    new_req->next = NULL;

    pthread_mutex_lock(&q->mutex);

    if (q->tail) {
        q->tail->next = new_req;
    } else {
        q->head = new_req;
    }
    q->tail = new_req;

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

Request* dequeue(RequestQueue* q)
{
    pthread_mutex_lock(&q->mutex);

    while (q->head == NULL) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    Request* req = q->head;
    q->head = req->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }

    //CommandType cmd = req->cmd;
    //free(req);

    pthread_mutex_unlock(&q->mutex);
    return req;
}

int is_empty(RequestQueue* q)
{
    int empty;

    pthread_mutex_lock(&q->mutex);
    empty = (q->head == NULL);
    pthread_mutex_unlock(&q->mutex);

    return empty;
}