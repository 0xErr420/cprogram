#include "consume_produce.h"
#include "group.h"

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
    for (size_t i = 0; i < cp->buffer.capacity; i++) // Free allocated groups
    {
        group grp;
        if (cb_pop_front(&cp->buffer, &grp) != 0)
            break;
        group_free(&grp);
    }

    cb_free(&cp->buffer);

    if (sem_destroy(&cp->sem_empty) != 0 || sem_destroy(&cp->sem_filled) != 0)
        return -1;

    if (pthread_mutex_destroy(&cp->mutex_buffer) != 0)
        return -1;

    return 0;
}
