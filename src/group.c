#include "group.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int group_init(group *grp, size_t capacity, size_t sz)
{
    grp->pgrp = malloc(capacity * sz);
    if (grp->pgrp == NULL)
    {
        return -1;
    }
    grp->capacity = capacity;
    grp->size = sz;
    grp->count = 0;
    grp->uncount = 0;
    return 0;
}

void group_free(group *grp)
{
    free(grp->pgrp);
}

void group_info(group *grp)
{
    printf("*pgrp: %p  capacity: %d  size: %d  count: %d  uncount: %d\n", grp->pgrp, grp->capacity, grp->size, grp->count, grp->uncount);
}

int group_push(group *grp, const void *item)
{
    if (grp->count >= grp->capacity)
    {
        return -1;
    }
    memcpy(grp->pgrp + (grp->size * grp->count), item, grp->size);
    grp->count++;
    return 0;
}

int group_pop(group *grp, void *item)
{
    if (grp->uncount >= grp->count)
    {
        return -1;
    }
    memcpy(item, grp->pgrp + (grp->size * grp->uncount), grp->size);
    grp->uncount++;
    return 0;
}

int group_get(group *grp, void *item, int n)
{
    if (n >= grp->count || n < 0)
    {
        return -1;
    }
    memcpy(item, grp->pgrp + (grp->size * n), grp->size);
    return 0;
}