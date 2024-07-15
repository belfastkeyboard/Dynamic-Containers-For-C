// Queue implementation in C based on C++ vector implementation

// TODO: test

/*  HOW TO USE:

    Call QUEUE(type) with the desired type, multiple types can be used.
    Call constructor_queue(type) to define attributes and function pointers.
    Call destructor(que) in order to clean up.
    If queue goes out of scope without destructor being called, a memory leak will occur.

    example:

    QUEUE(int)

    int main(void)
    {
        queue_int que = constructor_queue(int);

        que.push_int(&que, 1);
        que.push_int(&que, 2);

        destructor(que);
        
        return 0;
    }

    Do not manually modify: _elements, _capacity, _type_size, or _array.
    Use function pointers to do so.

*/

#pragma once

// CONSIDER: pop operations can be slow if many items need to be popped quickly
//  due to having to shift the entire array down by an element every pop

// CONSIDER: no ability to shrink allocated memory

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#undef GROW_FACTOR
#undef QUEUE_MIN

#define GROW_FACTOR 2
#define QUEUE_MIN 1

#define constructor_queue(type)                                                                   \
{                                                                                                 \
    ._elements = 0, ._capacity = 0, ._type_size = sizeof(type), ._array = calloc(0, sizeof(int)), \
    .push = queue_push_##type,                                                                    \
    .pop = queue_pop_##type,                                                                      \
    .front = queue_front_##type, .back = queue_back_##type,                                       \
    .empty = queue_empty_##type, .size = queue_size_##type                                        \
}

#ifndef destructor
    #define destructor(item) \
        free(item._array);   \
        item._array = NULL;  \
        item._elements = 0;  \
        item._capacity = 0;  \
        item._type_size = 0
#endif 

#define QUEUE(type) typedef struct queue_##type                             \
{                                                                           \
    type*  _array;                                                           \
    size_t _elements;                                                       \
    size_t _capacity;                                                       \
    size_t _type_size;                                                      \
    void   (*push)(struct queue_##type*, type);                             \
    void   (*pop)(struct queue_##type*);                                    \
    type   (*front)(struct queue_##type*);                                  \
    type   (*back)(struct queue_##type*);                                   \
    bool   (*empty)(struct queue_##type*);                                  \
    size_t (*size)(struct queue_##type*);                                   \
} queue_##type;                                                             \
                                                                            \
void queue_push_##type(struct queue_##type* que, type elem)                 \
{                                                                           \
    if (que->_elements >= que->_capacity)                                   \
    {                                                                       \
        que->_capacity = (que->_capacity > 0) ?                             \
            que->_capacity * GROW_FACTOR : QUEUE_MIN;                       \
                                                                            \
        type* tmp = realloc(que->_array, que->_type_size * que->_capacity); \
        assert(tmp != NULL);                                                \
        que->_array = tmp;                                                  \
    }                                                                       \
    que->_array[que->_elements] = elem;                                     \
    que->_elements++;                                                       \
}                                                                           \
                                                                            \
void queue_pop_##type(struct queue_##type* que)                             \
{                                                                           \
    assert(que->_elements != 0);                                            \
    type *source = &que->_array[1];                                         \
    type *destination = &que->_array[0];                                    \
    size_t amount = (que->_elements - 1) * que->_type_size;                 \
    memmove(destination, source, amount);                                   \
    que->_elements--;                                                       \
}                                                                           \
                                                                            \
type queue_front_##type(struct queue_##type* que)                           \
{                                                                           \
    assert(que->_elements > 0);                                             \
    return que->_array[0];                                                  \
}                                                                           \
                                                                            \
type queue_back_##type(struct queue_##type* que)                            \
{                                                                           \
    assert(que->_elements > 0);                                             \
    return que->_array[que->_elements - 1];                                 \
}                                                                           \
                                                                            \
bool queue_empty_##type(struct queue_##type* que)                           \
{                                                                           \
    return (que->_elements == 0);                                           \
}                                                                           \
                                                                            \
size_t queue_size_##type(struct queue_##type* que)                          \
{                                                                           \
    return que->_elements;                                                  \
}
