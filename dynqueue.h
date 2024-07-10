// Queue implementation in C based on C++ vector implementation

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

    Do not manually modify: _elements, _capacity, _elem_size, or _array.
    Use function pointers to do so.

*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#define constructor_queue(type) {                                                                   \
    ._elements = 0, ._capacity = 0, ._elem_size = sizeof(type), ._array = malloc(sizeof(type) * 0), \
    .push_##type = queue_push_##type,                                                               \
    .pop_##type = queue_pop_##type,                                                                 \
    .front_##type = queue_front_##type, .back_##type = queue_back_##type,                           \
    .empty_##type = queue_empty_##type, .size_##type = queue_size_##type }

#ifndef destructor
    #define destructor(item) \
        free(item._array);   \
        item._array = NULL;  \
        item._elements = 0;  \
        item._capacity = 0;  \
        item._elem_size = 0;
#endif 

#define QUEUE(type) typedef struct queue_##type                         \
{                                                                       \
    type *_array;                                                       \
    size_t _elements;                                                   \
    size_t _capacity;                                                   \
    size_t _elem_size;                                                  \
    void   (*push_##type)(struct queue_##type*, type);                  \
    void   (*pop_##type)(struct queue_##type*);                         \
    type   (*front_##type)(struct queue_##type*);                       \
    type   (*back_##type)(struct queue_##type*);                        \
    bool   (*empty_##type)(struct queue_##type*);                       \
    size_t (*size_##type)(struct queue_##type*);                        \
} queue_##type;                                                         \
void queue_push_##type(struct queue_##type* que, type elem)             \
{                                                                       \
    if (que->_elements >= que->_capacity)                               \
    {                                                                   \
        size_t _capacity = que->_capacity;                              \
        type *cpy = malloc(que->_elem_size * _capacity);                \
        memcpy(cpy, que->_array, que->_elem_size * _capacity);          \
        free(que->_array);                                              \
        que->_capacity = (que->_capacity > 0) ? que->_capacity * 2 : 1; \
        que->_array = malloc(que->_elem_size * que->_capacity);         \
        memcpy(que->_array, cpy, que->_elem_size * _capacity);          \
        free(cpy);                                                      \
    }                                                                   \
    que->_array[que->_elements] = elem;                                 \
    que->_elements++;                                                   \
}                                                                       \
void queue_pop_##type(struct queue_##type* que)                         \
{                                                                       \
    assert(que->_elements != 0);                                        \
    type *source = &que->_array[1];                                     \
    type *destination = &que->_array[0];                                \
    size_t amount = (que->_elements - 1) * que->_elem_size;             \
    memmove(destination, source, amount);                               \
    que->_elements--;                                                   \
}                                                                       \
type queue_front_##type(struct queue_##type* que)                       \
{                                                                       \
    assert(que->_elements > 0);                                         \
    return que->_array[0];                                              \
}                                                                       \
type queue_back_##type(struct queue_##type* que)                        \
{                                                                       \
    assert(que->_elements > 0);                                         \
    return que->_array[que->_elements - 1];                             \
}                                                                       \
bool queue_empty_##type(struct queue_##type* que)                       \
{                                                                       \
    return (que->_elements == 0);                                       \
}                                                                       \
size_t queue_size_##type(struct queue_##type* que)                      \
{                                                                       \
    return que->_elements;                                              \
}
