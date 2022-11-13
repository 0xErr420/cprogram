#include "consume_produce.h"

int cp_init(consume_produce *cp, size_t buf_size, size_t item_size)
{
    if (pthread_mutex_init(&cp->mutex_buffer, NULL) != 0)
        return -1;

    if (sem_init(&cp->sem_empty, 0, buf_size) != 0 || sem_init(&cp->sem_filled, 0, 0) != 0)
        return -1;

    if (cb_init(&cp->buffer, buf_size, item_size) != 0)
        return -1;

    return 0;
}

int cp_destroy(consume_produce *cp)
{
    cb_free(&cp->buffer);

    if (sem_destroy(&cp->sem_empty) != 0 || sem_destroy(&cp->sem_filled) != 0)
        return -1;

    if (pthread_mutex_destroy(&cp->mutex_buffer) != 0)
        return -1;

    return 0;
}

int cp_produce(consume_produce *cp, const void *item)
{
    // Wait for empty
    if (sem_wait(&cp->sem_empty) != 0 ||
        pthread_mutex_lock(&cp->mutex_buffer) != 0)
        return -1;

    // Produce
    if (cb_push_back(&cp->buffer, item) != 0)
        return -1;

    // Tell other thread there is filled available
    if (pthread_mutex_unlock(&cp->mutex_buffer) != 0 ||
        sem_post(&cp->sem_filled) != 0)
        return -1;

    return 0;
}

int cp_consume(consume_produce *cp, void *item)
{
    // Wait for filled
    if (sem_wait(&cp->sem_filled) != 0 ||
        pthread_mutex_lock(&cp->mutex_buffer) != 0)
        return -1;

    // Consume
    if (cb_pop_front(&cp->buffer, item) != 0)
        return -1;

    // Tell other thread there is empty available
    if (pthread_mutex_unlock(&cp->mutex_buffer) != 0 ||
        sem_post(&cp->sem_empty) != 0)
        return -1;

    return 0;
}
