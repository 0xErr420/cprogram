#ifndef GROUP_H
#define GROUP_H
#include <stddef.h>

/// Group structure
typedef struct group
{
    void *pgrp;   // pointer to allocated memory (capacity * size)
    int capacity; // amount of elements
    int size;     // size of each element
    int count;    // count how many elements already added
    int uncount;  // used for pop operation
} group;

/// Initialize group
/// Dynamically allocates memory, it is necessary to call the function `group_free(...)`.
///
/// CAPACITY is how many elements in group, SZ is size of individual element
///
/// returns 0 if successful, -1 if failed
int group_init(group *grp, size_t capacity, size_t sz);

/// Free dynamically allocated memory for group
void group_free(group *grp);

/// Show info about group fields
/// (for debug purposes)
void group_info(group *grp);

/// Push element to group
///
/// ITEM is element to add
///
/// returns 0 if successful, -1 if failed
int group_push(group *grp, const void *item);

/// Pop element from group
/// (pops from group, use group_get to only get element)
///
/// ITEM used to return value
///
/// returns 0 if successful, -1 if failed
int group_pop(group *grp, void *item);

/// Get N element from group
/// (dont pop, just to get)
///
/// N specifies element, ITEM used to return value
///
/// returns 0 if successful, -1 if failed
int group_get(group *grp, void *item, int n);

#endif
